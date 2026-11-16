#include "plugin/ProcessorBridge.h"

#include "core/SettingsFromValues.h"

#include <algorithm>

namespace heptachord
{
namespace
{
// Kanaele, auf denen die aktuellen Einstellungen Noten ausgeben.
std::array<bool, 16> channelLayout (const EngineSettings& settings) noexcept
{
    std::array<bool, 16> used {};

    const auto mark = [&used] (int channel)
    {
        if (channel >= 1 && channel <= 16)
            used[static_cast<std::size_t> (channel - 1)] = true;
    };

    mark (settings.thru.channel);

    if (settings.outputMode == OutputMode::Multi)
        for (const auto& harmony : settings.harmony)
            mark (harmony.channel);

    return used;
}
} // namespace

ProcessorBridge::ProcessorBridge() = default;

void ProcessorBridge::prepare (double sampleRate, int maximumBlockSize)
{
    m_input.reserve (static_cast<std::size_t> (kMaxEventsPerBlock));

    juce::ignoreUnused (sampleRate, maximumBlockSize);
}

void ProcessorBridge::process (const ParameterValues& values,
                               const HostTimeline& timeline,
                               const juce::MidiBuffer& midiIn,
                               juce::MidiBuffer& midiOut)
{
    const EngineSettings settings = settingsFromValues (values);

    // 1. Eingabe einsammeln, dann die Ausgabe leeren (midiIn == midiOut ist
    //    erlaubt).
    collectInput (midiIn);
    midiOut.clear();

    // 2. Reset-Trigger: nur die Flanke 0 -> 1 loest aus (Entscheidung 67).
    const bool reset = values[static_cast<std::size_t> (params::id_reset)] >= 0.5f;
    if (reset && ! m_lastReset)
        m_engine.requestReset();
    m_lastReset = reset;

    // 3. Kanalbelegung: wechselt sie, bleiben sonst Noten auf dem alten Kanal
    //    haengen. Der Wechsel wirkt sofort (Spezifikation 8.2).
    applyLayoutChange (settings, midiOut);

    m_engine.setSettings (settings);

    // 4. Rechnen.
    m_engine.processBlock (timeline, m_input, m_events);

    // 5. Ausgeben.
    writeOutput (midiOut);

    m_input.clear();
    m_previousChannels = channelLayout (settings);
    m_havePrevious = true;
}

void ProcessorBridge::applyLayoutChange (const EngineSettings& settings, juce::MidiBuffer& midiOut)
{
    if (! m_havePrevious)
        return;

    const auto channels = channelLayout (settings);
    if (channels == m_previousChannels)
        return;

    // Kanaele, die wegfallen, bekommen ihr All-Notes-Off sofort. Die noch
    // belegten Kanaele erledigt die Engine beim naechsten Block.
    for (int channel = 0; channel < 16; ++channel)
        if (m_previousChannels[static_cast<std::size_t> (channel)]
            && ! channels[static_cast<std::size_t> (channel)])
            midiOut.addEvent (juce::MidiMessage::allNotesOff (channel + 1), 0);

    m_engine.requestAllNotesOff();
}

void ProcessorBridge::collectInput (const juce::MidiBuffer& midiIn)
{
    for (const auto metadata : midiIn)
    {
        const auto message = metadata.getMessage();
        TimedEvent event;

        event.offsetSamples = metadata.samplePosition;
        event.channel = message.getChannel();

        if (message.isNoteOn())
        {
            event.type = EventType::NoteOn;
            event.pitch = message.getNoteNumber();
            event.velocity = message.getVelocity();
        }
        else if (message.isNoteOff())
        {
            event.type = EventType::NoteOff;
            event.pitch = message.getNoteNumber();
        }
        else if (message.isAllNotesOff() || message.isAllSoundOff())
        {
            event.type = EventType::AllNotesOff;
        }
        else
        {
            continue;
        }

        m_input.push_back (event);
    }
}

void ProcessorBridge::writeOutput (juce::MidiBuffer& midiOut)
{
    for (int i = 0; i < m_events.size(); ++i)
    {
        const auto& event = m_events[i];

        switch (event.type)
        {
            case EventType::NoteOn:
                midiOut.addEvent (juce::MidiMessage::noteOn (event.channel, event.pitch,
                                                             static_cast<juce::uint8> (event.velocity)),
                                  event.offsetSamples);
                break;

            case EventType::NoteOff:
                midiOut.addEvent (juce::MidiMessage::noteOff (event.channel, event.pitch),
                                  event.offsetSamples);
                break;

            case EventType::AllNotesOff:
                midiOut.addEvent (juce::MidiMessage::allNotesOff (event.channel),
                                  event.offsetSamples);
                break;
        }
    }
}

} // namespace heptachord
