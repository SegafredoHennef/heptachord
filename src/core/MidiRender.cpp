#include "core/MidiRender.h"

#include "core/StepClock.h"
#include "core/StepPlanner.h"
#include "core/StepSequencer.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace heptachord::midirender
{
namespace
{
std::int64_t toTicks (double seconds) noexcept
{
    return static_cast<std::int64_t> (std::llround (seconds * static_cast<double> (kPpq)));
}

// Senderzustand einer Stimme waehrend des Render-Zyklus.
struct VoiceRuntime
{
    bool active = false;          // Note klingt gerade
    int pitch = 0;
    int channel = 1;
    bool hasGateOff = false;      // geplantes Gate-Ende (kein Hold)
    std::int64_t gateOffTick = 0;
};
} // namespace

std::vector<RenderedNote> renderCycle (const RenderOptions& options) noexcept
{
    std::vector<RenderedNote> notes;
    const auto& settings = options.settings;

    StepClock clock;
    clock.setRate (settings.rate);
    clock.setBpm (options.bpm > 0.0 ? options.bpm : 120.0);
    clock.setSwing (settings.swing);
    clock.setSlackMs (settings.slackMs);

    // Eine gemeinsame Zufallsquelle fuer Richtung "Zufall" und
    // Wahrscheinlichkeit - fester Seed -> bitgleicher Export (Decision 84).
    std::mt19937 rng (options.seed);
    std::uniform_real_distribution<double> distribution (0.0, 1.0);
    const RandomSource random01 = [&rng, &distribution] { return distribution (rng); };

    StepSequencer sequencer;
    sequencer.setSteps (settings.steps);
    sequencer.setDirection (settings.direction);
    sequencer.setRandomSource (random01);
    sequencer.reset();

    StepPlanner planner;
    planner.setSettings (settings);
    planner.setClock (clock);
    const int inputNote = (options.inputNote > 0 && options.inputNote <= 127)
                              ? options.inputNote : kFallbackNote;
    planner.setNote (inputNote, options.inputVelocity);

    const int activeCount = sequencer.activeCount();
    if (activeCount <= 0)
        return notes;

    std::array<VoiceState, kNumVoices> voiceState {};
    std::array<VoiceRuntime, kNumVoices> runtime {};

    // Genau ein Zyklus: jeder aktive Step einmal in Traversal-Reihenfolge.
    for (int slot = 0; slot < activeCount; ++slot)
    {
        int stepIndex = -1;
        if (! sequencer.nextStep (stepIndex))
            break;

        const StepPlan plan = planner.planSlot (stepIndex, slot);
        if (! plan.valid || plan.hits <= 0)
            continue;

        const auto& step = settings.steps[static_cast<std::size_t> (stepIndex)];
        const std::int64_t baseTick = toTicks (plan.startSeconds);

        for (int hit = 0; hit < plan.hits; ++hit)
        {
            const std::int64_t hitTick = baseTick
                + toTicks (static_cast<double> (hit) * plan.hitSpacingSeconds);

            // Wurf pro Stimme und Treffer - identisch zur Live-Engine
            // (Entscheidung 65, StepEngine::fireHit).
            std::array<bool, kNumVoices> hitMask {};
            for (int voice = 0; voice < kNumVoices; ++voice)
                hitMask[static_cast<std::size_t> (voice)] =
                    rollProbability (step.probability, random01());

            const StepActions actions = applyStep (plan, hitMask, voiceState);

            for (int voice = 0; voice < kNumVoices; ++voice)
            {
                const auto index = static_cast<std::size_t> (voice);
                auto& state = runtime[index];

                if (actions.noteOff[index] && state.active)
                {
                    notes.push_back ({ hitTick, state.pitch, 0, state.channel, voice, false });
                    state.active = false;
                    state.hasGateOff = false;
                }

                if (! actions.noteOn[index])
                    continue;

                const auto& trigger = plan.voices[index];
                notes.push_back ({ hitTick, trigger.pitch, trigger.velocity,
                                   trigger.channel, voice, true });

                state.active = true;
                state.pitch = trigger.pitch;
                state.channel = trigger.channel;
                state.hasGateOff = false;

                // Hold-Ketten laufen weiter; Play/Ratchet bekommen ein
                // Gate-Ende, das auch nach dem Zyklusende liegen darf.
                if (plan.mode != StepMode::Hold)
                {
                    state.hasGateOff = true;
                    state.gateOffTick = hitTick + toTicks (trigger.gateSeconds);
                }
            }
        }
    }

    // Offs einsammeln: geplante Gate-Enden und am Zyklusende noch klingende
    // Hold-Noten (Decision 86: eine Note darf ueber das Ende hinaus klingen).
    const std::int64_t cycleEndTick = toTicks (clock.slotTimeSeconds (activeCount));

    for (int voice = 0; voice < kNumVoices; ++voice)
    {
        const auto& state = runtime[static_cast<std::size_t> (voice)];

        if (state.hasGateOff)
        {
            notes.push_back ({ state.gateOffTick, state.pitch, 0, state.channel, voice, false });
        }
        else if (state.active)
        {
            notes.push_back ({ std::max (cycleEndTick, std::int64_t { 0 }),
                               state.pitch, 0, state.channel, voice, false });
        }
    }

    // Negatives Slack-Versatz korrigieren: alles nach vorn schieben, damit
    // der erste Tick bei 0 liegt (MIDI kennt keine negativen Zeiten).
    std::int64_t minTick = 0;
    for (const auto& note : notes)
        minTick = std::min (minTick, note.tick);

    if (minTick < 0)
        for (auto& note : notes)
            note.tick -= minTick;

    // Sortiert: Tick, Spur, Off vor On bei gleichem Tick, Tonhoehe.
    std::sort (notes.begin(), notes.end(), [] (const RenderedNote& a, const RenderedNote& b)
    {
        if (a.tick != b.tick)      return a.tick < b.tick;
        if (a.track != b.track)    return a.track < b.track;
        if (a.noteOn != b.noteOn)  return ! a.noteOn;   // Off zuerst
        return a.pitch < b.pitch;
    });

    return notes;
}

} // namespace heptachord::midirender
