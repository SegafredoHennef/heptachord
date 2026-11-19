#include "plugin/PluginProcessor.h"

#include "plugin/ParameterLayout.h"
#include "plugin/PluginEditor.h"

#include <algorithm>

namespace heptachord
{
namespace
{
// Der Standalone hat keinen Host-Transport (Entscheidung 29).
constexpr double kStandaloneBpm = 120.0;
} // namespace

Processor::Processor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    m_binding.connect (apvts);

    m_instrument = createTestSynthInstrument();
}

Processor::~Processor()
{
    // Der Eingang muss vor MidiClock verschwinden: Sein Callback-Thread
    // darf nicht mehr auf das bereits zerstoerte m_clock zugreifen.
    closeClockInput();
}

void Processor::prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock)
{
    m_bridge.prepare (sampleRate, maximumExpectedSamplesPerBlock);

    {
        // prepare() dimensioniert auch den Scratch-Puffer des VST-Hosts;
        // nicht gleichzeitig mit processBlock auf dem Audio-Thread laufen
        // lassen.
        const std::lock_guard<std::mutex> lock (m_instrumentMutex);
        if (m_instrument != nullptr)
            m_instrument->prepare (sampleRate);
    }

    m_standalonePpq = 0.0;

    // MIDI-Clock: automatisch der erste verfuegbare Eingang (Entscheidung 78).
    // Kein Geraet oder ein Fehler bedeutet still den Fallback auf 120 BPM.
    if (isStandalone())
    {
        const auto devices = juce::MidiInput::getAvailableDevices();

        if (! devices.isEmpty())
        {
            m_clockInput = juce::MidiInput::openDevice (devices.getReference (0).identifier, this);

            if (m_clockInput != nullptr)
                m_clockInput->start();
        }
    }
}

void Processor::setInstrument (std::unique_ptr<Instrument> instrument) noexcept
{
    // Neues Instrument vollstaendig vorbereiten, BEVOR es der Audio-Thread
    // sehen kann. Sonst kann processBlock bereits laufen, waehrend prepare()
    // den Scratch-Puffer dimensioniert -> Use-after-free im Plugin.
    if (instrument != nullptr)
    {
        const double sr = getSampleRate() > 0.0 ? getSampleRate() : 44100.0;
        instrument->prepare (sr);
    }

    std::unique_ptr<Instrument> old;

    {
        const std::lock_guard<std::mutex> lock (m_instrumentMutex);
        old = std::move (m_instrument);
        m_instrument = std::move (instrument);
    }

    // Erst nach dem Tausch aufraeumen: der Audio-Thread arbeitet jetzt mit
    // dem neuen Instrument und fasst das alte nicht mehr an.
    if (old != nullptr)
    {
        old->hideEditor();  // offenes Plugin-Fenster schliessen
        old->reset();       // alte Noten sofort stoppen
    }
}

void Processor::releaseResources()
{
    closeClockInput();
    m_bridge.requestAllNotesOff();

    const std::lock_guard<std::mutex> lock (m_instrumentMutex);
    if (m_instrument) m_instrument->reset();
}

bool Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet().isDisabled()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

HostTimeline Processor::buildTimeline (int numSamples) noexcept
{
    HostTimeline timeline;

    const double sampleRate = getSampleRate();
    timeline.sampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    timeline.numSamples = numSamples;
    timeline.bpm = m_binding.get (params::id_tempo);
    timeline.isPlaying = true;

    if (isStandalone())
    {
        timeline.positionValid = true;

        // MIDI-Clock hat Vorrang, solange er aktiv ist (Entscheidungen 79/80).
        const auto snap = m_clock.snapshot (juce::Time::getMillisecondCounterHiRes() * 0.001);

        if (snap.active)
        {
            timeline.bpm = snap.bpm;
            timeline.isPlaying = snap.playing;
            timeline.ppqPosition = snap.ppq;
            m_clockWasActive = true;
        }
        else
        {
            // Timeout: die letzte Clock-Position einmal uebernehmen, dann
            // der manuelle Fallback (120 BPM, Leertaste/Knopf).
            if (m_clockWasActive)
            {
                m_standalonePpq = snap.ppq;
                m_clockWasActive = false;
            }

            const double fallbackBpm = m_binding.get (params::id_tempo);
            timeline.bpm = fallbackBpm;
            timeline.isPlaying = m_standalonePlaying.load (std::memory_order_relaxed);
            timeline.ppqPosition = m_standalonePpq;

            if (timeline.isPlaying)
                m_standalonePpq += static_cast<double> (numSamples) * fallbackBpm / (60.0 * timeline.sampleRate);
            else
                m_standalonePpq = 0.0;
        }

        return timeline;
    }

    if (auto* hostPlayHead = getPlayHead())
    {
        if (const auto position = hostPlayHead->getPosition())
        {
            if (const auto bpm = position->getBpm())
                timeline.bpm = *bpm;

            timeline.isPlaying = position->getIsPlaying();

            if (const auto ppq = position->getPpqPosition())
            {
                timeline.ppqPosition = *ppq;
                timeline.positionValid = true;
            }
        }
    }

    return timeline;
}

