#include "plugin/MidiExport.h"

#include "core/ParameterTable.h"
#include "core/Scales.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>
#include <cmath>

namespace heptachord
{
namespace
{
juce::MidiMessage noteMessage (const midirender::RenderedNote& note)
{
    const auto vel = static_cast<juce::uint8> (juce::jlimit (1, 127, note.velocity));
    const auto pitch = static_cast<juce::uint8> (juce::jlimit (0, 127, note.pitch));
    const auto channel = juce::jlimit (1, 16, note.channel);

    auto message = note.noteOn ? juce::MidiMessage::noteOn (channel, pitch, vel)
                               : juce::MidiMessage::noteOff (channel, pitch);
    message.setTimeStamp (static_cast<double> (note.tick));
    return message;
}

// ASCII-Slug (ersetzt Nicht-ASCII und Sonderzeichen durch "-").
juce::String asciiSlug (const juce::String& text)
{
    juce::String result;
    for (auto character : text)
    {
        const auto code = static_cast<int> (character);
        if ((code >= 'a' && code <= 'z') || (code >= 'A' && code <= 'Z')
            || (code >= '0' && code <= '9'))
            result += juce::String::charToString (character);
        else
            result += "-";
    }
    return result;
}
} // namespace

bool MidiExport::writeTo (juce::OutputStream& destination,
                          const midirender::RenderOptions& options)
{
    const auto notes = midirender::renderCycle (options);
    if (notes.empty())
        return false;

    juce::MidiFile file;
    file.setTicksPerQuarterNote (midirender::kPpq);

    std::array<juce::MidiMessageSequence, static_cast<std::size_t> (kNumVoices)> tracks {};

    // Tempo + Takt in Spur 0 (Entscheidung 86), Tick 0.
    {
        const int microsPerQuarter = juce::jlimit (
            1, 0x7fffffff,
            static_cast<int> (std::llround (60000000.0 / (options.bpm > 0.0 ? options.bpm : 120.0))));
        auto tempo = juce::MidiMessage::tempoMetaEvent (microsPerQuarter);
        tempo.setTimeStamp (0.0);
        tracks[0].addEvent (tempo);

        auto signature = juce::MidiMessage::timeSignatureMetaEvent (4, 4);
        signature.setTimeStamp (0.0);
        tracks[0].addEvent (signature);
    }

    for (const auto& note : notes)
    {
        if (note.track < 0 || note.track >= kNumVoices)
            continue;
        tracks[static_cast<std::size_t> (note.track)].addEvent (noteMessage (note));
    }

    for (auto& track : tracks)
    {
        track.sort();
        file.addTrack (track);
    }

    return file.writeTo (destination, 1);  // Format 1
}

bool MidiExport::saveToFile (const juce::File& file,
                             const midirender::RenderOptions& options)
{
    juce::FileOutputStream stream (file);
    if (! stream.openedOk())
        return false;

    if (! writeTo (stream, options))
        return false;

    stream.flush();
    return true;
}

juce::String MidiExport::withInstrumentInfo (const juce::String& fileName,
                                             const juce::String& instrumentName,
                                             const juce::String& patchName)
{
    auto result = fileName;
    const auto extension = juce::File (fileName).getFileExtension();

    if (extension.isNotEmpty())
        result = result.dropLastCharacters (extension.length());

    const auto add = [&result] (const juce::String& text)
    {
        const auto slug = asciiSlug (text);
        if (slug.isNotEmpty())
            result += "_" + slug;
    };

    add (instrumentName);

    if (patchName.isNotEmpty())
        add (patchName);

    return result + extension;
}

juce::File MidiExport::writeTempFile (const midirender::RenderOptions& options,
                                      const juce::String& instrumentName,
                                      const juce::String& patchName)
{
    const auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory)
                         .getChildFile ("Heptachord");
    dir.createDirectory();

    const auto name = withInstrumentInfo (suggestedFileName (options),
                                          instrumentName, patchName);
    const auto file = dir.getChildFile (name);
    return saveToFile (file, options) ? file : juce::File {};
}

juce::String MidiExport::suggestedFileName (const midirender::RenderOptions& options)
{
    const auto& settings = options.settings;

    const int root = juce::jlimit (0, 11, settings.root);
    const int scale = juce::jlimit (0, kNumScales - 1, static_cast<int> (settings.scale));
    juce::String rootName = juce::String (params::kChoices0[root]);
    juce::String scaleName = juce::String (params::kChoices1[scale]);

    // Rate-Index aus dem Choice-Text ableiten (settings.rate ist der
    // Sekunden-Faktor, der Index steht im APVTS - hier ueber kRateValues).
    int rateIndex = 0;
    for (int i = 0; i < kNumRates; ++i)
    {
        if (std::abs (settings.rate - kRateValues[i]) < 0.0001f)
        {
            rateIndex = i;
            break;
        }
    }
    juce::ignoreUnused (rateIndex);

    // Entscheidung 97: Zeitstempel_Grundton_Skala_BPM (BPM als ganze Zahl).
    const juce::String stamp = juce::Time::getCurrentTime().formatted ("%Y%m%d_%H%M%S");
    const int bpm = juce::jlimit (1, 999, static_cast<int> (std::llround (options.bpm)));

    return "Heptachord_" + stamp + "_" + asciiSlug (rootName) + "_"
         + asciiSlug (scaleName) + "_" + juce::String (bpm) + ".mid";
}

juce::String MidiExport::suggestedRecordingFileName (const midirender::RenderOptions& options)
{
    const auto base = suggestedFileName (options);
    return base.replace ("Heptachord_", "Heptachord_Aufnahme_", false);
}

} // namespace heptachord
