#include "plugin/TestSynth.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace heptachord;

namespace
{
float peakOf (const juce::AudioBuffer<float>& buffer)
{
    float peak = 0.0f;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        peak = juce::jmax (peak, buffer.getMagnitude (channel, 0, buffer.getNumSamples()));

    return peak;
}
} // namespace

TEST_CASE ("Test-Synth bleibt ohne Noten still", "[synth]")
{
    TestSynth synth;
    synth.prepare (48000.0);

    juce::AudioBuffer<float> buffer (2, 512);
    buffer.clear();

    juce::MidiBuffer midi;
    synth.process (midi, buffer);

    REQUIRE (peakOf (buffer) == 0.0f);
}

TEST_CASE ("Test-Synth erzeugt nach Note-On ein Signal", "[synth]")
{
    TestSynth synth;
    synth.prepare (48000.0);

    juce::AudioBuffer<float> buffer (2, 4800);
    buffer.clear();

    juce::MidiBuffer midi;
    midi.addEvent (juce::MidiMessage::noteOn (1, 69, static_cast<juce::uint8> (100)), 0);
    synth.process (midi, buffer);

    REQUIRE (peakOf (buffer) > 0.01f);
    REQUIRE (peakOf (buffer) <= 1.0f);

    // Beide Kanaele bekommen dasselbe Signal.
    REQUIRE (buffer.getMagnitude (0, 0, 4800) == Catch::Approx (buffer.getMagnitude (1, 0, 4800)));
}

TEST_CASE ("Test-Synth klingt nach Note-Off und Release aus", "[synth]")
{
    TestSynth synth;
    synth.prepare (48000.0);

    juce::AudioBuffer<float> on (2, 2400);
    on.clear();

    juce::MidiBuffer start;
    start.addEvent (juce::MidiMessage::noteOn (1, 69, static_cast<juce::uint8> (100)), 0);
    synth.process (start, on);

    juce::AudioBuffer<float> tail (2, 48000);
    tail.clear();

    juce::MidiBuffer stop;
    stop.addEvent (juce::MidiMessage::noteOff (1, 69), 0);
    synth.process (stop, tail);

    // Nach einer Sekunde (Release 0,12 s) ist die Stimme aus.
    REQUIRE (tail.getMagnitude (0, 36000, 12000) < 0.0001f);
}
