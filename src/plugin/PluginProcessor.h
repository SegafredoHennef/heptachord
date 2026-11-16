#pragma once

#include "plugin/ParameterBinding.h"
#include "plugin/ProcessorBridge.h"
#include "plugin/TestSynth.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>

namespace heptachord
{

class Editor;

// Instrument mit stillem Stereo-Ausgang (ADR 0001): kein Audio, Noten raus.
// Die Ereignisschicht liegt im Core (StepEngine); hier werden Parameter,
// Host-Transport und MIDI angebunden. Im Standalone kommt ein Test-Synth
// dazu (Spezifikation Abschnitt 10).
class Processor final : public juce::AudioProcessor
{
public:
    Processor();
    ~Processor() override;

    void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Standalone-Transport (Spezifikation 10, Entscheidung 29).
    bool isStandalone() const noexcept { return wrapperType == wrapperType_Standalone; }
    void toggleStandaloneTransport() noexcept;
    bool isStandalonePlaying() const noexcept { return m_standalonePlaying.load (std::memory_order_relaxed); }

    // Reset-Taster des Editors: loest die Flanke ohne Parameter-Rückschreiben
    // aus (Entscheidung 67).
    void triggerReset() noexcept { m_bridge.requestReset(); }

    juce::AudioProcessorValueTreeState apvts;

private:
    HostTimeline buildTimeline (int numSamples) noexcept;

    ParameterBinding m_binding {};
    ProcessorBridge m_bridge {};
    TestSynth m_synth {};

    std::atomic<bool> m_standalonePlaying { true };
    double m_standalonePpq = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Processor)
};

} // namespace heptachord
