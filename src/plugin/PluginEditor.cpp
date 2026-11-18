#include "plugin/PluginEditor.h"

#include "core/NoteNames.h"
#include "core/ParameterTable.h"
#include "plugin/ComputerKeyboard.h"
#include "plugin/MidiExport.h"
#include "plugin/AudioExport.h"
#include "plugin/PluginProcessor.h"

#include <functional>

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
} // namespace

Editor::Editor (Processor& processorToEdit)
    : AudioProcessorEditor (&processorToEdit), processor (processorToEdit)
{
    setLookAndFeel (&lookAndFeel);
    standalone = processor.isStandalone();

    buildControls();

    factoryPresets = Preset::factoryPresets();

    presetBox = combos.add (new juce::ComboBox());
    presetBox->setJustificationType (juce::Justification::centredLeft);
    presetBox->setTooltip ("Factory-Preset laden");
    addAndMakeVisible (presetBox);

    presetBox->setSelectedItemIndex (-1, juce::dontSendNotification);
    presetBox->onChange = [this] { applySelectedPreset(); };

    addAndMakeVisible (resetButton);
    resetButton.setTooltip ("Setzt das Raster am naechsten Step-Rand auf Step 1");
    resetButton.onClick = [this] { processor.triggerReset(); };

    if (standalone)
    {
        addAndMakeVisible (transportButton);
        transportButton.onClick = [this]
        {
            processor.toggleStandaloneTransport();
            updateTransportButton();
        };

        addAndMakeVisible (exportButton);
        exportButton.setTooltip ("Einen Zyklus als MIDI-Datei speichern (fester Seed 1)");
        exportButton.onClick = [this] { exportMidi(); };

        addAndMakeVisible (dragButton);
        dragButton.setTooltip ("MIDI-Datei auf ein anderes Programm ziehen");
        dragButton.onDragStart = [this] { dragMidi(); };

        addAndMakeVisible (wavButton);
        wavButton.setTooltip ("Einen Zyklus als WAV rendern (Test-Synth)");
        wavButton.onClick = [this] { exportWav(); };

        addAndMakeVisible (notesButton);
        notesButton.setTooltip ("Notenfolge als MIDI + Text/CSV exportieren");
        notesButton.onClick = [this] { exportNotes(); };

        addAndMakeVisible (instrumentBox);
        instrumentBox->setJustificationType (juce::Justification::centredLeft);
        instrumentBox->setTooltip ("Aktives Instrument (Standalone)");
        instrumentBox->addItem ("Interner Synth", 1);
        instrumentBox->setSelectedItemIndex (0, juce::dontSendNotification);
        instrumentBox->onChange = [this] { /* Auswahl aendern, wenn noetig */ };

        addAndMakeVisible (loadButton);
        loadButton.setTooltip ("VST3-Plugin als Instrument laden (Standalone)");
        loadButton.onClick = [this] { loadInstrument(); };

        addAndMakeVisible (editButton);
        editButton.setTooltip ("Plugin-Editor-Fenster oeffnen");
        editButton.onClick = [this] { showInstrumentEditor(); };
        editButton.setEnabled (false);

        startTimerHz (30);
        setWantsKeyboardFocus (true);
    }

    // Bezeichnungsmodus (Decision 83): Listener registrieren und die
    // Anzeige-Labels einmal aufbauen (Default ist Deutsch).
    processor.apvts.addParameterListener ("names.mode", this);
    refreshNameLabels();

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
    addAndMakeVisible (slider);

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
    addAndMakeVisible (box);

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
    addAndMakeVisible (button);

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
    directionBox = &addChoice ("direction", "Richtung");
    swingKnob = &addKnob ("swing", "Swing");
    slackKnob = &addKnob ("slack", "Slack");
    harmVelocityKnob = &addKnob ("harm.velocity", "Vel H1-H3");
    harmSpreadKnob = &addKnob ("harm.spread", "Spread");
    tempoKnob = &addKnob ("tempo", "Tempo");
    latchButton = &addToggle ("latch", "LATCH");
    outputBox = &addChoice ("output.mode", "Ausgabe");

    // Kopfzeile: Bezeichnungsmodus ohne Beschriftung (Decision 83).
    namesBox = &addChoice ("names.mode", "");

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
        repaint();
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

void Editor::exportNotes()
{
    const auto options = processor.exportOptions();

    // Dateiname mit Zeitstempel (ASCII-kompatibel)
    juce::String timestamp = juce::Time::getCurrentTime().formatted ("%Y%m%d_%H%M%S");
    juce::String baseName = "Heptachord_Noten_" + timestamp;

    juce::File desktop = juce::File::getSpecialLocation (juce::File::userDesktopDirectory);
    juce::File midiFile = desktop.getChildFile (baseName + ".mid");
    juce::File txtFile = desktop.getChildFile (baseName + ".txt");

    // 1. MIDI-Datei (Format 1, PPQ 480, 4 Spuren, Tempo-Meta)
    if (! MidiExport::saveToFile (midiFile, options))
    {
        juce::NativeMessageBox::showMessageBoxAsync (
            juce::MessageBoxIconType::WarningIcon, "Heptachord",
            "Die MIDI-Datei konnte nicht gespeichert werden.", this);
        return;
    }

    // 2. Text/CSV mit den notierten Steps (aus demselben deterministischen Render)
    const auto notes = midirender::renderCycle (options);

    juce::String csv;
    csv += "# Heptachord — Noten-Export\n";
    csv += "# BPM: " + juce::String (options.bpm, 1) + ", Rate: "
         + juce::String (options.settings.rate, 3) + ", Seed: 1\n";
    csv += "Step,Tick,Channel,Pitch,Velocity,Event\n";

    for (const auto& note : notes)
    {
        csv += juce::String (note.tick) + ","
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
}

void Editor::loadInstrument()
{
    instrumentFileChooser = std::make_unique<juce::FileChooser> (
        "VST3-Plugin als Instrument laden",
        juce::File::getSpecialLocation (juce::File::userDesktopDirectory),
        "*.vst3");

    instrumentFileChooser->launchAsync (
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            if (file == juce::File {})
                return;

            auto inst = createVstPluginInstrument();
            auto* vstInst = static_cast<VstPluginInstrument*> (inst.get());
            if (! vstInst->load (file))
            {
                juce::NativeMessageBox::showMessageBoxAsync (
                    juce::MessageBoxIconType::WarningIcon, "Heptachord",
                    "Das VST3-Plugin konnte nicht geladen werden.", this);
                return;
            }

            processor.setInstrument (std::move (inst));

            instrumentBox->addItem (processor.instrument()->getName(),
                                    instrumentBox->getNumItems() + 1);
            instrumentBox->setSelectedItemIndex (instrumentBox->getNumItems() - 1,
                                                 juce::dontSendNotification);
            editButton.setEnabled (processor.instrument()->hasEditor());
        });
}

void Editor::showInstrumentEditor()
{
    if (processor.instrument() != nullptr)
        processor.instrument()->showEditor (this);
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

void Editor::paint (juce::Graphics& g)
{
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

    // Step-Highlight: Rahmen um den aktuell abgespielten Step (Standalone).
    if (standalone && m_highlightedStep >= 0 && m_highlightedStep < 8)
    {
        g.setColour (HeptachordLookAndFeel::accent());
        g.drawRect (m_stepColumnAreas[static_cast<std::size_t> (m_highlightedStep)].toFloat(), 2.0f);
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

    auto bounds = getLocalBounds().reduced (kMargin);

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

    // Export-Knoepfe in der Steps-Titelzeile (M1/M2, Decision 84/85).
    if (standalone)
    {
        auto titleRight = stepsArea.reduced (kPanelPadding, 3)
                              .removeFromTop (kZoneTitleHeight)
                              .removeFromRight (262);
        exportButton.setBounds (titleRight.removeFromLeft (72).reduced (0, 2));
        dragButton.setBounds (titleRight.removeFromLeft (70).reduced (0, 2));
        wavButton.setBounds (titleRight.removeFromLeft (60).reduced (0, 2));
        notesButton.setBounds (titleRight.removeFromLeft (60).reduced (0, 2));
    }

    for (const auto& entry : controlCaptions)
    {
        if (entry.anchor == nullptr)
            continue;

        const auto anchor = entry.anchor->getBounds();
        staticCaptions.push_back ({ { anchor.getX(), anchor.getY() - 12, anchor.getWidth(), 11 },
                                    entry.text });
    }
}

void Editor::layoutHeader (juce::Rectangle<int> area)
{
    auto buttons = area;

    resetButton.setBounds (buttons.removeFromRight (84).reduced (2, 6));

    if (standalone)
        transportButton.setBounds (buttons.removeFromRight (112).reduced (2, 6));

    // Bezeichnungsmodus links neben dem Preset-Selektor (Decision 83).
    namesBox->setBounds (buttons.removeFromRight (150).reduced (2, 6));
    presetBox->setBounds (buttons.removeFromRight (160).reduced (2, 6));

    if (standalone)
    {
        instrumentBox->setBounds (buttons.removeFromRight (140).reduced (2, 6));
        loadButton.setBounds (buttons.removeFromRight (70).reduced (2, 6));
        editButton.setBounds (buttons.removeFromRight (60).reduced (2, 6));
    }
}

void Editor::layoutGlobal (juce::Rectangle<int> area)
{
    auto inner = area.reduced (kPanelPadding);
    inner.removeFromTop (kZoneTitleHeight);

    constexpr int count = 11;
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
