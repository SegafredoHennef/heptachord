// WAV-Export des Test-Synths: rendert einen Zyklus durch den Test-Synth
// und speichert das Ergebnis als 16-Bit-WAV (M3, Decision 84).
// Reine Plugin-Schicht (TestSynth + juce_audio_formats), keine Core-Abhaengigkeit
// ausser MidiRender (ADR 0007).
#pragma once

#include "core/MidiRender.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include <array>

namespace heptachord
{

class AudioExport
{
public:
    // Rendert den Zyklus durch den Test-Synth und schreibt ein 16-Bit-WAV.
    // Rueckgabe false bei leerem Zyklus oder Schreibfehler.
    static bool saveWavFile (const juce::File& file,
                             const midirender::RenderOptions& options,
                             double sampleRate = 44100.0);

    // Dateiname-Vorschlag: Heptachord_<Grundton>_<Skala>_<Rate>.wav
    static juce::String suggestedFileName (const midirender::RenderOptions& options);
};

} // namespace heptachord
