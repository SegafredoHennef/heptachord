#include "core/StepPlanner.h"
#include "core/StepSequencer.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <array>

using namespace heptachord;
using Catch::Approx;

namespace
{
struct Fixture
{
    EngineSettings settings {};
    StepClock clock {};
    StepPlanner planner {};

    Fixture()
    {
        clock.setRate (0.125f);       // 0.25 s bei 120 BPM
        clock.setBpm (120.0);

        settings.rate = 0.125f;
        settings.harmony[0].degree = 2;   // E
        settings.harmony[1].degree = 4;   // G
        settings.harmony[2].degree = 6;   // B

        planner.setSettings (settings);
        planner.setClock (clock);
    }
};

std::array<bool, kNumVoices> allHit()
{
    return { true, true, true, true };
}
} // namespace

TEST_CASE ("Ohne gehaltene Note entstehen keine Gates", "[planner][T9]")
{
    Fixture f;
    f.planner.clearNote();

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE (plan.valid);
    REQUIRE (plan.hits == 0);
    REQUIRE (plan.voices[0].selected == false);
}

TEST_CASE ("Play triggert alle vier Stimmen", "[planner]")
{
    Fixture f;
    f.planner.setNote (60, 100);

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE (plan.valid);
    REQUIRE (plan.hits == 1);
    REQUIRE (plan.mode == StepMode::Play);
    REQUIRE (plan.startSeconds == Approx (0.0));

    for (int voice = 0; voice < kNumVoices; ++voice)
        REQUIRE (plan.voices[static_cast<std::size_t> (voice)].selected);

    // Thru spielt die Rohnote, H1..H3 die diatonischen Grade.
    REQUIRE (plan.voices[0].pitch == 60);
    REQUIRE (plan.voices[1].pitch == 64);   // Grad 2 = E
    REQUIRE (plan.voices[2].pitch == 67);   // Grad 4 = G
    REQUIRE (plan.voices[3].pitch == 71);   // Grad 6 = B
}

TEST_CASE ("Stimmmaske blendet einzelne Stimmen aus", "[planner]")
{
    Fixture f;
    f.planner.setNote (60, 100);
    f.settings.steps[0].voiceMask = kMaskThru | kMaskH2;
    f.planner.setSettings (f.settings);

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE (plan.voices[0].selected);
    REQUIRE_FALSE (plan.voices[1].selected);
    REQUIRE (plan.voices[2].selected);
    REQUIRE_FALSE (plan.voices[3].selected);
}

TEST_CASE ("Deaktivierte Stimme wird nicht ausgewaehlt", "[planner]")
{
    Fixture f;
    f.planner.setNote (60, 100);
    f.settings.harmony[1].enable = false;
    f.planner.setSettings (f.settings);

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE_FALSE (plan.voices[2].selected);
}

TEST_CASE ("Step-Offset verschiebt nur die Harmonien", "[planner]")
{
    Fixture f;
    f.planner.setNote (60, 100);
    f.settings.steps[0].offset = 2;
    f.planner.setSettings (f.settings);

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE (plan.voices[0].pitch == 60);   // Thru unveraendert
    REQUIRE (plan.voices[1].pitch == 67);   // Grad 2 + 2 = Grad 4 = G
    REQUIRE (plan.voices[2].pitch == 71);
    REQUIRE (plan.voices[3].pitch == 74);   // Grad 6 + 2 = Grad 8 = D
}

TEST_CASE ("Oktave wirkt chromatisch nach dem Grad", "[planner]")
{
    Fixture f;
    f.planner.setNote (60, 100);
    f.settings.harmony[0].octave = 1;
    f.planner.setSettings (f.settings);

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE (plan.voices[1].pitch == 64 + 12);
}

TEST_CASE ("Thru mitrasten rastet nur die Thru-Stimme", "[planner][A1]")
{
    Fixture f;
    f.planner.setNote (63, 100);          // Es

    const auto planRoh = f.planner.planSlot (0, 0);
    REQUIRE (planRoh.voices[0].pitch == 63);   // Thru spielt die Rohnote
    REQUIRE (planRoh.voices[1].pitch == 65);   // Basisgrad D + Grad 2 = F

    f.settings.thru.snap = true;
    f.planner.setSettings (f.settings);

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE (plan.voices[0].pitch == 62);   // Es -> D (Gleichstand, tiefer)
    REQUIRE (plan.voices[1].pitch == 65);   // Harmonien unveraendert
}

