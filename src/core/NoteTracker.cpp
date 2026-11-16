#include "core/NoteTracker.h"

#include <algorithm>

namespace heptachord
{

void NoteTracker::setLatch (bool latch) noexcept
{
    m_latch = latch;

    // Latch aus loescht den Speicher (Entscheidung 2.2 der Spezifikation).
    if (! latch)
        m_hasLatch = false;
}

void NoteTracker::noteOn (int pitch, int velocity) noexcept
{
    pitch = std::clamp (pitch, 0, 127);

    // Eine erneut gedrueckte Note wandert nach oben im Stapel.
    remove (pitch);

    if (m_count < static_cast<int> (m_held.size()))
        m_held[static_cast<std::size_t> (m_count++)] = { pitch, std::clamp (velocity, 1, 127) };

    m_lastPitch = pitch;
    m_lastVelocity = std::clamp (velocity, 1, 127);
    m_hasLatch = true;
}

void NoteTracker::noteOff (int pitch) noexcept
{
    remove (std::clamp (pitch, 0, 127));
}

void NoteTracker::releaseAll() noexcept
{
    m_count = 0;
}

void NoteTracker::reset() noexcept
{
    m_count = 0;
    m_hasLatch = false;
}

bool NoteTracker::active() const noexcept
{
    return m_count > 0 || (m_latch && m_hasLatch);
}

int NoteTracker::pitch() const noexcept
{
    if (m_count > 0)
        return m_held[static_cast<std::size_t> (m_count - 1)].pitch;

    return m_lastPitch;
}

int NoteTracker::velocity() const noexcept
{
    // Die Velocity gehoert zur klingenden Note: faellt die letzte Note weg,
    // gilt die Velocity der naechsten gehaltenen (nicht die der geloesten).
    if (m_count > 0)
        return m_held[static_cast<std::size_t> (m_count - 1)].velocity;

    return m_lastVelocity;
}

void NoteTracker::remove (int pitch) noexcept
{
    for (int i = 0; i < m_count; ++i)
    {
        if (m_held[static_cast<std::size_t> (i)].pitch != pitch)
            continue;

        for (int j = i; j + 1 < m_count; ++j)
            m_held[static_cast<std::size_t> (j)] = m_held[static_cast<std::size_t> (j + 1)];

        --m_count;
        return;
    }
}

} // namespace heptachord
