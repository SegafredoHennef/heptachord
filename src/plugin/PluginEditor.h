#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
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
    bool keyPressed (const juce::KeyPress& key) override;

private:
    Processor& processor;
    juce::TextButton transportButton { "Start / Stop (Leertaste)" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)
};

} // namespace heptachord
