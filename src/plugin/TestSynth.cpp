#include "plugin/TestSynth.h"

#include <algorithm>
#include <cmath>

namespace heptachord
{

TestSynth::TestSynth() = default;

void TestSynth::prepare (double sampleRate)
{
    m_sampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;

    for (auto& voice : m_voices)
    {
        voice.envelope.setSampleRate (m_sampleRate);
        voice.envelope.setParameters (m_parameters);
    }
}

void TestSynth::reset()
{
    for (auto& voice : m_voices)
        voice = {};

    m_nextVoice = 0;
}

void TestSynth::process (const juce::MidiBuffer& midi, juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    if (numSamples <= 0)
        return;

    int start = 0;

    for (const auto metadata : midi)
    {
        const int position = std::clamp (metadata.samplePosition, 0, numSamples);

        if (position > start)
        {
            render (buffer, start, position - start);
            start = position;
        }

        handleMessage (metadata.getMessage());
    }

    if (start < numSamples)
        render (buffer, start, numSamples - start);
}

void TestSynth::handleMessage (const juce::MidiMessage& message)
{
    if (message.isNoteOn())
    {
        startVoice (message);
    }
    else if (message.isNoteOff())
    {
        stopVoice (message.getNoteNumber(), message.getChannel());
    }
    else if (message.isAllNotesOff() || message.isAllSoundOff())
    {
        for (auto& voice : m_voices)
            if (voice.active)
                voice.envelope.noteOff();
    }
}

void TestSynth::startVoice (const juce::MidiMessage& message)
{
    const int pitch = message.getNoteNumber();
    const int channel = message.getChannel();

    Voice* target = nullptr;

    // Gleiche Note erneut: dieselbe Stimme weiterverwenden.
    for (auto& voice : m_voices)
        if (voice.active && voice.pitch == pitch && voice.channel == channel)
            target = &voice;

    // Sonst eine freie Stimme, danach runde Roehrung (Stehlen).
    if (target == nullptr)
    {
        for (auto& voice : m_voices)
            if (! voice.active)
                target = &voice;

        if (target == nullptr)
            target = &m_voices[static_cast<std::size_t> (m_nextVoice++ % kNumVoices)];
    }

    const double frequency = 440.0 * std::pow (2.0, (static_cast<double> (pitch) - 69.0) / 12.0);
    const double cutoff = std::min (frequency * 8.0, m_sampleRate * 0.45);
    const double coefficient = 1.0 - std::exp (-2.0 * juce::MathConstants<double>::pi * cutoff / m_sampleRate);

    target->active = true;
    target->pitch = pitch;
    target->channel = channel;
    target->gain = static_cast<float> (message.getVelocity()) / 127.0f * 0.15f;
    target->phase = 0.0;
    target->increment = frequency / m_sampleRate;
    target->filter = 0.0f;
    target->filterCoefficient = static_cast<float> (coefficient);
    target->envelope.setSampleRate (m_sampleRate);
    target->envelope.setParameters (m_parameters);
    target->envelope.noteOn();
}

void TestSynth::stopVoice (int pitch, int channel)
{
    for (auto& voice : m_voices)
        if (voice.active && voice.pitch == pitch && voice.channel == channel)
            voice.envelope.noteOff();
}

void TestSynth::render (juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (numSamples <= 0)
        return;

    const int numChannels = buffer.getNumChannels();

    for (auto& voice : m_voices)
    {
        if (! voice.active)
            continue;

        for (int i = 0; i < numSamples; ++i)
        {
            const float envelope = voice.envelope.getNextSample();

            // Saegezahn: Phase 0 ... 1, Ausgabe -1 ... 1.
            const auto saw = static_cast<float> (2.0 * voice.phase - 1.0);
            voice.phase += voice.increment;
            if (voice.phase >= 1.0)
                voice.phase -= 1.0;

            voice.filter += voice.filterCoefficient * (saw - voice.filter);
            const float sample = voice.filter * voice.gain * envelope;

            for (int channel = 0; channel < numChannels; ++channel)
                buffer.addSample (channel, startSample + i, sample);
        }

        if (! voice.envelope.isActive())
        {
            voice.active = false;
            voice.pitch = -1;
        }
    }
}

} // namespace heptachord
