#include "core/StepEngine.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <vector>

using namespace heptachord;

namespace
{
struct AbsEvent
{
    long long sample = 0;
    TimedEvent event {};
};

// Fuehrt die Engine blockweise aus und merkt sich die absoluten Positionen.
struct Runner
{
    StepEngine engine {};
    EventList out {};
    std::vector<TimedEvent> input {};

    long long position = 0;
    double sampleRate = 48000.0;
    double bpm = 120.0;
    bool playing = true;
    bool positionValid = true;

    double ppq() const
    {
        return static_cast<double> (position) * bpm / (60.0 * sampleRate);
    }

    std::vector<AbsEvent> run (int numSamples)
    {
        HostTimeline timeline;
        timeline.sampleRate = sampleRate;
        timeline.numSamples = numSamples;
        timeline.bpm = bpm;
        timeline.isPlaying = playing;
        timeline.positionValid = positionValid;
        timeline.ppqPosition = ppq();

        engine.processBlock (timeline, input, out);
        input.clear();

        std::vector<AbsEvent> result;
        result.reserve (static_cast<std::size_t> (out.size()));

        for (int i = 0; i < out.size(); ++i)
            result.push_back ({ position + out[i].offsetSamples, out[i] });

        position += numSamples;
        return result;
    }

    std::vector<AbsEvent> runFor (long long total, int blockSize)
    {
        std::vector<AbsEvent> result;

        for (long long done = 0; done < total; done += blockSize)
        {
            const auto remaining = total - done;
            const auto size = static_cast<int> (std::min<long long> (blockSize, remaining));
            const auto block = run (size);
            result.insert (result.end(), block.begin(), block.end());
        }

        return result;
    }
};

EngineSettings makeSettings()
{
    EngineSettings settings;

    settings.rate = 0.125f;      // 12000 Samples bei 48 kHz / 120 BPM
    settings.root = 0;
    settings.scale = Scale::Dur;

    settings.thru.channel = 1;
    settings.harmony[0].channel = 2;
    settings.harmony[1].channel = 3;
    settings.harmony[2].channel = 4;

    settings.harmony[0].degree = 2;   // E
    settings.harmony[1].degree = 4;   // G
    settings.harmony[2].degree = 6;   // B

    settings.harmony[0].gate = 100.0f;
    settings.harmony[1].gate = 100.0f;
    settings.harmony[2].gate = 100.0f;

    return settings;
}

EngineSettings withStepOffsets (EngineSettings settings)
{
    for (int i = 0; i < kNumSteps; ++i)
        settings.steps[static_cast<std::size_t> (i)].offset = i;

    return settings;
}

std::vector<AbsEvent> filter (const std::vector<AbsEvent>& events, EventType type, int channel = -1)
{
    std::vector<AbsEvent> result;

    std::copy_if (events.begin(), events.end(), std::back_inserter (result),
                  [type, channel] (const AbsEvent& e)
                  {
                      return e.event.type == type && (channel < 0 || e.event.channel == channel);
                  });

    return result;
}

std::vector<long long> samplesOf (const std::vector<AbsEvent>& events)
{
    std::vector<long long> result;
    result.reserve (events.size());

    for (const auto& event : events)
        result.push_back (event.sample);

    return result;
}

std::vector<int> pitchesOf (const std::vector<AbsEvent>& events)
{
    std::vector<int> result;
    result.reserve (events.size());

    for (const auto& event : events)
        result.push_back (event.event.pitch);

    return result;
}
} // namespace

TEST_CASE ("Transport-Start triggert alle vier Stimmen an der ersten Kante", "[engine]")
{
    Runner runner;
    runner.engine.setSettings (makeSettings());
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    const auto events = runner.run (512);
    const auto ons = filter (events, EventType::NoteOn);

    REQUIRE (ons.size() == 4);
    REQUIRE (samplesOf (ons) == std::vector<long long> { 0, 0, 0, 0 });
    REQUIRE (pitchesOf (ons) == std::vector<int> { 60, 64, 67, 71 });

    // Kanaele in Stimmenreihenfolge Thru, H1, H2, H3.
    REQUIRE (ons[0].event.channel == 1);
    REQUIRE (ons[1].event.channel == 2);
    REQUIRE (ons[2].event.channel == 3);
    REQUIRE (ons[3].event.channel == 4);

    // Velocity: Thru 100 %, H1 100 %, H2 85 %, H3 70 % (Spread 0.3).
    REQUIRE (ons[0].event.velocity == 100);
    REQUIRE (ons[1].event.velocity == 100);
    REQUIRE (ons[2].event.velocity == 85);
    REQUIRE (ons[3].event.velocity == 70);
}