TEST_CASE ("Velocity folgt der Eingabe und dem Spread", "[planner][T12]")
{
    Fixture f;
    f.planner.setNote (60, 100);

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE (plan.voices[0].velocity == 100);
    REQUIRE (plan.voices[1].velocity == 100);   // spread_i = 1
    REQUIRE (plan.voices[2].velocity == 85);    // 1 - 0.3/2
    REQUIRE (plan.voices[3].velocity == 70);    // 1 - 0.3
}

TEST_CASE ("Velocity wird geklemmt", "[planner][T12]")
{
    Fixture f;
    f.planner.setNote (60, 127);
    f.settings.thru.velocity = 200.0f;   // 127 * 2 = 254 -> 127
    f.planner.setSettings (f.settings);

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE (plan.voices[0].velocity == 127);

    Fixture g;
    g.planner.setNote (60, 10);
    g.settings.thru.velocity = 0.0f;     // 0 -> auf 1 geklemmt
    g.planner.setSettings (g.settings);

    const auto plan2 = g.planner.planSlot (0, 0);
    REQUIRE (plan2.voices[0].velocity == 1);
}

TEST_CASE ("follow ignoriert die Voice-Velocity", "[planner][T12][A2]")
{
    Fixture f;
    f.planner.setNote (60, 100);
    f.settings.harmony[0].follow = true;
    f.settings.harmony[0].velocity = 50.0f;
    f.planner.setSettings (f.settings);

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE (plan.voices[1].velocity == 100);

    f.settings.harmony[0].follow = false;
    f.planner.setSettings (f.settings);

    const auto plan2 = f.planner.planSlot (0, 0);
    REQUIRE (plan2.voices[1].velocity == 50);
}

TEST_CASE ("harm.velocity wirkt als Gruppenfaktor auf H1...H3", "[planner][T12]")
{
    Fixture f;
    f.planner.setNote (60, 100);
    f.settings.harmVelocity = 50.0f;
    f.settings.harmSpread = 0.2f;      // 100 % / 90 % / 80 %
    f.planner.setSettings (f.settings);

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE (plan.voices[0].velocity == 100);   // Thru unberuehrt
    REQUIRE (plan.voices[1].velocity == 50);
    REQUIRE (plan.voices[2].velocity == 45);
    REQUIRE (plan.voices[3].velocity == 40);
}

TEST_CASE ("Step-Akzent wirkt auf alle Stimmen", "[planner][T12]")
{
    Fixture f;
    f.planner.setNote (60, 80);
    f.settings.steps[0].velocity = 150.0f;
    f.planner.setSettings (f.settings);

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE (plan.voices[0].velocity == 120);   // 80 * 150 %
    REQUIRE (plan.voices[1].velocity == 120);
    REQUIRE (plan.voices[3].velocity == 84);    // 80 * 70 % * 150 %
}

TEST_CASE ("Gate endet nie nach der Step-Grenze", "[planner][T13]")
{
    Fixture f;
    f.planner.setNote (60, 100);
    f.settings.harmony[0].gate = 50.0f;
    f.planner.setSettings (f.settings);

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE (plan.voices[1].gateSeconds == Approx (0.125));   // 50 % von 0.25 s
    REQUIRE (plan.voices[0].gateSeconds == Approx (0.25));    // Thru: 100 %

    f.settings.harmony[0].gate = 500.0f;                      // wird geklemmt
    f.planner.setSettings (f.settings);
    const auto plan2 = f.planner.planSlot (0, 0);
    REQUIRE (plan2.voices[1].gateSeconds == Approx (0.25));
}

TEST_CASE ("Ratchet erzeugt mehrere Treffer im Slot", "[planner][T8]")
{
    Fixture f;
    f.planner.setNote (60, 100);
    f.settings.steps[0].mode = StepMode::Ratchet;
    f.settings.steps[0].ratchet = 4;
    f.planner.setSettings (f.settings);

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE (plan.hits == 4);
    REQUIRE (plan.hitSpacingSeconds == Approx (0.0625));

    // Das Gate darf nicht in den naechsten Treffer laufen.
    REQUIRE (plan.voices[1].gateSeconds <= plan.hitSpacingSeconds);
}

