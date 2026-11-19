#include "plugin/AudioExport.h"
#include "plugin/TestSynth.h"

#include "core/MidiRender.h"
#include "core/ParameterTable.h"
#include "core/Scales.h"
#include "core/StepClock.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include <array>
#include <cmath>
#include <vector>

namespace heptachord
{
namespace
{
// Tail nach dem Zyklusende fuer ausklingende Noten (Sekunden).
constexpr double kTailSeconds = 2.0;

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

bool AudioExport::saveWavFile (const juce::File& file,
                               const midirender::RenderOptions& options,
                               double sampleRate)
{
    if (sampleRate <= 0.0)
        sampleRate = 44100.0;

    const auto notes = midirender::renderCycle (options);
    if (notes.empty())
        return false;

    double cycleSeconds = 0.0;
    for (const auto& note : notes)
        if (note.tick > 0)
            cycleSeconds = std::max (cycleSeconds,
                                     static_cast<double> (note.tick) / midirender::kPpq
                                         * (60.0 / (options.bpm > 0.0 ? options.bpm : 120.0)));
    if (cycleSeconds <= 0.0)
        cycleSeconds = 1.0;

    const double totalSeconds = cycleSeconds + kTailSeconds;
    const int totalSamples = static_cast<int> (std::ceil (totalSeconds * sampleRate));
    if (totalSamples <= 0)
        return false;

    juce::MidiBuffer midi;
    for (const auto& note : notes)
    {
        const double seconds = static_cast<double> (note.tick) / midirender::kPpq
                               * (60.0 / (options.bpm > 0.0 ? options.bpm : 120.0));
        const int sample = juce::jlimit (0, totalSamples - 1,
                                         static_cast<int> (std::round (seconds * sampleRate)));
        const auto channel = static_cast<juce::uint8> (juce::jlimit (1, 16, note.channel));
        const auto pitch = static_cast<juce::uint8> (juce::jlimit (0, 127, note.pitch));

        if (note.noteOn)
        {
            const auto vel = static_cast<juce::uint8> (juce::jlimit (1, 127, note.velocity));
            midi.addEvent (juce::MidiMessage::noteOn (channel, pitch, vel), sample);
        }
        else
        {
            midi.addEvent (juce::MidiMessage::noteOff (channel, pitch), sample);
        }
    }

    TestSynth synth;
    synth.prepare (sampleRate);
    synth.reset();

    juce::AudioBuffer<float> buffer (2, totalSamples);
    buffer.clear();

    synth.process (midi, buffer);

    // WAV schreiben (16-Bit) — JUCE 9: Stream-Ownership durch den Writer.
    juce::WavAudioFormat format;
    std::unique_ptr<juce::OutputStream> stream (new juce::FileOutputStream (file));
    if (! static_cast<juce::FileOutputStream*> (stream.get())->openedOk())
        return false;

    auto writerOptions = juce::AudioFormatWriterOptions()
        .withSampleRate (sampleRate)
        .withChannelLayout (juce::AudioChannelSet::stereo())
        .withBitsPerSample (16);

    std::unique_ptr<juce::AudioFormatWriter> writer (
        format.createWriterFor (stream, writerOptions));
    if (writer == nullptr)
        return false;

    // stream ist jetzt im Besitz des writers; nicht mehr manuell freigeben.
    const bool ok = writer->writeFromAudioSampleBuffer (buffer, 0, totalSamples);
    writer.reset();
    return ok;
}

juce::String AudioExport::suggestedFileName (const midirender::RenderOptions& options)
{
    const int root = juce::jlimit (0, 11, options.settings.root);
    const int scale = juce::jlimit (0, kNumScales - 1, static_cast<int> (options.settings.scale));

    juce::String rootName = juce::String (params::kChoices0[root]);
    juce::String scaleName = juce::String (params::kChoices1[scale]);

    int rateIndex = 0;
    for (int i = 0; i < kNumRates; ++i)
        if (std::abs (options.settings.rate - kRateValues[i]) < 0.0001f)
        {
            rateIndex = i;
            break;
        }
    juce::ignoreUnused (rateIndex);

    // Entscheidung 97: Zeitstempel_Grundton_Skala_BPM (BPM als ganze Zahl).
    const juce::String stamp = juce::Time::getCurrentTime().formatted ("%Y%m%d_%H%M%S");
    const int bpm = juce::jlimit (1, 999, static_cast<int> (std::llround (options.bpm)));

    return "Heptachord_" + stamp + "_" + asciiSlug (rootName) + "_"
         + asciiSlug (scaleName) + "_" + juce::String (bpm) + ".wav";
}
} // namespace heptachord
