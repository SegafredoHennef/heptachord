#pragma once

#include <cstdint>

namespace heptachord
{

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
