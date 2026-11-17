#include "core/ParameterTable.h"
#include "core/SettingsFromValues.h"
#include "plugin/Preset.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <string>

using namespace heptachord;
using Catch::Approx;

namespace
{
// Eine Repraesentation der 117 Parameterwerte, die sich von den Defaults
// unterscheidet (Akzente, Ratchets, Wahrscheinlichkeiten, Masken).
ParameterValues makeNonDefaultValues()
{
    ParameterValues values = defaultParameterValues();

    values[static_cast<std::size_t> (params::id_root)] = 2.0f;          // D
    values[static_cast<std::size_t> (params::id_scale)] = 4.0f;         // Dorisch
    values[static_cast<std::size_t> (params::id_rate)] = 4.0f;          // 1/4
    values[static_cast<std::size_t> (params::id_direction)] = 2.0f;     // PingPong
    values[static_cast<std::size_t> (params::id_swing)] = 40.0f;
    values[static_cast<std::size_t> (params::id_slack)] = -20.0f;
    values[static_cast<std::size_t> (params::id_latch)] = 1.0f;

    values[static_cast<std::size_t> (params::id_thru_velocity)] = 120.0f;
    values[static_cast<std::size_t> (params::id_harm_spread)] = 0.6f;

    values[static_cast<std::size_t> (params::id_h1_degree)] = 3.0f;
    values[static_cast<std::size_t> (params::id_h1_octave)] = -1.0f;
    values[static_cast<std::size_t> (params::id_h1_velocity)] = 90.0f;
    values[static_cast<std::size_t> (params::id_h1_follow)] = 0.0f;
    values[static_cast<std::size_t> (params::id_h1_gate)] = 75.0f;
    values[static_cast<std::size_t> (params::id_h1_channel)] = 5.0f;

    for (int step = 0; step < 8; ++step)
    {
        const int base = params::id_s1_active + step * 10;
        values[static_cast<std::size_t> (base + 0)] = (step % 2 == 0) ? 1.0f : 0.0f;
        values[static_cast<std::size_t> (base + 1)] = (step == 3) ? 1.0f : 0.0f; // Hold
        values[static_cast<std::size_t> (base + 2)] = static_cast<float> (step - 4);
        values[static_cast<std::size_t> (base + 3)] = 80.0f + step * 5.0f;
        values[static_cast<std::size_t> (base + 4)] = (step == 5) ? 50.0f : 100.0f;
        values[static_cast<std::size_t> (base + 5)] = (step == 7) ? 4.0f : 2.0f;
        values[static_cast<std::size_t> (base + 6)] = (step != 1) ? 1.0f : 0.0f;
        values[static_cast<std::size_t> (base + 7)] = 1.0f;
        values[static_cast<std::size_t> (base + 8)] = 1.0f;
        values[static_cast<std::size_t> (base + 9)] = 0.0f;
    }

    return values;
}
} // namespace

TEST_CASE ("Preset-Roundtrip: Zustand -> JSON -> Zustand ist identisch", "[preset]")
{
    const ParameterValues original = makeNonDefaultValues();

    const Preset saved ("Rundum-Sorglos", original);
    const juce::String json = saved.toJson();

    const Preset loaded = Preset::fromJson (json);

    REQUIRE (loaded.getName() == "Rundum-Sorglos");
    REQUIRE (loaded.getValues().size() == original.size());

    for (std::size_t i = 0; i < original.size(); ++i)
    {
        // Trigger (reset) wird nicht gespeichert; dort bleibt der Default.
        if (i == static_cast<std::size_t> (params::id_reset))
            continue;

        INFO ("Parameter " << params::kParams[i].id << " weicht ab");
        REQUIRE (loaded.getValues()[i] == Approx (original[i]).margin (0.0001f));
    }
}

TEST_CASE ("Default-Preset roundtripped ist identisch zu den Defaults", "[preset]")
{
    const ParameterValues original = defaultParameterValues();
    const Preset saved ("Defaults", original);
    const Preset loaded = Preset::fromJson (saved.toJson());

    for (std::size_t i = 0; i < original.size(); ++i)
    {
        if (i == static_cast<std::size_t> (params::id_reset))
            continue;

        INFO ("Parameter " << params::kParams[i].id << " weicht ab");
        REQUIRE (loaded.getValues()[i] == Approx (original[i]).margin (0.0001f));
    }
}

