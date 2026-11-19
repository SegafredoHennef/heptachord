#include "plugin/PluginEditor.h"

#include "core/NoteNames.h"
#include "core/MidiRender.h"
#include "core/ParameterTable.h"
#include "plugin/ComputerKeyboard.h"
#include "plugin/MidiExport.h"
#include "plugin/AudioExport.h"
#include "plugin/PluginProcessor.h"

#include <functional>
#include <fstream>

namespace heptachord
{
namespace
{
constexpr int kWindowWidth = 1000;
constexpr int kWindowHeight = 664;
constexpr int kMargin = 12;
constexpr int kGap = 8;
constexpr int kHeaderHeight = 42;
constexpr int kGlobalHeight = 98;
constexpr int kVoicesHeight = 214;
constexpr int kPanelPadding = 8;
constexpr int kZoneTitleHeight = 14;

juce::String stepId (int index, const char* suffix)
{
    return "s" + juce::String (index + 1) + "." + suffix;
}

juce::String voiceId (int index, const char* suffix)
{
    return "h" + juce::String (index + 1) + "." + suffix;
}

// Bezeichnungsmodus (Decision 83): baut die Items einer Combo aus
// Anzeige-Labels neu auf und erhaelt die Auswahl (Indices bleiben stabil).
void refreshChoiceItems (juce::ComboBox* box, int count,
                         const std::function<juce::String (int)>& labelFor)
{
    if (box == nullptr)
        return;

    const int selected = box->getSelectedItemIndex();
    box->clear (juce::dontSendNotification);

    for (int i = 0; i < count; ++i)
        box->addItem (labelFor (i), i + 1);

    box->setSelectedItemIndex (selected, juce::dontSendNotification);
}

// Debug-Log fuer den Auto-Test (siehe InstrumentHost.cpp).
void autoLog (const juce::String& line)
{
    static const double start = juce::Time::getMillisecondCounterHiRes();
    const double ms = juce::Time::getMillisecondCounterHiRes() - start;
    std::ofstream out (debugLogFile().getFullPathName().toStdString(), std::ios::app);
    out << juce::String (ms, 0).toStdString() << "ms " << line.toStdString() << '\n';
}

// Nur fuer den Auto-Test: haelt ein geladenes, aber nicht als Instrument
// gesetztes VST am Leben (Kontroll-Experiment).
std::unique_ptr<Instrument> g_autoLoadedOnly;

struct VoiceProbe { const char* name; const char* id; };
constexpr VoiceProbe kChannelProbes[] = {
    { "THRU", "thru.enable" }, { "H1", "h1.enable" },
    { "H2", "h2.enable" }, { "H3", "h3.enable" }
};
} // namespace

Editor::Editor (Processor& processorToEdit)
    : AudioProcessorEditor (&processorToEdit), processor (processorToEdit)
{
    setLookAndFeel (&lookAndFeel);
    standalone = processor.isStandalone();

    // Der Container nimmt alle Controls auf und bleibt in Basis-Koordinaten.
    m_content.setBounds (0, 0, kWindowWidth, kWindowHeight);
    juce::Component::addAndMakeVisible (m_content);

    buildControls();

    factoryPresets = Preset::factoryPresets();

    presetBox = combos.add (new juce::ComboBox());
    presetBox->setJustificationType (juce::Justification::centredLeft);
    presetBox->setTooltip ("Factory-Preset laden");
    presetBox->setVisible (false);  // aus der Kopfzeile entfernt (redundant)

    presetBox->setSelectedItemIndex (-1, juce::dontSendNotification);
    presetBox->onChange = [this] { applySelectedPreset(); };

    // GUI-Zoom in festen Stufen (Standalone + VST3). Nur der Inhalts-
    // Container wird transformiert, die Editor-Groesse waechst linear mit.
    zoomBox = combos.add (new juce::ComboBox());
    zoomBox->setJustificationType (juce::Justification::centredLeft);
    zoomBox->addItem ("100 %", 1);
    zoomBox->addItem ("125 %", 2);
    zoomBox->addItem ("150 %", 3);
    zoomBox->addItem ("175 %", 4);
    zoomBox->addItem ("200 %", 5);
    zoomBox->setSelectedItemIndex (0, juce::dontSendNotification);
    zoomBox->setTooltip ("GUI-Zoom (Standalone + VST3)");
    zoomBox->onChange = [this]
    {
        const int idx = zoomBox->getSelectedItemIndex();
        m_zoomFactor = 1.0f + static_cast<float> (juce::jmax (0, idx)) * 0.25f;

        const int w = juce::roundToInt (static_cast<float> (kWindowWidth) * m_zoomFactor);
        const int h = juce::roundToInt (static_cast<float> (kWindowHeight) * m_zoomFactor);
        if (getWidth() != w || getHeight() != h)
            setSize (w, h);

        resized();
    };
    m_content.addAndMakeVisible (*zoomBox);

    m_content.addAndMakeVisible (resetButton);
    resetButton.setTooltip ("Setzt das Raster am naechsten Step-Rand auf Step 1");
    resetButton.onClick = [this] { processor.triggerReset(); };

    if (standalone)
    {
        m_content.addAndMakeVisible (transportButton);
        transportButton.onClick = [this]
        {
            processor.toggleStandaloneTransport();
            updateTransportButton();
        };

        m_content.addAndMakeVisible (exportButton);
        exportButton.setTooltip ("Einen Zyklus als MIDI-Datei speichern (fester Seed 1)");
        exportButton.onClick = [this] { exportMidi(); };

        m_content.addAndMakeVisible (dragButton);
        dragButton.setTooltip ("MIDI-Datei auf ein anderes Programm ziehen");
        dragButton.onDragStart = [this] { dragMidi(); };

        m_content.addAndMakeVisible (wavButton);
        wavButton.setTooltip ("Einen Zyklus als WAV rendern (Test-Synth)");
        wavButton.onClick = [this] { exportWav(); };

        notesButton.setTooltip ("Notenfolge als MIDI + Text/CSV exportieren");
        notesButton.onClick = [this] { exportNotes(); };
        m_content.addAndMakeVisible (notesButton);

        recButton.setTooltip ("Aufnahme an der naechsten Step-Grenze starten (ersetzt den vorherigen Take)");
        recButton.onClick = [this]
        {
            if (processor.isRecording() || processor.isRecordingArmed())
                processor.stopRecording();
            else
                processor.startRecording();
            updateRecorderButtons();
        };
        m_content.addAndMakeVisible (recButton);

        playButton.setTooltip ("Aufnahme abspielen (ohne Engine-Noten)");
        playButton.onClick = [this] { if (processor.isPlaying()) processor.stopPlayback(); else processor.startPlayback(); updateRecorderButtons(); };
        m_content.addAndMakeVisible (playButton);

        stopButton.setTooltip ("Aufnahme, Wiedergabe und Transport stoppen");
        stopButton.onClick = [this]
        {
            processor.stopRecording();
            processor.stopPlayback();

            // Auch den Transport anhalten - sonst laeuft die Step-Engine
            // weiter und der STOP-Knopf scheint wirkungslos zu sein.
            if (processor.isStandalonePlaying())
                processor.toggleStandaloneTransport();

            updateRecorderButtons();
            updateTransportButton();
            updateRecordingStatus();
        };
        m_content.addAndMakeVisible (stopButton);

        clearButton.setTooltip ("Aufnahme loeschen");
        clearButton.onClick = [this] { processor.clearRecording(); updateRecorderButtons(); updateRecordingStatus(); };
        m_content.addAndMakeVisible (clearButton);

        saveRecordingButton.setTooltip ("Die gesamte Aufnahme (alle Zyklen) als MIDI-Datei speichern");
        saveRecordingButton.onClick = [this] { saveRecording(); };
        m_content.addAndMakeVisible (saveRecordingButton);

        recordingStatusLabel.setFont (juce::FontOptions (12.0f));
        recordingStatusLabel.setColour (juce::Label::textColourId, HeptachordLookAndFeel::accent());
        recordingStatusLabel.setJustificationType (juce::Justification::centredRight);
        m_content.addAndMakeVisible (recordingStatusLabel);

        recordingTargetBox = combos.add (new juce::ComboBox());
        recordingTargetBox->addItem ("Ziel: unbegrenzt", 1);
        recordingTargetBox->addItem ("Ziel: 1 Zyklus", 2);
        recordingTargetBox->addItem ("Ziel: 2 Zyklen", 3);
        recordingTargetBox->addItem ("Ziel: 4 Zyklen", 4);
        recordingTargetBox->addItem ("Ziel: 8 Zyklen", 5);
        recordingTargetBox->setSelectedItemIndex (0, juce::dontSendNotification);
        recordingTargetBox->setTooltip ("Auto-Stop nach dieser Anzahl Zyklen (unbegrenzt = bis STOP)");
        recordingTargetBox->onChange = [this]
        {
            static constexpr int targets[] = { 0, 1, 2, 4, 8 };
            const int index = recordingTargetBox->getSelectedItemIndex();
            processor.setRecordingTarget (index >= 0 && index < 5 ? targets[index] : 0);
            updateRecordingStatus();
        };
        m_content.addAndMakeVisible (recordingTargetBox);

        updateRecorderButtons();
        updateRecordingStatus();

        instrumentBox = combos.add (new juce::ComboBox());
        instrumentBox->setJustificationType (juce::Justification::centredLeft);
        instrumentBox->setTooltip ("Aktives Instrument (Standalone)");
        instrumentBox->addItem ("Interner Synth", 1);
        instrumentBox->setSelectedItemIndex (0, juce::dontSendNotification);
        instrumentBox->onChange = [this] { onInstrumentSelected (instrumentBox->getSelectedItemIndex()); };
        m_content.addAndMakeVisible (instrumentBox);

        loadButton.setTooltip ("VST3-Plugin als Instrument laden (Standalone)");
        loadButton.onClick = [this] { loadInstrument(); };
        m_content.addAndMakeVisible (loadButton);

        editButton.setTooltip ("Plugin-Editor-Fenster oeffnen/schliessen");
        editButton.onClick = [this]
        {
            auto* inst = processor.instrument();
            if (inst == nullptr || ! inst->hasEditor())
                return;

            if (inst->isEditorOpen())
                inst->hideEditor();
            else
                showInstrumentEditor();
        };
        editButton.setEnabled (false);
        m_content.addAndMakeVisible (editButton);

        startTimerHz (30);
        setWantsKeyboardFocus (true);
    }

    // Bezeichnungsmodus (Decision 83): Listener registrieren und die
    // Anzeige-Labels einmal aufbauen (Default ist Deutsch).
    processor.apvts.addParameterListener ("names.mode", this);
    refreshNameLabels();

    // Auto-Test (nur wenn HEPTACHORD_AUTO_VST gesetzt ist): VST3 laden,
    // Editor oeffnen/schliessen/neu oeffnen, zum internen Synth wechseln
    // und zurueck — fuer automatisierte Tests ohne GUI-Bedienung.
    if (standalone)
    {
        auto requestQuit = []
        {
            if (auto* app = juce::JUCEApplication::getInstance())
                app->quit();
        };

        // Kanal-Diagnose: HEPTACHORD_AUTO_CHANNELS=<VST-Pfad>. Nacheinander
        // wird nur eine Stimme aktiviert und der Audio-Peak gemessen, um zu
        // pruefen, ob das Plugin auf Kanal 2/3/4 (H1/H2/H3) reagiert.
        const auto autoChannels = juce::SystemStats::getEnvironmentVariable ("HEPTACHORD_AUTO_CHANNELS", {});

        if (autoChannels.isNotEmpty() && loadInstrumentFromFile (juce::File (autoChannels)))
        {
            for (std::size_t v = 0; v < 4; ++v)
            {
                const int delay = 800 + static_cast<int> (v) * 2000;

                juce::Timer::callAfterDelay (delay, [this, v]
                {
                    for (const auto& p : kChannelProbes)
                        processor.apvts.getParameter (p.id)->setValueNotifyingHost (
                            p.id == kChannelProbes[v].id ? 1.0f : 0.0f);

                    processor.resetOutputPeak();
                    processor.injectKeyboardNoteOn (60, 100);
                    autoLog ("CHAN probe " + juce::String (kChannelProbes[v].name));
                });

                juce::Timer::callAfterDelay (delay + 1700, [this]
                {
                    autoLog ("CHAN peak=" + juce::String (processor.outputPeak(), 4));
                    processor.injectKeyboardNoteOff (60);
                });
            }

            juce::Timer::callAfterDelay (800 + 4 * 2000, [this, requestQuit]
            {
                for (const auto& id : { "thru.enable", "h1.enable", "h2.enable", "h3.enable" })
                    processor.apvts.getParameter (id)->setValueNotifyingHost (1.0f);
                autoLog ("CHAN done");
                requestQuit();
            });

            return;
        }

        // Recorder-Auto-Test: HEPTACHORD_AUTO_RECORDER=<Zielzyklen, 0 = frei>.
        const auto autoRecorder = juce::SystemStats::getEnvironmentVariable ("HEPTACHORD_AUTO_RECORDER", {});

        if (autoRecorder.isNotEmpty())
        {
            const int targetCycles = juce::String (autoRecorder).getIntValue();
            processor.setRecordingTarget (targetCycles > 0 ? targetCycles : 0);

            juce::Timer::callAfterDelay (500, [this]
            {
                processor.startRecording();
                processor.injectKeyboardNoteOn (60, 100);
                autoLog ("AUTO: recorder started");
            });

            juce::Timer::callAfterDelay (5400, [this]
            {
                processor.injectKeyboardNoteOff (60);
            });

            juce::Timer::callAfterDelay (5000, [this]
            {
                processor.stopRecording();
                autoLog ("AUTO: recorder stopped cycle=" + juce::String (processor.recordingCycle())
                         + " per=" + juce::String (processor.recordingStepsInCycle())
                         + " target=" + juce::String (processor.recordingTarget()));
            });

            juce::Timer::callAfterDelay (5500, [this, requestQuit]
            {
                const auto file = debugLogFile().getParentDirectory().getChildFile ("recorder_test.mid");
                file.deleteFile();
                const bool ok = processor.saveRecordingToFile (file);
                autoLog ("AUTO: recorder saved=" + juce::String ((int) ok)
                         + " bytes=" + juce::String ((int) file.getSize()));
                requestQuit();
            });

            return;
        }

        const auto autoVstPath = juce::SystemStats::getEnvironmentVariable ("HEPTACHORD_AUTO_VST", {});

        if (autoVstPath.isNotEmpty())
        {
            juce::Timer::callAfterDelay (1500, [this, autoVstPath]
            {
                auto quitApp = []
                {
                    if (auto* app = juce::JUCEApplication::getInstance())
                        app->quit();
                };

                const bool noActivate = juce::SystemStats::getEnvironmentVariable ("HEPTACHORD_AUTO_NOACTIVATE", {}).isNotEmpty();

                if (noActivate)
                {
                    auto inst = createVstPluginInstrument();
                    auto* vstInst = static_cast<VstPluginInstrument*> (inst.get());

                    if (! vstInst->load (juce::File (autoVstPath), processor.getSampleRate()))
                    {
                        autoLog ("AUTO: load failed");
                        quitApp();
                        return;
                    }

                    g_autoLoadedOnly = std::move (inst);
                    autoLog ("AUTO: loaded (nicht aktiv) name=" + g_autoLoadedOnly->getName());
                }
                else
                {
                    if (! loadInstrumentFromFile (juce::File (autoVstPath)))
                    {
                        const juce::File file (autoVstPath);
                        const auto reason = blockedPluginReason (file);
                        autoLog (reason.isNotEmpty()
                                     ? "AUTO: blocked name=" + file.getFileName()
                                     : juce::String ("AUTO: load failed"));
                        quitApp();
                        return;
                    }
                    autoLog ("AUTO: loaded name=" + processor.instrument()->getName());
                }

                const bool skipEditor = juce::SystemStats::getEnvironmentVariable ("HEPTACHORD_AUTO_NOEDITOR", {}).isNotEmpty();
                const bool noSwitch = juce::SystemStats::getEnvironmentVariable ("HEPTACHORD_AUTO_NOSWITCH", {}).isNotEmpty();
                const bool noNotes = juce::SystemStats::getEnvironmentVariable ("HEPTACHORD_AUTO_NONOTES", {}).isNotEmpty();

                if (! skipEditor)
                {
                    juce::Timer::callAfterDelay (1000, [this]
                    {
                        showInstrumentEditor();
                        autoLog ("AUTO: editor open1=" + juce::String ((int) processor.instrument()->isEditorOpen()));
                    });

                    juce::Timer::callAfterDelay (1500, [this]
                    {
                        processor.instrument()->hideEditor();
                        autoLog ("AUTO: editor hidden=" + juce::String ((int) processor.instrument()->isEditorOpen()));
                    });

                    juce::Timer::callAfterDelay (2000, [this]
                    {
                        showInstrumentEditor();
                        autoLog ("AUTO: editor open2=" + juce::String ((int) processor.instrument()->isEditorOpen()));
                    });
                }

                if (! noSwitch)
                {
                    juce::Timer::callAfterDelay (3000, [this]
                    {
                        onInstrumentSelected (0);
                        autoLog ("AUTO: switched to internal name=" + processor.instrument()->getName());
                    });
                }

                if (! noNotes)
                {
                    juce::Timer::callAfterDelay (3500, [this]
                    {
                        processor.injectKeyboardNoteOn (60, 100);
                        autoLog ("AUTO: note on internal");
                    });
                }

                juce::Timer::callAfterDelay (6000, [this]
                {
                    processor.injectKeyboardNoteOff (60);
                });

                if (! noActivate)
                {
                    if (! noSwitch)
                    {
                        juce::Timer::callAfterDelay (7000, [this]
                        {
                            onInstrumentSelected (1);
                            autoLog ("AUTO: switched back name=" + processor.instrument()->getName());
                        });
                    }

                    if (! noNotes)
                    {
                        juce::Timer::callAfterDelay (7500, [this]
                        {
                            processor.injectKeyboardNoteOn (62, 100);
                            autoLog ("AUTO: note on vst");
                        });
                    }
                }

                juce::Timer::callAfterDelay (10000, [this]
                {
                    processor.injectKeyboardNoteOff (62);
                });

                juce::Timer::callAfterDelay (11000, [this, quitApp]
                {
                    autoLog ("AUTO: done");
                    if (processor.instrument() != nullptr)
                        processor.instrument()->hideEditor();
                    autoLog ("AUTO: editor closed before quit");
                    quitApp();
                });
            });
        }
    }

    setSize (kWindowWidth, kWindowHeight);
}

Editor::~Editor()
{
    processor.apvts.removeParameterListener ("names.mode", this);

    if (standalone)
        for (const auto& entry : heldNotes)
            processor.injectKeyboardNoteOff (entry.second);

    setLookAndFeel (nullptr);
}

juce::Slider& Editor::addKnob (const juce::String& paramId, const juce::String& caption)
{
    auto* slider = sliders.add (new juce::Slider());
    slider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 48, 13);
    slider->setPopupDisplayEnabled (false, false, this);
    m_content.addAndMakeVisible (slider);

