// Tests fuer den Bezeichnungsmodus (Decision 83): Anzeige-Labels fuer
// Grundtoene und Skalen in drei Konventionen, mit Klemmung.
#include "core/NoteNames.h"

#include <catch2/catch_test_macros.hpp>

#include <string>

using namespace heptachord;
using namespace heptachord::notenames;

TEST_CASE ("Deutsch liest die Choice-Tabellen der Parameter-Tabelle", "[notenames]")
{
    CHECK (std::string (rootDisplayLabel (0, kNameModeDeutsch)) == "C");
    CHECK (std::string (rootDisplayLabel (1, kNameModeDeutsch)) == "Cis");
    CHECK (std::string (rootDisplayLabel (3, kNameModeDeutsch)) == "Dis");
    CHECK (std::string (rootDisplayLabel (6, kNameModeDeutsch)) == "Fis");
    CHECK (std::string (rootDisplayLabel (10, kNameModeDeutsch)) == "Ais");
    CHECK (std::string (rootDisplayLabel (11, kNameModeDeutsch)) == "H");

    CHECK (std::string (scaleDisplayLabel (0, kNameModeDeutsch)) == "Dur");
    CHECK (std::string (scaleDisplayLabel (1, kNameModeDeutsch)) == "Moll");
    CHECK (std::string (scaleDisplayLabel (2, kNameModeDeutsch)) == "Harmonisch Moll");
    CHECK (std::string (scaleDisplayLabel (7, kNameModeDeutsch)) == "Mixolydisch");
}

TEST_CASE ("International (b) liefert die vereinbarten Tabellen", "[notenames]")
{
    CHECK (std::string (rootDisplayLabel (0, kNameModeInternationalFlat)) == "C");
    CHECK (std::string (rootDisplayLabel (1, kNameModeInternationalFlat)) == "Db");
    CHECK (std::string (rootDisplayLabel (3, kNameModeInternationalFlat)) == "Eb");
    CHECK (std::string (rootDisplayLabel (6, kNameModeInternationalFlat)) == "Gb");
    CHECK (std::string (rootDisplayLabel (10, kNameModeInternationalFlat)) == "Bb");
    CHECK (std::string (rootDisplayLabel (11, kNameModeInternationalFlat)) == "B");

    CHECK (std::string (scaleDisplayLabel (0, kNameModeInternationalFlat)) == "Major");
    CHECK (std::string (scaleDisplayLabel (1, kNameModeInternationalFlat)) == "Minor");
    CHECK (std::string (scaleDisplayLabel (2, kNameModeInternationalFlat)) == "Harmonic Minor");
    CHECK (std::string (scaleDisplayLabel (3, kNameModeInternationalFlat)) == "Melodic Minor");
    CHECK (std::string (scaleDisplayLabel (7, kNameModeInternationalFlat)) == "Mixolydian");
}

TEST_CASE ("International (#) liefert die vereinbarten Tabellen", "[notenames]")
{
    CHECK (std::string (rootDisplayLabel (0, kNameModeInternationalSharp)) == "C");
    CHECK (std::string (rootDisplayLabel (1, kNameModeInternationalSharp)) == "C#");
    CHECK (std::string (rootDisplayLabel (3, kNameModeInternationalSharp)) == "D#");
    CHECK (std::string (rootDisplayLabel (6, kNameModeInternationalSharp)) == "F#");
    CHECK (std::string (rootDisplayLabel (10, kNameModeInternationalSharp)) == "A#");
    CHECK (std::string (rootDisplayLabel (11, kNameModeInternationalSharp)) == "B");

    CHECK (std::string (scaleDisplayLabel (0, kNameModeInternationalSharp)) == "Major");
    CHECK (std::string (scaleDisplayLabel (4, kNameModeInternationalSharp)) == "Dorian");
    CHECK (std::string (scaleDisplayLabel (6, kNameModeInternationalSharp)) == "Lydian");
}

TEST_CASE ("Index-Klemmung fuer Toene, Skalen und Modus", "[notenames]")
{
    CHECK (std::string (rootDisplayLabel (-5, kNameModeInternationalSharp)) == "C");
    CHECK (std::string (rootDisplayLabel (99, kNameModeInternationalSharp)) == "B");
    CHECK (std::string (scaleDisplayLabel (-1, kNameModeInternationalFlat)) == "Major");
    CHECK (std::string (scaleDisplayLabel (42, kNameModeInternationalFlat)) == "Mixolydian");

    // Modus wird geklemmt: zu hoch -> International (#), zu niedrig -> Deutsch.
    CHECK (std::string (rootDisplayLabel (0, 7)) == "C");
    CHECK (std::string (rootDisplayLabel (1, 7)) == "C#");
    CHECK (std::string (rootDisplayLabel (0, -3)) == "C");
    CHECK (std::string (rootDisplayLabel (1, -3)) == "Cis");
}
