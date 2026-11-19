#include "plugin/InstrumentHost.h"

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <cmath>
#include <fstream>
#include <algorithm>

namespace heptachord
{

namespace
{
// Debug-Log nur auf Anforderung (Entwicklung/Diagnose ueber
// HEPTACHORD_VST_DEBUG=1). Im normalen Betrieb wird keine Datei geschrieben.
bool vstDebugEnabled()
{
    static const bool enabled =
        juce::SystemStats::getEnvironmentVariable ("HEPTACHORD_VST_DEBUG", {}).isNotEmpty();
    return enabled;
}

void vstLog (const juce::String& line)
{
    if (! vstDebugEnabled())
        return;

    static const double start = juce::Time::getMillisecondCounterHiRes();
    const double ms = juce::Time::getMillisecondCounterHiRes() - start;
    std::ofstream out (debugLogFile().getFullPathName().toStdString(), std::ios::app);
    out << juce::String (ms, 1).toStdString() << "ms " << line.toStdString() << '\n';
}

struct BlockedPluginEntry
{
    const char* fileName;
    const char* reason;
};

// Plugins, die nachweislich im eigenen Code abstuerzen (nicht durch den Host
// verursacht). Reason Rack Plugin 14.0.0 (Build 8573) schreibt wenige
// processBlock-Aufrufe nach dem Start in ungueltige Kanalpuffer (AV bei
// RVA 0x1701850) und korrumpiert beim Entladen den Heap. Siehe ADR 0009.
constexpr BlockedPluginEntry kBlockedPlugins[] =
{
    { "Reason Rack Plugin.vst3",
      "Reason Rack Plugin (Reason Studios) ist in dieser Version nicht mit "
      "Heptachord kompatibel: Das Plugin stuerzt in seinem eigenen Audio-Code "
      "ab. Bitte ein Update des Herstellers installieren." }
};
} // namespace

juce::String blockedPluginReason (const juce::File& file)
{
    const auto name = file.getFileName();

    for (const auto& entry : kBlockedPlugins)
        if (name.equalsIgnoreCase (entry.fileName))
            return entry.reason;

    return {};
}

juce::File debugLogFile()
{
    return juce::File::getSpecialLocation (juce::File::tempDirectory)
             .getChildFile ("opencode")
             .getChildFile ("dbg_audio.txt");
}

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
    vstLog ("PREP begin sr=" + juce::String (sampleRate, 0));
    m_sampleRate = sampleRate;
    if (m_plugin != nullptr)
    {
        // Nicht mehrfach hintereinander mit identischen Werten vorbereiten:
        // manche Plugins (z.B. Reason Rack) initialisieren ihren Audio-Graph
        // bei jedem prepareToPlay neu und geraten dabei in einen defekten
        // Zustand. Nur bei echter Aenderung der Rate neu vorbereiten.
        if (! m_prepared
            || std::abs (m_plugin->getSampleRate() - sampleRate) > 1.0)
        {
            m_plugin->prepareToPlay (sampleRate, 512);
            m_prepared = true;
        }

        // Scratch-Puffer fuer Plugins mit mehr Kanaelen als der Host-Puffer
        // (Multi-Bus-Instrumente wie Osirus/OsTIrus/Reason Rack). Kein
        // Allokieren im Audio-Thread: hier einmal passend vorbereiten.
        const int needed = std::max ({ 2,
                                       m_plugin->getTotalNumInputChannels(),
                                       m_plugin->getTotalNumOutputChannels() });
        m_processBuffer.setSize (needed, 512, false, false, true);
        vstLog ("PREP scratch ch=" + juce::String (m_processBuffer.getNumChannels()));
    }
    vstLog ("PREP end");
}

void VstPluginInstrument::process (const juce::MidiBuffer& midi, juce::AudioBuffer<float>& buffer)
{
    if (m_plugin == nullptr)
        return;

    // VITAL (und andere VST3s) brauchen die MIDI-Eingabe.
    juce::MidiBuffer midiIn (midi);

    // Ein VST3 mit mehreren Ausgangsbussen erwartet fuer JEDEN Bus einen
    // gueltigen Kanalpuffer. Unser Host-Puffer hat nur Stereo; deshalb in
    // einen Scratch-Puffer mit der vollen Kanalzahl prozessieren und danach
    // die ersten beiden Kanaele zurueckkopieren.
    const int neededChannels = std::max ({ buffer.getNumChannels(),
                                           m_plugin->getTotalNumInputChannels(),
                                           m_plugin->getTotalNumOutputChannels() });

    juce::AudioBuffer<float>* processBuffer = &buffer;

    if (neededChannels > buffer.getNumChannels())
    {
        // Sollte durch prepare() bereits passen; nur im Notfall wachsen.
        if (m_processBuffer.getNumChannels() < neededChannels
            || m_processBuffer.getNumSamples() < buffer.getNumSamples())
            m_processBuffer.setSize (neededChannels, buffer.getNumSamples(), false, false, true);

        // Nur die aktuell anliegende Blockgroesse verarbeiten, nicht die
        // Allokationsgroesse: sonst bekommt das Plugin mehr Samples, als der
        // Host tatsaechlich liefert.
        juce::AudioBuffer<float> view (m_processBuffer.getArrayOfWritePointers(),
                                       neededChannels, buffer.getNumSamples());
        view.clear();

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            view.copyFrom (ch, 0, buffer, ch, 0, buffer.getNumSamples());

        processBuffer = &view;
    }

    m_plugin->processBlock (*processBuffer, midiIn);

    if (processBuffer != &buffer)
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.copyFrom (ch, 0, *processBuffer, ch, 0, buffer.getNumSamples());
}