    sliderAttachments.push_back (
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processor.apvts, paramId, *slider));

    addTooltipFromTable (*slider, paramId);

    if (caption.isNotEmpty())
        addCaptionAnchor (*slider, caption);

    return *slider;
}

juce::ComboBox& Editor::addChoice (const juce::String& paramId, const juce::String& caption)
{
    auto* box = combos.add (new juce::ComboBox());
    box->setJustificationType (juce::Justification::centredLeft);
    m_content.addAndMakeVisible (box);

    comboAttachments.push_back (
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.apvts, paramId, *box));

    addTooltipFromTable (*box, paramId);

    if (caption.isNotEmpty())
        addCaptionAnchor (*box, caption);

    return *box;
}

juce::ToggleButton& Editor::addToggle (const juce::String& paramId, const juce::String& caption)
{
    auto* button = toggles.add (new juce::ToggleButton());
    button->setButtonText (caption);
    m_content.addAndMakeVisible (button);

    buttonAttachments.push_back (
        std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            processor.apvts, paramId, *button));

    addTooltipFromTable (*button, paramId);

    return *button;
}

void Editor::addTooltipFromTable (juce::SettableTooltipClient& component, const juce::String& paramId)
{
    const int index = params::indexOf (paramId.toRawUTF8());

    if (index < 0)
        return;

    const char* note = params::kParams[static_cast<std::size_t> (index)].note;

    if (note != nullptr && *note != '\0')
        component.setTooltip (juce::String::fromUTF8 (note));
}

