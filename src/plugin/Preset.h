#pragma once

#include "core/SettingsFromValues.h"

#include <juce_core/juce_core.h>

#include <string>

namespace heptachord
{

// JSON-Preset fuer Heptachord (nur in der Plugin-Schicht, ADR 0003).
// Format:
//   { "schemaVersion": 1, "name": "...", "parameters": { "root": 0, ... } }
// Jeder Parameter-ID aus dem Golden File kann als Key vorkommen; fehlende
// Parameter behalten ihren Default-Wert.
class Preset
{
public:
    Preset() = default;

    Preset (juce::String name, ParameterValues values)
        : name (std::move (name)), values (std::move (values)) {}

    const juce::String& getName() const noexcept { return name; }
    const ParameterValues& getValues() const noexcept { return values; }

    // Serialisiert zu einem JSON-String.
    juce::String toJson() const;

    // Erzeugt ein Preset aus einem JSON-String. Wirft std::runtime_error bei
    // ungueltigem Format oder schemaVersion-Abweichung.
    static Preset fromJson (const juce::String& json);

    // Normalisierte Werte (0..1), wie sie das APVTS erwartet:
    // Choice = Index/(n-1), Bool = 0/1, sonst (wert-min)/(max-min).
    std::array<float, static_cast<std::size_t> (params::kNumParameters)> normalizedValues() const;

    // Liest eine .heptachord-Datei.
    static Preset loadFromFile (const juce::File& file);

    // Schreibt dieses Preset als .heptachord-Datei.
    void saveToFile (const juce::File& file) const;

    // Die Factory-Presets (8 Stueck, eine Referenz je Skala).
    static std::vector<Preset> factoryPresets();

private:
    juce::String name;
    ParameterValues values;

    static juce::String parametersToJson (const ParameterValues& v);
    static ParameterValues jsonToParameters (const juce::var& root);
};

// Der zu erwartende Schema-Wert (aus docs/entscheidungen.md Nr. 40).
inline constexpr int kPresetSchemaVersion = 1;

} // namespace heptachord
