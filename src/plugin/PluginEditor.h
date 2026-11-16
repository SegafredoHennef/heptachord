#pragma once

#include "plugin/LookAndFeel.h"
#include "plugin/Preset.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace heptachord
{

class Processor;

// Eine Seite, drei Zonen (Entscheidung 70): globale Leiste, Stimmen,
// acht Step-Spalten. Alle Parameter sind gleichzeitig sichtbar.
class Editor final : public juce::AudioProcessorEditor
{
public:
    explicit Editor (Processor& processorToEdit);
    ~Editor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress& key) override;
    bool keyStateChanged (bool isKeyDown) override;
    void focusLost (FocusChangeType) override;

private:
    struct CaptionEntry
    {
        juce::Component* anchor = nullptr;
        juce::String text;
    };

    struct StaticCaption
    {
        juce::Rectangle<int> area;
        juce::String text;
    };

    struct VoiceRow
    {
        juce::ToggleButton* enable = nullptr;
        juce::Slider* degree = nullptr;
        juce::Slider* octave = nullptr;
        juce::Slider* velocity = nullptr;
        juce::ToggleButton* follow = nullptr;
        juce::Slider* gate = nullptr;
        juce::Slider* channel = nullptr;
    };

    struct StepColumn
    {
        juce::ToggleButton* active = nullptr;
        juce::ComboBox* mode = nullptr;
        juce::Slider* offset = nullptr;
        juce::Slider* velocity = nullptr;
        juce::Slider* probability = nullptr;
        juce::Slider* ratchet = nullptr;
        std::array<juce::ToggleButton*, 4> mask {};
    };

    juce::Slider& addKnob (const juce::String& paramId, const juce::String& caption);
    juce::ComboBox& addChoice (const juce::String& paramId, const juce::String& caption);
    juce::ToggleButton& addToggle (const juce::String& paramId, const juce::String& caption);
    void addCaptionAnchor (juce::Component& anchor, const juce::String& text);
    void addTooltipFromTable (juce::SettableTooltipClient& component, const juce::String& paramId);

    void buildControls();
    void layoutHeader (juce::Rectangle<int> area);
    void layoutGlobal (juce::Rectangle<int> area);
    void layoutVoices (juce::Rectangle<int> area);
    void layoutSteps (juce::Rectangle<int> area);
    void updateTransportButton();
    void applySelectedPreset();
    void applyPreset (const Preset& preset);

    Processor& processor;
    HeptachordLookAndFeel lookAndFeel;

    juce::OwnedArray<juce::Slider> sliders;
    juce::OwnedArray<juce::ComboBox> combos;
    juce::OwnedArray<juce::ToggleButton> toggles;

    // Nach den Controls deklariert: werden vor ihnen abgebaut.
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;

    // Globale Leiste.
    juce::ComboBox* presetBox = nullptr;
    juce::ComboBox* rootBox = nullptr;
    juce::ComboBox* scaleBox = nullptr;
    juce::ComboBox* rateBox = nullptr;
    juce::ComboBox* directionBox = nullptr;
    juce::Slider* swingKnob = nullptr;
    juce::Slider* slackKnob = nullptr;
    juce::Slider* harmVelocityKnob = nullptr;
    juce::Slider* harmSpreadKnob = nullptr;
    juce::ToggleButton* latchButton = nullptr;
    juce::ComboBox* outputBox = nullptr;

    // Thru.
    juce::ToggleButton* thruEnableButton = nullptr;
    juce::ToggleButton* thruSnapButton = nullptr;
    juce::Slider* thruVelocityKnob = nullptr;
    juce::Slider* thruChannelKnob = nullptr;

    // H1 ... H3.
    std::array<VoiceRow, 3> voices {};

    // S1 ... S8.
    std::array<StepColumn, 8> steps {};

    juce::TextButton resetButton { "RESET" };
    juce::TextButton transportButton { "START" };

    std::vector<CaptionEntry> controlCaptions;
    std::vector<StaticCaption> staticCaptions;

    juce::Rectangle<int> headerArea;
    juce::Rectangle<int> globalArea;
    juce::Rectangle<int> voicesArea;
    juce::Rectangle<int> stepsArea;

    bool standalone = false;

    // Tastatur des Standalones (QWERTZ-freundlich, siehe ComputerKeyboard.h).
    int keyboardOctave = 0;
    std::set<int> pressedKeys;
    std::map<int, int> heldNotes; // Tastencode -> MIDI-Note

    // Factory-Presets (M5).
    std::vector<Preset> factoryPresets;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)
};

} // namespace heptachord
