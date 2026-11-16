#include "plugin/ProcessorBridge.h"

#include "core/SettingsFromValues.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <vector>

using namespace heptachord;

namespace
{
struct AbsMidi
{
    long long sample = 0;
    juce::MidiMessage message;
};

// Fuehrt den Bridge blockweise aus und merkt sich die absoluten Positionen.
struct BridgeRunner
{
    ProcessorBridge bridge {};
    ParameterValues values = defaultParameterValues();
    juce::MidiBuffer midi {};

    long long position = 0;
    double sampleRate = 48000.0;
    double bpm = 120.0;
    bool playing = true;
    bool positionValid = true;

    double ppq() const
    {
        return static_cast<double> (position) * bpm / (60.0 * sampleRate);
    }

    void noteOn (int pitch, int velocity = 100, int channel = 1)
    {
        midi.addEvent (juce::MidiMessage::noteOn (channel, pitch, static_cast<juce::uint8> (velocity)), 0);
    }

    void noteOff (int pitch, int channel = 1)
    {
        midi.addEvent (juce::MidiMessage::noteOff (channel, pitch), 0);
    }

    std::vector<AbsMidi> run (int numSamples)
    {
        HostTimeline timeline;
        timeline.sampleRate = sampleRate;
        timeline.numSamples = numSamples;
        timeline.bpm = bpm;
        timeline.isPlaying = playing;
        timeline.positionValid = positionValid;
        timeline.ppqPosition = ppq();

        bridge.process (values, timeline, midi, midi);

        std::vector<AbsMidi> result;

        for (const auto metadata : midi)
            result.push_back ({ position + metadata.samplePosition, metadata.getMessage() });

        midi.clear();
        position += numSamples;
        return result;
    }

    std::vector<AbsMidi> runFor (long long total, int blockSize)
    {
        std::vector<AbsMidi> result;

        for (long long done = 0; done < total; done += blockSize)
        {
            const auto remaining = total - done;
            const auto size = static_cast<int> (std::min<long long> (blockSize, remaining));
            const auto block = run (size);
            result.insert (result.end(), block.begin(), block.end());
        }

        return result;
    }
};

// Setzt sN.offset = N (0 ... 7), damit sich die Steps im Pitch unterscheiden.
void useStepOffsets (ParameterValues& values)
{
    for (int step = 0; step < kNumSteps; ++step)
        values[static_cast<std::size_t> (params::id_s1_offset + step * 10)] = static_cast<float> (step);
}

std::vector<AbsMidi> filter (const std::vector<AbsMidi>& events, const char* kind, int channel = -1)
{
    std::vector<AbsMidi> result;

    std::copy_if (events.begin(), events.end(), std::back_inserter (result),
                  [kind, channel] (const AbsMidi& event)
                  {
                      const bool matches = juce::String (kind) == "on"  ? event.message.isNoteOn()
                                         : juce::String (kind) == "off" ? event.message.isNoteOff()
                                                                        : event.message.isAllNotesOff();
                      return matches && (channel < 0 || event.message.getChannel() == channel);
                  });

    return result;
}

std::vector<long long> samplesOf (const std::vector<AbsMidi>& events)
{
    std::vector<long long> result;
    result.reserve (events.size());

    for (const auto& event : events)
        result.push_back (event.sample);

    return result;
}

std::vector<int> pitchesOf (const std::vector<AbsMidi>& events)
{
    std::vector<int> result;
    result.reserve (events.size());

    for (const auto& event : events)
        result.push_back (event.message.getNoteNumber());

    return result;
}
} // namespace

TEST_CASE ("Bridge gibt Noten als MIDI aus", "[bridge]")
{
    BridgeRunner runner;
    runner.noteOn (60);

    const auto events = runner.run (512);
    const auto ons = filter (events, "on");

    REQUIRE (ons.size() == 4);
    REQUIRE (samplesOf (ons) == std::vector<long long> { 0, 0, 0, 0 });
    REQUIRE (pitchesOf (ons) == std::vector<int> { 60, 64, 67, 71 });

    REQUIRE (ons[0].message.getChannel() == 1);
    REQUIRE (ons[1].message.getChannel() == 2);
    REQUIRE (ons[2].message.getChannel() == 3);
    REQUIRE (ons[3].message.getChannel() == 4);

    REQUIRE (ons[0].message.getVelocity() == 100);
    REQUIRE (ons[2].message.getVelocity() == 85);
}

