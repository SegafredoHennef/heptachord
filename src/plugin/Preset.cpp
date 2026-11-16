#include "plugin/Preset.h"

#include "core/ParameterTable.h"

#include <juce_core/juce_core.h>

#include <stdexcept>

namespace heptachord
{

juce::String Preset::toJson() const
{
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    root->setProperty ("schemaVersion", kPresetSchemaVersion);
    root->setProperty ("name", name);

    juce::DynamicObject::Ptr params = new juce::DynamicObject();
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        const auto& p = params::kParams[i];
        switch (p.type)
        {
            case params::Type::Bool:
                params->setProperty (juce::Identifier (p.id), values[i] >= 0.5f);
                break;
            case params::Type::Int:
                params->setProperty (juce::Identifier (p.id), static_cast<int> (values[i]));
                break;
            case params::Type::Trigger:
                // Trigger werden nicht gespeichert (flankengetriggert, Nr. 67).
                break;
            default:
                params->setProperty (juce::Identifier (p.id), values[i]);
                break;
        }
    }
    root->setProperty ("parameters", juce::var (params));

    return juce::JSON::toString (juce::var (root), true);
}

Preset Preset::fromJson (const juce::String& json)
{
    juce::var parsed;
    if (! juce::JSON::parse (json, parsed) || ! parsed.isObject())
        throw std::runtime_error ("Ungueltiges JSON-Format");

    const auto* root = parsed.getDynamicObject();
    if (root == nullptr)
        throw std::runtime_error ("JSON ist kein Objekt");

    const int schemaVersion = root->getProperty ("schemaVersion");
    if (schemaVersion != kPresetSchemaVersion)
        throw std::runtime_error ("Unbekannte schemaVersion: " + std::to_string (schemaVersion));

    Preset result;
    result.name = root->getProperty ("name").toString();
    result.values = defaultParameterValues();

    const auto* paramsObj = root->getProperty ("parameters").getDynamicObject();
    if (paramsObj != nullptr)
    {
        for (const auto& prop : paramsObj->getProperties())
        {
            const int index = params::indexOf (prop.name.toString().toRawUTF8());
            if (index < 0)
                continue;

            const auto& p = params::kParams[static_cast<std::size_t> (index)];
            const auto value = prop.value;

            switch (p.type)
            {
                case params::Type::Bool:
                    result.values[static_cast<std::size_t> (index)] = static_cast<bool> (value) ? 1.0f : 0.0f;
                    break;
                case params::Type::Int:
                    result.values[static_cast<std::size_t> (index)] = static_cast<float> (static_cast<int> (value));
                    break;
                case params::Type::Trigger:
                    break;
                default:
                    result.values[static_cast<std::size_t> (index)] = static_cast<float> (static_cast<double> (value));
                    break;
            }
        }
    }

    return result;
}

Preset Preset::loadFromFile (const juce::File& file)
{
    if (! file.existsAsFile())
        throw std::runtime_error ("Datei nicht gefunden: " + file.getFullPathName().toStdString());

    const juce::String json = file.loadFileAsString();
    return fromJson (json);
}

void Preset::saveToFile (const juce::File& file) const
{
    file.replaceWithText (toJson());
}

std::array<float, static_cast<std::size_t> (params::kNumParameters)> Preset::normalizedValues() const
{
    std::array<float, static_cast<std::size_t> (params::kNumParameters)> result {};

    for (std::size_t i = 0; i < result.size(); ++i)
    {
        const auto& p = params::kParams[i];

        switch (p.type)
        {
            case params::Type::Bool:
                result[i] = values[i] >= 0.5f ? 1.0f : 0.0f;
                break;

            case params::Type::Choice:
                result[i] = p.numChoices > 1 ? values[i] / static_cast<float> (p.numChoices - 1) : 0.0f;
                break;

            case params::Type::Trigger:
                result[i] = 0.0f;
                break;

            default:
                result[i] = p.max > p.min ? (values[i] - p.min) / (p.max - p.min) : 0.0f;
                break;
        }
    }

    return result;
}

std::vector<Preset> Preset::factoryPresets()
{
    std::vector<Preset> presets;

#ifdef HEPTACHORD_PRESETS_DIR
    std::vector<juce::File> candidates { juce::File (HEPTACHORD_PRESETS_DIR) };
#else
    const juce::File appFile = juce::File::getSpecialLocation (juce::File::currentApplicationFile);

    // Kandidaten: Arbeitsverzeichnis, neben der Binary, eine Ebene darueber
    // (Standalone liegt in Release/Standalone, die Presets in Release/presets).
    std::vector<juce::File> candidates {
        juce::File::getCurrentWorkingDirectory().getChildFile ("presets"),
        appFile.getParentDirectory().getChildFile ("presets"),
        appFile.getParentDirectory().getParentDirectory().getChildFile ("presets"),
    };
#endif

    for (const auto& presetsDir : candidates)
    {
        if (! presetsDir.isDirectory())
            continue;

        juce::Array<juce::File> files;
        presetsDir.findChildFiles (files, juce::File::findFiles, false, "*.heptachord");

        for (const auto& file : files)
        {
            try
            {
                presets.push_back (loadFromFile (file));
            }
            catch (const std::exception&)
            {
                // Fehlerhafte Factory-Presets werden ignoriert; der Editor
                // meldet den Fehler dem Nutzer.
            }
        }

        if (! presets.empty())
            break;
    }

    return presets;
}

} // namespace heptachord
