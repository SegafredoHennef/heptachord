#include "plugin/ComputerKeyboard.h"

#include <catch2/catch_test_macros.hpp>

using namespace heptachord;

TEST_CASE ("Tastatur: A-K spielen C bis C einer Oktave", "[keyboard]")
{
    const int expected[8] = { 60, 62, 64, 65, 67, 69, 71, 72 };
    const char keys[8] = { 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k' };

    for (int i = 0; i < 8; ++i)
    {
        const auto action = mapKey (keys[i], 0);
        INFO ("Taste " << keys[i]);
        REQUIRE (action.kind == KeyAction::Kind::note);
        REQUIRE (action.note == expected[i]);
    }
}

TEST_CASE ("Tastatur: Oktavenversatz schiebt die Noten", "[keyboard]")
{
    REQUIRE (mapKey ('a', 1).note == 72);
    REQUIRE (mapKey ('k', 1).note == 84);
    REQUIRE (mapKey ('a', -1).note == 48);
    REQUIRE (mapKey ('k', -1).note == 60);
}

TEST_CASE ("Tastatur: Oktavenversatz bleibt im MIDI-Bereich", "[keyboard]")
{
    REQUIRE (mapKey ('a', -2).note == 36);
    REQUIRE (mapKey ('k', 2).note == 96);
}

TEST_CASE ("Tastatur: Y und Z senken die Oktave, X hebt sie", "[keyboard]")
{
    REQUIRE (mapKey ('y', 0).kind == KeyAction::Kind::octaveDown);
    REQUIRE (mapKey ('z', 0).kind == KeyAction::Kind::octaveDown);
    REQUIRE (mapKey ('x', 0).kind == KeyAction::Kind::octaveUp);
}

TEST_CASE ("Tastatur: Leertaste ist der Transport", "[keyboard]")
{
    const auto action = mapKey (32, 0); // juce::KeyPress::spaceKey
    REQUIRE (action.kind == KeyAction::Kind::transport);
}

TEST_CASE ("Tastatur: VK-Codes echter Tastaturen werden gemappt", "[keyboard]")
{
    // Windows liefert keyPressed bei echter Eingabe WM_KEYDOWN mit VK-Codes
    // ('A'..'Z' = 65..90); Zeichencodes ('a'..'z' = 97..122) kommen nur ueber
    // Texteingabe-Pfade. Beide muessen dieselbe Taste bedienen (Bug v0.1.0:
    // die Klaviatur blieb bei echter Tastatur stumm).
    const int expected[8] = { 60, 62, 64, 65, 67, 69, 71, 72 };
    const char vkKeys[8] = { 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K' };

    for (int i = 0; i < 8; ++i)
    {
        const auto action = mapKey (vkKeys[i], 0);
        INFO ("VK-Taste " << vkKeys[i]);
        REQUIRE (action.kind == KeyAction::Kind::note);
        REQUIRE (action.note == expected[i]);
    }

    REQUIRE (mapKey ('Y', 0).kind == KeyAction::Kind::octaveDown);
    REQUIRE (mapKey ('Z', 0).kind == KeyAction::Kind::octaveDown);
    REQUIRE (mapKey ('X', 0).kind == KeyAction::Kind::octaveUp);
}

TEST_CASE ("Tastatur: unbelegte Tasten ergeben keine Aktion", "[keyboard]")
{
    REQUIRE (mapKey ('q', 0).kind == KeyAction::Kind::none);
    REQUIRE (mapKey ('w', 0).kind == KeyAction::Kind::none);
    REQUIRE (mapKey ('0', 0).kind == KeyAction::Kind::none);
    REQUIRE (mapKey ('\n', 0).kind == KeyAction::Kind::none);
}