TEST_CASE ("Ohne gehaltene Note entstehen keine Tore", "[engine]")
{
    Runner runner;
    runner.engine.setSettings (makeSettings());

    const auto events = runner.runFor (40000, 512);
    REQUIRE (events.empty());
}

TEST_CASE ("Step-Kanten liegen auf Sample-Grenzen", "[engine][T17]")
{
    Runner runner;
    runner.engine.setSettings (makeSettings());
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    const auto events = runner.runFor (30000, 1000);
    const auto ons = filter (events, EventType::NoteOn, 2);   // H1

    REQUIRE (samplesOf (ons) == std::vector<long long> { 0, 12000, 24000 });
}

TEST_CASE ("Blockzerlegung veraendert die Ereignisse nicht", "[engine][T17]")
{
    const auto build = [] (int blockSize)
    {
        Runner runner;
        runner.engine.setSettings (makeSettings());
        runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });
        return runner.runFor (30000, blockSize);
    };

    const auto coarse = build (3000);
    const auto fine = build (97);
    const auto single = build (1);

    REQUIRE (coarse.size() == fine.size());
    REQUIRE (coarse.size() == single.size());

    for (std::size_t i = 0; i < coarse.size(); ++i)
    {
        REQUIRE (coarse[i].sample == fine[i].sample);
        REQUIRE (coarse[i].sample == single[i].sample);
        REQUIRE (coarse[i].event.type == fine[i].event.type);
        REQUIRE (coarse[i].event.pitch == fine[i].event.pitch);
        REQUIRE (coarse[i].event.velocity == fine[i].event.velocity);
        REQUIRE (coarse[i].event.channel == fine[i].event.channel);
    }
}

TEST_CASE ("Swing verschiebt die ungeraden Kanten", "[engine][T6]")
{
    Runner runner;
    auto settings = makeSettings();
    settings.swing = 25.0f;      // 25 % von 12000 = 3000 Samples
    runner.engine.setSettings (settings);
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    const auto events = runner.runFor (40000, 512);
    const auto ons = filter (events, EventType::NoteOn, 2);

    REQUIRE (samplesOf (ons) == std::vector<long long> { 0, 15000, 24000, 39000 });
}

TEST_CASE ("Gate-Ende steht vor dem naechsten Note-On", "[engine][T13][T15]")
{
    Runner runner;
    runner.engine.setSettings (makeSettings());
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    const auto events = runner.runFor (13000, 13000);

    // Kante bei 12000: erst vier Note-Off, dann vier Note-On.
    std::vector<std::pair<long long, EventType>> first;
    for (const auto& event : events)
        if (event.sample == 12000)
            first.emplace_back (event.sample, event.event.type);

    REQUIRE (first.size() == 8);

    for (std::size_t i = 0; i < 4; ++i)
        REQUIRE (first[i].second == EventType::NoteOff);

    for (std::size_t i = 4; i < 8; ++i)
        REQUIRE (first[i].second == EventType::NoteOn);
}

TEST_CASE ("Gate-Laenge bestimmt das Note-Off", "[engine][T13]")
{
    Runner runner;
    auto settings = makeSettings();
    settings.harmony[0].gate = 50.0f;    // 6000 Samples
    runner.engine.setSettings (settings);
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    const auto events = runner.runFor (12000, 512);

    const auto h1Offs = filter (events, EventType::NoteOff, 2);
    REQUIRE (samplesOf (h1Offs) == std::vector<long long> { 6000 });

    // H2 und H3 haben 100 % Gate und enden erst an der Step-Grenze.
    const auto h2Offs = filter (events, EventType::NoteOff, 3);
    REQUIRE (samplesOf (h2Offs) == std::vector<long long> { });
}

