#include "plugin/PluginProcessor.h"

#include "plugin/ParameterLayout.h"
#include "plugin/PluginEditor.h"

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
    m_instrument->prepare (sampleRate);
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

void Processor::releaseResources()
{
    closeClockInput();
    m_bridge.requestAllNotesOff();
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

        // Aufnahme: Output-Events einfangen.
        if (state == 1)
        {
            const int currentStep = m_currentStepIndex.load (std::memory_order_relaxed);

            // Start-Step beim ersten Block merken.
            if (m_recordingStartStep < 0 && currentStep >= 0)
                m_recordingStartStep = currentStep;

            const std::lock_guard<std::mutex> lock (m_recordingMutex);
            for (const auto metadata : midiMessages)
            {
                const auto msg = metadata.getMessage();
                if (msg.isNoteOn() || msg.isNoteOff())
                {
                    const std::int64_t tick = blockStartTick + static_cast<std::int64_t> (
                        static_cast<double> (metadata.samplePosition) * bpm * midirender::kPpq / (60.0 * sampleRate));
                    m_recording.push_back ({ tick, msg.getChannel(), msg.getNoteNumber(),
                                             msg.isNoteOn() ? msg.getVelocity() : 0, msg.isNoteOn() });
                }
            }
            // Start-Tick beim ersten Event setzen.
            if (m_recordingStartTick == 0 && ! m_recording.empty())
                m_recordingStartTick = m_recording.front().tick;

            // Zyklus-Erkennung: wenn der Sequencer zum Start-Step zurueckkehrt
            // (nach mindestens einem Step), Aufnahme stoppen (nur im 1x-Modus).
            if (m_cycleRecording.load (std::memory_order_relaxed) && m_recordingStartStep >= 0
                && currentStep >= 0 && currentStep == m_recordingStartStep && m_stepsRecorded > 0)
            {
                m_recorderState.store (0, std::memory_order_relaxed);
                m_recordingLength.store (m_recording.empty() ? 0 : m_recording.back().tick + 1,
                                         std::memory_order_relaxed);
            }

            // Steps zaehlen (fuer 1x-Modus).
            if (currentStep >= 0 && (m_stepsRecorded == 0 || currentStep != m_lastRecordedStep))
            {
                m_stepsRecorded++;
                m_lastRecordedStep = currentStep;
            }
        }

        // Wiedergabe: aufgenommene Events einspielen.
        if (state == 2 && ! m_recording.empty())
        {
            const std::int64_t recLen = m_recordingLength.load (std::memory_order_relaxed);
            const std::int64_t baseLen = (recLen > 0) ? recLen : (m_recording.back().tick - m_recordingStartTick + 1);
            const int cycles = m_loopCycles.load (std::memory_order_relaxed);
            const std::int64_t loopLen = baseLen * cycles;

            std::vector<RecordedMidiEvent> toPlay;
            {
                const std::lock_guard<std::mutex> lock (m_recordingMutex);
                for (const auto& ev : m_recording)
                {
                    // Relativer Tick innerhalb der Aufnahme.
                    std::int64_t relTick = ev.tick - m_recordingStartTick;
                    // Loop.
                    if (loopLen > 0)
                        relTick = relTick % loopLen;
                    std::int64_t absTick = m_playbackTick + relTick;
                    // Nur Events in diesem Block.
                    if (absTick >= blockStartTick && absTick < blockEndTick)
                    {
                        int samplePos = static_cast<int> (
                            (static_cast<double> (absTick - blockStartTick) / (blockEndTick - blockStartTick)) *
                            static_cast<double> (buffer.getNumSamples()));
                        samplePos = std::clamp (samplePos, 0, buffer.getNumSamples() - 1);
                        juce::MidiMessage msg = ev.noteOn
                            ? juce::MidiMessage::noteOn (ev.channel, ev.pitch, (juce::uint8) ev.velocity)
                            : juce::MidiMessage::noteOff (ev.channel, ev.pitch);
                        msg.setTimeStamp (samplePos);
                        midiMessages.addEvent (msg, samplePos);
                    }
                }
            }
            // Playback-Tick vorruecken.
            m_playbackTick += (blockEndTick - blockStartTick);
        }

        if (m_instrument != nullptr)
            m_instrument->process (midiMessages, buffer);
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

void Processor::startRecording() noexcept
{
    if (isStandalone())
    {
        const std::lock_guard<std::mutex> lock (m_recordingMutex);
        // Overdub: wenn bereits eine Aufnahme existiert, nicht loeschen.
        if (m_recording.empty())
        {
            m_recordingStartTick = 0;  // wird im processBlock gesetzt
            m_recordingStartStep = -1;
            m_stepsRecorded = 0;
        }
        m_cycleRecording.store (false, std::memory_order_relaxed);
        m_recorderState.store (1, std::memory_order_relaxed);
    }
}

void Processor::startCycleRecording() noexcept
{
    if (isStandalone())
    {
        const std::lock_guard<std::mutex> lock (m_recordingMutex);
        // Overdub: wenn bereits eine Aufnahme existiert, nicht loeschen.
        if (m_recording.empty())
        {
            m_recordingStartTick = 0;
            m_recordingStartStep = -1;
            m_stepsRecorded = 0;
        }
        m_cycleRecording.store (true, std::memory_order_relaxed);
        m_recorderState.store (1, std::memory_order_relaxed);
    }
}

void Processor::stopRecording() noexcept
{
    if (isStandalone())
    {
        m_recorderState.store (0, std::memory_order_relaxed);
        const std::lock_guard<std::mutex> lock (m_recordingMutex);
        m_recordingLength.store (m_recording.empty() ? 0 : m_recording.back().tick + 1,
                                 std::memory_order_relaxed);
    }
}

void Processor::startPlayback() noexcept
{
    if (isStandalone() && hasRecording())
    {
        m_playbackTick = 0;
        m_recorderState.store (2, std::memory_order_relaxed);
    }
}

void Processor::stopPlayback() noexcept
{
    if (isStandalone())
        m_recorderState.store (0, std::memory_order_relaxed);
}

void Processor::clearRecording() noexcept
{
    if (isStandalone())
    {
        const std::lock_guard<std::mutex> lock (m_recordingMutex);
        m_recording.clear();
        m_recordingLength.store (0, std::memory_order_relaxed);
        m_recorderState.store (0, std::memory_order_relaxed);
    }
}

void Processor::setLoopCycles (int cycles) noexcept
{
    if (cycles == 1 || cycles == 2 || cycles == 4 || cycles == 8)
        m_loopCycles.store (cycles, std::memory_order_relaxed);
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
