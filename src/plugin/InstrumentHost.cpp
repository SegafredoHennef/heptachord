#include "plugin/InstrumentHost.h"

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace heptachord
{

// -----------------------------------------------------------------------------
// TestSynthInstrument
// -----------------------------------------------------------------------------

TestSynthInstrument::TestSynthInstrument() = default;

void TestSynthInstrument::prepare (double sampleRate)
{
    m_synth.prepare (sampleRate);
}

void TestSynthInstrument::process (const juce::MidiBuffer& midi, juce::AudioBuffer<float>& buffer)
{
    m_synth.process (midi, buffer);
}

void TestSynthInstrument::reset()
{
    m_synth.reset();
}


// -----------------------------------------------------------------------------
// VstPluginInstrument
// -----------------------------------------------------------------------------

VstPluginInstrument::VstPluginInstrument()
{
    m_formatManager = std::make_unique<juce::AudioPluginFormatManager>();
    juce::addDefaultFormatsToManager (*m_formatManager);
}

VstPluginInstrument::~VstPluginInstrument()
{
    unload();
}

void VstPluginInstrument::prepare (double sampleRate)
{
    m_sampleRate = sampleRate;
    if (m_plugin != nullptr)
        m_plugin->prepareToPlay (sampleRate, 512);
}

void VstPluginInstrument::process (const juce::MidiBuffer& midi, juce::AudioBuffer<float>& buffer)
{
    if (m_plugin == nullptr)
        return;

    // VITAL (und andere VST3s) brauchen die MIDI-Eingabe.
    juce::MidiBuffer midiIn (midi);
    m_plugin->processBlock (buffer, midiIn);
}

void VstPluginInstrument::reset()
{
    if (m_plugin != nullptr)
        m_plugin->reset();
}

bool VstPluginInstrument::load (const juce::File& file)
{
    unload();

    juce::AudioPluginFormat* format = nullptr;
    for (int i = 0; i < m_formatManager->getNumFormats(); ++i)
    {
        auto* f = m_formatManager->getFormat (i);
        if (f != nullptr && f->getName() == "VST3")
        {
            format = f;
            break;
        }
    }
    if (format == nullptr)
        return false;

    // Zuerst scannen, um eine korrekte PluginDescription zu erhalten.
    juce::OwnedArray<juce::PluginDescription> descriptions;
    format->findAllTypesForFile (descriptions, file.getFullPathName());

    if (descriptions.isEmpty())
        return false;

    m_plugin = format->createInstanceFromDescription (*descriptions[0], m_sampleRate, 512);

    if (m_plugin == nullptr)
        return false;

    m_plugin->prepareToPlay (m_sampleRate, 512);
    m_pluginName = m_plugin->getName();
    return true;
}

void VstPluginInstrument::unload()
{
    m_editor.reset();
    m_plugin.reset();
    m_pluginName.clear();
}

bool VstPluginInstrument::hasEditor() const
{
    return m_plugin != nullptr;
}

void VstPluginInstrument::showEditor (juce::Component* parent)
{
    if (m_plugin == nullptr || parent == nullptr)
        return;

    if (m_editor == nullptr)
        m_editor.reset (m_plugin->createEditorIfNeeded());

    if (m_editor != nullptr)
    {
        m_editor->setTopLeftPosition (0, 0);
        parent->addChildComponent (m_editor.get());
        m_editor->setVisible (true);
        m_editor->grabKeyboardFocus();
    }
}

void VstPluginInstrument::hideEditor()
{
    if (m_editor != nullptr)
    {
        m_editor->setVisible (false);
        if (m_editor->getParentComponent() != nullptr)
            m_editor->getParentComponent()->removeChildComponent (m_editor.get());
    }
}

juce::String VstPluginInstrument::getName() const
{
    return m_pluginName.isNotEmpty() ? m_pluginName : "VST-Plugin";
}

// -----------------------------------------------------------------------------
// Factory
// -----------------------------------------------------------------------------

std::unique_ptr<Instrument> createTestSynthInstrument()
{
    return std::make_unique<TestSynthInstrument>();
}

std::unique_ptr<Instrument> createVstPluginInstrument()
{
    return std::make_unique<VstPluginInstrument>();
}

} // namespace heptachord
