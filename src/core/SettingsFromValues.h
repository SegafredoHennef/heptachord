#pragma once

#include "core/EngineTypes.h"
#include "core/ParameterTable.h"

#include <array>

namespace heptachord
{

// Parameterwerte in der Reihenfolge des Golden Files
// (tests/parameter_ids.txt). Die Plugin-Schicht fuellt das Array aus dem
// APVTS; alles Weitere passiert hier, ohne JUCE (ADR 0007).
using ParameterValues = std::array<float, static_cast<std::size_t> (params::kNumParameters)>;

// Defaults aus parameters.toml. Fuer Tests und fuer den Fall, dass ein Host
// noch keinen Wert geliefert hat.
ParameterValues defaultParameterValues() noexcept;

// Baut die flache Engine-Konfiguration aus den rohen Parameterwerten.
// Choice-Indizes werden auf ihre Bedeutung abgebildet (Skala, Richtung,
// Modus, Rate); unbekannte Indizes werden geklemmt.
EngineSettings settingsFromValues (const ParameterValues& values) noexcept;

} // namespace heptachord
