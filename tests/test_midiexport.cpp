// Tests fuer den MIDI-File-Writer (Decision 84/85): Format 1, PPQ 480,
// vier Spuren, Tempo-Meta, Dateiname.
#include "core/SettingsFromValues.h"
#include "plugin/MidiExport.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <juce_audio_basics/juce_audio_basics.h>

#include <cstring>

using namespace heptachord;
using Catch::Approx;

namespace
{
midirender::RenderOptions baseOptions()
{
    midirender::RenderOptions options;
    options.settings = settingsFromValues (defaultParameterValues());
    options.inputNote = 60;
    options.bpm = 120.0;
    return options;
}
} // namespace

TEST_CASE ("MidiExport schreibt ein gueltiges Format-1-File mit PPQ 480", "[midiexport]")
{
    juce::MemoryOutputStream stream;
    REQUIRE (MidiExport::writeTo (stream, baseOptions()));

    const auto bytes = static_cast<const unsigned char*> (stream.getData());
    const auto size = stream.getDataSize();
    REQUIRE (size > 14);

    // Header: MThd, Laenge 6, Format 1, 4 Spuren, PPQ 480.
    REQUIRE (bytes[0] == 'M');
    REQUIRE (bytes[1] == 'T');
    REQUIRE (bytes[2] == 'h');
    REQUIRE (bytes[3] == 'd');
    REQUIRE (static_cast<unsigned char> (bytes[7]) == 6);
    REQUIRE (static_cast<unsigned char> (bytes[9]) == 1);              // Format 1
    REQUIRE (static_cast<unsigned char> (bytes[11]) == kNumVoices);      // 4 Spuren
    REQUIRE (static_cast<unsigned char> (bytes[12]) == (midirender::kPpq >> 8));
    REQUIRE (static_cast<unsigned char> (bytes[13]) == (midirender::kPpq & 0xff));
}

TEST_CASE ("MidiExport liest das File mit juce::MidiFile zurueck", "[midiexport]")
{
    juce::MemoryOutputStream stream;
    REQUIRE (MidiExport::writeTo (stream, baseOptions()));

    juce::MemoryInputStream input (stream.getData(), stream.getDataSize(), false);
    juce::MidiFile file;
    REQUIRE (file.readFrom (input));
    REQUIRE (file.getTimeFormat() == midirender::kPpq);
    REQUIRE (file.getNumTracks() == kNumVoices);

    int events = 0;
    for (int track = 0; track < file.getNumTracks(); ++track)
        events += file.getTrack (track)->getNumEvents();

    REQUIRE (events > 0);
}

TEST_CASE ("Tempo-Meta entspricht der uebergebenen BPM", "[midiexport]")
{
    auto options = baseOptions();
    options.bpm = 90.0;

    juce::MemoryOutputStream stream;
    REQUIRE (MidiExport::writeTo (stream, options));

    juce::MemoryInputStream input (stream.getData(), stream.getDataSize(), false);
    juce::MidiFile file;
    REQUIRE (file.readFrom (input));

    int foundTempo = 0;
    for (const auto* event : *file.getTrack (0))
    {
        if (event->message.isTempoMetaEvent())
        {
            foundTempo++;
            REQUIRE (event->message.getTempoSecondsPerQuarterNote() == Approx (60.0 / 90.0).epsilon (0.001));
        }
    }
    REQUIRE (foundTempo == 1);
}

TEST_CASE ("Leerer Zyklus schreibt kein File", "[midiexport]")
{
    auto options = baseOptions();
    for (int step = 0; step < kNumSteps; ++step)
        options.settings.steps[static_cast<std::size_t> (step)].active = false;

    juce::MemoryOutputStream stream;
    REQUIRE_FALSE (MidiExport::writeTo (stream, options));
}

TEST_CASE ("Dateiname ist ASCII und enthaelt Zeitstempel, Grundton, Skala, BPM", "[midiexport]")
{
    auto options = baseOptions();
    options.settings.root = 0;
    options.settings.scale = Scale::Dur;
    options.bpm = 120.0;

    const auto name = MidiExport::suggestedFileName (options);
    REQUIRE (name.startsWith ("Heptachord_"));
    REQUIRE (name.endsWith ("_C_Dur_120.mid"));
    REQUIRE (name.length() == juce::String ("Heptachord_20261119_143025_C_Dur_120.mid").length());

    options.settings.root = 11;  // H
    options.settings.scale = Scale::Moll;
    REQUIRE (MidiExport::suggestedFileName (options).endsWith ("_H_Moll_120.mid"));

    const auto take = MidiExport::suggestedRecordingFileName (options);
    REQUIRE (take.startsWith ("Heptachord_Aufnahme_"));
    REQUIRE (take.endsWith ("_H_Moll_120.mid"));

    // Entscheidung 99: nur Drag-Dateien bekommen VST-Info angehaengt.
    REQUIRE (MidiExport::withInstrumentInfo (name, "Osirus", "Init Patch")
             == name.replace (".mid", "_Osirus_Init-Patch.mid"));
    REQUIRE (MidiExport::withInstrumentInfo (name, {}, {}) == name);
    REQUIRE (MidiExport::withInstrumentInfo (take, "OsTIrus", "Bass 01")
             .endsWith ("_OsTIrus_Bass-01.mid"));
}

TEST_CASE ("Zwei Exporte bei gleichem Seed sind bitgleich", "[midiexport]")
{
    juce::MemoryOutputStream a;
    juce::MemoryOutputStream b;
    REQUIRE (MidiExport::writeTo (a, baseOptions()));
    REQUIRE (MidiExport::writeTo (b, baseOptions()));

    REQUIRE (a.getDataSize() == b.getDataSize());
    REQUIRE (std::memcmp (a.getData(), b.getData(), a.getDataSize()) == 0);
}
