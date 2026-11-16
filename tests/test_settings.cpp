#include "core/SettingsFromValues.h"

#include "core/ParameterTable.h"
#include "core/StepClock.h"

#include <catch2/catch_test_macros.hpp>

using namespace heptachord;

TEST_CASE ("Defaults aus parameters.toml ergeben die Engine-Defaults", "[settings]")
{
    const auto values = defaultParameterValues();
    const auto settings = settingsFromValues (values);

    REQUIRE (settings.root == 0);
    REQUIRE (settings.scale == Scale::Dur);
    REQUIRE (settings.rate == 0.125f);
    REQUIRE (settings.direction == Direction::Forward);
    REQUIRE (settings.swing == 0.0f);
    REQUIRE (settings.slackMs == 0.0f);
    REQUIRE_FALSE (settings.latch);
    REQUIRE (settings.outputMode == OutputMode::Multi);

    REQUIRE (settings.thru.enable);
    REQUIRE_FALSE (settings.thru.snap);
    REQUIRE (settings.thru.velocity == 100.0f);
    REQUIRE (settings.thru.channel == 1);

    REQUIRE (settings.harmony[0].degree == 2);
    REQUIRE (settings.harmony[1].degree == 4);
    REQUIRE (settings.harmony[2].degree == 6);
    REQUIRE (settings.harmony[0].channel == 2);
    REQUIRE (settings.harmony[1].channel == 3);
    REQUIRE (settings.harmony[2].channel == 4);
    REQUIRE (settings.harmony[2].follow);

    for (const auto& step : settings.steps)
    {
        REQUIRE (step.active);
        REQUIRE (step.mode == StepMode::Play);
        REQUIRE (step.offset == 0);
        REQUIRE (step.velocity == 100.0f);
        REQUIRE (step.probability == 100.0f);
        REQUIRE (step.ratchet == 2);
        REQUIRE (step.voiceMask == kMaskAll);
    }
}

TEST_CASE ("Jeder Choice-Wert der Rate hat einen Faktor", "[settings]")
{
    REQUIRE (params::kParams[params::id_rate].numChoices == kNumRates);

    const float expected[kNumRates] = {
        0.0625f, 1.0f / 12.0f, 0.125f, 1.0f / 6.0f, 0.25f, 1.0f / 3.0f, 0.375f,
        0.5f, 2.0f / 3.0f, 0.75f, 1.0f, 4.0f / 3.0f, 1.5f, 2.0f, 3.0f, 4.0f, 8.0f
    };

    auto values = defaultParameterValues();

    for (int index = 0; index < kNumRates; ++index)
    {
        values[static_cast<std::size_t> (params::id_rate)] = static_cast<float> (index);
        REQUIRE (settingsFromValues (values).rate == expected[index]);
    }

    // Zu klein/zu gross wird geklemmt.
    values[static_cast<std::size_t> (params::id_rate)] = -5.0f;
    REQUIRE (settingsFromValues (values).rate == expected[0]);

    values[static_cast<std::size_t> (params::id_rate)] = 99.0f;
    REQUIRE (settingsFromValues (values).rate == expected[kNumRates - 1]);
}

TEST_CASE ("Skala, Richtung und Modus folgen dem Choice-Index", "[settings]")
{
    auto values = defaultParameterValues();

    for (int index = 0; index < kNumScales; ++index)
    {
        values[static_cast<std::size_t> (params::id_scale)] = static_cast<float> (index);
        REQUIRE (static_cast<int> (settingsFromValues (values).scale) == index);
    }

    for (int index = 0; index < 4; ++index)
    {
        values[static_cast<std::size_t> (params::id_direction)] = static_cast<float> (index);
        REQUIRE (static_cast<int> (settingsFromValues (values).direction) == index);
    }

    values[static_cast<std::size_t> (params::id_output_mode)] = 1.0f;
    REQUIRE (settingsFromValues (values).outputMode == OutputMode::Single);
}