void Processor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Kein Audio im VST3: der Bus ist still, damit Live/Cubase das Instrument
    // laden. Nur der Standalone-Synth schreibt in den Puffer.
    buffer.clear();

    const ParameterValues values = m_binding.current();
    const HostTimeline timeline = buildTimeline (buffer.getNumSamples());

    // Letzte Eingangsnote fuer den Export merken (M1, Decision 84).
    // Der Audio-Thread schreibt, der UI-Thread liest beim EXPORT-Klick.
    for (const auto metadata : midiMessages)
        if (metadata.getMessage().isNoteOn())
            m_lastInputNote.store (metadata.getMessage().getNoteNumber(),
                                    std::memory_order_relaxed);

    // Tastatur des Standalones: eingegangene Noten in den Eingangspuffer
    // mischen. Der Zeitstempel waechst pro Ereignis, damit die Reihenfolge
    // (Note-Off vor Note-On) erhalten bleibt. Im VST3 bleibt alles unberuehrt.
    if (isStandalone())
    {
        const std::lock_guard<std::mutex> lock (m_keyboardMutex);

        int sample = 0;
        for (const auto metadata : m_keyboardMidi)
        {
            midiMessages.addEvent (metadata.getMessage(), sample);
            ++sample;
        }

        m_keyboardMidi.clear();
    }

    // midiMessages ist Ein- und Ausgabe zugleich; der Bridge kopiert zuerst.
    m_bridge.process (values, timeline, midiMessages, midiMessages);

    // Aktuellen Step-Index fuer das GUI-Highlight merken (Standalone).
    m_currentStepIndex.store (m_bridge.engine().currentStepIndex(),
                              std::memory_order_relaxed);

    if (isStandalone())
    {
        const double sampleRate = getSampleRate() > 0.0 ? getSampleRate() : 44100.0;
        const double bpm = timeline.bpm > 0.0 ? timeline.bpm : 120.0;
        const std::int64_t blockStartTick = static_cast<std::int64_t> (timeline.ppqPosition * midirender::kPpq);
        const std::int64_t blockEndTick = blockStartTick + static_cast<std::int64_t> (
            static_cast<double> (buffer.getNumSamples()) * bpm * midirender::kPpq / (60.0 * sampleRate));

        const int state = m_recorderState.load (std::memory_order_relaxed);
        const std::int64_t slotIndex = m_bridge.engine().nextSlotIndex();

        // Aufnahme erst an der naechsten Step-Grenze aktivieren (Q2 Runde 3).
        if (state == 3)
        {
            if (m_armSlot.load (std::memory_order_relaxed) < 0)
                m_armSlot.store (slotIndex, std::memory_order_relaxed);
            else if (slotIndex > m_armSlot.load (std::memory_order_relaxed))
            {
                const std::lock_guard<std::mutex> lock (m_recordingMutex);
                m_recording.clear();
                m_activeNotes.clear();
                m_recordingStartTick = 0;
                m_recordingLength.store (0, std::memory_order_relaxed);
                m_hasRecording.store (false, std::memory_order_relaxed);
                m_recordStartSlot = slotIndex;
                m_lastSlotIndex.store (slotIndex, std::memory_order_relaxed);
                m_recordedSlots.store (0, std::memory_order_relaxed);
                m_stepsPerCycle.store (std::max (1, m_bridge.engine().activeStepCount()),
                                       std::memory_order_relaxed);
                m_armSlot.store (-1, std::memory_order_relaxed);
                m_recorderState.store (1, std::memory_order_relaxed);
            }
        }

        bool capturing = m_recorderState.load (std::memory_order_relaxed) == 1;

        if (capturing)
        {
            // Slotbasierte Zaehlung: richtungsunabhaengig und identisch mit
            // der Definition eines Zyklus im Export (activeCount Slots).
            const std::int64_t lastSlot = m_lastSlotIndex.load (std::memory_order_relaxed);
            if (lastSlot >= 0 && slotIndex > lastSlot)
            {
                m_recordedSlots.fetch_add (static_cast<int> (slotIndex - lastSlot),
                                           std::memory_order_relaxed);
                m_lastSlotIndex.store (slotIndex, std::memory_order_relaxed);
            }

            {
                const std::lock_guard<std::mutex> lock (m_recordingMutex);
                for (const auto metadata : midiMessages)
                {
                    const auto msg = metadata.getMessage();
                    if (! msg.isNoteOn() && ! msg.isNoteOff())
                        continue;

                    const std::int64_t tick = blockStartTick + static_cast<std::int64_t> (
                        static_cast<double> (metadata.samplePosition) * bpm * midirender::kPpq / (60.0 * sampleRate));

                    if (msg.isNoteOn())
                        m_activeNotes.insert ({ msg.getChannel(), msg.getNoteNumber() });
                    else
                        m_activeNotes.erase ({ msg.getChannel(), msg.getNoteNumber() });

                    m_recording.push_back ({ tick, msg.getChannel(), msg.getNoteNumber(),
                                             msg.isNoteOn() ? msg.getVelocity() : 0, msg.isNoteOn() });
                }

                if (m_recordingStartTick == 0 && ! m_recording.empty())
                    m_recordingStartTick = m_recording.front().tick;

                m_hasRecording.store (! m_recording.empty(), std::memory_order_relaxed);
            }

            const int perCycle = m_stepsPerCycle.load (std::memory_order_relaxed);
            const int target = m_recordingTarget.load (std::memory_order_relaxed);
            const int slots = m_recordedSlots.load (std::memory_order_relaxed);

            if (midirender::recordingTargetReached (slots, perCycle, target))
            {
                stopRecording();
                capturing = false;
            }
        }

        // Wiedergabe: nur die Aufnahme; die Engine laeuert zwar weiter, ihre
        // Noten werden aber verworfen (Q6 Runde 3).
        if (m_recorderState.load (std::memory_order_relaxed) == 2
            && m_hasRecording.load (std::memory_order_relaxed))
        {
            midiMessages.clear();

            std::int64_t firstTick = 0;
            {
                const std::lock_guard<std::mutex> lock (m_recordingMutex);
                if (! m_recording.empty())
                    firstTick = m_recording.front().tick;
            }

            std::int64_t playbackStart = m_playbackStartTick.load (std::memory_order_relaxed);
            if (playbackStart < 0)
            {
                playbackStart = blockStartTick;
                m_playbackStartTick.store (playbackStart, std::memory_order_relaxed);
            }

            {
                const std::lock_guard<std::mutex> lock (m_recordingMutex);
                for (const auto& ev : m_recording)
                {
                    const std::int64_t absTick = playbackStart + (ev.tick - firstTick);
                    if (absTick < blockStartTick || absTick >= blockEndTick)
                        continue;

                    const int samplePos = std::clamp (
                        static_cast<int> (
                            static_cast<double> (absTick - blockStartTick) /
                            static_cast<double> (blockEndTick - blockStartTick) *
                            static_cast<double> (buffer.getNumSamples())),
                        0, buffer.getNumSamples() - 1);

                    juce::MidiMessage msg = ev.noteOn
                        ? juce::MidiMessage::noteOn (ev.channel, ev.pitch, (juce::uint8) ev.velocity)
                        : juce::MidiMessage::noteOff (ev.channel, ev.pitch);
                    msg.setTimeStamp (samplePos);
                    midiMessages.addEvent (msg, samplePos);
                }
            }

            const std::int64_t recLen = m_recordingLength.load (std::memory_order_relaxed);
            if (playbackStart + recLen <= blockEndTick)
                stopPlayback();
        }

        {
            // Tausch nur kurz gesperrt; der Audio-Thread sieht entweder das
            // alte oder das neue, vollstaendig vorbereitete Instrument.
            const std::lock_guard<std::mutex> lock (m_instrumentMutex);

            if (m_instrument != nullptr)
                m_instrument->process (midiMessages, buffer);
        }

        // Diagnose-Peak des Stereo-Ausgangs (fuer die Kanal-Pruefung).
        float peak = m_outputPeak.load (std::memory_order_relaxed);
        for (int ch = 0; ch < std::min (2, buffer.getNumChannels()); ++ch)
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                peak = std::max (peak, std::abs (buffer.getSample (ch, i)));
        m_outputPeak.store (peak, std::memory_order_relaxed);
    }
}

