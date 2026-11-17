#pragma once

namespace heptachord
{

// QWERTZ-freundliche Mini-Klaviatur fuer den Standalone (nur Plugin-Schicht,
// ADR 0007). Reine Abbildungstabelle ohne JUCE-GUI.
//
// Die Tabelle erwartet Zeichencodes ('a'..'z'); mapKey normalisiert die
// VK-Codes echter Tastaturen ('A'..'Z' = 65..90) davor, denn juce::KeyPress
// liefert auf Windows WM_KEYDOWN-Werte (VK), nicht WM_CHAR-Werte. Dadurch
// funktionieren QWERTY und QWERTZ ohne Erkennung:
//
//   A S D F G H J K  ->  C D E F G A B C  (je Oktave +12)
//   Y / Z             ->  Oktave runter
//   X                 ->  Oktave rauf
//   Leertaste         ->  Transport
struct KeyAction
{
    enum class Kind
    {
        none,
        note,
        octaveDown,
        octaveUp,
        transport
    };

    Kind kind = Kind::none;
    int note = 0; // MIDI-Note, gueltig bei Kind::note
};

inline constexpr int kMinOctave = -2;
inline constexpr int kMaxOctave = 2;

// Die MIDI-Note einer Note-Taste (-1, wenn es keine Note-Taste ist).
int noteFor (int keyCode, int octave) noexcept;

// Ordnet einen Tastencode einer Aktion zu.
KeyAction mapKey (int keyCode, int octave) noexcept;

} // namespace heptachord