void Editor::addCaptionAnchor (juce::Component& anchor, const juce::String& text)
{
    controlCaptions.push_back ({ &anchor, text });
}

void Editor::buildControls()
{
    rootBox = &addChoice ("root", "Grundton");
    scaleBox = &addChoice ("scale", "Skala");
    rateBox = &addChoice ("rate", "Rate");
    for (int i = 0; i < 17; ++i)
        rateBox->addItem (juce::String (params::kChoices2[i]), i + 1);
    rateBox->setSelectedItemIndex (2, juce::dontSendNotification);

    outputBox = &addChoice ("output.mode", "Ausgabe");
    outputBox->addItem ("Multi", 1);
    outputBox->addItem ("Single", 2);
    outputBox->setSelectedItemIndex (0, juce::dontSendNotification);

    directionBox = &addChoice ("direction", "Richtung");
    // ComboBox manuell befuellen (AudioParameterChoice-Attachment populated
    // die Items nicht automatisch).
    for (int i = 0; i < 6; ++i)
        // kChoices3 enthaelt Umlaute ("Vorwärts"/"Rückwärts"): der
        // juce::String-char*-Konstruktor nimmt ASCII an und wuerde UTF-8
        // zu "VorwÃ¤rts" verzerren - daher explizit als UTF-8 dekodieren.
        directionBox->addItem (juce::String::fromUTF8 (params::kChoices3[i]), i + 1);
    directionBox->setSelectedItemIndex (
        static_cast<int> (processor.apvts.getRawParameterValue ("direction")->load ()),
        juce::dontSendNotification);
    swingKnob = &addKnob ("swing", "Swing");
    slackKnob = &addKnob ("slack", "Slack");
    harmVelocityKnob = &addKnob ("harm.velocity", "Vel H1-H3");
    harmSpreadKnob = &addKnob ("harm.spread", "Spread");
    tempoKnob = &addKnob ("tempo", "Tempo");
    latchButton = &addToggle ("latch", "LATCH");

    // Kopfzeile: Bezeichnungsmodus ohne Beschriftung (Decision 83).
    namesBox = &addChoice ("names.mode", "");
    namesBox->setVisible (false);  // aus der Kopfzeile entfernt (redundant)

    thruEnableButton = &addToggle ("thru.enable", "AKTIV");
    thruSnapButton = &addToggle ("thru.snap", "SNAP");
    thruVelocityKnob = &addKnob ("thru.velocity", "");
    thruChannelKnob = &addKnob ("thru.channel", "");

    for (int voice = 0; voice < 3; ++voice)
    {
        auto& row = voices[static_cast<std::size_t> (voice)];

        row.enable = &addToggle (voiceId (voice, "enable"), "AKTIV");
        row.degree = &addKnob (voiceId (voice, "degree"), "");
        row.octave = &addKnob (voiceId (voice, "octave"), "");
        row.velocity = &addKnob (voiceId (voice, "velocity"), "");
        row.follow = &addToggle (voiceId (voice, "follow"), "FOLGT");
        row.gate = &addKnob (voiceId (voice, "gate"), "");
        row.channel = &addKnob (voiceId (voice, "channel"), "");
    }

    const char* maskNames[4] = { "THR", "H1", "H2", "H3" };

    for (int index = 0; index < 8; ++index)
    {
        auto& column = steps[static_cast<std::size_t> (index)];

        column.active = &addToggle (stepId (index, "active"), "AN");
        column.mode = &addChoice (stepId (index, "mode"), "");
        column.offset = &addKnob (stepId (index, "offset"), "Offset");
        column.velocity = &addKnob (stepId (index, "velocity"), "Velocity");
        column.probability = &addKnob (stepId (index, "probability"), "Wahrsch.");
        column.ratchet = &addKnob (stepId (index, "ratchet"), "Ratchet");

        for (int voice = 0; voice < 4; ++voice)
        {
            const char* suffix = voice == 0 ? "v1" : voice == 1 ? "v2" : voice == 2 ? "v3" : "v4";
            column.mask[static_cast<std::size_t> (voice)] =
                &addToggle (stepId (index, suffix), maskNames[voice]);
        }
    }
}