void Processor::injectKeyboardNoteOn (int noteNumber, int velocity)
{
    if (! isStandalone())
        return;

    const std::lock_guard<std::mutex> lock (m_keyboardMutex);
    m_keyboardMidi.addEvent (
        juce::MidiMessage::noteOn (1, noteNumber, static_cast<juce::uint8> (velocity)), 0);
}

void Processor::injectKeyboardNoteOff (int noteNumber)
{
    if (! isStandalone())
        return;

    const std::lock_guard<std::mutex> lock (m_keyboardMutex);
    m_keyboardMidi.addEvent (juce::MidiMessage::noteOff (1, noteNumber), 0);
}

void Processor::toggleStandaloneTransport() noexcept
{
    m_standalonePlaying.store (! isStandalonePlaying(), std::memory_order_relaxed);
}

void Processor::handleIncomingMidiMessage (juce::MidiInput* source, const juce::MidiMessage& message)
{
    juce::ignoreUnused (source);

    if (! isStandalone())
        return;

    const double now = juce::Time::getMillisecondCounterHiRes() * 0.001;

    if (message.isMidiStart())
        m_clock.start (now);
    else if (message.isMidiContinue())
        m_clock.continueClock (now);
    else if (message.isMidiStop())
        m_clock.stop (now);
    else if (message.isMidiClock())
        m_clock.tick (now);
}

