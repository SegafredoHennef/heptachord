#include "core/StepSequencer.h"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <vector>

using namespace heptachord;

namespace
{
std::array<StepSettings, kNumSteps> makeSteps (const std::vector<int>& activeIndices)
{
    std::array<StepSettings, kNumSteps> steps {};
    for (auto& step : steps)
        step.active = false;

    for (const int index : activeIndices)
        steps[static_cast<std::size_t> (index)].active = true;

    return steps;
}

std::vector<int> collect (StepSequencer& sequencer, int count)
{
    std::vector<int> result;
    for (int i = 0; i < count; ++i)
    {
        int stepIndex = -1;
        REQUIRE (sequencer.nextStep (stepIndex));
        result.push_back (stepIndex);
    }
    return result;
}
} // namespace

TEST_CASE ("Vorwaerts laeuft ueber alle aktiven Steps", "[sequencer][T9]")
{
    StepSequencer sequencer;
    sequencer.setSteps (makeSteps ({ 0, 1, 2, 3, 4, 5, 6, 7 }));
    sequencer.setDirection (Direction::Forward);
    sequencer.reset();

    REQUIRE (sequencer.activeCount() == 8);
    const auto order = collect (sequencer, 10);
    REQUIRE (order == std::vector<int> { 0, 1, 2, 3, 4, 5, 6, 7, 0, 1 });
}

TEST_CASE ("Inaktive Steps fallen aus dem Zyklus", "[sequencer][T9]")
{
    StepSequencer sequencer;
    sequencer.setSteps (makeSteps ({ 0, 2, 5 }));
    sequencer.setDirection (Direction::Forward);
    sequencer.reset();

    REQUIRE (sequencer.activeCount() == 3);
    const auto order = collect (sequencer, 6);
    REQUIRE (order == std::vector<int> { 0, 2, 5, 0, 2, 5 });
}

TEST_CASE ("Rueckwaerts startet beim letzten aktiven Step", "[sequencer]")
{
    StepSequencer sequencer;
    sequencer.setSteps (makeSteps ({ 1, 3, 6 }));
    sequencer.setDirection (Direction::Backward);
    sequencer.reset();

    const auto order = collect (sequencer, 6);
    REQUIRE (order == std::vector<int> { 6, 3, 1, 6, 3, 1 });
}

TEST_CASE ("PingPong wiederholt die Endpunkte nicht", "[sequencer][T10]")
{
    StepSequencer sequencer;
    sequencer.setSteps (makeSteps ({ 0, 1, 2, 3 }));
    sequencer.setDirection (Direction::PendelI);
    sequencer.reset();

    const auto order = collect (sequencer, 12);
    REQUIRE (order == std::vector<int> { 0, 1, 2, 3, 2, 1, 0, 1, 2, 3, 2, 1 });
}

TEST_CASE ("PingPong laeuft nur ueber aktive Steps", "[sequencer][T10]")
{
    StepSequencer sequencer;
    sequencer.setSteps (makeSteps ({ 1, 4, 6 }));
    sequencer.setDirection (Direction::PendelI);
    sequencer.reset();

    const auto order = collect (sequencer, 8);
    REQUIRE (order == std::vector<int> { 1, 4, 6, 4, 1, 4, 6, 4 });
}

TEST_CASE ("PingPong mit zwei Steps wechselt", "[sequencer][T10]")
{
    StepSequencer sequencer;
    sequencer.setSteps (makeSteps ({ 2, 5 }));
    sequencer.setDirection (Direction::PendelI);
    sequencer.reset();

    const auto order = collect (sequencer, 4);
    REQUIRE (order == std::vector<int> { 2, 5, 2, 5 });
}

TEST_CASE ("Zufall bleibt im Bereich der aktiven Steps", "[sequencer]")
{
    StepSequencer sequencer;
    sequencer.setSteps (makeSteps ({ 1, 4, 6 }));

    // Deterministische Zufallsquelle: 0.0, 0.5, 0.99
    const std::vector<double> values { 0.0, 0.5, 0.99, 0.25 };
    std::size_t index = 0;
    sequencer.setRandomSource ([&values, &index] {
        return values[index++ % values.size()];
    });

    sequencer.setDirection (Direction::Random);
    sequencer.reset();

    const auto order = collect (sequencer, 4);
    REQUIRE (order == std::vector<int> { 1, 4, 6, 1 });
}

TEST_CASE ("Kein aktiver Step liefert keinen Schritt", "[sequencer][T9]")
{
    StepSequencer sequencer;
    sequencer.setSteps (makeSteps ({}));
    sequencer.setDirection (Direction::Forward);
    sequencer.reset();

    REQUIRE (sequencer.activeCount() == 0);

    int stepIndex = -1;
    REQUIRE_FALSE (sequencer.nextStep (stepIndex));
    REQUIRE (stepIndex == -1);
}

TEST_CASE ("Rest-Steps bleiben im Zyklus", "[sequencer][T9]")
{
    auto steps = makeSteps ({ 0, 1, 2, 3 });
    steps[2].mode = StepMode::Rest;

    StepSequencer sequencer;
    sequencer.setSteps (steps);
    sequencer.setDirection (Direction::Forward);
    sequencer.reset();

    const auto order = collect (sequencer, 4);
    REQUIRE (order == std::vector<int> { 0, 1, 2, 3 });
}

TEST_CASE ("Reset setzt den Cursor auf den Anfang", "[sequencer]")
{
    StepSequencer sequencer;
    sequencer.setSteps (makeSteps ({ 0, 1, 2 }));
    sequencer.setDirection (Direction::Forward);
    sequencer.reset();

    int stepIndex = -1;
    REQUIRE (sequencer.nextStep (stepIndex));
    REQUIRE (stepIndex == 0);
    REQUIRE (sequencer.nextStep (stepIndex));
    REQUIRE (stepIndex == 1);

    sequencer.reset();

    REQUIRE (sequencer.nextStep (stepIndex));
    REQUIRE (stepIndex == 0);
}