TEST_CASE ("Pitchwechsel ist Note-Off und Note-On, kein Pitchbend", "[engine][T15]")
{
    Runner runner;
    auto settings = makeSettings();
    runner.engine.setSettings (settings);
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    std::vector<AbsEvent> events = runner.runFor (12000, 512);

    settings.harmony[0].degree = 3;      // E -> F
    runner.engine.setSettings (settings);

    const auto second = runner.runFor (13000, 512);
    events.insert (events.end(), second.begin(), second.end());

    // An der Kante 12000: kein Pitchbend-Ereignis, sondern Off + On.
    int offIndex = -1;
    int onIndex = -1;

    for (int i = 0; i < static_cast<int> (events.size()); ++i)
    {
        const auto& event = events[static_cast<std::size_t> (i)];

        if (event.sample != 12000 || event.event.channel != 2)
            continue;

        if (event.event.type == EventType::NoteOff)
        {
            REQUIRE (event.event.pitch == 64);
            offIndex = i;
        }
        else if (event.event.type == EventType::NoteOn)
        {
            REQUIRE (event.event.pitch == 65);
            onIndex = i;
        }
    }

    REQUIRE (offIndex >= 0);
    REQUIRE (onIndex > offIndex);

    for (const auto& event : events)
        REQUIRE (event.event.type != EventType::AllNotesOff);
}

TEST_CASE ("Ratchet verteilt die Treffer im Slot", "[engine][T8]")
{
    Runner runner;
    auto settings = makeSettings();
    for (auto& step : settings.steps)
    {
        step.mode = StepMode::Ratchet;
        step.ratchet = 4;
    }
    runner.engine.setSettings (settings);
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    const auto events = runner.runFor (20000, 512);
    const auto ons = filter (events, EventType::NoteOn, 2);

    REQUIRE (samplesOf (ons) == std::vector<long long> { 0, 3000, 6000, 9000, 12000, 15000, 18000 });
}

TEST_CASE ("Ratchet-Gate ueberlappt nicht", "[engine][T8][T13]")
{
    Runner runner;
    auto settings = makeSettings();
    settings.steps[0].mode = StepMode::Ratchet;
    settings.steps[0].ratchet = 4;
    runner.engine.setSettings (settings);
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    const auto events = runner.runFor (12000, 512);
    const auto h1Offs = filter (events, EventType::NoteOff, 2);

    // Gate 100 % wird auf den Trefferabstand (3000) geklemmt.
    REQUIRE (samplesOf (h1Offs) == std::vector<long long> { 3000, 6000, 9000 });

    // Jeder Treffer hat sein eigenes Paar (Off vor On am gleichen Sample).
    std::vector<EventType> atSix;
    for (const auto& event : events)
        if (event.sample == 6000 && event.event.channel == 2)
            atSix.push_back (event.event.type);

    REQUIRE (atSix.size() == 2);
    REQUIRE (atSix[0] == EventType::NoteOff);
    REQUIRE (atSix[1] == EventType::NoteOn);
}

TEST_CASE ("Hold haelt ueber Step-Grenzen", "[engine]")
{
    Runner runner;
    auto settings = makeSettings();
    settings.steps[0].mode = StepMode::Hold;
    settings.steps[1].mode = StepMode::Hold;
    settings.steps[2].mode = StepMode::Play;
    runner.engine.setSettings (settings);
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    const auto events = runner.runFor (25000, 512);

    // An der Grenze 12000 (zweiter Hold-Step) passiert nichts.
    for (const auto& event : events)
        REQUIRE (event.sample != 12000);

    // An 24000 (Play) endet die Kette mit Off + On.
    const auto h1Ons = filter (events, EventType::NoteOn, 2);
    const auto h1Offs = filter (events, EventType::NoteOff, 2);

    REQUIRE (samplesOf (h1Ons) == std::vector<long long> { 0, 24000 });
    REQUIRE (samplesOf (h1Offs) == std::vector<long long> { 24000 });
}

TEST_CASE ("Transport-Stop beendet alle Noten", "[engine]")
{
    Runner runner;
    runner.engine.setSettings (makeSettings());
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    const auto playing = runner.run (512);
    REQUIRE (filter (playing, EventType::NoteOn).size() == 4);

    runner.playing = false;
    const auto stopped = runner.run (512);

    const auto allOff = filter (stopped, EventType::AllNotesOff);
    REQUIRE (allOff.size() == 4);
    REQUIRE (samplesOf (allOff) == std::vector<long long> { 512, 512, 512, 512 });

    // Neustart bei Taktanfang: die Note kommt wieder.
    runner.position = 0;
    runner.playing = true;
    const auto restarted = runner.run (512);
    REQUIRE (filter (restarted, EventType::NoteOn).size() == 4);
}

TEST_CASE ("Locate beendet Noten und setzt die Traversal-Folge zurueck", "[engine]")
{
    Runner runner;
    runner.engine.setSettings (withStepOffsets (makeSettings()));
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    runner.runFor (1536, 512);

    // Sprung auf Taktposition 2.0 Viertelnoten = Slot 4.
    runner.position = 48000;

    const auto events = runner.run (512);

    REQUIRE (filter (events, EventType::AllNotesOff).size() == 4);

    const auto ons = filter (events, EventType::NoteOn, 2);
    REQUIRE (ons.size() == 1);
    REQUIRE (ons[0].sample == 48000);
    REQUIRE (ons[0].event.pitch == 64);   // Step 0, nicht Step 4 (71)
}