void VstPluginInstrument::reset()
{
    if (m_plugin != nullptr)
        m_plugin->reset();
}

bool VstPluginInstrument::load (const juce::File& file, double sampleRate)
{
    unload();

    if (const auto reason = blockedPluginReason (file); reason.isNotEmpty())
    {
        vstLog ("LOAD blocked " + file.getFileName() + ": " + reason);
        return false;
    }

    if (sampleRate > 0.0)
        m_sampleRate = sampleRate;

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
    // und das Plugin bleibt still. Stereo explizit anfordern. Weitere Busse
    // bleiben in ihrem Default-Zustand; der Prozess-Puffer wird in process()
    // auf die volle Kanalzahl erweitert.
    bool busesLayoutOk = true;
    {
        auto layout = m_plugin->getBusesLayout();

        if (layout.outputBuses.isEmpty())
        {
            busesLayoutOk = false;
        }
        else
        {
            bool layoutChanged = false;

            if (layout.outputBuses[0].isDisabled() || layout.outputBuses[0].size() == 0)
            {
                layout.outputBuses.set (0, juce::AudioChannelSet::stereo());
                layoutChanged = true;
            }

            // Nur bei echter Aenderung anwenden: ein unnoetiges setBusesLayout
            // reinitialisiert den Audio-Graph des Plugins.
            if (layoutChanged)
                busesLayoutOk = m_plugin->setBusesLayout (layout);

            // Zustand der Busse protokollieren (welche sind aktiv?).
            {
                std::ofstream dbg ("C:/Users/zichosch.thomas/AppData/Local/Temp/opencode/dbg_audio.txt", std::ios::app);
                for (int i = 0; i < layout.outputBuses.size(); ++i)
                    dbg << "BUSOUT " << i << " ch=" << layout.outputBuses[i].size()
                        << " enabled=" << (! layout.outputBuses[i].isDisabled() ? 1 : 0) << '\n';
                for (int i = 0; i < layout.inputBuses.size(); ++i)
                    dbg << "BUSIN " << i << " ch=" << layout.inputBuses[i].size()
                        << " enabled=" << (! layout.inputBuses[i].isDisabled() ? 1 : 0) << '\n';
            }
        }
    }

    m_plugin->prepareToPlay (m_sampleRate, 512);
    m_prepared = true;
    m_pluginName = m_plugin->getName();

    // Debug: Zustand nach der Aktivierung.
    {
        std::ofstream dbg ("C:/Users/zichosch.thomas/AppData/Local/Temp/opencode/dbg_audio.txt", std::ios::app);
        dbg << "LOAD " << m_pluginName
            << " reqRate=" << m_sampleRate
            << " sr=" << m_plugin->getSampleRate()
            << " bs=" << m_plugin->getBlockSize()
            << " outCh=" << m_plugin->getTotalNumOutputChannels()
            << " inCh=" << m_plugin->getTotalNumInputChannels()
            << " outBuses=" << m_plugin->getBusCount (false)
            << " inBuses=" << m_plugin->getBusCount (true)
            << " layoutOk=" << (busesLayoutOk ? 1 : 0) << '\n';
    }
    return true;
}

void VstPluginInstrument::unload()
{
    // Zuerst Fenster und Editor zerstoeren, dann das Plugin — der Editor
    // darf nicht an einem toten AudioProcessor haengen.
    if (m_editorWindow != nullptr)
    {
        m_editorWindow->setVisible (false);
        m_editorWindow.reset();  // zerstoert auch den Editor (setContentOwned)
    }
    m_editor.reset();
    m_plugin.reset();
    m_prepared = false;
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

    // Fenster schon vorhanden (z.B. nach dem Schliessen mit X): nur wieder
    // anzeigen — der Editor bleibt erhalten, damit ein zweites Oeffnen
    // jederzeit funktioniert.
    if (m_editorWindow != nullptr)
    {
        m_editorWindow->setVisible (true);
        m_editorWindow->toFront (true);
        return;
    }

    if (m_editor == nullptr)
        m_editor.reset (m_plugin->createEditorIfNeeded());

    if (m_editor == nullptr)
        return;

    auto* window = new VstEditorWindow (m_plugin->getName(), this);
    m_editorWindow.reset (window);
    window->setContentOwned (m_editor.release(), true);
    window->centreWithSize (window->getContentComponent()->getWidth(),
                            window->getContentComponent()->getHeight());

    m_editorWindow->setVisible (true);
    m_editorWindow->toFront (true);
}

void VstPluginInstrument::hideEditor()
{
    // Nur verstecken — Fenster und Editor bleiben erhalten, damit der
    // EDITOR-Knopf den Editor jederzeit wieder oeffnen kann.
    if (m_editorWindow != nullptr)
        m_editorWindow->setVisible (false);
}

bool VstPluginInstrument::isEditorOpen() const
{
    return m_editorWindow != nullptr && m_editorWindow->isVisible();
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