bool Editor::keyPressed (const juce::KeyPress& key)
{
    if (standalone)
    {
        const int code = key.getKeyCode();
        const auto action = mapKey (code, keyboardOctave);

        // Wiederholte Key-Down-Events beim Halten einer Taste ignorieren.
        if (action.kind != KeyAction::Kind::none && pressedKeys.find (code) == pressedKeys.end())
        {
            pressedKeys.insert (code);

            switch (action.kind)
            {
                case KeyAction::Kind::note:
                    heldNotes[code] = action.note;
                    processor.injectKeyboardNoteOn (action.note, 100);
                    return true;

                case KeyAction::Kind::octaveDown:
                    keyboardOctave = juce::jmax (kMinOctave, keyboardOctave - 1);
                    return true;

                case KeyAction::Kind::octaveUp:
                    keyboardOctave = juce::jmin (kMaxOctave, keyboardOctave + 1);
                    return true;

                case KeyAction::Kind::transport:
                    processor.toggleStandaloneTransport();
                    updateTransportButton();
                    return true;

                case KeyAction::Kind::none:
                    break;
            }
        }
    }

    return juce::AudioProcessorEditor::keyPressed (key);
}

bool Editor::keyStateChanged (bool isKeyDown)
{
    if (! standalone || isKeyDown)
        return juce::AudioProcessorEditor::keyStateChanged (isKeyDown);

    // Beim Loslassen kann juce nicht sagen, welche Taste es war; daher alle
    // gehaltenen Tasten pruefen, ob sie physisch noch gedrueckt sind.
    bool used = false;

    for (auto it = pressedKeys.begin(); it != pressedKeys.end();)
    {
        const int code = *it;

        if (juce::KeyPress::isKeyCurrentlyDown (code))
        {
            ++it;
            continue;
        }

        if (const auto found = heldNotes.find (code); found != heldNotes.end())
        {
            processor.injectKeyboardNoteOff (found->second);
            heldNotes.erase (found);
        }

        it = pressedKeys.erase (it);
        used = true;
    }

    return used;
}

void Editor::focusLost (FocusChangeType)
{
    if (! standalone)
        return;

    for (const auto& entry : heldNotes)
        processor.injectKeyboardNoteOff (entry.second);

    heldNotes.clear();
    pressedKeys.clear();
}

void Editor::timerCallback()
{
    if (! standalone)
        return;

    const int current = processor.currentStepIndex();

    if (current != m_highlightedStep)
    {
        m_highlightedStep = current;

        HeptachordLookAndFeel::setCurrentStepActiveButton (
            (current >= 0 && current < 8) ? steps[static_cast<std::size_t> (current)].active : nullptr);

        m_content.repaint();
    }

    if (standalone)
    {
        updateRecorderButtons();
        updateRecordingStatus();

        // Rueckmeldung am EDITOR-Knopf: offenes Plugin-Fenster = markiert,
        // und der Knopf schliesst es dann wieder.
        auto* inst = processor.instrument();
        const bool editorOpen = inst != nullptr && inst->isEditorOpen();
        const juce::String text = editorOpen ? "EDITOR ●" : "EDITOR";

        if (editButton.getButtonText() != text)
        {
            editButton.setButtonText (text);
            editButton.setColour (juce::TextButton::buttonColourId,
                editorOpen ? HeptachordLookAndFeel::accent()
                           : HeptachordLookAndFeel::panelRaised());
        }
    }
}

void Editor::updateTransportButton()
{
    if (! standalone)
        return;

    const bool playing = processor.isStandalonePlaying();
    transportButton.setButtonText (playing ? "STOP" : "START");
    transportButton.setToggleState (playing, juce::dontSendNotification);
}

void Editor::applySelectedPreset()
{
    const int index = presetBox->getSelectedItemIndex();

    if (index < 0 || index >= static_cast<int> (factoryPresets.size()))
        return;

    applyPreset (factoryPresets[static_cast<std::size_t> (index)]);
}

void Editor::applyPreset (const Preset& preset)
{
    const auto normalized = preset.normalizedValues();

    for (int i = 0; i < params::kNumParameters; ++i)
    {
        if (i == params::id_reset) // Trigger wird nicht gespeichert (Nr. 67).
            continue;

        if (i == params::id_names_mode) // Anzeige-Modus bleibt erhalten (Decision 83).
            continue;

        const auto& p = params::kParams[static_cast<std::size_t> (i)];
        auto* parameter = processor.apvts.getParameter (p.id);

        if (parameter == nullptr)
            continue;

        parameter->setValueNotifyingHost (normalized[static_cast<std::size_t> (i)]);
    }

    // Preset-Wechsel beendet klingende Noten sofort (Spezifikation 8.2/8.3).
    processor.notifyPresetChanged();
}

