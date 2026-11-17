// Tests fuer den WAV-Export (Decision 84/85/86, M3).
#include "core/SettingsFromValues.h"
#include "plugin/AudioExport.h"

#include <catch2/catch_test_macros.hpp>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include <cstring>

using namespace heptachord;

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

TEST_CASE ("AudioExport schreibt eine gueltige WAV-Datei", "[audioexport]")
{
    auto file = juce::File::getSpecialLocation (juce::File::tempDirectory)
                    .getChildFile ("heptachord_test.wav");
    file.deleteFile();

    REQUIRE (AudioExport::saveWavFile (file, baseOptions(), 44100.0));
    REQUIRE (file.existsAsFile());
    REQUIRE (file.getSize() > 44);

    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader (format.createReaderFor (file.createInputStream().release(), true));
    REQUIRE (reader != nullptr);
    REQUIRE (reader->sampleRate == 44100.0);
    REQUIRE (reader->numChannels == 2);
    REQUIRE (reader->lengthInSamples > 0);

    juce::AudioBuffer<float> buffer (2, static_cast<int> (reader->lengthInSamples));
    REQUIRE (reader->read (&buffer, 0, static_cast<int> (reader->lengthInSamples), 0, true, true));

    float peak = 0.0f;
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            peak = std::max (peak, std::abs (buffer.getSample (ch, i)));

    REQUIRE (peak > 0.001f);

    file.deleteFile();
}

TEST_CASE ("Leerer Zyklus schreibt keine WAV", "[audioexport]")
{
    auto options = baseOptions();
    for (int step = 0; step < kNumSteps; ++step)
        options.settings.steps[static_cast<std::size_t> (step)].active = false;

    auto file = juce::File::getSpecialLocation (juce::File::tempDirectory)
                    .getChildFile ("heptachord_empty.wav");
    file.deleteFile();

    REQUIRE_FALSE (AudioExport::saveWavFile (file, options, 44100.0));

    file.deleteFile();
}

TEST_CASE ("Dateiname ist ASCII mit .wav", "[audioexport]")
{
    auto options = baseOptions();
    options.settings.root = 0;
    options.settings.scale = Scale::Dur;
    options.settings.rate = kRateValues[2];

    REQUIRE (AudioExport::suggestedFileName (options) == "Heptachord_C_Dur_1-8.wav");

    options.settings.root = 11;
    options.settings.scale = Scale::Moll;
    REQUIRE (AudioExport::suggestedFileName (options) == "Heptachord_H_Moll_1-8.wav");
}
