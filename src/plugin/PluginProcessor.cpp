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
    m_synth.prepare (sampleRate);
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
    m_synth.reset();
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
    timeline.bpm = kStandaloneBpm;
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

            timeline.bpm = kStandaloneBpm;
            timeline.isPlaying = m_standalonePlaying.load (std::memory_order_relaxed);
            timeline.ppqPosition = m_standalonePpq;

            if (timeline.isPlaying)
                m_standalonePpq += static_cast<double> (numSamples) * kStandaloneBpm / (60.0 * timeline.sampleRate);
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

    if (isStandalone())
        m_synth.process (midiMessages, buffer);
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
    options.bpm = clockActive() ? clockBpm() : kStandaloneBpm;
    options.inputNote = m_lastInputNote.load (std::memory_order_relaxed);
    options.inputVelocity = midirender::kFallbackVelocity;
    options.seed = 1;  // fester Seed (Decision 84)
    return options;
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