void Editor::loadInstrument()
{
    // Voreingestellter Ordner: der systemweite VST3-Standardpfad. Falls er
    // nicht existiert, als Rueckfall der Desktop.
    juce::File vst3Folder ("C:/Program Files/Common Files/VST3");
    if (! vst3Folder.isDirectory())
        vst3Folder = juce::File::getSpecialLocation (juce::File::userDesktopDirectory);

    instrumentFileChooser = std::make_unique<juce::FileChooser> (
        "VST3-Plugin als Instrument laden",
        vst3Folder,
        "*.vst3");

    instrumentFileChooser->launchAsync (
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            if (file == juce::File {})
                return;

            if (! loadInstrumentFromFile (file))
            {
                const auto reason = blockedPluginReason (file);

                juce::NativeMessageBox::showMessageBoxAsync (
                    juce::MessageBoxIconType::WarningIcon, "Heptachord",
                    reason.isNotEmpty()
                        ? reason
                        : juce::String ("Das VST3-Plugin konnte nicht geladen werden."),
                    this);
            }
        });
}

bool Editor::loadInstrumentFromFile (const juce::File& file)
{
    auto inst = createVstPluginInstrument();
    auto* vstInst = static_cast<VstPluginInstrument*> (inst.get());

    if (! vstInst->load (file, processor.getSampleRate()))
        return false;

    processor.setInstrument (std::move (inst));

    // Eintrag in der ComboBox: schon vorhanden? Dann nur auswaehlen,
    // sonst neuen Eintrag anhaengen (Box-Index = Datei-Index + 1).
    int fileIndex = m_instrumentFiles.indexOf (file);
    if (fileIndex < 0)
    {
        m_instrumentFiles.add (file);
        fileIndex = m_instrumentFiles.size() - 1;
        instrumentBox->addItem (processor.instrument()->getName(),
                                instrumentBox->getNumItems() + 1);
    }

    instrumentBox->setSelectedItemIndex (fileIndex + 1, juce::dontSendNotification);
    editButton.setEnabled (processor.instrument()->hasEditor());
    return true;
}

void Editor::onInstrumentSelected (int boxIndex)
{
    if (boxIndex < 0)
        return;

    // 0 = interner Synth.
    if (boxIndex == 0)
    {
        processor.setInstrument (createTestSynthInstrument());
        editButton.setEnabled (false);
        return;
    }

    const int fileIndex = boxIndex - 1;
    if (fileIndex < 0 || fileIndex >= m_instrumentFiles.size())
        return;

    const juce::File file = m_instrumentFiles[fileIndex];

    if (loadInstrumentFromFile (file))
        return;

    // Laden fehlgeschlagen: zurueck auf den internen Synth.
    instrumentBox->setSelectedItemIndex (0, juce::dontSendNotification);
    processor.setInstrument (createTestSynthInstrument());
    editButton.setEnabled (false);

    const auto reason = blockedPluginReason (file);
    juce::NativeMessageBox::showMessageBoxAsync (
        juce::MessageBoxIconType::WarningIcon, "Heptachord",
        reason.isNotEmpty()
            ? reason
            : juce::String ("Das VST3-Plugin konnte nicht geladen werden."),
        this);
}

void Editor::showInstrumentEditor()
{
    if (processor.instrument() == nullptr)
        return;

    auto* inst = processor.instrument();
    if (! inst->hasEditor())
        return;

    inst->showEditor (this);
}

void Editor::updateRecorderButtons()
{
    const bool recording = processor.isRecording();
    const bool armed = processor.isRecordingArmed();
    const bool playing = processor.isPlaying();
    const bool hasRec = processor.hasRecording();

    recButton.setButtonText (recording ? "REC ●" : armed ? "REC …" : "REC");
    playButton.setButtonText (playing ? "PLAY ▶" : "PLAY");

    // Farbliche Rueckmeldung: REC rot bei Aufnahme, PLAY gruen bei Wiedergabe.
    recButton.setColour (juce::TextButton::buttonColourId,
        recording || armed ? juce::Colour (0xffcc3333) : HeptachordLookAndFeel::panelRaised());
    recButton.setColour (juce::TextButton::buttonOnColourId,
        recording || armed ? juce::Colour (0xffcc3333) : HeptachordLookAndFeel::accent());
    playButton.setColour (juce::TextButton::buttonColourId,
        playing ? juce::Colour (0xff33aa55) : HeptachordLookAndFeel::panelRaised());
    playButton.setColour (juce::TextButton::buttonOnColourId,
        playing ? juce::Colour (0xff33aa55) : HeptachordLookAndFeel::accent());

    recButton.setEnabled (! playing);
    playButton.setEnabled (hasRec && ! recording && ! armed);
    stopButton.setEnabled (true);  // immer verfuegbar: stoppt auch den Transport
    clearButton.setEnabled (hasRec || recording || armed);
    saveRecordingButton.setEnabled (hasRec);

    recButton.repaint();
    playButton.repaint();
    stopButton.repaint();
    clearButton.repaint();
    saveRecordingButton.repaint();
}

void Editor::updateRecordingStatus()
{
    if (! standalone)
        return;

    juce::String text;

    if (processor.isRecording() || processor.hasRecording())
    {
        const int cycle = processor.recordingCycle();
        const int step = processor.recordingStep();
        const int per = processor.recordingStepsInCycle();
        const int target = processor.recordingTarget();

        if (cycle > 0 && per > 0)
        {
            // Am Zielpunkt ist der naechste Zyklus gerade begonnen; die
            // Anzeige bleibt beim erreichten Ziel statt "3/2" zu zeigen.
            const int shown = target > 0 ? juce::jmin (cycle, target) : cycle;
            text = "Zyklus " + juce::String (shown);
            if (target > 0)
                text += "/" + juce::String (target);
            text += " · Step " + juce::String (step) + "/" + juce::String (per);
        }
    }
    else if (processor.isRecordingArmed())
    {
        text = "REC bereit …";
    }

    recordingStatusLabel.setText (text, juce::dontSendNotification);
}

void Editor::saveRecording()
{
    if (! processor.hasRecording())
        return;

    const juce::String stamp = juce::Time::getCurrentTime().formatted ("%Y%m%d_%H%M%S");

    fileChooser = std::make_unique<juce::FileChooser> (
        "Aufnahme speichern",
        juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
            .getChildFile ("Heptachord_Aufnahme_" + stamp + ".mid"),
        "*.mid");

    fileChooser->launchAsync (
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            if (file == juce::File {})
                return;

            if (! file.hasFileExtension (".mid"))
                file = file.withFileExtension (".mid");

            if (! processor.saveRecordingToFile (file))
                juce::NativeMessageBox::showMessageBoxAsync (
                    juce::MessageBoxIconType::WarningIcon, "Heptachord",
                    "Die Aufnahme konnte nicht gespeichert werden.", this);
        });
}

