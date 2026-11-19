#pragma once

#include "plugin/ParameterBinding.h"
#include "plugin/ProcessorBridge.h"
#include "plugin/TestSynth.h"
#include "plugin/InstrumentHost.h"

#include <vector>

#include "core/MidiClock.h"
#include "core/MidiRender.h"

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <set>
#include <utility>

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

    // Instrument auswaehlen (nur Standalone). Uebergibt die ownership.
    // Instrument tauschen; das neue wird mit der aktuellen Sample-Rate
    // vorbereitet (sonst haelt ein spaeter geladenes VST3 die Falsche).
    void setInstrument (std::unique_ptr<Instrument> instrument) noexcept;

    // Aktuelles Instrument (nullptr wenn noch keines geladen).
    Instrument* instrument() const noexcept { return m_instrument.get(); }

    // Export-Kontext (M1/M2, Decision 84): aktuelle Parameter, BPM und
    // letzte Eingangsnote (0 = noch keine gespielt -> C4-Fallback).
    midirender::RenderOptions exportOptions() const noexcept;

    // --- MIDI-Rekorder (Standalone) -----------------------------------------
    // Ein aufgenommenes MIDI-Ereignis (Tick in PPQ, 480 Ticks = 1 Viertel).
    struct RecordedMidiEvent
    {
        std::int64_t tick = 0;
        int channel = 1;
        int pitch = 60;
        int velocity = 100;
        bool noteOn = true;
    };

    // Rekorder-Steuerung (audio-thread-sicher ueber Atomics/Mutex).
    void startRecording() noexcept;
    void stopRecording() noexcept;
    void startPlayback() noexcept;
    void stopPlayback() noexcept;
    void clearRecording() noexcept;

    // Ziel der Aufnahme in Zyklen: 0 = unbegrenzt (bis STOP), sonst 1/2/4/8.
    void setRecordingTarget (int cycles) noexcept;
    int recordingTarget() const noexcept { return m_recordingTarget.load (std::memory_order_relaxed); }

    bool isRecording() const noexcept { return m_recorderState.load (std::memory_order_relaxed) == 1; }
    bool isRecordingArmed() const noexcept { return m_recorderState.load (std::memory_order_relaxed) == 3; }
    bool isPlaying() const noexcept { return m_recorderState.load (std::memory_order_relaxed) == 2; }
    bool hasRecording() const noexcept { return m_hasRecording.load (std::memory_order_relaxed); }
    std::int64_t recordingLengthTicks() const noexcept { return m_recordingLength.load (std::memory_order_relaxed); }

    // Fortschritt der laufenden Aufnahme (1-basiert; 0 = keine Aufnahme).
    int recordingCycle() const noexcept;
    int recordingStepsInCycle() const noexcept { return m_stepsPerCycle.load (std::memory_order_relaxed); }
    int recordingStep() const noexcept;

    // Diagnose: Peak des Stereo-Ausgangs seit dem letzten Reset.
    float outputPeak() const noexcept { return m_outputPeak.load (std::memory_order_relaxed); }
    void resetOutputPeak() noexcept { m_outputPeak.store (0.0f, std::memory_order_relaxed); }

    // Aufnahme als Standard-MIDI-Datei speichern (alle Zyklen, Tick 0 = erste Note).
    bool saveRecordingToFile (const juce::File& file);

    juce::AudioProcessorValueTreeState apvts;

private:
    HostTimeline buildTimeline (int numSamples) noexcept;

    // Laufende Aufnahme abschliessen: offene Noten mit Note-Off versehen,
    // Laenge setzen. Muss mit gehaltenem m_recordingMutex aufgerufen werden.
    void finalizeRecordingLocked() noexcept;

    // MIDI-Clock-Eingang schliessen (Destruktor, releaseResources).
    void closeClockInput() noexcept;

    ParameterBinding m_binding {};
    ProcessorBridge m_bridge {};
    std::unique_ptr<Instrument> m_instrument {};
    mutable std::mutex m_instrumentMutex;  // schuetzt Tausch vs. Audio-Thread

    std::atomic<bool> m_standalonePlaying { true };
    std::atomic<int> m_lastInputNote { 0 };    // 0 = noch keine Note gespielt
    std::atomic<int> m_currentStepIndex { -1 }; // aktuell abgespielter Step (0..7), -1 = keiner

    // MIDI-Rekorder (Standalone): ein Take ueber einen oder mehrere Zyklen.
    std::atomic<int> m_recorderState { 0 };   // 0=idle, 3=armed, 1=recording, 2=playing
    std::vector<RecordedMidiEvent> m_recording;
    std::set<std::pair<int, int>> m_activeNotes;  // (Kanal, Pitch) fuer Abschluss-Offs
    std::mutex m_recordingMutex;
    std::atomic<bool> m_hasRecording { false };
    std::int64_t m_recordingStartTick = 0;
    std::atomic<std::int64_t> m_playbackStartTick { -1 };  // -1 = steht am Blockanfang
    std::atomic<std::int64_t> m_recordingLength { 0 };
    std::atomic<int> m_recordingTarget { 0 };  // 0 = unbegrenzt, sonst 1/2/4/8
    std::atomic<std::int64_t> m_armSlot { -1 };        // REC: Warten auf Step-Grenze
    std::int64_t m_recordStartSlot = 0;                // Slot bei Aufnahme-Beginn
    std::atomic<std::int64_t> m_lastSlotIndex { -1 };  // letzter beobachteter Slot
    std::atomic<int> m_recordedSlots { 0 };    // vergangene Step-Slots seit Beginn
    std::atomic<int> m_stepsPerCycle { 0 };    // activeCount bei Aufnahme-Beginn
    double m_standalonePpq = 0.0;
    std::atomic<float> m_outputPeak { 0.0f };

    juce::MidiBuffer m_keyboardMidi;
    std::mutex m_keyboardMutex;

    // MIDI-Clock (M6): erster verfuegbarer Eingang, nur im Standalone.
    std::unique_ptr<juce::MidiInput> m_clockInput;
    MidiClock m_clock;
    bool m_clockWasActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Processor)
};

} // namespace heptachord
