#pragma once

#include "core/SettingsFromValues.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <atomic>

namespace heptachord
{

// Bruecke APVTS -> flache Parameterwerte.
//
// Die Zeiger auf die atomaren Werte werden einmal verbunden und danach pro
// Block gelesen; kein ValueTree-Zugriff und keine Allokation im Audiothread.
// Die Reihenfolge entspricht dem Golden File (tests/parameter_ids.txt).
class ParameterBinding
{
public:
    void connect (juce::AudioProcessorValueTreeState& apvts);

    bool isConnected() const noexcept { return m_connected; }

    // Liest alle 116 Werte. Fehlt ein Parameter, bleibt der Default stehen.
    ParameterValues current() const noexcept;

    float get (int index) const noexcept;

private:
    std::array<std::atomic<float>*, static_cast<std::size_t> (params::kNumParameters)> m_values {};
    bool m_connected = false;
};

} // namespace heptachord
