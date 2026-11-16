#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace heptachord
{

// Ein MIDI-Ereignis mit Sample-Offset innerhalb eines Blocks.
// Die Plugin-Schicht uebersetzt das in juce::MidiMessage (M2b); der Core kennt
// weder JUCE noch VST3 (ADR 0007).
enum class EventType : std::uint8_t
{
    NoteOn = 0,
    NoteOff,
    AllNotesOff
};

struct TimedEvent
{
    int offsetSamples = 0;   // 0 ... Blocklaenge - 1
    EventType type = EventType::NoteOn;
    int pitch = 0;           // 0 ... 127 (bei AllNotesOff unbenutzt)
    int velocity = 0;        // 1 ... 127 bei NoteOn, sonst 0
    int channel = 1;         // MIDI-Kanal 1 ... 16
};

// Feste Kapazitaet statt std::vector: keine Allokation im Audiothread.
// 1024 Ereignisse reichen fuer jeden realistischen Block (groesster Fall:
// mehrere Ratchet-Slots mit vier Stimmen).
inline constexpr int kMaxEventsPerBlock = 1024;

class EventList
{
public:
    void clear() noexcept { m_count = 0; }

    bool add (const TimedEvent& event) noexcept
    {
        if (m_count >= kMaxEventsPerBlock)
            return false;

        m_events[static_cast<std::size_t> (m_count++)] = event;
        return true;
    }

    int size() const noexcept { return m_count; }
    bool empty() const noexcept { return m_count == 0; }

    const TimedEvent& operator[] (int index) const noexcept
    {
        return m_events[static_cast<std::size_t> (index)];
    }

private:
    std::array<TimedEvent, kMaxEventsPerBlock> m_events {};
    int m_count = 0;
};

} // namespace heptachord
