// Schreibt den Core-Render (MidiRender) als Standard-MIDI-File (M1/M2,
// Decision 84): Format 1, PPQ 480, vier Spuren (Thru, H1, H2, H3),
// Tempo-Meta aus der aktuellen BPM. Reine Plugin-Schicht: das File-Format
// ist JUCE-Sache, der Render bleibt JUCE-frei (ADR 0007).
#pragma once

#include "core/MidiRender.h"

#include <juce_core/juce_core.h>

namespace heptachord
{

class MidiExport
{
public:
    // Baut die MIDI-Daten aus den Einstellungen. bpm = aktuelle
    // Standalone-BPM (clockBpm() oder 120 Fallback). inputNote <= 0 ->
    // C4-Fallback. Rueckgabe false bei leerem Zyklus (keine aktiven Steps).
    static bool writeTo (juce::OutputStream& destination,
                         const midirender::RenderOptions& options);

    // Spichert in eine Datei (EXPORT-Knopf). Erzeugt das Verzeichnis nicht.
    static bool saveToFile (const juce::File& file,
                            const midirender::RenderOptions& options);

    // Schreibt in eine temporaere Datei fuer Drag & Drop (M2, Decision 85).
    // Der Rueckgabepfad bleibt bis zum naechsten Aufruf gueltig.
    // Optional werden VST-Name und Patch an den Vorschlag gehaengt (Entscheidung 99).
    static juce::File writeTempFile (const midirender::RenderOptions& options,
                                     const juce::String& instrumentName = {},
                                     const juce::String& patchName = {});

    // Haengt VST-Name (und ggf. Patch) an einen Dateinamen an, bevor die
    // Endung kommt. Leere Namen werden uebergangen.
    static juce::String withInstrumentInfo (const juce::String& fileName,
                                            const juce::String& instrumentName,
                                            const juce::String& patchName);

    // Dateiname-Vorschlag (Entscheidung 97):
    // Heptachord_<Zeitstempel>_<Grundton>_<Skala>_<BPM>.mid
    static juce::String suggestedFileName (const midirender::RenderOptions& options);

    // Vorschlag fuer die aufgenommene Aufnahme:
    // Heptachord_Aufnahme_<Zeitstempel>_<Grundton>_<Skala>_<BPM>.mid
    static juce::String suggestedRecordingFileName (const midirender::RenderOptions& options);
};

} // namespace heptachord