void Editor::exportNotes()
{
    const auto options = processor.exportOptions();

    // Ein Speichern-Dialog fuer beide Dateien: die gewaehlte Basis ergibt
    // <basis>.mid und <basis>.txt (NOTEN = MIDI + Text).
    const juce::String suggested =
        juce::File (MidiExport::suggestedFileName (options)).getFileNameWithoutExtension() + ".txt";

    fileChooser = std::make_unique<juce::FileChooser> (
        "Notenfolge als MIDI und Text speichern",
        juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
            .getChildFile (suggested),
        "*.txt");

    fileChooser->launchAsync (
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this, options] (const juce::FileChooser& chooser)
        {
            const auto file = chooser.getResult();
            if (file == juce::File {})
                return;

            const juce::File folder = file.getParentDirectory();
            const juce::String base = file.getFileNameWithoutExtension();
            const juce::File midiFile = folder.getChildFile (base + ".mid");
            const juce::File txtFile = folder.getChildFile (base + ".txt");

            if (! MidiExport::saveToFile (midiFile, options))
            {
                juce::NativeMessageBox::showMessageBoxAsync (
                    juce::MessageBoxIconType::WarningIcon, "Heptachord",
                    "Die MIDI-Datei konnte nicht gespeichert werden.", this);
                return;
            }

            const auto notes = midirender::renderCycle (options);

            juce::String csv;
            csv += "# Heptachord - Noten-Export\n";
            csv += "# BPM: " + juce::String (options.bpm, 1) + ", Rate: "
                 + juce::String (options.settings.rate, 3) + ", Seed: 1\n";
            csv += "Step,Tick,Channel,Pitch,Velocity,Event\n";

            for (const auto& note : notes)
            {
                csv += juce::String (note.step) + ","
                     + juce::String (note.tick) + ","
                     + juce::String (note.channel) + ","
                     + juce::String (note.pitch) + ","
                     + juce::String (note.velocity) + ","
                     + juce::String (note.noteOn ? "On" : "Off") + "\n";
            }

            if (! txtFile.replaceWithText (csv))
            {
                juce::NativeMessageBox::showMessageBoxAsync (
                    juce::MessageBoxIconType::WarningIcon, "Heptachord",
                    "Die Text-Datei konnte nicht gespeichert werden.", this);
                return;
            }

            lastExportedFile = midiFile;
        });
}

void Editor::exportWav()
{
    const auto options = processor.exportOptions();

    fileChooser = std::make_unique<juce::FileChooser> (
        "WAV-Zyklus speichern",
        juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
            .getChildFile (AudioExport::suggestedFileName (options)),
        "*.wav");

    fileChooser->launchAsync (
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this, options] (const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            if (file == juce::File {})
                return;

            if (file.getFileExtension().isEmpty())
                file = file.withFileExtension ("wav");

            lastExportedFile = file;

            if (! AudioExport::saveWavFile (file, options))
                juce::NativeMessageBox::showMessageBoxAsync (
                    juce::MessageBoxIconType::WarningIcon, "Heptachord",
                    "Die WAV-Datei konnte nicht gespeichert werden.", this);
        });
}

void Editor::exportMidi()
{
    const auto options = processor.exportOptions();
    // FileChooser muss als Member leben, bis der Callback zurueckkommt.
    fileChooser = std::make_unique<juce::FileChooser> (
        "MIDI-Zyklus speichern",
        juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
            .getChildFile (MidiExport::suggestedFileName (options)),
        "*.mid;*.midi");

    fileChooser->launchAsync (
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this, options] (const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            if (file == juce::File {})
                return;

            if (file.getFileExtension().isEmpty())
                file = file.withFileExtension ("mid");

            lastExportedFile = file;

            if (! MidiExport::saveToFile (file, options))
                juce::NativeMessageBox::showMessageBoxAsync (
                    juce::MessageBoxIconType::WarningIcon, "Heptachord",
                    "Die MIDI-Datei konnte nicht gespeichert werden.", this);
        });
}

void Editor::dragMidi()
{
    // Temporaere Datei schreiben, dann den nativen Drag-Vorgang starten.
    // juce::Component::startDragging muss aus einem Maus-Event kommen: der
    // Knopf ist ein Toggle, der onClick dreht - wir starten den Drag hier
    // ueber das Quell-Component und den letzten Mausdruck.
    const auto file = MidiExport::writeTempFile (processor.exportOptions());

    if (! file.existsAsFile())
        return;

    lastExportedFile = file;

    juce::Array<juce::var> files;
    files.add (file.getFullPathName());

    // Der Editor ist selbst der DragAndDropContainer (siehe Header).
    startDragging (juce::var (files), &dragButton);
}

int Editor::currentNameMode() const
{
    auto* parameter = processor.apvts.getParameter ("names.mode");

    if (parameter == nullptr)
        return notenames::kNameModeDeutsch;

    return juce::roundToInt (parameter->convertFrom0to1 (parameter->getValue()));
}

juce::String Editor::presetDisplayName (const Preset& preset) const
{
    const auto& values = preset.getValues();
    const int root = static_cast<int> (values[static_cast<std::size_t> (params::id_root)]);
    const int scale = static_cast<int> (values[static_cast<std::size_t> (params::id_scale)]);
    const int mode = currentNameMode();

    return juce::String (notenames::rootDisplayLabel (root, mode)) + " "
         + juce::String (notenames::scaleDisplayLabel (scale, mode));
}

void Editor::rebuildPresetItems()
{
    const int selected = presetBox->getSelectedItemIndex();
    presetBox->clear (juce::dontSendNotification);

    for (std::size_t i = 0; i < factoryPresets.size(); ++i)
        presetBox->addItem (presetDisplayName (factoryPresets[i]), static_cast<int> (i) + 1);

    presetBox->setSelectedItemIndex (selected, juce::dontSendNotification);
}

void Editor::refreshNameLabels()
{
    const int mode = currentNameMode();

    refreshChoiceItems (rootBox, notenames::kNumRoots, [mode] (int i)
                        { return juce::String (notenames::rootDisplayLabel (i, mode)); });

    refreshChoiceItems (scaleBox, notenames::kNumScales, [mode] (int i)
                        { return juce::String (notenames::scaleDisplayLabel (i, mode)); });

    rebuildPresetItems();
}

void Editor::parameterChanged (const juce::String& parameterID, float /*newValue*/)
{
    if (parameterID != "names.mode")
        return;

    // Kann aus jedem Thread kommen (Host-Automation); UI-Aenderung auf den
    // Message-Thread umleiten, SafePointer gegen geschlossenes Fenster.
    juce::Component::SafePointer<Editor> safeThis { this };
    juce::MessageManager::callAsync ([safeThis]
    {
        if (safeThis != nullptr)
            safeThis->refreshNameLabels();
    });
}

void Editor::paint (juce::Graphics&)
{
    // Der Inhalt zeichnet im ContentComponent (dort wirkt der Zoom-Transform).
}

void Editor::paintOverChildren (juce::Graphics&)
{
}

void Editor::paintContentOver (juce::Graphics& g)
{
    if (standalone && m_highlightedStep >= 0 && m_highlightedStep < 8)
    {
        auto* btn = steps[static_cast<std::size_t> (m_highlightedStep)].active;
        if (btn != nullptr)
        {
            auto b = btn->getBounds().toFloat();
            g.setColour (HeptachordLookAndFeel::highlightYellow());
            g.fillRoundedRectangle (b.reduced (1.0f), 3.5f);
            g.setColour (HeptachordLookAndFeel::highlightYellow());
            g.drawRoundedRectangle (b.reduced (1.0f), 3.5f, 1.0f);
            g.setColour (HeptachordLookAndFeel::background());
            g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
            g.drawText (btn->getButtonText(), b.reduced (1.0f), juce::Justification::centred);
        }
    }
}

