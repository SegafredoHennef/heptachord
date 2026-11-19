// Tests fuer den deterministischen Zyklus-Render (Decision 84, M1).
#include "core/MidiRender.h"
#include "core/SettingsFromValues.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <tuple>
#include <vector>

using namespace heptachord;

namespace
{
midirender::RenderOptions baseOptions()
{
    midirender::RenderOptions options;
    options.settings = settingsFromValues (defaultParameterValues());
    return options;
}
} // namespace

TEST_CASE ("Render ist bitgleich bei festem Seed", "[midirender]")
{
    auto options = baseOptions();
    options.inputNote = 60;
    options.bpm = 120.0;

    const auto first = midirender::renderCycle (options);
    const auto second = midirender::renderCycle (options);

    REQUIRE_FALSE (first.empty());
    REQUIRE (first.size() == second.size());

    for (std::size_t i = 0; i < first.size(); ++i)
    {
        REQUIRE (first[i].tick == second[i].tick);
        REQUIRE (first[i].pitch == second[i].pitch);
        REQUIRE (first[i].velocity == second[i].velocity);
        REQUIRE (first[i].channel == second[i].channel);
        REQUIRE (first[i].track == second[i].track);
        REQUIRE (first[i].noteOn == second[i].noteOn);
        REQUIRE (first[i].step == second[i].step);
    }
}

TEST_CASE ("Render erzeugt Noten fuer vier Spuren und respektiert die Kanaele", "[midirender]")
{
    auto options = baseOptions();
    options.inputNote = 60;

    const auto notes = midirender::renderCycle (options);
    REQUIRE_FALSE (notes.empty());

    std::array<bool, kNumVoices> seenTrack {};
    for (const auto& note : notes)
    {
        REQUIRE (note.pitch >= 0);
        REQUIRE (note.pitch <= 127);
        if (note.noteOn)
        {
            REQUIRE (note.velocity >= 1);
            REQUIRE (note.velocity <= 127);
        }
        REQUIRE (note.channel >= 1);
        REQUIRE (note.channel <= 16);
        REQUIRE (note.track >= 0);
        REQUIRE (note.track < kNumVoices);
        REQUIRE (note.tick >= 0);
        seenTrack[static_cast<std::size_t> (note.track)] = true;
    }

    // Default: Multi-Modus, alle vier Stimmen aktiv und maskiert.
    for (bool seen : seenTrack)
        REQUIRE (seen);
}

TEST_CASE ("Render ohne Eingangsnote nutzt C4-Fallback", "[midirender]")
{
    auto options = baseOptions();
    options.inputNote = 0;  // Fallback

    const auto notes = midirender::renderCycle (options);
    REQUIRE_FALSE (notes.empty());

    bool hasThru = false;
    for (const auto& note : notes)
        if (note.track == 0 && note.noteOn)
            hasThru = true;

    REQUIRE (hasThru);
}

TEST_CASE ("C4-Fallback und explizites C4 liefern identische Noten", "[midirender]")
{
    auto fallback = baseOptions();
    fallback.inputNote = 0;
    auto explicitC4 = baseOptions();
    explicitC4.inputNote = 60;

    REQUIRE (midirender::renderCycle (fallback) == midirender::renderCycle (explicitC4));
}

TEST_CASE ("Bei schnellerem Tempo liegen die Ticks dichter", "[midirender]")
{
    auto slow = baseOptions();
    slow.bpm = 60.0;
    auto fast = baseOptions();
    fast.bpm = 240.0;

    const auto a = midirender::renderCycle (slow);
    const auto b = midirender::renderCycle (fast);
    REQUIRE_FALSE (a.empty());
    REQUIRE_FALSE (b.empty());

    // Der letzte Tick bei 240 BPM muss vor dem bei 60 BPM liegen.
    REQUIRE (b.back().tick < a.back().tick);
}

TEST_CASE ("Keine aktiven Steps ergeben einen leeren Render", "[midirender]")
{
    auto options = baseOptions();

    for (int step = 0; step < kNumSteps; ++step)
        options.settings.steps[static_cast<std::size_t> (step)].active = false;

    REQUIRE (midirender::renderCycle (options).empty());
}

TEST_CASE ("Ein Zyklus enthaelt genau die aktiven Steps einmal", "[midirender]")
{
    auto options = baseOptions();

    // Nur Step 1 und Step 5 aktiv.
    for (int step = 0; step < kNumSteps; ++step)
        options.settings.steps[static_cast<std::size_t> (step)].active = (step == 0 || step == 4);

    const auto notes = midirender::renderCycle (options);
    REQUIRE_FALSE (notes.empty());

    // Zwei Steps, jeweils mit vier Stimmen -> mehr als ein Step, aber exakt
    // zwei Step-Kanten (unterschiedliche Ticks).
    std::vector<std::int64_t> distinctTicks;
    for (const auto& note : notes)
    {
        bool found = false;
        for (auto tick : distinctTicks)
            if (tick == note.tick)
                found = true;
        if (! found)
            distinctTicks.push_back (note.tick);
    }
    REQUIRE (distinctTicks.size() >= 2);
    REQUIRE (distinctTicks.size() <= 4);  // 2 Step-Kanten + Gate-Enden
}

TEST_CASE ("RenderedNote traegt den 1-basierten Step; Offs gehoeren zum On", "[midirender]")
{
    auto options = baseOptions();

    // Nur Step 1 und Step 5 aktiv (1-basiert: Index 0 und 4).
    for (int step = 0; step < kNumSteps; ++step)
        options.settings.steps[static_cast<std::size_t> (step)].active = (step == 0 || step == 4);

    const auto notes = midirender::renderCycle (options);
    REQUIRE_FALSE (notes.empty());

    using Key = std::tuple<int, int, int>;  // Spur, Tonhoehe, Step
    std::vector<Key> onKeys;

    for (const auto& note : notes)
    {
        REQUIRE (note.step >= 1);
        REQUIRE (note.step <= kNumSteps);
        REQUIRE ((note.step == 1 || note.step == 5));

        if (note.noteOn)
            onKeys.emplace_back (note.track, note.pitch, note.step);
    }

    REQUIRE_FALSE (onKeys.empty());
    REQUIRE (std::find_if (onKeys.begin(), onKeys.end(), [] (const Key& key)
    { return std::get<2> (key) == 1; }) != onKeys.end());
    REQUIRE (std::find_if (onKeys.begin(), onKeys.end(), [] (const Key& key)
    { return std::get<2> (key) == 5; }) != onKeys.end());

    // Jedes Off gehoert zu einem On derselben Spur, Tonhoehe und Step.
    for (const auto& note : notes)
    {
        if (note.noteOn)
            continue;

        const Key key { note.track, note.pitch, note.step };
        REQUIRE (std::find (onKeys.begin(), onKeys.end(), key) != onKeys.end());
    }
}

TEST_CASE ("RenderedNote ist Vergleichbar (fuer Bitgleichheit)", "[midirender]")
{
    midirender::RenderedNote a {};
    a.tick = 100; a.pitch = 60; a.track = 1; a.noteOn = true;

    midirender::RenderedNote b = a;
    REQUIRE (a.tick == b.tick);
    REQUIRE (a.pitch == b.pitch);
    REQUIRE (a.noteOn == b.noteOn);
}
