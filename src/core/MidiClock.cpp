#include "core/MidiClock.h"

#include <algorithm>

namespace heptachord
{

void MidiClock::reset() noexcept
{
    const std::lock_guard<std::mutex> lock (m_mutex);
    m_playing = false;
    m_seen = false;
    m_ticks = 0;
    m_lastEvent = 0.0;
    m_windowCount = 0;
    m_windowNext = 0;
}

void MidiClock::start (double nowSeconds) noexcept
{
    const std::lock_guard<std::mutex> lock (m_mutex);
    m_playing = true;
    m_seen = true;
    m_ticks = 0;
    m_lastEvent = nowSeconds;
    m_windowCount = 0;
    m_windowNext = 0;
}

void MidiClock::stop (double nowSeconds) noexcept
{
    const std::lock_guard<std::mutex> lock (m_mutex);
    m_playing = false;
    m_seen = true;
    m_lastEvent = nowSeconds;
}

void MidiClock::continueClock (double nowSeconds) noexcept
{
    const std::lock_guard<std::mutex> lock (m_mutex);
    m_playing = true;
    m_seen = true;
    m_lastEvent = nowSeconds;
}

void MidiClock::tick (double nowSeconds) noexcept
{
    const std::lock_guard<std::mutex> lock (m_mutex);
    m_seen = true;
    m_lastEvent = nowSeconds;

    // Fenster fuehren, auch wenn der Transport steht: die Schaetzung soll
    // schon vor FA bereit sein.
    if (m_windowCount < static_cast<int> (m_window.size()))
        m_window[static_cast<std::size_t> (m_windowCount++)] = nowSeconds;
    else
    {
        m_window[static_cast<std::size_t> (m_windowNext)] = nowSeconds;
        m_windowNext = (m_windowNext + 1) % static_cast<int> (m_window.size());
    }

    if (m_playing)
        ++m_ticks;
}

MidiClock::Snapshot MidiClock::snapshot (double nowSeconds) const noexcept
{
    Snapshot result;

    {
        const std::lock_guard<std::mutex> lock (m_mutex);
        result.playing = m_playing;
        result.active = m_seen && (nowSeconds - m_lastEvent <= kClockTimeoutSeconds);
        result.bpm = estimateBpm();
        result.ppq = static_cast<double> (m_ticks) / 24.0;
    }

    return result;
}

double MidiClock::estimateBpm() const noexcept
{
    if (m_windowCount < static_cast<int> (m_window.size()))
        return kDefaultClockBpm;

    // 24 Intervalle = ein Viertel: span = t[24] - t[0], bpm = 60 / span.
    const double oldest = m_window[static_cast<std::size_t> (m_windowNext)];
    const double newest = m_window[static_cast<std::size_t> ((m_windowNext + 24) % 25)];
    const double span = newest - oldest;

    if (span <= 0.0)
        return kDefaultClockBpm;

    return std::clamp (60.0 / span, kClockMinBpm, kClockMaxBpm);
}

} // namespace heptachord