TEST_CASE ("Wahrscheinlichkeit 0 schlaegt nie an, 100 immer", "[engine][T11]")
{
    Runner runner;
    auto settings = makeSettings();
    for (auto& step : settings.steps)
        step.probability = 0.0f;
    runner.engine.setSettings (settings);
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    REQUIRE (filter (runner.runFor (25000, 512), EventType::NoteOn).empty());

    for (auto& step : settings.steps)
        step.probability = 100.0f;
    runner.engine.setSettings (settings);

    REQUIRE_FALSE (filter (runner.runFor (13000, 512), EventType::NoteOn).empty());
}

TEST_CASE ("Inaktive Steps fallen aus der Traversal-Folge", "[engine][T9]")
{
    Runner runner;
    auto settings = withStepOffsets (makeSettings());
    settings.steps[2].active = false;
    settings.steps[3].active = false;
    runner.engine.setSettings (settings);
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    const auto events = runner.runFor (72000, 512);
    const auto ons = filter (events, EventType::NoteOn, 2);

    REQUIRE (samplesOf (ons) == std::vector<long long> { 0, 12000, 24000, 36000, 48000, 60000 });
    REQUIRE (pitchesOf (ons) == std::vector<int> { 64, 65, 71, 72, 74, 76 });
}

TEST_CASE ("reset beginnt beim ersten aktiven Step", "[engine]")
{
    Runner runner;
    runner.engine.setSettings (withStepOffsets (makeSettings()));
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    const auto first = runner.runFor (12000, 512);
    REQUIRE (pitchesOf (filter (first, EventType::NoteOn, 2)) == std::vector<int> { 64 });

    runner.engine.requestReset();

    const auto second = runner.runFor (12000, 512);
    const auto ons = filter (second, EventType::NoteOn, 2);

    REQUIRE (ons.size() == 1);
    REQUIRE (ons[0].event.pitch == 64);   // wieder Step 0 statt Step 1 (65)
}

TEST_CASE ("Losgelassene Note stoppt die Trigger", "[engine]")
{
    Runner runner;
    runner.engine.setSettings (makeSettings());
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    runner.runFor (512, 512);

    runner.input.push_back ({ 0, EventType::NoteOff, 60, 0, 1 });
    const auto events = runner.runFor (24000, 512);

    REQUIRE (filter (events, EventType::NoteOn).empty());

    // Das laufende Gate endet regulaer an der Kante.
    REQUIRE (filter (events, EventType::NoteOff).size() == 4);
}

TEST_CASE ("Latch haelt die Noten nach dem Loslassen", "[engine]")
{
    Runner runner;
    auto settings = makeSettings();
    settings.latch = true;
    runner.engine.setSettings (settings);

    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });
    runner.runFor (512, 512);

    runner.input.push_back ({ 0, EventType::NoteOff, 60, 0, 1 });
    const auto events = runner.runFor (24000, 512);

    const auto ons = filter (events, EventType::NoteOn, 2);
    REQUIRE (samplesOf (ons) == std::vector<long long> { 12000, 24000 });
}

TEST_CASE ("All-Notes-Off im Single-Modus geht auf einen Kanal", "[engine][T14]")
{
    Runner runner;
    auto settings = makeSettings();
    settings.outputMode = OutputMode::Single;
    settings.thru.channel = 5;
    runner.engine.setSettings (settings);
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    runner.run (512);
    runner.playing = false;
    const auto stopped = runner.run (512);

    const auto allOff = filter (stopped, EventType::AllNotesOff);
    REQUIRE (allOff.size() == 1);
    REQUIRE (allOff[0].event.channel == 5);
}

TEST_CASE ("Ohne Host-Position laeuft die Uhr intern weiter", "[engine]")
{
    Runner runner;
    runner.engine.setSettings (makeSettings());
    runner.input.push_back ({ 0, EventType::NoteOn, 60, 100, 1 });

    runner.positionValid = false;
    const auto events = runner.runFor (30000, 1000);
    const auto ons = filter (events, EventType::NoteOn, 2);

    REQUIRE (samplesOf (ons) == std::vector<long long> { 0, 12000, 24000 });
}
