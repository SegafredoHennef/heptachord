#pragma once

#include "core/SettingsFromValues.h"
#include "core/StepEngine.h"
#include "core/TimedEvent.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>
#include <vector>

namespace heptachord
{

// Plugin-Schicht: Parameterwerte und Host-Zeit rein, MIDI-Ereignisse raus.
//
// Der Bridge kennt weder APVTS noch AudioPlayHead (das macht der Processor),
// sondern nur das flache Parameter-Array aus `SettingsFromValues.h`. Dadurch
// laesst er sich mit juce::MidiBuffer allein testen (ADR 0007).
class ProcessorBridge
{
public:
    ProcessorBridge();

    void prepare (double sampleRate, int maximumBlockSize);

    // Ein Block. midiIn darf dasselbe Objekt wie midiOut sein: die Eingabe
    // wird zuerst kopiert, dann wird die Ausgabe geleert und gefuellt.
    void process (const ParameterValues& values,
                  const HostTimeline& timeline,
                  const juce::MidiBuffer& midiIn,
                  juce::MidiBuffer& midiOut);

    // Preset-Wechsel, Editor schliessen, Deaktivierung.
    void requestAllNotesOff() noexcept { m_engine.requestAllNotesOff(); }

    // Reset-Taster (Flanke, ohne Parameter-Rückschreiben).
    void requestReset() noexcept { m_engine.requestReset(); }

    // Nur fuer Tests und Diagnose.
    const StepEngine& engine() const noexcept { return m_engine; }
    const EngineSettings& settings() const noexcept { return m_engine.settings(); }

private:
    void applyLayoutChange (const EngineSettings& settings, juce::MidiBuffer& midiOut);
    void collectInput (const juce::MidiBuffer& midiIn);
    void writeOutput (juce::MidiBuffer& midiOut);

    StepEngine m_engine {};
    EventList m_events {};
    std::vector<TimedEvent> m_input {};

    std::array<bool, 16> m_previousChannels {};
    bool m_havePrevious = false;
    bool m_lastReset = false;};

} // namespace heptachord
