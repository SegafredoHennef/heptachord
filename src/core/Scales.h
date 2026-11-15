#pragma once

#include <array>
#include <span>

namespace heptachord
{

inline constexpr int kNumScales = 8;
inline constexpr int kDegreesPerOctave = 7;

enum class Scale : int
{
    Dur = 0,
    Moll,
    HarmonischMoll,
    MelodischMoll,
    Dorisch,
    Phrygisch,
    Lydisch,
    Mixolydisch
};

using ScaleTable = std::array<int, kDegreesPerOctave>;

std::span<const ScaleTable> scaleTables() noexcept;
const ScaleTable& scaleTable (Scale scale) noexcept;

int floorDiv (int a, int b) noexcept;

// Diatonischer Abstand (Skalenschritte) -> Halbton-Offset gegenueber dem
// Grundton. Kann negativ sein, wird nicht geklemmt.
int degreeToSemitone (int degree, Scale scale) noexcept;

// Grundton (0...11) + Grad -> MIDI-Note, geklemmt auf 0...127.
int degreeToNote (int root, int degree, Scale scale) noexcept;

// Chromatischer Ton -> naechster Skalenton (Rasterung). Bei Gleichstand
// gewinnt der tiefere Kandidat. Ergebnis 0...127.
int snapToScale (int note, Scale scale) noexcept;

} // namespace heptachord
