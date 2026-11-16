#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace heptachord
{

// Dunkles, eigenstaendiges Design (kein Material der Vorlage).
// Eine Akzentfarbe, ruhige Flaechen, kompakte Typografie.
class HeptachordLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    HeptachordLookAndFeel();

    static juce::Colour background() { return juce::Colour (0xff13161a); }
    static juce::Colour panel() { return juce::Colour (0xff1b1f25); }
    static juce::Colour panelRaised() { return juce::Colour (0xff232830); }
    static juce::Colour line() { return juce::Colour (0xff2c333d); }
    static juce::Colour text() { return juce::Colour (0xffe8e6e1); }
    static juce::Colour muted() { return juce::Colour (0xff8b93a1); }
    static juce::Colour accent() { return juce::Colour (0xffff8a3d); }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override;

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override;

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override;

    juce::Label* createSliderTextBox (juce::Slider& slider) override;

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont (juce::TextButton& button, int buttonHeight) override;
};

} // namespace heptachord
