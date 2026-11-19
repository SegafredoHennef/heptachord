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

    bool load (const juce::File& file);
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
    double m_sampleRate = 44100.0;
};

std::unique_ptr<Instrument> createTestSynthInstrument();
std::unique_ptr<Instrument> createVstPluginInstrument();

} // namespace heptachord
