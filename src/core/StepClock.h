#pragma once

#include <cstdint>

namespace heptachord
{

// Die 17 Raten aus parameters.toml (Choice-Index -> Faktor). Reihenfolge und
// Bedeutung sind eingefroren (Parameter-ID-Golden-File); 1.0 = ein Takt.
inline constexpr int kNumRates = 17;
inline constexpr float kRateValues[kNumRates] = {
    0.0625f,        // 1/16
    1.0f / 12.0f,   // 1/12
    0.125f,         // 1/8
    1.0f / 6.0f,    // 1/6
    0.25f,          // 1/4
    1.0f / 3.0f,    // 1/3
    0.375f,         // 3/8
    0.5f,           // 1/2
    2.0f / 3.0f,    // 2/3
    0.75f,          // 3/4
    1.0f,           // 1
    4.0f / 3.0f,    // 4/3
    1.5f,           // 3/2
    2.0f,           // 2
    3.0f,           // 3
    4.0f,           // 4
    8.0f            // 8
};

// Rechnet Step-Laengen und Slot-Zeiten. Kennt weder Host noch JUCE.
//
// Definitionen aus docs/03-spezifikation.md, Abschnitt 4:
//   stepSeconds = rate * 4 * 60 / BPM
//   T(n)        = n * stepSeconds + (n ungerade ? swing/100 * stepSeconds : 0) + slack
//
// n zaehlt absolut ab Transport-Start (0-basiert). "ungerade" meint damit den
// 2., 4., 6. Slot - genau die Steps, die der Swing verzoegert.
class StepClock
{
public:
    void setRate (float rate) noexcept;      // 1/16 ... 8
    void setBpm (double bpm) noexcept;
    void setSwing (float percent) noexcept;  // 0 ... 75
    void setSlackMs (float ms) noexcept;     // -100 ... +100

    double bpm() const noexcept { return m_bpm; }
    double stepSeconds() const noexcept;
    double swingSeconds() const noexcept;
    double slackSeconds() const noexcept;

    // Zeitpunkt des Slots n in Sekunden (relativ zum Transport-Start).
    double slotTimeSeconds (std::int64_t slotIndex) const noexcept;

private:
    double m_rate = 0.125;
    double m_bpm = 120.0;
    double m_swingPercent = 0.0;
    double m_slackMs = 0.0;
};

} // namespace heptachord