TEST_CASE ("Rest erzeugt keinen Trigger", "[planner][T9]")
{
    Fixture f;
    f.planner.setNote (60, 100);
    f.settings.steps[0].mode = StepMode::Rest;
    f.planner.setSettings (f.settings);

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE (plan.valid);
    REQUIRE (plan.hits == 0);
}

TEST_CASE ("Wahrscheinlichkeit: 0 % nie, 100 % immer", "[planner][T11]")
{
    REQUIRE_FALSE (rollProbability (0.0f, 0.0));
    REQUIRE_FALSE (rollProbability (0.0f, 0.99));
    REQUIRE (rollProbability (100.0f, 0.0));
    REQUIRE (rollProbability (100.0f, 0.99));
    REQUIRE (rollProbability (50.0f, 0.49));
    REQUIRE_FALSE (rollProbability (50.0f, 0.50));
}

TEST_CASE ("Play schlaegt an, Rest laesst alles unveraendert", "[planner][T11]")
{
    Fixture f;
    f.planner.setNote (60, 100);

    std::array<VoiceState, kNumVoices> state {};

    const auto plan = f.planner.planSlot (0, 0);
    const auto actions = applyStep (plan, allHit(), state);

    REQUIRE (actions.noteOn[0]);
    REQUIRE (actions.noteOn[3]);
    REQUIRE_FALSE (actions.noteOff[0]);
    REQUIRE (state[0].sounding);
    REQUIRE (state[0].held == false);

    // Ein Rest-Schritt beendet nichts.
    f.settings.steps[1].mode = StepMode::Rest;
    f.planner.setSettings (f.settings);

    const auto restPlan = f.planner.planSlot (1, 1);
    const auto restActions = applyStep (restPlan, allHit(), state);
    REQUIRE_FALSE (restActions.noteOff[0]);
    REQUIRE_FALSE (restActions.noteOn[0]);
    REQUIRE (state[0].sounding);
}

TEST_CASE ("Hold haelt die Note ueber Step-Grenzen", "[planner]")
{
    Fixture f;
    f.planner.setNote (60, 100);
    f.settings.steps[0].mode = StepMode::Hold;
    f.settings.steps[1].mode = StepMode::Hold;
    f.planner.setSettings (f.settings);

    std::array<VoiceState, kNumVoices> state {};

    const auto plan = f.planner.planSlot (0, 0);
    const auto actions = applyStep (plan, allHit(), state);
    REQUIRE (actions.noteOn[0]);
    REQUIRE (state[0].held);

    // Der naechste Hold-Step schlaegt nicht erneut an.
    const auto plan2 = f.planner.planSlot (1, 1);
    const auto actions2 = applyStep (plan2, allHit(), state);
    REQUIRE_FALSE (actions2.noteOn[0]);
    REQUIRE_FALSE (actions2.noteOff[0]);
    REQUIRE (state[0].sounding);
}

TEST_CASE ("Ein Play-Step schlaegt die gehaltene Note neu an", "[planner]")
{
    Fixture f;
    f.planner.setNote (60, 100);
    f.settings.steps[0].mode = StepMode::Hold;
    f.settings.steps[1].mode = StepMode::Play;
    f.planner.setSettings (f.settings);

    std::array<VoiceState, kNumVoices> state {};

    const auto plan = f.planner.planSlot (0, 0);
    applyStep (plan, allHit(), state);
    REQUIRE (state[0].sounding);

    const auto plan2 = f.planner.planSlot (1, 1);
    const auto actions = applyStep (plan2, allHit(), state);
    REQUIRE (actions.noteOff[0]);
    REQUIRE (actions.noteOn[0]);
    REQUIRE_FALSE (state[0].held);
}

