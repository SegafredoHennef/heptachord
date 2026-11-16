#include "plugin/LookAndFeel.h"

#include <cmath>

namespace heptachord
{

HeptachordLookAndFeel::HeptachordLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, background());
    setColour (juce::Label::textColourId, text());
    setColour (juce::Slider::textBoxTextColourId, text());
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::thumbColourId, accent());
    setColour (juce::Slider::rotarySliderFillColourId, accent());
    setColour (juce::Slider::rotarySliderOutlineColourId, line());
    setColour (juce::ToggleButton::textColourId, text());
    setColour (juce::ComboBox::textColourId, text());
    setColour (juce::ComboBox::backgroundColourId, panel());
    setColour (juce::ComboBox::outlineColourId, line());
    setColour (juce::ComboBox::arrowColourId, muted());
    setColour (juce::PopupMenu::backgroundColourId, panelRaised());
    setColour (juce::PopupMenu::textColourId, text());
    setColour (juce::PopupMenu::highlightedBackgroundColourId, accent().withAlpha (0.25f));
    setColour (juce::PopupMenu::highlightedTextColourId, text());
    setColour (juce::TextButton::buttonColourId, panelRaised());
    setColour (juce::TextButton::buttonOnColourId, accent());
    setColour (juce::TextButton::textColourOffId, text());
    setColour (juce::TextButton::textColourOnId, background());
}

void HeptachordLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float rotaryStartAngle,
                                              float rotaryEndAngle, juce::Slider&)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.5f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto trackWidth = juce::jmax (2.0f, radius * 0.15f);

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius, radius, 0.0f,
                         rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (line());
    g.strokePath (track, juce::PathStrokeType (trackWidth, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    if (sliderPos > 0.0001f)
    {
        juce::Path value;
        value.addCentredArc (centre.x, centre.y, radius, radius, 0.0f,
                             rotaryStartAngle, angle, true);
        g.setColour (accent());
        g.strokePath (value, juce::PathStrokeType (trackWidth, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
    }

    const auto bodyRadius = radius * 0.58f;
    g.setColour (panelRaised());
    g.fillEllipse (centre.x - bodyRadius, centre.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f);
    g.setColour (line());
    g.drawEllipse (centre.x - bodyRadius, centre.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f, 1.0f);

    const auto pointer = radius * 0.78f;
    const juce::Point<float> tip (centre.x + pointer * std::sin (angle),
                                  centre.y - pointer * std::cos (angle));
    g.setColour (text());
    g.drawLine (centre.x, centre.y, tip.x, tip.y, juce::jmax (1.5f, bodyRadius * 0.16f));
}

void HeptachordLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    const bool on = button.getToggleState();

    auto fill = on ? accent() : panelRaised();

    if (shouldDrawButtonAsDown)
        fill = fill.brighter (0.15f);
    else if (shouldDrawButtonAsHighlighted)
        fill = fill.brighter (0.07f);

    g.setColour (fill);
    g.fillRoundedRectangle (bounds, 3.5f);

    g.setColour (on ? accent() : line());
    g.drawRoundedRectangle (bounds, 3.5f, 1.0f);

    g.setColour (on ? background() : muted());
    g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    g.drawText (button.getButtonText(), bounds, juce::Justification::centred);
}

void HeptachordLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                          int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float> (0.0f, 0.0f, static_cast<float> (width),
                                          static_cast<float> (height)).reduced (1.0f);

    g.setColour (panelRaised());
    g.fillRoundedRectangle (bounds, 3.5f);

    g.setColour (box.isMouseOver() ? accent().withAlpha (0.7f) : line());
    g.drawRoundedRectangle (bounds, 3.5f, 1.0f);

    const auto arrowSize = 4.0f;
    const auto arrowX = static_cast<float> (width) - 12.0f;
    const auto arrowY = static_cast<float> (height) * 0.5f;

    juce::Path arrow;
    arrow.startNewSubPath (arrowX - arrowSize, arrowY - arrowSize * 0.5f);
    arrow.lineTo (arrowX, arrowY + arrowSize * 0.5f);
    arrow.lineTo (arrowX + arrowSize, arrowY - arrowSize * 0.5f);

    g.setColour (muted());
    g.strokePath (arrow, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
}

void HeptachordLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (7, 0, box.getWidth() - 22, box.getHeight());
    label.setFont (juce::FontOptions (12.0f));
    label.setJustificationType (juce::Justification::centredLeft);
}

juce::Label* HeptachordLookAndFeel::createSliderTextBox (juce::Slider& slider)
{
    auto* label = juce::LookAndFeel_V4::createSliderTextBox (slider);
    label->setFont (juce::FontOptions (11.0f));
    label->setJustificationType (juce::Justification::centred);
    return label;
}

void HeptachordLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                                  const juce::Colour& backgroundColour,
                                                  bool shouldDrawButtonAsHighlighted,
                                                  bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);

    auto fill = button.getToggleState() ? accent() : backgroundColour;

    if (shouldDrawButtonAsDown)
        fill = fill.brighter (0.18f);
    else if (shouldDrawButtonAsHighlighted)
        fill = fill.brighter (0.08f);

    g.setColour (fill);
    g.fillRoundedRectangle (bounds, 4.0f);

    g.setColour (button.getToggleState() ? accent() : line());
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}

juce::Font HeptachordLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return juce::FontOptions (juce::jmin (13.0f, static_cast<float> (buttonHeight) * 0.48f),
                              juce::Font::bold);
}

} // namespace heptachord
