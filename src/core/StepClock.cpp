#include "core/StepClock.h"

#include <algorithm>

namespace heptachord
{
namespace
{
// Kleinster und groesster zulaessiger Rate-Wert (1/16 ... 8).
constexpr double kMinRate = 0.0625;
constexpr double kMaxRate = 8.0;

// Tempo-Grenzen: 20 ... 400 BPM. 0 liefert der Host, wenn er kein Tempo kennt.
constexpr double kMinBpm = 20.0;
constexpr double kMaxBpm = 400.0;
} // namespace

void StepClock::setRate (float rate) noexcept
{
    m_rate = std::clamp (static_cast<double> (rate), kMinRate, kMaxRate);
}

void StepClock::setBpm (double bpm) noexcept
{
    m_bpm = std::clamp (bpm, kMinBpm, kMaxBpm);
}

void StepClock::setSwing (float percent) noexcept
{
    m_swingPercent = std::clamp (static_cast<double> (percent), 0.0, 75.0);
}

void StepClock::setSlackMs (float ms) noexcept
{
    m_slackMs = std::clamp (static_cast<double> (ms), -100.0, 100.0);
}

double StepClock::stepSeconds() const noexcept
{
    return m_rate * 4.0 * 60.0 / m_bpm;
}

double StepClock::swingSeconds() const noexcept
{
    return m_swingPercent / 100.0 * stepSeconds();
}

double StepClock::slackSeconds() const noexcept
{
    // Slack wird auf weniger als eine Step-Laenge geklemmt.
    const double limit = stepSeconds() * 0.999;
    return std::clamp (m_slackMs / 1000.0, -limit, limit);
}

double StepClock::slotTimeSeconds (std::int64_t slotIndex) const noexcept
{
    const double n = static_cast<double> (slotIndex);
    const double swing = (slotIndex % 2 != 0) ? swingSeconds() : 0.0;
    return n * stepSeconds() + swing + slackSeconds();
}

} // namespace heptachord
