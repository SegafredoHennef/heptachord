#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>

namespace heptachord
{

// Einfacher Test-Synth fuer den Standalone (Spezifikation Abschnitt 10):
// vier Stimmen, Saegezahn, Tiefpass, ADSR. Er laeuft nur im Standalone; der
// VST3-Ausgang bleibt still (ADR 0001).
class TestSynth
{
public:
    TestSynth();

    void prepare (double sampleRate);
    void reset();

    // Spielt die Noten aus midi in den (bereits geleerten) Stereopuffer.
    void process (const juce::MidiBuffer& midi, juce::AudioBuffer<float>& buffer);

private:
    struct Voice
    {
        bool active = false;
        int pitch = -1;
        int channel = 0;
        float gain = 0.0f;
        double phase = 0.0;
        double increment = 0.0;
        float filter = 0.0f;
        float filterCoefficient = 0.2f;
        juce::ADSR envelope;
    };

    void handleMessage (const juce::MidiMessage& message);
    void startVoice (const juce::MidiMessage& message);
    void stopVoice (int pitch, int channel);
    void render (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    static constexpr int kNumVoices = 4;

    std::array<Voice, kNumVoices> m_voices {};
    juce::ADSR::Parameters m_parameters { 0.005f, 0.08f, 0.7f, 0.12f };
    double m_sampleRate = 44100.0;
    int m_nextVoice = 0;
};

} // namespace heptachord
