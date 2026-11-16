#include "core/Scales.h"

#include <algorithm>
#include <cstdlib>

namespace heptachord
{
namespace
{
// Reihenfolge wie im Original (docs/02-behavior.md, Abschnitt 2).
constexpr std::array<ScaleTable, kNumScales> kTables {{
    { 0, 2, 4, 5, 7, 9, 11 },  // Dur
    { 0, 2, 3, 5, 7, 8, 10 },  // Moll
    { 0, 2, 3, 5, 7, 8, 11 },  // Harmonisch Moll
    { 0, 2, 3, 5, 7, 9, 11 },  // Melodisch Moll
    { 0, 2, 3, 5, 7, 9, 10 },  // Dorisch
    { 0, 1, 3, 5, 7, 8, 10 },  // Phrygisch
    { 0, 2, 4, 6, 7, 9, 11 },  // Lydisch
    { 0, 2, 4, 5, 7, 9, 10 },  // Mixolydisch
}};
} // namespace

std::span<const ScaleTable> scaleTables() noexcept
{
    return kTables;
}

const ScaleTable& scaleTable (Scale scale) noexcept
{
    const auto index = std::clamp (static_cast<int> (scale), 0, kNumScales - 1);
    return kTables[static_cast<std::size_t> (index)];
}

int floorDiv (int a, int b) noexcept
{
    const int q = a / b;
    return (a % b != 0 && ((a < 0) != (b < 0))) ? q - 1 : q;
}

int degreeToSemitone (int degree, Scale scale) noexcept
{
    const auto& table = scaleTable (scale);
    const int octave = floorDiv (degree, kDegreesPerOctave);
    const int index = degree - kDegreesPerOctave * octave;
    return 12 * octave + table[static_cast<std::size_t> (index)];
}

int degreeToNote (int root, int degree, Scale scale) noexcept
{
    return std::clamp (root + degreeToSemitone (degree, scale), 0, 127);
}

int snapToScale (int note, Scale scale, int root) noexcept
{
    const auto& table = scaleTable (scale);
    const int clamped = std::clamp (note, 0, 127);
    const int rootPitch = ((root % 12) + 12) % 12;

    int best = clamped;
    int bestDistance = 128;

    // Die Oktaven -3 ... 10 decken den gesamten MIDI-Bereich ab.
    for (int octave = -3; octave <= 10; ++octave)
    {
        for (const int degreeSemitone : table)
        {
            const int candidate = rootPitch + 12 * octave + degreeSemitone;
            if (candidate < 0 || candidate > 127)
                continue;

            const int distance = std::abs (candidate - clamped);
            if (distance < bestDistance || (distance == bestDistance && candidate < best))
            {
                bestDistance = distance;
                best = candidate;
            }
        }
    }

    return best;
}

int baseDegree (int note, int root, Scale scale) noexcept
{
    const auto& table = scaleTable (scale);
    const int snapped = snapToScale (note, scale, root);
    const int rootPitch = ((root % 12) + 12) % 12;

    const int relative = snapped - rootPitch;
    const int octave = floorDiv (relative, 12);
    const int inner = relative - 12 * octave;   // 0 ... 11

    // Groesster Skalengrad, der nicht ueber dem Ton liegt.
    int index = 0;
    for (int i = 0; i < kDegreesPerOctave; ++i)
        if (table[static_cast<std::size_t> (i)] <= inner)
            index = i;

    return kDegreesPerOctave * octave + index;
}

} // namespace heptachord