bool Processor::clockActive() const noexcept
{
    return m_clock.snapshot (juce::Time::getMillisecondCounterHiRes() * 0.001).active;
}

double Processor::clockBpm() const noexcept
{
    return m_clock.snapshot (juce::Time::getMillisecondCounterHiRes() * 0.001).bpm;
}

midirender::RenderOptions Processor::exportOptions() const noexcept
{
    midirender::RenderOptions options;
    options.settings = settingsFromValues (m_binding.current());
    options.bpm = clockActive() ? clockBpm() : m_binding.get (params::id_tempo);
    options.inputNote = m_lastInputNote.load (std::memory_order_relaxed);
    options.inputVelocity = midirender::kFallbackVelocity;
    options.seed = 1;  // fester Seed (Decision 84)
    return options;
}

// --- MIDI-Rekorder -----------------------------------------------------------

void Processor::finalizeRecordingLocked() noexcept
{
    if (! m_recording.empty())
    {
        const std::int64_t endTick = m_recording.back().tick + 1;

        // Hängende Noten bekommen ein Note-Off, sonst klingt das gespeicherte
        // MIDI laenger als das Gehörte (Q3, Runde 3).
        for (const auto& note : m_activeNotes)
            m_recording.push_back ({ endTick, note.first, note.second, 0, false });

        m_activeNotes.clear();
        m_recordingLength.store (m_recording.back().tick + 1, std::memory_order_relaxed);
        m_hasRecording.store (true, std::memory_order_relaxed);
    }
    else
    {
        m_recordingLength.store (0, std::memory_order_relaxed);
        m_hasRecording.store (false, std::memory_order_relaxed);
    }
}

void Processor::startRecording() noexcept
{
    if (! isStandalone())
        return;

    {
        const std::lock_guard<std::mutex> lock (m_recordingMutex);
        m_recording.clear();
        m_activeNotes.clear();
        m_recordingStartTick = 0;
        m_recordingLength.store (0, std::memory_order_relaxed);
        m_hasRecording.store (false, std::memory_order_relaxed);
    }

    stopPlayback();
    m_recordedSlots.store (0, std::memory_order_relaxed);
    m_stepsPerCycle.store (0, std::memory_order_relaxed);
    m_armSlot.store (-1, std::memory_order_relaxed);
    m_lastSlotIndex.store (-1, std::memory_order_relaxed);

    // Ohne laufenden Transport gibt es keine Noten zum Aufnehmen.
    if (! isStandalonePlaying())
        m_standalonePlaying.store (true, std::memory_order_relaxed);

    m_recorderState.store (3, std::memory_order_relaxed);  // an der Step-Grenze starten
}

void Processor::stopRecording() noexcept
{
    if (! isStandalone())
        return;

    m_recorderState.store (0, std::memory_order_relaxed);
    const std::lock_guard<std::mutex> lock (m_recordingMutex);
    finalizeRecordingLocked();
}