TEST_CASE ("Stimmenmaske setzt sich aus vier Bools zusammen", "[settings]")
{
    const int masks[kNumSteps] = { params::id_s1_active, params::id_s2_active, params::id_s3_active,
                                   params::id_s4_active, params::id_s5_active, params::id_s6_active,
                                   params::id_s7_active, params::id_s8_active };

    for (int step = 0; step < kNumSteps; ++step)
    {
        for (int combination = 0; combination < 16; ++combination)
        {
            auto values = defaultParameterValues();
            const int base = masks[step];

            for (int bit = 0; bit < 4; ++bit)
                values[static_cast<std::size_t> (base + 6 + bit)] =
                    (combination & (1 << bit)) != 0 ? 1.0f : 0.0f;

            const auto settings = settingsFromValues (values);
            REQUIRE (settings.steps[static_cast<std::size_t> (step)].voiceMask
                     == static_cast<std::uint8_t> (combination));
        }
    }
}

TEST_CASE ("Harmoniestimmen und Steps werden positionsgenau gelesen", "[settings]")
{
    auto values = defaultParameterValues();

    values[static_cast<std::size_t> (params::id_h2_enable)] = 0.0f;
    values[static_cast<std::size_t> (params::id_h2_degree)] = -13.0f;
    values[static_cast<std::size_t> (params::id_h2_octave)] = 2.0f;
    values[static_cast<std::size_t> (params::id_h2_velocity)] = 150.0f;
    values[static_cast<std::size_t> (params::id_h2_follow)] = 0.0f;
    values[static_cast<std::size_t> (params::id_h2_gate)] = 25.0f;
    values[static_cast<std::size_t> (params::id_h2_channel)] = 9.0f;

    const auto harmony = settingsFromValues (values).harmony[1];
    REQUIRE_FALSE (harmony.enable);
    REQUIRE (harmony.degree == -13);
    REQUIRE (harmony.octave == 2);
    REQUIRE (harmony.velocity == 150.0f);
    REQUIRE_FALSE (harmony.follow);
    REQUIRE (harmony.gate == 25.0f);
    REQUIRE (harmony.channel == 9);

    const int base = params::id_s3_active;
    values[static_cast<std::size_t> (base + 0)] = 0.0f;
    values[static_cast<std::size_t> (base + 1)] = 3.0f;
    values[static_cast<std::size_t> (base + 2)] = -7.0f;
    values[static_cast<std::size_t> (base + 3)] = 175.0f;
    values[static_cast<std::size_t> (base + 4)] = 40.0f;
    values[static_cast<std::size_t> (base + 5)] = 7.0f;

    const auto step = settingsFromValues (values).steps[2];
    REQUIRE_FALSE (step.active);
    REQUIRE (step.mode == StepMode::Ratchet);
    REQUIRE (step.offset == -7);
    REQUIRE (step.velocity == 175.0f);
    REQUIRE (step.probability == 40.0f);
    REQUIRE (step.ratchet == 7);
}

TEST_CASE ("Kanaele und Bereiche werden geklemmt", "[settings]")
{
    auto values = defaultParameterValues();

    values[static_cast<std::size_t> (params::id_thru_channel)] = 0.0f;
    REQUIRE (settingsFromValues (values).thru.channel == 1);

    values[static_cast<std::size_t> (params::id_thru_channel)] = 99.0f;
    REQUIRE (settingsFromValues (values).thru.channel == 16);

    values[static_cast<std::size_t> (params::id_h1_degree)] = -99.0f;
    REQUIRE (settingsFromValues (values).harmony[0].degree == -14);

    values[static_cast<std::size_t> (params::id_s1_offset)] = 99.0f;
    REQUIRE (settingsFromValues (values).steps[0].offset == 7);

    values[static_cast<std::size_t> (params::id_s1_ratchet)] = 1.0f;
    REQUIRE (settingsFromValues (values).steps[0].ratchet == 2);
}
