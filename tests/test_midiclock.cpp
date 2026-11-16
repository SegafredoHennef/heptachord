#include "core/MidiClock.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace heptachord;
using Catch::Approx;

namespace
{
// Simuliert einen Master: FA, dann F8 alle quarter/24 Sekunden.
// Rueckgabe: Zeitstempel des letzten F8.
double feedAt (MidiClock& clock, double bpm, int quarters)
{
    double now = 100.0;
    clock.start (now);

    const double interval = (60.0 / bpm) / 24.0;
    for (int q = 0; q < quarters * 24; ++q)
    {
        now += interval;
        clock.tick (now);
    }

    return now;
}
} // namespace

TEST_CASE ("MIDI-Clock: 24 Clocks pro Viertel ergeben das BPM", "[clock]")
{
    MidiClock clock;
    const double end = feedAt (clock, 120.0, 2);

    const auto snap = clock.snapshot (end + 0.001);
    REQUIRE (snap.active);
    REQUIRE (snap.playing);
    REQUIRE (snap.bpm == Approx (120.0).margin (0.5));
    REQUIRE (snap.ppq == Approx (2.0).margin (0.001));
}

TEST_CASE ("MIDI-Clock: schnelleres Tempo wird erkannt", "[clock]")
{
    MidiClock clock;
    const double end = feedAt (clock, 240.0, 2);
    REQUIRE (clock.snapshot (end + 0.001).bpm == Approx (240.0).margin (1.0));
}

TEST_CASE ("MIDI-Clock: Temposchranken klemmen", "[clock]")
{
    MidiClock slow;
    const double endSlow = feedAt (slow, 10.0, 2);
    REQUIRE (slow.snapshot (endSlow + 0.001).bpm == Approx (30.0).margin (0.001));

    MidiClock fast;
    const double endFast = feedAt (fast, 600.0, 2);
    REQUIRE (fast.snapshot (endFast + 0.001).bpm == Approx (300.0).margin (0.001));
}

TEST_CASE ("MIDI-Clock: FA startet, FC stoppt, FB setzt ohne Positionsverlust fort", "[clock]")
{
    MidiClock clock;
    double now = 0.0;

    clock.start (now);
    for (int i = 0; i < 24; ++i)
        clock.tick (now += 0.5 / 24.0);

    REQUIRE (clock.snapshot (now).ppq == Approx (1.0).margin (0.001));

    clock.stop (now += 1.0);
    REQUIRE_FALSE (clock.snapshot (now).playing);

    // Clocks waehrend des Stopps zaehlen nicht zur Position.
    for (int i = 0; i < 24; ++i)
        clock.tick (now += 0.5 / 24.0);

    REQUIRE (clock.snapshot (now).ppq == Approx (1.0).margin (0.001));

    clock.continueClock (now += 0.1);
    REQUIRE (clock.snapshot (now).playing);

    for (int i = 0; i < 24; ++i)
        clock.tick (now += 0.5 / 24.0);

    REQUIRE (clock.snapshot (now).ppq == Approx (2.0).margin (0.001));
}

TEST_CASE ("MIDI-Clock: F8 ohne FA laesst den Transport stehen", "[clock]")
{
    MidiClock clock;
    double now = 0.0;

    for (int i = 0; i < 48; ++i)
        clock.tick (now += 0.5 / 24.0);

    const auto snap = clock.snapshot (now);
    REQUIRE (snap.active);
    REQUIRE_FALSE (snap.playing);
    REQUIRE (snap.ppq == Approx (0.0).margin (0.001));
}

TEST_CASE ("MIDI-Clock: Timeout ohne F8 deaktiviert die Quelle", "[clock]")
{
    MidiClock clock;
    clock.start (100.0);
    clock.tick (100.1);

    REQUIRE (clock.snapshot (101.9).active);
    REQUIRE_FALSE (clock.snapshot (102.2).active);
}

TEST_CASE ("MIDI-Clock: ohne Ereignisse bleibt der Default", "[clock]")
{
    MidiClock clock;
    const auto snap = clock.snapshot (5.0);
    REQUIRE_FALSE (snap.active);
    REQUIRE_FALSE (snap.playing);
    REQUIRE (snap.bpm == Approx (120.0).margin (0.001));
    REQUIRE (snap.ppq == Approx (0.0).margin (0.001));
}

TEST_CASE ("MIDI-Clock: reset loescht alles", "[clock]")
{
    MidiClock clock;
    feedAt (clock, 120.0, 2);
    clock.reset();

    const auto snap = clock.snapshot (1000.0);
    REQUIRE_FALSE (snap.active);
    REQUIRE (snap.bpm == Approx (120.0).margin (0.001));
}
