#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace heptachord
{

class Processor;

class Editor final : public juce::AudioProcessorEditor
{
public:
    explicit Editor (Processor& processorToEdit);
    ~Editor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    Processor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)
};

} // namespace heptachord
