#include "core/StepClock.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <vector>

using namespace heptachord;
using Catch::Approx;

namespace
{
// Die 17 Raten in der Reihenfolge des Parameters (parameters.toml).
const std::vector<float> kRates {
    0.0625f, 0.0833f, 0.125f, 0.1667f, 0.25f, 0.3333f, 0.375f, 0.5f, 0.6667f,
    0.75f, 1.0f, 1.3333f, 1.5f, 2.0f, 3.0f, 4.0f, 8.0f
};
} // namespace

TEST_CASE ("Step-Laenge folgt rate * 4 * 60 / BPM", "[clock][T5]")
{
    StepClock clock;
    clock.setBpm (120.0);

    for (const float rate : kRates)
    {
        clock.setRate (rate);
        REQUIRE (clock.stepSeconds() == Approx (static_cast<double> (rate) * 4.0 * 60.0 / 120.0));
    }

    // 1/8 bei 120 BPM: 0.125 * 4 * 60 / 120 = 0.25 s
    clock.setRate (0.125f);
    REQUIRE (clock.stepSeconds() == Approx (0.25));

    // 1.0 = ein Takt: bei 120 BPM sind das 2 Sekunden.
    clock.setRate (1.0f);
    REQUIRE (clock.stepSeconds() == Approx (2.0));
}

TEST_CASE ("Tempo aendert die Step-Laenge", "[clock][T5]")
{
    StepClock clock;
    clock.setRate (0.125f);

    clock.setBpm (120.0);
    REQUIRE (clock.stepSeconds() == Approx (0.25));
    clock.setBpm (60.0);
    REQUIRE (clock.stepSeconds() == Approx (0.5));
    clock.setBpm (240.0);
    REQUIRE (clock.stepSeconds() == Approx (0.125));
}

TEST_CASE ("Swing verzoegert die ungeraden Slots", "[clock][T6]")
{
    StepClock clock;
    clock.setRate (0.125f);      // 0.25 s
    clock.setBpm (120.0);
    clock.setSwing (50.0f);      // 50 % = 0.125 s

    REQUIRE (clock.swingSeconds() == Approx (0.125));

    // Slot 0 (1. Slot) unveraendert, Slot 1 (2. Slot) verzoegert.
    REQUIRE (clock.slotTimeSeconds (0) == Approx (0.0));
    REQUIRE (clock.slotTimeSeconds (1) == Approx (0.25 + 0.125));
    REQUIRE (clock.slotTimeSeconds (2) == Approx (0.5));
    REQUIRE (clock.slotTimeSeconds (3) == Approx (0.75 + 0.125));
}

TEST_CASE ("Slots bleiben bei 75 % Swing monoton", "[clock][T6]")
{
    StepClock clock;
    clock.setRate (0.125f);
    clock.setBpm (120.0);
    clock.setSwing (75.0f);

    double previous = -1.0;
    for (int slot = 0; slot < 16; ++slot)
    {
        const double time = clock.slotTimeSeconds (slot);
        REQUIRE (time > previous);
        previous = time;
    }
}

TEST_CASE ("Swing wird auf 75 % geklemmt", "[clock][T6]")
{
    StepClock clock;
    clock.setRate (0.125f);
    clock.setBpm (120.0);
    clock.setSwing (200.0f);

    REQUIRE (clock.swingSeconds() == Approx (0.75 * clock.stepSeconds()));
}

TEST_CASE ("Slack verschiebt alle Slots gleichermassen", "[clock][T7]")
{
    StepClock clock;
    clock.setRate (0.125f);
    clock.setBpm (120.0);
    clock.setSlackMs (50.0f);

    REQUIRE (clock.slackSeconds() == Approx (0.05));
    REQUIRE (clock.slotTimeSeconds (0) == Approx (0.05));
    REQUIRE (clock.slotTimeSeconds (1) == Approx (0.25 + 0.05));
    REQUIRE (clock.slotTimeSeconds (4) == Approx (1.0 + 0.05));
}

TEST_CASE ("Slack wird auf weniger als eine Step-Laenge geklemmt", "[clock][T7]")
{
    StepClock clock;
    clock.setRate (0.0625f);     // 1/16 bei 240 BPM = 0.0625 s
    clock.setBpm (240.0);
    clock.setSlackMs (100.0f);   // der Parameter erlaubt maximal 100 ms

    // 100 ms waeren laenger als der Step, also greift die Klemme.
    REQUIRE (clock.slackSeconds() == Approx (clock.stepSeconds() * 0.999));
    REQUIRE (clock.slackSeconds() < clock.stepSeconds());

    clock.setSlackMs (-100.0f);
    REQUIRE (clock.slackSeconds() > -clock.stepSeconds());
}

TEST_CASE ("Slack und Swing wirken zusammen", "[clock][T6][T7]")
{
    StepClock clock;
    clock.setRate (0.25f);       // 0.5 s
    clock.setBpm (120.0);
    clock.setSwing (25.0f);      // 0.125 s
    clock.setSlackMs (-20.0f);   // -0.02 s

    REQUIRE (clock.slotTimeSeconds (0) == Approx (-0.02));
    REQUIRE (clock.slotTimeSeconds (1) == Approx (0.5 + 0.125 - 0.02));
    REQUIRE (clock.slotTimeSeconds (2) == Approx (1.0 - 0.02));
}
