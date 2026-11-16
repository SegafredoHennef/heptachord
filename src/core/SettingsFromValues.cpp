#include "core/SettingsFromValues.h"

#include "core/StepClock.h"

#include <algorithm>

namespace heptachord
{
namespace
{
int asInt (float value) noexcept
{
    return static_cast<int> (value);
}

bool asBool (float value) noexcept
{
    return value >= 0.5f;
}

int clampInt (float value, int low, int high) noexcept
{
    return std::clamp (asInt (value), low, high);
}

Scale asScale (float value) noexcept
{
    return static_cast<Scale> (clampInt (value, 0, kNumScales - 1));
}

Direction asDirection (float value) noexcept
{
    return static_cast<Direction> (clampInt (value, 0, 3));
}

StepMode asStepMode (float value) noexcept
{
    return static_cast<StepMode> (clampInt (value, 0, 3));
}

float asRate (float value) noexcept
{
    return kRateValues[clampInt (value, 0, kNumRates - 1)];
}

std::uint8_t asVoiceMask (float v1, float v2, float v3, float v4) noexcept
{
    std::uint8_t mask = 0;

    if (asBool (v1))
        mask |= kMaskThru;
    if (asBool (v2))
        mask |= kMaskH1;
    if (asBool (v3))
        mask |= kMaskH2;
    if (asBool (v4))
        mask |= kMaskH3;

    return mask;
}
} // namespace

ParameterValues defaultParameterValues() noexcept
{
    ParameterValues values {};

    for (std::size_t i = 0; i < values.size(); ++i)
        values[i] = params::kParams[i].defaultValue;

    return values;
}

EngineSettings settingsFromValues (const ParameterValues& values) noexcept
{
    const auto value = [&values] (int index)
    {
        return values[static_cast<std::size_t> (index)];
    };

    EngineSettings settings;

    settings.root = clampInt (value (params::id_root), 0, 11);
    settings.scale = asScale (value (params::id_scale));
    settings.rate = asRate (value (params::id_rate));
    settings.direction = asDirection (value (params::id_direction));
    settings.swing = value (params::id_swing);
    settings.slackMs = value (params::id_slack);
    settings.latch = asBool (value (params::id_latch));
    settings.outputMode = asBool (value (params::id_output_mode)) ? OutputMode::Single : OutputMode::Multi;

    settings.harmVelocity = value (params::id_harm_velocity);
    settings.harmSpread = value (params::id_harm_spread);

    settings.thru.enable = asBool (value (params::id_thru_enable));
    settings.thru.snap = asBool (value (params::id_thru_snap));
    settings.thru.velocity = value (params::id_thru_velocity);
    settings.thru.channel = clampInt (value (params::id_thru_channel), 1, 16);

    const int harmonyIds[kNumHarmonyVoices] = { params::id_h1_enable, params::id_h2_enable, params::id_h3_enable };

    for (int voice = 0; voice < kNumHarmonyVoices; ++voice)
    {
        const int base = harmonyIds[voice];

        auto& harmony = settings.harmony[static_cast<std::size_t> (voice)];
        harmony.enable = asBool (value (base + 0));
        harmony.degree = clampInt (value (base + 1), -14, 14);
        harmony.octave = clampInt (value (base + 2), -3, 3);
        harmony.velocity = value (base + 3);
        harmony.follow = asBool (value (base + 4));
        harmony.gate = value (base + 5);
        harmony.channel = clampInt (value (base + 6), 1, 16);
    }

    for (int step = 0; step < kNumSteps; ++step)
    {
        const int base = params::id_s1_active + step * 10;

        auto& target = settings.steps[static_cast<std::size_t> (step)];
        target.active = asBool (value (base + 0));
        target.mode = asStepMode (value (base + 1));
        target.offset = clampInt (value (base + 2), -7, 7);
        target.velocity = value (base + 3);
        target.probability = value (base + 4);
        target.ratchet = clampInt (value (base + 5), 2, 8);
        target.voiceMask = asVoiceMask (value (base + 6),
                                        value (base + 7),
                                        value (base + 8),
                                        value (base + 9));
    }

    return settings;
}

} // namespace heptachord
