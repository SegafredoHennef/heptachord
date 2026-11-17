// Anzeige-Bezeichnungen fuer Noten- und Skalennamen (Decision 83).
// Nur Anzeige: Presets speichern Indizes, der Bezeichnungsmodus legt fest,
// wie root/scale/Preset-Namen dargestellt werden. JUCE-frei wie der Rest
// des Cores (ADR 0007); Deutsch liest die Choice-Tabellen der
// Parameter-Tabelle (Single Source of Truth).
#pragma once

#include "core/ParameterTable.h"

namespace heptachord::notenames
{

inline constexpr int kNumRoots = 12;
inline constexpr int kNumScales = 8;

// Indizes in names.mode (kChoices9).
inline constexpr int kNameModeDeutsch = 0;
inline constexpr int kNameModeInternationalFlat = 1;
inline constexpr int kNameModeInternationalSharp = 2;
inline constexpr int kNumNameModes = 3;

// International (b): C, Db, ..., Bb, B.
inline constexpr const char* kInternationalFlatRoots[] = {
    "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B",
};

// International (#): C, C#, ..., A#, B.
inline constexpr const char* kInternationalSharpRoots[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
};

// Skalen unter internationalen Bezeichnungen (gleiche Reihenfolge wie kChoices1).
inline constexpr const char* kInternationalScales[] = {
    "Major", "Minor", "Harmonic Minor", "Melodic Minor",
    "Dorian", "Phrygian", "Lydian", "Mixolydian",
};

inline int clampRootIndex (int index)
{
    return index < 0 ? 0 : index >= kNumRoots ? kNumRoots - 1 : index;
}

inline int clampScaleIndex (int index)
{
    return index < 0 ? 0 : index >= kNumScales ? kNumScales - 1 : index;
}

inline int clampNameMode (int mode)
{
    return mode < kNameModeDeutsch ? kNameModeDeutsch
         : mode >= kNumNameModes ? kNumNameModes - 1
         : mode;
}

// Anzeige-Label eines Grundtons (Index 0..11, geklemmt) im gewaehlten Modus.
// Deutsch liest kChoices0 ("C", "Cis", ..., "Ais", "H").
inline const char* rootDisplayLabel (int rootIndex, int nameMode)
{
    const int root = clampRootIndex (rootIndex);

    switch (clampNameMode (nameMode))
    {
        case kNameModeInternationalFlat:
            return kInternationalFlatRoots[root];
        case kNameModeInternationalSharp:
            return kInternationalSharpRoots[root];
        default:
            return params::kChoices0[static_cast<std::size_t> (root)];
    }
}

// Anzeige-Label einer Skala (Index 0..7, geklemmt) im gewaehlten Modus.
// Deutsch liest kChoices1 ("Dur", "Moll", ...).
inline const char* scaleDisplayLabel (int scaleIndex, int nameMode)
{
    const int scale = clampScaleIndex (scaleIndex);

    switch (clampNameMode (nameMode))
    {
        case kNameModeInternationalFlat:
        case kNameModeInternationalSharp:
            return kInternationalScales[scale];
        default:
            return params::kChoices1[static_cast<std::size_t> (scale)];
    }
}

} // namespace heptachord::notenames
