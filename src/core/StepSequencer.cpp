#include "core/StepSequencer.h"

#include <algorithm>

namespace heptachord
{

void StepSequencer::setSteps (const std::array<StepSettings, kNumSteps>& steps) noexcept
{
    m_steps = steps;
    rebuildActive();
    reset();
}

void StepSequencer::setDirection (Direction direction) noexcept
{
    if (m_direction == direction)
        return;

    m_direction = direction;
    reset();
}

void StepSequencer::setRandomSource (RandomSource source)
{
    m_random = std::move (source);
}

void StepSequencer::rebuildActive() noexcept
{
    m_active.clear();
    for (int i = 0; i < kNumSteps; ++i)
        if (m_steps[static_cast<std::size_t> (i)].active)
            m_active.push_back (i);
}

void StepSequencer::reset() noexcept
{
    m_pingPongDirection = 1;

    switch (m_direction)
    {
        case Direction::Forward:
            m_position = -1;             // der naechste Aufruf liefert 0
            break;
        case Direction::Backward:
            m_position = static_cast<int> (m_active.size());
            break;
        case Direction::PingPong:
            m_position = -1;             // der naechste Aufruf liefert 0
            break;
        case Direction::Random:
            m_position = -1;
            break;
    }
}

bool StepSequencer::nextStep (int& stepIndex) noexcept
{
    if (m_active.empty())
    {
        stepIndex = -1;
        return false;
    }

    const int count = static_cast<int> (m_active.size());

    switch (m_direction)
    {
        case Direction::Forward:
            m_position = ((m_position + 1) % count + count) % count;
            break;

        case Direction::Backward:
            m_position = ((m_position - 1) % count + count) % count;
            break;

        case Direction::PingPong:
        {
            if (count <= 2)
            {
                // Bei zwei Steps wechseln die Richtungen; bei einem bleibt
                // der Cursor stehen.
                m_position = (count == 1) ? 0 : (m_position == 0 ? 1 : 0);
                break;
            }

            if (m_position < 0)
            {
                // Erster Schritt der Folge.
                m_position = 0;
                m_pingPongDirection = 1;
                break;
            }

            int next = m_position + m_pingPongDirection;
            if (next > count - 1)
            {
                m_pingPongDirection = -1;
                next = count - 2;
            }
            else if (next < 0)
            {
                m_pingPongDirection = 1;
                next = 1;
            }
            m_position = next;
            break;
        }

        case Direction::Random:
        {
            const double value = m_random ? m_random() : 0.5;
            int index = static_cast<int> (value * count);
            index = std::clamp (index, 0, count - 1);
            m_position = index;
            break;
        }
    }

    stepIndex = m_active[static_cast<std::size_t> (m_position)];
    return true;
}

int StepSequencer::activeCount() const noexcept
{
    return static_cast<int> (m_active.size());
}

} // namespace heptachord
