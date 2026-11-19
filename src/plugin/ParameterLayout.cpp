#include "plugin/ParameterLayout.h"

#include "core/ParameterTable.h"

#include <memory>
#include <vector>

namespace heptachord
{
namespace
{
juce::StringArray toChoices (const params::Param& p)
{
    juce::StringArray choices;
    for (int i = 0; i < p.numChoices; ++i)
        // Choice-Texte sind UTF-8 (z. B. "Vorwärts"); der char*-Konstruktor
        // von juce::String interpretiert sie als ASCII und verzernt Umlaute.
        choices.add (juce::String::fromUTF8 (p.choices[i]));
    return choices;
}

std::unique_ptr<juce::RangedAudioParameter> makeParameter (const params::Param& p)
{
    const juce::ParameterID id { juce::String (p.id), 1 };
    const juce::String name { p.label };
    const juce::String unit { p.unit == nullptr ? "" : p.unit };

    switch (p.type)
    {
        case params::Type::Bool:
        case params::Type::Trigger:
            // Trigger werden als momentary-artiger Bool abgebildet; das Verhalten
            // (Step 1 am naechsten Rand) implementiert der Core.
            return std::make_unique<juce::AudioParameterBool> (id, name, p.defaultValue >= 0.5f);

        case params::Type::Int:
            return std::make_unique<juce::AudioParameterInt> (
                id, name, static_cast<int> (p.min), static_cast<int> (p.max),
                static_cast<int> (p.defaultValue));

        case params::Type::Float:
            return std::make_unique<juce::AudioParameterFloat> (
                id, name, juce::NormalisableRange<float> (p.min, p.max), p.defaultValue,
                juce::AudioParameterFloatAttributes().withLabel (unit));

        case params::Type::Choice:
            return std::make_unique<juce::AudioParameterChoice> (
                id, name, toChoices (p), static_cast<int> (p.defaultValue));
    }

    jassertfalse;
    return nullptr;
}
} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> list;
    list.reserve (static_cast<std::size_t> (params::kNumParameters));

    for (const auto& p : params::kParams)
        if (auto parameter = makeParameter (p))
            list.push_back (std::move (parameter));

    return { list.begin(), list.end() };
}

} // namespace heptachord
