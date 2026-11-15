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

int snapToScale (int note, Scale scale) noexcept
{
    const auto& table = scaleTable (scale);
    const int clamped = std::clamp (note, 0, 127);
    const int baseOctave = (clamped / 12) * 12;

    int best = clamped;
    int bestDistance = 128;

    for (int octaveOffset = -12; octaveOffset <= 12; octaveOffset += 12)
    {
        for (const int degreeSemitone : table)
        {
            const int candidate = baseOctave + octaveOffset + degreeSemitone;
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

} // namespace heptachord