TEST_CASE ("Factory-Presets sind valide und laden ohne Fehler", "[preset]")
{
    const auto presets = Preset::factoryPresets();
    REQUIRE (presets.size() == 96);

    for (const auto& preset : presets)
    {
        INFO ("Preset: " << preset.getName().toStdString());
        REQUIRE_FALSE (preset.getName().isEmpty());

        // Jeder Parameter liegt im definierten Bereich.
        for (int i = 0; i < params::kNumParameters; ++i)
        {
            const auto& p = params::kParams[static_cast<std::size_t> (i)];
            const float v = preset.getValues()[static_cast<std::size_t> (i)];

            switch (p.type)
            {
                case params::Type::Choice:
                    REQUIRE ((v >= 0.0f && v < static_cast<float> (p.numChoices)));
                    break;
                case params::Type::Bool:
                    REQUIRE ((v == 0.0f || v == 1.0f));
                    break;
                case params::Type::Trigger:
                    break;
                default:
                    REQUIRE ((v >= p.min && v <= p.max));
                    break;
            }
        }
    }
}

TEST_CASE ("Factory-Presets bilden die 96er-Matrix in chromatischer Reihenfolge", "[preset]")
{
    const auto presets = Preset::factoryPresets();
    REQUIRE (presets.size() == 96);

    // Slot i = Grundton * 8 + Skala (Decision 82).
    for (int i = 0; i < 96; ++i)
    {
        const int root = i / 8;
        const int scale = i % 8;
        const auto& preset = presets[static_cast<std::size_t> (i)];

        INFO ("Slot " << i << " erwartet Grundton " << root << ", Skala " << scale);
        REQUIRE (preset.getValues()[static_cast<std::size_t> (params::id_root)] == static_cast<float> (root));
        REQUIRE (preset.getValues()[static_cast<std::size_t> (params::id_scale)] == static_cast<float> (scale));
    }
}

TEST_CASE ("Charakter-Pattern ist pro Skala identisch, nur der Grundton transponiert", "[preset]")
{
    const auto presets = Preset::factoryPresets();
    REQUIRE (presets.size() == 96);

    // Alle Werte von C-Dur bis H-Mixolydisch: Grundton C gegen Grundton F
    // (Index 5) - ausser root darf nichts abweichen (Decision 82).
    for (int scale = 0; scale < 8; ++scale)
    {
        const auto& fromC = presets[static_cast<std::size_t> (scale)];
        const auto& fromF = presets[static_cast<std::size_t> (5 * 8 + scale)];

        for (int i = 0; i < params::kNumParameters; ++i)
        {
            if (i == params::id_root)
                continue;

            INFO ("Skala " << scale << ", Parameter " << params::kParams[static_cast<std::size_t> (i)].id);
            REQUIRE (fromC.getValues()[static_cast<std::size_t> (i)]
                     == fromF.getValues()[static_cast<std::size_t> (i)]);
        }
    }
}

TEST_CASE ("Preset-Namen sind Grundton-zuerst in deutscher Kanon-Form", "[preset]")
{
    const auto presets = Preset::factoryPresets();
    REQUIRE (presets.size() == 96);

    REQUIRE (presets[0].getName() == "C Dur");
    REQUIRE (presets[1].getName() == "C Moll");
    REQUIRE (presets[7].getName() == "C Mixolydisch");
    REQUIRE (presets[8].getName() == "Cis Dur");
    REQUIRE (presets[10 * 8 + 1].getName() == "Ais Moll");   // Slot 81
    REQUIRE (presets[11 * 8].getName() == "H Dur");          // Slot 89
    REQUIRE (presets[95].getName() == "H Mixolydisch");
}

TEST_CASE ("Falsche schemaVersion wird abgelehnt", "[preset]")
{
    const juce::String json = R"({"schemaVersion": 99, "name": "Falsch", "parameters": {}})";
    REQUIRE_THROWS_AS (Preset::fromJson (json), std::runtime_error);
}

TEST_CASE ("Ungueltiges JSON wird abgelehnt", "[preset]")
{
    REQUIRE_THROWS_AS (Preset::fromJson ("{kein json"), std::runtime_error);
}

TEST_CASE ("Preset-Normalisierung: Wert -> 0..1 -> Wert ist identisch", "[preset]")
{
    const ParameterValues original = makeNonDefaultValues();
    const Preset preset ("Norm", original);
    const auto normalized = preset.normalizedValues();

    for (std::size_t i = 0; i < original.size(); ++i)
    {
        const auto& p = params::kParams[i];
        float back = 0.0f;

        switch (p.type)
        {
            case params::Type::Bool:
                back = normalized[i] >= 0.5f ? 1.0f : 0.0f;
                break;

            case params::Type::Choice:
                back = static_cast<float> (
                    std::lround (normalized[i] * static_cast<float> (p.numChoices - 1)));
                break;

            case params::Type::Trigger:
                back = original[i];
                break;

            case params::Type::Int:
                back = static_cast<float> (
                    std::lround (p.min + normalized[i] * (p.max - p.min)));
                break;

            default:
                back = p.min + normalized[i] * (p.max - p.min);
                break;
        }

        INFO ("Parameter " << p.id << " weicht ab");
        REQUIRE (back == Approx (original[i]).margin (0.001f));
    }
}