void Editor::paintContent (juce::Graphics& g)
{
    // Step-Highlight (gelb): Active-Toggle des aktuellen Steps registrieren.
    {
        juce::ToggleButton* current = nullptr;
        if (standalone && m_highlightedStep >= 0 && m_highlightedStep < 8)
            current = steps[static_cast<std::size_t> (m_highlightedStep)].active;
        HeptachordLookAndFeel::setCurrentStepActiveButton (current);
    }

    g.fillAll (HeptachordLookAndFeel::background());

    // Kopfzeile
    {
        auto area = headerArea;

        g.setColour (HeptachordLookAndFeel::text());
        g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
        g.drawText ("HEPTACHORD", area.removeFromTop (26), juce::Justification::centredLeft);

        g.setColour (HeptachordLookAndFeel::muted());
        g.setFont (juce::FontOptions (11.0f));
        juce::String subtitle = juce::String (params::kNumParameters) + " Parameter";

        if (standalone)
        {
            subtitle += "  |  Tastatur A-K, Y/Z Oktave, Leertaste Start/Stop";

            if (processor.clockActive())
                subtitle += "  |  Clock: " + juce::String (processor.clockBpm(), 0) + " BPM (MIDI)";
            else
                subtitle += "  |  Clock: intern 120 BPM";
        }

        g.drawText (subtitle, area.removeFromTop (14), juce::Justification::centredLeft);
    }

    const auto panel = [&g] (juce::Rectangle<int> area, const juce::String& title)
    {
        const auto bounds = area.toFloat();

        g.setColour (HeptachordLookAndFeel::panel());
        g.fillRoundedRectangle (bounds, 6.0f);

        g.setColour (HeptachordLookAndFeel::line());
        g.drawRoundedRectangle (bounds.reduced (0.5f), 6.0f, 1.0f);

        g.setColour (HeptachordLookAndFeel::muted());
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.drawText (title,
                    area.reduced (kPanelPadding, 3).removeFromTop (kZoneTitleHeight),
                    juce::Justification::centredLeft);
    };

    panel (globalArea, "GLOBAL");
    panel (voicesArea, "STIMMEN");
    panel (stepsArea, "STEPS");

    // Step-Highlight: gefuellter Hintergrund der aktiven Spalte + Playhead (Standalone).
    if (standalone && m_highlightedStep >= 0 && m_highlightedStep < 8)
    {
        const auto& col = m_stepColumnAreas[static_cast<std::size_t> (m_highlightedStep)];

        g.setColour (HeptachordLookAndFeel::panelRaised());
        g.fillRect (col.toFloat());

        g.setColour (HeptachordLookAndFeel::accent());
        g.fillRect (col.getX(), col.getY(), 3, col.getHeight());

        juce::Path playhead;
        const int cx = col.getX() + col.getWidth() / 2;
        const int cy = col.getY() - 8;
        playhead.addTriangle (cx - 5, cy + 5, cx + 5, cy + 5, cx, cy - 5);
        g.fillPath (playhead);
    }

    g.setFont (juce::FontOptions (10.0f));

    for (const auto& caption : staticCaptions)
    {
        g.setColour (HeptachordLookAndFeel::muted());
        g.drawText (caption.text, caption.area, juce::Justification::centredLeft);
    }

    // Kleiner Akzent unter dem Titel.
    g.setColour (HeptachordLookAndFeel::accent());
    g.fillRect (headerArea.getX(), headerArea.getBottom() + 1, 42, 2);
}

void Editor::resized()
{
    staticCaptions.clear();

    // Immer in Basis-Koordinaten layouten; der Zoom transformiert nur den
    // Container. Damit bleibt das Layout bei allen Stufen identisch.
    auto bounds = juce::Rectangle<int> (0, 0, kWindowWidth, kWindowHeight).reduced (kMargin);

    headerArea = bounds.removeFromTop (kHeaderHeight);
    bounds.removeFromTop (kGap);
    globalArea = bounds.removeFromTop (kGlobalHeight);
    bounds.removeFromTop (kGap);
    voicesArea = bounds.removeFromTop (kVoicesHeight);
    bounds.removeFromTop (kGap);
    stepsArea = bounds;

    layoutHeader (headerArea);
    layoutGlobal (globalArea);
    layoutVoices (voicesArea);
    layoutSteps (stepsArea);

    // Export- und Recorder-Knoepfe in der Steps-Titelzeile (M1/M2, ADR 0010).
    if (standalone)
    {
        auto titleRight = stepsArea.reduced (kPanelPadding, 3)
                              .removeFromTop (kZoneTitleHeight)
                              .removeFromRight (850);
        exportButton.setBounds (titleRight.removeFromLeft (72).reduced (0, 1));
        dragButton.setBounds (titleRight.removeFromLeft (70).reduced (0, 1));
        wavButton.setBounds (titleRight.removeFromLeft (60).reduced (0, 1));
        notesButton.setBounds (titleRight.removeFromLeft (60).reduced (0, 1));
        recordingStatusLabel.setBounds (titleRight.removeFromLeft (155).reduced (0, 1));
        recButton.setBounds (titleRight.removeFromLeft (52).reduced (0, 0));
        playButton.setBounds (titleRight.removeFromLeft (52).reduced (0, 0));
        stopButton.setBounds (titleRight.removeFromLeft (52).reduced (0, 0));
        clearButton.setBounds (titleRight.removeFromLeft (52).reduced (0, 0));
        saveRecordingButton.setBounds (titleRight.removeFromLeft (80).reduced (0, 1));
        recordingTargetBox->setBounds (titleRight.removeFromLeft (130).reduced (0, 0));
    }

    for (const auto& entry : controlCaptions)
    {
        if (entry.anchor == nullptr)
            continue;

        const auto anchor = entry.anchor->getBounds();
        staticCaptions.push_back ({ { anchor.getX(), anchor.getY() - 12, anchor.getWidth(), 11 },
                                    entry.text });
    }

    // Nur der Container skaliert - der Editor bleibt fuer den Host untransformiert.
    m_content.setTransform (m_zoomFactor == 1.0f
                                ? juce::AffineTransform()
                                : juce::AffineTransform::scale (m_zoomFactor));
}

void Editor::layoutHeader (juce::Rectangle<int> area)
{
    auto buttons = area;

    // Von rechts: RESET, START/STOP, ZOOM, Bezeichnung, Preset. Der
    // Transport-Knopf liegt damit zwischen Zoom und Reset; der Zoom bleibt
    // weit genug rechts, um den Untertitel ("Clock ...") nicht zu ueberdecken.
    resetButton.setBounds (buttons.removeFromRight (84).reduced (2, 6));

    if (standalone)
        transportButton.setBounds (buttons.removeFromRight (112).reduced (2, 6));

    zoomBox->setBounds (buttons.removeFromRight (110).reduced (2, 6));

    // Bezeichnungsmodus links neben dem Preset-Selektor (Decision 83).
    namesBox->setBounds (buttons.removeFromRight (150).reduced (2, 6));
    presetBox->setBounds (buttons.removeFromRight (160).reduced (2, 6));
}

