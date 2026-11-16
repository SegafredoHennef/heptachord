#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>

namespace heptachord
{

// MIDI-Clock-Eingang (24 pulses per quarter note) als reiner Zustandautomat,
// ohne JUCE (ADR 0007). Der Standalone speist ihn aus dem ersten MIDI-Eingang
// (FA/FC/F8/FB); das VST3 laesst ihn ausser Acht (Host-PlayHead ist Quelle).
//
// Zeitstempel sind monotone Sekunden (z.B. getMillisecondCounterHiResolution
// / 1000). Alle Methoden sind thread-safe: der MIDI-Eingangs-Thread schreibt,
// der Audiothread liest ueber snapshot().
class MidiClock
{
public:
    struct Snapshot
    {
        bool playing = false;  // FA/FB aktiv, FC nicht empfangen
        bool active = false;   // innerhalb des Timeouts zuletzt ein Clock-Event
        double bpm = 120.0;    // 120, solange kein Takt schaetzbar
        double ppq = 0.0;      // Position in Vierteln seit FA
    };

    void reset() noexcept;

    void start (double nowSeconds) noexcept;         // FA
    void stop (double nowSeconds) noexcept;          // FC
    void continueClock (double nowSeconds) noexcept; // FB
    void tick (double nowSeconds) noexcept;          // F8

    Snapshot snapshot (double nowSeconds) const noexcept;

private:
    double estimateBpm() const noexcept;

    mutable std::mutex m_mutex;
    bool m_playing { false };
    bool m_seen { false };
    std::uint64_t m_ticks { 0 };
    double m_lastEvent { 0.0 };

    // Ring der letzten 25 Zeitstempel: 24 Intervalle = genau ein Viertel.
    std::array<double, 25> m_window {};
    int m_windowCount { 0 };
    int m_windowNext { 0 };
};

// Timeout ohne F8, danach faellt der Standalone auf 120 BPM zurueck.
inline constexpr double kClockTimeoutSeconds = 2.0;

// Temposchranken fuer die Schaetzung.
inline constexpr double kClockMinBpm = 30.0;
inline constexpr double kClockMaxBpm = 300.0;

// Default, solange keine Clock kommt (Entscheidung 29).
inline constexpr double kDefaultClockBpm = 120.0;

} // namespace heptachord
