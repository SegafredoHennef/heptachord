#include "plugin/PluginEditor.h"

#include "core/ParameterTable.h"
#include "plugin/PluginProcessor.h"

namespace heptachord
{

Editor::Editor (Processor& processorToEdit)
    : AudioProcessorEditor (&processorToEdit), processor (processorToEdit)
{
    setSize (640, 360);
}

Editor::~Editor() = default;

void Editor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff14161a));

    auto area = getLocalBounds().reduced (24);

    g.setColour (juce::Colour (0xffe8e6e1));
    g.setFont (juce::FontOptions (28.0f, juce::Font::bold));
    g.drawText ("Heptachord", area.removeFromTop (40), juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff8b93a1));
    g.setFont (juce::FontOptions (15.0f));
    g.drawText (juce::String (params::kNumParameters) + " Parameter, schemaVersion "
                    + juce::String (params::kSchemaVersion),
                area.removeFromTop (24), juce::Justification::centredLeft);

    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("M0 — Grundgeruest. Die Step-Engine folgt in M1.",
                area.removeFromTop (22), juce::Justification::centredLeft);
}

void Editor::resized()
{
}

} // namespace heptachord