void Editor::layoutGlobal (juce::Rectangle<int> area)
{
    auto inner = area.reduced (kPanelPadding);
    inner.removeFromTop (kZoneTitleHeight);

    constexpr int count = 12;
    const int cellWidth = inner.getWidth() / count;

    const auto cellAt = [&inner, cellWidth] (int index)
    {
        const int x = inner.getX() + index * cellWidth;
        const int width = index == count - 1 ? inner.getRight() - x : cellWidth;
        return juce::Rectangle<int> (x, inner.getY(), width, inner.getHeight()).reduced (3, 0);
    };

    const auto placeKnob = [] (juce::Slider* knob, juce::Rectangle<int> cell)
    {
        knob->setBounds (cell.getX(), cell.getY() + 12, cell.getWidth(), 54);
    };

    const auto placeChoice = [] (juce::ComboBox* box, juce::Rectangle<int> cell)
    {
        box->setBounds (cell.getX(), cell.getY() + 26, cell.getWidth(), 26);
    };

    const auto placeToggle = [] (juce::ToggleButton* button, juce::Rectangle<int> cell)
    {
        button->setBounds (cell.getX(), cell.getCentreY() - 11, cell.getWidth(), 22);
    };

    placeChoice (rootBox, cellAt (0));
    placeChoice (scaleBox, cellAt (1));
    placeChoice (rateBox, cellAt (2));
    placeChoice (directionBox, cellAt (3));
    placeKnob (swingKnob, cellAt (4));
    placeKnob (slackKnob, cellAt (5));
    placeKnob (harmVelocityKnob, cellAt (6));
    placeKnob (harmSpreadKnob, cellAt (7));
    placeToggle (latchButton, cellAt (8));
    placeChoice (outputBox, cellAt (9));
    placeKnob (tempoKnob, cellAt (10));

    // INSTRUMENT-Panel im Global-Bereich (v0.1.6, auf stabile Basis portiert):
    // ComboBox oben, LADEN... und EDITOR nebeneinander darunter.
    if (standalone)
    {
        const auto instCell = cellAt (11);
        instrumentBox->setBounds (instCell.getX(), instCell.getY() + 6,
                                  instCell.getWidth(), 24);

        const int half = instCell.getWidth() / 2;
        loadButton.setBounds (instCell.getX(), instCell.getY() + 34, half - 2, 22);
        editButton.setBounds (instCell.getX() + half + 2, instCell.getY() + 34, half - 4, 22);
    }
}

void Editor::layoutVoices (juce::Rectangle<int> area)
{
    auto inner = area.reduced (kPanelPadding);
    inner.removeFromTop (kZoneTitleHeight);

    constexpr int nameWidth = 56;
    constexpr int columnCount = 7;
    const char* titles[columnCount] = { "AKTIV", "GRAD", "OKTAVE", "VELOCITY",
                                        "FOLGT", "GATE", "KANAL" };

    auto header = inner.removeFromTop (12);
    auto headerCells = header.withTrimmedLeft (nameWidth);
    const int columnWidth = headerCells.getWidth() / columnCount;

    const auto cellAt = [] (juce::Rectangle<int> row, int index, int width)
    {
        return row.withX (row.getX() + index * width).withWidth (width).reduced (3, 0);
    };

    for (int i = 0; i < columnCount; ++i)
        staticCaptions.push_back ({ cellAt (headerCells, i, columnWidth), titles[i] });

    const auto placeKnob = [] (juce::Slider* knob, juce::Rectangle<int> cell, int height)
    {
        knob->setBounds (cell.getX(), cell.getY(), cell.getWidth(), height);
    };

    const auto placeToggle = [] (juce::ToggleButton* button, juce::Rectangle<int> cell)
    {
        button->setBounds (cell.getX(), cell.getCentreY() - 10, cell.getWidth(), 20);
    };

    const char* rowNames[4] = { "THRU", "H1", "H2", "H3" };
    const int rowHeight = inner.getHeight() / 4;
    const int knobHeight = rowHeight;

    // Thru: nur Aktiv, Snap, Velocity und Kanal (Gate ist fix, kein Grad/Oktave/Folgt).
    {
        auto row = inner.removeFromTop (rowHeight);
        staticCaptions.push_back ({ { row.getX(), row.getY(), nameWidth - 6, rowHeight },
                                    rowNames[0] });

        auto cellsArea = row.withTrimmedLeft (nameWidth);

        placeToggle (thruEnableButton, cellAt (cellsArea, 0, columnWidth));
        placeToggle (thruSnapButton, cellAt (cellsArea, 1, columnWidth));
        placeKnob (thruVelocityKnob, cellAt (cellsArea, 2, columnWidth), knobHeight);
        placeKnob (thruChannelKnob, cellAt (cellsArea, 6, columnWidth), knobHeight);
    }

    for (int voice = 0; voice < 3; ++voice)
    {
        auto row = inner.removeFromTop (rowHeight);
        staticCaptions.push_back ({ { row.getX(), row.getY(), nameWidth - 6, rowHeight },
                                    rowNames[voice + 1] });

        auto& controls = voices[static_cast<std::size_t> (voice)];
        auto cellsArea = row.withTrimmedLeft (nameWidth);

        placeToggle (controls.enable, cellAt (cellsArea, 0, columnWidth));
        placeKnob (controls.degree, cellAt (cellsArea, 1, columnWidth), knobHeight);
        placeKnob (controls.octave, cellAt (cellsArea, 2, columnWidth), knobHeight);
        placeKnob (controls.velocity, cellAt (cellsArea, 3, columnWidth), knobHeight);
        placeToggle (controls.follow, cellAt (cellsArea, 4, columnWidth));
        placeKnob (controls.gate, cellAt (cellsArea, 5, columnWidth), knobHeight);
        placeKnob (controls.channel, cellAt (cellsArea, 6, columnWidth), knobHeight);
    }
}

void Editor::layoutSteps (juce::Rectangle<int> area)
{
    auto inner = area.reduced (kPanelPadding);
    inner.removeFromTop (kZoneTitleHeight);

    const int columnWidth = inner.getWidth() / 8;

    for (int index = 0; index < 8; ++index)
    {
        auto column = inner.withX (inner.getX() + index * columnWidth).withWidth (columnWidth)
                           .reduced (4, 0);

        m_stepColumnAreas[static_cast<std::size_t> (index)] = column;

        auto& controls = steps[static_cast<std::size_t> (index)];

        auto title = column.removeFromTop (14);
        staticCaptions.push_back ({ title, "S" + juce::String (index + 1) });

        controls.active->setBounds (column.removeFromTop (19));
        column.removeFromTop (4);
        controls.mode->setBounds (column.removeFromTop (22));
        column.removeFromTop (6);

        const auto placePair = [&column] (juce::Slider* left, juce::Slider* right)
        {
            auto row = column.removeFromTop (66);
            const int half = row.getWidth() / 2;

            left->setBounds (row.getX() + 2, row.getY() + 12, half - 4, 54);
            right->setBounds (row.getX() + half + 2, row.getY() + 12, half - 4, 54);
        };

        placePair (controls.offset, controls.velocity);
        column.removeFromTop (5);
        placePair (controls.probability, controls.ratchet);
        column.removeFromTop (6);

        auto maskRow = column.removeFromTop (19);
        const int maskWidth = maskRow.getWidth() / 4;

        for (int voice = 0; voice < 4; ++voice)
            controls.mask[static_cast<std::size_t> (voice)]->setBounds (
                maskRow.getX() + voice * maskWidth + 1, maskRow.getY(), maskWidth - 2, 19);
    }
}

} // namespace heptachord