TEST_CASE ("Hold endet, wenn die Stimme nicht mehr maskiert ist", "[planner]")
{
    Fixture f;
    f.planner.setNote (60, 100);
    f.settings.steps[0].mode = StepMode::Hold;
    f.settings.steps[1].mode = StepMode::Hold;
    f.settings.steps[1].voiceMask = kMaskThru;   // H1..H3 fallen weg
    f.planner.setSettings (f.settings);

    std::array<VoiceState, kNumVoices> state {};

    const auto plan = f.planner.planSlot (0, 0);
    applyStep (plan, allHit(), state);
    REQUIRE (state[1].sounding);

    const auto plan2 = f.planner.planSlot (1, 1);
    const auto actions = applyStep (plan2, allHit(), state);
    REQUIRE (actions.noteOff[1]);
    REQUIRE_FALSE (state[1].sounding);
}

TEST_CASE ("Pitchwechsel ist Note-Off und Note-On", "[planner]")
{
    Fixture f;
    f.planner.setNote (60, 100);

    std::array<VoiceState, kNumVoices> state {};

    const auto plan = f.planner.planSlot (0, 0);
    applyStep (plan, allHit(), state);
    REQUIRE (state[1].sounding);

    // Anderer Grad -> neue Note.
    f.settings.harmony[0].degree = 3;
    f.planner.setSettings (f.settings);

    const auto plan2 = f.planner.planSlot (1, 1);
    const auto actions = applyStep (plan2, allHit(), state);
    REQUIRE (actions.noteOff[1]);
    REQUIRE (actions.noteOn[1]);
}

TEST_CASE ("Verfehlter Wurf laesst die Stimme in Ruhe", "[planner][T11]")
{
    Fixture f;
    f.planner.setNote (60, 100);

    std::array<VoiceState, kNumVoices> state {};

    const auto plan = f.planner.planSlot (0, 0);
    const std::array<bool, kNumVoices> hit { true, false, true, true };
    const auto actions = applyStep (plan, hit, state);

    REQUIRE_FALSE (actions.noteOn[1]);
    REQUIRE (actions.noteOn[2]);
    REQUIRE_FALSE (state[1].sounding);
}

TEST_CASE ("Single-Modus legt alle Stimmen auf einen Kanal", "[planner][T14]")
{
    Fixture f;
    f.planner.setNote (60, 100);
    f.settings.outputMode = OutputMode::Single;
    f.settings.thru.channel = 3;
    f.planner.setSettings (f.settings);

    const auto plan = f.planner.planSlot (0, 0);
    for (int voice = 0; voice < kNumVoices; ++voice)
        REQUIRE (plan.voices[static_cast<std::size_t> (voice)].channel == 3);
}

TEST_CASE ("Multi-Modus nutzt vier verschiedene Kanaele", "[planner][T14]")
{
    Fixture f;
    f.planner.setNote (60, 100);
    f.settings.thru.channel = 1;
    f.settings.harmony[0].channel = 2;
    f.settings.harmony[1].channel = 3;
    f.settings.harmony[2].channel = 4;
    f.planner.setSettings (f.settings);

    const auto plan = f.planner.planSlot (0, 0);
    REQUIRE (plan.voices[0].channel == 1);
    REQUIRE (plan.voices[1].channel == 2);
    REQUIRE (plan.voices[2].channel == 3);
    REQUIRE (plan.voices[3].channel == 4);
}

TEST_CASE ("Sequenz und Planung passen zusammen", "[planner][integration]")
{
    Fixture f;
    f.planner.setNote (60, 100);
    f.settings.steps[2].active = false;
    f.settings.steps[5].mode = StepMode::Rest;
    f.planner.setSettings (f.settings);

    StepSequencer sequencer;
    sequencer.setSteps (f.settings.steps);
    sequencer.setDirection (Direction::Forward);
    sequencer.reset();

    std::int64_t slot = 0;
    for (int i = 0; i < 8; ++i)
    {
        int stepIndex = -1;
        REQUIRE (sequencer.nextStep (stepIndex));
        REQUIRE (stepIndex != 2);          // inaktiver Step wird nie besucht

        const auto plan = f.planner.planSlot (stepIndex, slot);
        REQUIRE (plan.valid);

        if (stepIndex == 5)
            REQUIRE (plan.hits == 0);       // Rest
        else
            REQUIRE (plan.hits == 1);

        ++slot;
    }
}