void Processor::startPlayback() noexcept
{
    if (! isStandalone() || ! hasRecording() || isRecording() || isRecordingArmed())
        return;

    m_playbackStartTick.store (-1, std::memory_order_relaxed);
    m_recorderState.store (2, std::memory_order_relaxed);
}

void Processor::stopPlayback() noexcept
{
    if (isStandalone())
        m_recorderState.store (0, std::memory_order_relaxed);
}

void Processor::clearRecording() noexcept
{
    if (! isStandalone())
        return;

    const std::lock_guard<std::mutex> lock (m_recordingMutex);
    m_recording.clear();
    m_activeNotes.clear();
    m_recordingStartTick = 0;
    m_recordingLength.store (0, std::memory_order_relaxed);
    m_hasRecording.store (false, std::memory_order_relaxed);
    m_recorderState.store (0, std::memory_order_relaxed);
}

void Processor::setRecordingTarget (int cycles) noexcept
{
    if (cycles == 0 || cycles == 1 || cycles == 2 || cycles == 4 || cycles == 8)
        m_recordingTarget.store (cycles, std::memory_order_relaxed);
}

int Processor::recordingCycle() const noexcept
{
    if (! hasRecording() && ! isRecording())
        return 0;

    return midirender::cyclePosition (
               m_recordedSlots.load (std::memory_order_relaxed),
               m_stepsPerCycle.load (std::memory_order_relaxed)).cycle;
}

int Processor::recordingStep() const noexcept
{
    if (! hasRecording() && ! isRecording())
        return 0;

    return midirender::cyclePosition (
               m_recordedSlots.load (std::memory_order_relaxed),
               m_stepsPerCycle.load (std::memory_order_relaxed)).step;
}

bool Processor::saveRecordingToFile (const juce::File& file)
{
    std::int64_t firstTick = 0;
    {
        const std::lock_guard<std::mutex> lock (m_recordingMutex);
        if (m_recording.empty())
            return false;
        firstTick = m_recording.front().tick;
    }

    juce::FileOutputStream stream (file);
    if (! stream.openedOk())
        return false;

    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote (midirender::kPpq);

    std::array<juce::MidiMessageSequence, 4> tracks {};

    // Tempo + Takt in Spur 0.
    {
        const double bpm = m_binding.get (params::id_tempo);
        const int microsPerQuarter = juce::jlimit (
            1, 0x7fffffff,
            static_cast<int> (std::llround (60000000.0 / (bpm > 0.0 ? bpm : 120.0))));
        auto tempo = juce::MidiMessage::tempoMetaEvent (microsPerQuarter);
        tempo.setTimeStamp (0.0);
        tracks[0].addEvent (tempo);

        auto signature = juce::MidiMessage::timeSignatureMetaEvent (4, 4);
        signature.setTimeStamp (0.0);
        tracks[0].addEvent (signature);
    }

    const std::lock_guard<std::mutex> lock (m_recordingMutex);
    for (const auto& ev : m_recording)
    {
        const int trackIndex = juce::jlimit (0, 3, ev.channel - 1);
        auto msg = ev.noteOn
            ? juce::MidiMessage::noteOn (ev.channel, ev.pitch, static_cast<juce::uint8> (juce::jlimit (1, 127, ev.velocity)))
            : juce::MidiMessage::noteOff (ev.channel, ev.pitch);
        // Keine führende Stille: die erste Note liegt bei Tick 0.
        msg.setTimeStamp (static_cast<double> (ev.tick - firstTick));
        tracks[static_cast<std::size_t> (trackIndex)].addEvent (msg);
    }

    for (auto& track : tracks)
    {
        track.sort();
        midiFile.addTrack (track);
    }

    if (! midiFile.writeTo (stream, 1))
        return false;

    stream.flush();
    return true;
}

void Processor::closeClockInput() noexcept
{
    if (m_clockInput != nullptr)
    {
        m_clockInput->stop();
        m_clockInput.reset();
    }
}

juce::AudioProcessorEditor* Processor::createEditor()
{
    return new Editor (*this);
}

void Processor::getStateInformation (juce::MemoryBlock& destData)
{
    if (const auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void Processor::setStateInformation (const void* data, int sizeInBytes)
{
    if (const auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));

            // Preset-Wechsel: klingende Noten sofort beenden (Spezifikation 8.2).
            m_bridge.requestAllNotesOff();
        }
    }
}

} // namespace heptachord

// Der Wrapper erzeugt die Instanz ueber diese Factory.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new heptachord::Processor();
}
