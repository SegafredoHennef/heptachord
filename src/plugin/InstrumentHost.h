#pragma once

#include "plugin/TestSynth.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <memory>

namespace heptachord
{

// Schnittstelle fuer einen Klang-Erzeuger, der MIDI-Events empfängt und Audio
// produziert. Der TestSynth und ein geladenes VST-Plugin sind Adapter.
class Instrument
{
public:
    virtual ~Instrument() = default;

    virtual void prepare (double sampleRate) = 0;
    virtual void process (const juce::MidiBuffer& midi, juce::AudioBuffer<float>& buffer) = 0;
    virtual void reset() = 0;

    // Gibt true zurueck, wenn das Instrument ein eigenes Editor-Fenster hat.
    virtual bool hasEditor() const = 0;
    // Oeffnet das Editor-Fenster (nur wenn hasEditor() true).
    virtual void showEditor (juce::Component* parent) = 0;
    // Schliesst das Editor-Fenster.
    virtual void hideEditor() = 0;

    // Gibt true zurueck, wenn das Editor-Fenster gerade sichtbar ist.
    virtual bool isEditorOpen() const { return false; }

    // Anzeigename fuer die UI.
    virtual juce::String getName() const = 0;
};

// Adapter um den eingebauten TestSynth.
class TestSynthInstrument final : public Instrument
{
public:
    TestSynthInstrument();
    ~TestSynthInstrument() override = default;

    void prepare (double sampleRate) override;
    void process (const juce::MidiBuffer& midi, juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    bool hasEditor() const override { return false; }
    void showEditor (juce::Component* parent) override {}
    void hideEditor() override {}
    juce::String getName() const override { return "Interner Synth"; }

private:
    TestSynth m_synth {};
};

// Adapter um ein geladenes VST3-Plugin (nur Standalone).
class VstPluginInstrument final : public Instrument
{
public:
    VstPluginInstrument();
    ~VstPluginInstrument() override;

    void prepare (double sampleRate) override;
    void process (const juce::MidiBuffer& midi, juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    // Laedt ein VST3. sampleRate ist die aktuelle Geraete-Rate (0 = Standard
    // 48 kHz); damit wird das Plugin nicht unnoetig zweimal vorbereitet.
    bool load (const juce::File& file, double sampleRate = 0.0);
    void unload();
    int formatCount() const { return m_formatManager ? m_formatManager->getNumFormats() : 0; }
    juce::String formatNames() const;

    bool hasEditor() const override;
    void showEditor (juce::Component* parent) override;
    void hideEditor() override;
    bool isEditorOpen() const override;
    juce::String getName() const override;

private:
    std::unique_ptr<juce::AudioPluginFormatManager> m_formatManager;
    std::unique_ptr<juce::AudioPluginInstance> m_plugin;
    std::unique_ptr<juce::AudioProcessorEditor> m_editor;
    std::unique_ptr<juce::DocumentWindow> m_editorWindow;
    juce::String m_pluginName;
    double m_sampleRate = 48000.0;
    bool m_prepared = false;
    juce::AudioBuffer<float> m_processBuffer;  // Scratch fuer Mehrkanal-Plugins
};

std::unique_ptr<Instrument> createTestSynthInstrument();
std::unique_ptr<Instrument> createVstPluginInstrument();

// Blocklist fuer VST3s, die im eigenen Code abstuerzen (nicht durch den Host
// verursacht). Gibt den Begruendungstext fuer die UI zurueck; ein leerer
// String bedeutet "nicht blockiert". Geprueft wird der Dateiname, damit die
// DLL gar nicht erst geladen wird.
juce::String blockedPluginReason (const juce::File& file);

// Pfad des Debug-Logs fuer Auto-Test/Diagnose. Geschrieben wird nur, wenn
// HEPTACHORD_AUTO_VST bzw. HEPTACHORD_VST_DEBUG gesetzt ist.
juce::File debugLogFile();

// Diagnose: Peak ueber ALLE Plugin-Kanaele aus dem letzten processBlock.
float vstAllChannelPeak() noexcept;

} // namespace heptachord
