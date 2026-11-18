#pragma once

#include "plugin/ParameterBinding.h"
#include "plugin/ProcessorBridge.h"
#include "plugin/TestSynth.h"
#include "plugin/InstrumentHost.h"

#include "core/MidiClock.h"
#include "core/MidiRender.h"

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>
#include <memory>
#include <mutex>

namespace heptachord
{

class Editor;

// Instrument mit stillem Stereo-Ausgang (ADR 0001): kein Audio, Noten raus.
// Die Ereignisschicht liegt im Core (StepEngine); hier werden Parameter,
// Host-Transport und MIDI angebunden. Im Standalone kommt ein Test-Synth
// dazu (Spezifikation Abschnitt 10).
class Processor final : public juce::AudioProcessor,
                        private juce::MidiInputCallback
{
public:
    Processor();
    ~Processor() override;

    void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    // MIDI-Clock-Eingang (juce::MidiInputCallback), nur im Standalone genutzt.
    void handleIncomingMidiMessage (juce::MidiInput* source,
                                    const juce::MidiMessage& message) override;

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

    // MIDI-Clock-Anzeige fuer den Editor (Entscheidung 79).
    bool clockActive() const noexcept;
    double clockBpm() const noexcept;

    // Aktuell abgespielter Step (0 .. 7), -1 wenn keiner (Standalone-Highlight).
    int currentStepIndex() const noexcept { return m_currentStepIndex.load (std::memory_order_relaxed); }

    // Reset-Taster des Editors: loest die Flanke ohne Parameter-Rückschreiben
    // aus (Entscheidung 67).
    void triggerReset() noexcept { m_bridge.requestReset(); }

    // QWERTZ-Mini-Klaviatur des Standalones (nur Plugin-Schicht, ADR 0007).
    // Die Noten landen in einem Puffer und werden im processBlock in den
    // Eingangspuffer gemischt.
    void injectKeyboardNoteOn (int noteNumber, int velocity);
    void injectKeyboardNoteOff (int noteNumber);

    // Preset-Wechsel aus dem Editor: klingende Noten sofort beenden
    // (Spezifikation 8.2/8.3).
    void notifyPresetChanged() { m_bridge.requestAllNotesOff(); }

    // Instrument auswaechlen (nur Standalone). Uebergibt die ownership.
    void setInstrument (std::unique_ptr<Instrument> instrument) noexcept { m_instrument = std::move (instrument); }

    // Aktuelles Instrument (nullptr wenn noch keines geladen).
    Instrument* instrument() const noexcept { return m_instrument.get(); }

    // Export-Kontext (M1/M2, Decision 84): aktuelle Parameter, BPM und
    // letzte Eingangsnote (0 = noch keine gespielt -> C4-Fallback).
    midirender::RenderOptions exportOptions() const noexcept;

    juce::AudioProcessorValueTreeState apvts;

private:
    HostTimeline buildTimeline (int numSamples) noexcept;

    // MIDI-Clock-Eingang schliessen (Destruktor, releaseResources).
    void closeClockInput() noexcept;

    ParameterBinding m_binding {};
    ProcessorBridge m_bridge {};
    std::unique_ptr<Instrument> m_instrument {};

    std::atomic<bool> m_standalonePlaying { true };
    std::atomic<int> m_lastInputNote { 0 };    // 0 = noch keine Note gespielt
    std::atomic<int> m_currentStepIndex { -1 }; // aktuell abgespielter Step (0..7), -1 = keiner
    double m_standalonePpq = 0.0;

    juce::MidiBuffer m_keyboardMidi;
    std::mutex m_keyboardMutex;

    // MIDI-Clock (M6): erster verfuegbarer Eingang, nur im Standalone.
    std::unique_ptr<juce::MidiInput> m_clockInput;
    MidiClock m_clock;
    bool m_clockWasActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Processor)
};

} // namespace heptachord