TEST_CASE ("Bridge verarbeitet Ein- und Ausgabe im selben Puffer", "[bridge]")
{
    BridgeRunner runner;
    runner.noteOn (60);

    const auto events = runner.run (512);

    // Das eingehende Note-On wird nicht durchgereicht, sondern ersetzt.
    REQUIRE (filter (events, "on").size() == 4);
    REQUIRE (events.size() == 4);
}

TEST_CASE ("Bridge folgt den Parameterwerten", "[bridge][T14]")
{
    BridgeRunner runner;
    runner.values[static_cast<std::size_t> (params::id_output_mode)] = 1.0f;   // Single
    runner.values[static_cast<std::size_t> (params::id_thru_channel)] = 7.0f;
    runner.noteOn (60);

    const auto ons = filter (runner.run (512), "on");
    REQUIRE (ons.size() == 4);

    for (const auto& event : ons)
        REQUIRE (event.message.getChannel() == 7);

    REQUIRE (runner.bridge.settings().thru.channel == 7);
}

TEST_CASE ("Bridge beendet Noten bei Transport-Stop", "[bridge]")
{
    BridgeRunner runner;
    runner.noteOn (60);
    runner.run (512);

    runner.playing = false;
    const auto events = runner.run (512);

    const auto offs = filter (events, "all");
    REQUIRE (offs.size() == 4);
    REQUIRE (samplesOf (offs) == std::vector<long long> { 512, 512, 512, 512 });
}

TEST_CASE ("Bridge setzt auf Anfrage alle Noten zurueck", "[bridge]")
{
    BridgeRunner runner;
    runner.noteOn (60);
    runner.run (512);

    runner.bridge.requestAllNotesOff();
    const auto events = runner.run (512);

    REQUIRE (filter (events, "on").empty());
    REQUIRE (filter (events, "all").size() == 4);
}

TEST_CASE ("Reset-Trigger wirkt nur auf die steigende Flanke", "[bridge]")
{
    BridgeRunner runner;
    useStepOffsets (runner.values);
    runner.noteOn (60);

    const auto first = runner.runFor (12000, 512);
    REQUIRE (pitchesOf (filter (first, "on", 2)) == std::vector<int> { 64 });

    // Flanke 0 -> 1: der naechste Step-Rand beginnt wieder bei Step 0.
    runner.values[static_cast<std::size_t> (params::id_reset)] = 1.0f;
    const auto second = runner.runFor (12000, 512);
    REQUIRE (pitchesOf (filter (second, "on", 2)) == std::vector<int> { 64 });

    // Pegel bleibt 1: kein erneuter Reset.
    const auto third = runner.runFor (12000, 512);
    REQUIRE (pitchesOf (filter (third, "on", 2)) == std::vector<int> { 65 });

    // Neue Flanke: wieder Step 0.
    runner.values[static_cast<std::size_t> (params::id_reset)] = 0.0f;
    runner.runFor (12000, 512);
    runner.values[static_cast<std::size_t> (params::id_reset)] = 1.0f;

    const auto fourth = runner.runFor (12000, 512);
    REQUIRE (pitchesOf (filter (fourth, "on", 2)) == std::vector<int> { 64 });
}

TEST_CASE ("Kanalwechsel beendet Noten auf den verlassenen Kanaelen", "[bridge][T14]")
{
    BridgeRunner runner;
    runner.noteOn (60);
    runner.run (512);

    runner.values[static_cast<std::size_t> (params::id_output_mode)] = 1.0f;   // Single
    const auto events = runner.run (512);

    const auto offs = filter (events, "all");
    REQUIRE (offs.size() == 4);
    REQUIRE (samplesOf (offs) == std::vector<long long> { 512, 512, 512, 512 });

    std::vector<int> channels;
    for (const auto& event : offs)
        channels.push_back (event.message.getChannel());

    std::sort (channels.begin(), channels.end());
    REQUIRE (channels == std::vector<int> { 1, 2, 3, 4 });
}

TEST_CASE ("Noten-Off beendet die Trigger", "[bridge]")
{
    BridgeRunner runner;
    runner.noteOn (60);
    runner.run (512);

    runner.noteOff (60);
    const auto events = runner.runFor (24000, 512);

    REQUIRE (filter (events, "on").empty());
    REQUIRE (filter (events, "off").size() == 4);
}

TEST_CASE ("Ohne Host-Position laeuft der Bridge intern weiter", "[bridge]")
{
    BridgeRunner runner;
    runner.positionValid = false;
    runner.noteOn (60);

    const auto ons = filter (runner.runFor (30000, 1000), "on", 2);
    REQUIRE (samplesOf (ons) == std::vector<long long> { 0, 12000, 24000 });
}
