#include "plugin/ComputerKeyboard.h"

#include <algorithm>

namespace heptachord
{
namespace
{
// Weiße Tasten: Zeichencode -> Halbtonabstand von C.
struct NoteKey
{
    int keyCode;
    int semitone;
};

constexpr NoteKey kNoteKeys[] = {
    { 'a', 0 },
    { 's', 2 },
    { 'd', 4 },
    { 'f', 5 },
    { 'g', 7 },
    { 'h', 9 },
    { 'j', 11 },
    { 'k', 12 },
};

constexpr int kBaseNote = 60; // C4
constexpr int kSpaceKey = 32; // juce::KeyPress::spaceKey

int clampNote (int note) noexcept
{
    return std::clamp (note, 0, 127);
}
} // namespace

int noteFor (int keyCode, int octave) noexcept
{
    for (const auto& entry : kNoteKeys)
        if (keyCode == entry.keyCode)
            return clampNote (kBaseNote + octave * 12 + entry.semitone);

    return -1;
}

KeyAction mapKey (int keyCode, int octave) noexcept
{
    // Echte Tastaturen liefern auf Windows WM_KEYDOWN mit VK-Codes
    // ('A'..'Z' = 65..90); Zeichencodes ('a'..'z' = 97..122) erreichen
    // keyPressed nur ueber Texteingabe-Pfade. Normalisiert wird auf die
    // Zeichencodes der Tabelle, damit beide Wege dieselbe Taste bedienen.
    if (keyCode >= 'A' && keyCode <= 'Z')
        keyCode += 'a' - 'A';

    if (const int note = noteFor (keyCode, octave); note >= 0)
        return { KeyAction::Kind::note, note };

    // Y und Z liegen auf QWERTY und QWERTZ an verschiedenen Stellen; beide
    // bedienen die Oktave runter, damit die Taste nach ihrer Beschriftung
    // funktioniert.
    if (keyCode == 'y' || keyCode == 'z')
        return { KeyAction::Kind::octaveDown, 0 };

    if (keyCode == 'x')
        return { KeyAction::Kind::octaveUp, 0 };

    if (keyCode == kSpaceKey)
        return { KeyAction::Kind::transport, 0 };

    return {};
}

} // namespace heptachord
