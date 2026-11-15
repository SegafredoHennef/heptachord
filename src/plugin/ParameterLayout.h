#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace heptachord
{

// Baut den APVTS-Baum aus der generierten Parameter-Tabelle (parameters.toml).
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

} // namespace heptachord
