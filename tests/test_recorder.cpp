// Tests fuer die slotbasierte Zykluszaehlung der Aufnahme (ADR 0010).
// Die Aufnahme zaehlt vergangene Step-Slots, nicht die Rueckkehr zu einem
// bestimmten Step - damit ist sie fuer alle Richtungen gleich definiert.
#include "core/MidiRender.h"

#include <catch2/catch_test_macros.hpp>

using namespace heptachord;

TEST_CASE ("Zyklusposition ist 1-basiert und richtungsunabhaengig", "[recorder]")
{
    const auto begin = midirender::cyclePosition (0, 8);
    REQUIRE (begin.cycle == 1);
    REQUIRE (begin.step == 1);

    const auto mid = midirender::cyclePosition (11, 8);
    REQUIRE (mid.cycle == 2);
    REQUIRE (mid.step == 4);

    const auto boundary = midirender::cyclePosition (8, 8);
    REQUIRE (boundary.cycle == 2);
    REQUIRE (boundary.step == 1);
}

TEST_CASE ("Aufnahme-Ziel wird an Zyklusgrenzen erreicht", "[recorder]")
{
    REQUIRE_FALSE (midirender::recordingTargetReached (7, 8, 1));
    REQUIRE (midirender::recordingTargetReached (8, 8, 1));

    REQUIRE_FALSE (midirender::recordingTargetReached (15, 8, 2));
    REQUIRE (midirender::recordingTargetReached (16, 8, 2));

    // 0 = unbegrenzt: kein Auto-Stop.
    REQUIRE_FALSE (midirender::recordingTargetReached (16, 8, 0));
    REQUIRE_FALSE (midirender::recordingTargetReached (16, 8, 4));
}

TEST_CASE ("Unbekannte Zykluslaenge liefert keine Position", "[recorder]")
{
    const auto none = midirender::cyclePosition (3, 0);
    REQUIRE (none.cycle == 0);
    REQUIRE (none.step == 0);

    REQUIRE_FALSE (midirender::recordingTargetReached (8, 0, 1));
}
