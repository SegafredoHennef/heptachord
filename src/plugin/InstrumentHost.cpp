#include "plugin/InstrumentHost.h"

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <cmath>
#include <fstream>

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

    float peakBefore = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            peakBefore = std::max (peakBefore, std::abs (buffer.getSample (ch, i)));

    m_plugin->processBlock (buffer, midiIn);

    float peakAfter = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            peakAfter = std::max (peakAfter, std::abs (buffer.getSample (ch, i)));

    // Debug: erste 3 Bloecke immer, dann nur bei MIDI oder Audio.
    static int blockCount = 0;
    static int logCount = 0;
    ++blockCount;
    if (logCount < 40 && (logCount < 3 || midi.getNumEvents() > 0 || peakAfter > 0.0001f))
    {
        std::ofstream dbg ("C:/Users/zichosch.thomas/AppData/Local/Temp/opencode/dbg_audio.txt", std::ios::app);
        dbg << "block=" << blockCount << " midiIn=" << midi.getNumEvents()
            << " peakBefore=" << peakBefore << " peakAfter=" << peakAfter
            << " sr=" << m_plugin->getSampleRate()
            << " bs=" << m_plugin->getBlockSize()
            << " outCh=" << m_plugin->getTotalNumOutputChannels()
            << " channels=" << buffer.getNumChannels()
            << " samples=" << buffer.getNumSamples() << '\n';
        logCount++;
    }
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

    // Ausgabe-Bus aktivieren: VITAL meldet seinen Main-Out nicht als
    // kDefaultActive, JUCE legt den Bus dadurch deaktiviert an (0 Kanaele)
    // und das Plugin bleibt still. Stereo explizit anfordern.
    bool busesLayoutOk = true;
    {
        auto layout = m_plugin->getBusesLayout();

        if (layout.outputBuses.isEmpty())
        {
            busesLayoutOk = false;
        }
        else
        {
            if (layout.outputBuses[0].isDisabled() || layout.outputBuses[0].size() == 0)
                layout.outputBuses.set (0, juce::AudioChannelSet::stereo());

            busesLayoutOk = m_plugin->setBusesLayout (layout);
        }
    }

    m_plugin->prepareToPlay (m_sampleRate, 512);
    m_pluginName = m_plugin->getName();

    // Debug: Zustand nach der Aktivierung.
    {
        std::ofstream dbg ("C:/Users/zichosch.thomas/AppData/Local/Temp/opencode/dbg_audio.txt", std::ios::app);
        dbg << "LOAD " << m_pluginName
            << " reqRate=" << m_sampleRate
            << " sr=" << m_plugin->getSampleRate()
            << " bs=" << m_plugin->getBlockSize()
            << " outCh=" << m_plugin->getTotalNumOutputChannels()
            << " outBuses=" << m_plugin->getBusCount (false)
            << " inBuses=" << m_plugin->getBusCount (true)
            << " layoutOk=" << (busesLayoutOk ? 1 : 0) << '\n';
    }
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

// Fenster mit Titelleiste und Schliessen-Button fuer VST-Editor.
namespace
{
class VstEditorWindow : public juce::DocumentWindow
{
public:
    VstEditorWindow (const juce::String& name, VstPluginInstrument* ownerIn)
        : juce::DocumentWindow (name, juce::Colours::darkgrey,
                                juce::DocumentWindow::allButtons),
          owner (ownerIn)
    {
        setUsingNativeTitleBar (true);
        setResizable (true, true);
    }

    void closeButtonPressed() override
    {
        if (owner != nullptr)
            owner->hideEditor();
    }

    VstPluginInstrument* owner = nullptr;
};
} // namespace

void VstPluginInstrument::showEditor (juce::Component* /*parent*/)
{
    if (m_plugin == nullptr)
        return;

    if (m_editor == nullptr)
        m_editor.reset (m_plugin->createEditorIfNeeded());

    if (m_editor == nullptr)
        return;

    if (m_editorWindow == nullptr)
    {
        auto* window = new VstEditorWindow (m_plugin->getName(), this);
        m_editorWindow.reset (window);
        window->setContentOwned (m_editor.release(), true);
        window->centreWithSize (window->getContentComponent()->getWidth(),
                                window->getContentComponent()->getHeight());
    }

    m_editorWindow->setVisible (true);
    m_editorWindow->toFront (true);
}

void VstPluginInstrument::hideEditor()
{
    if (m_editorWindow != nullptr)
    {
        m_editorWindow->setVisible (false);
        m_editor.reset();  // Window besitzt den Editor (setContentOwned true)
        m_editorWindow.reset();
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
