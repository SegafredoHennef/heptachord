#include "core/Scales.h"

#include <catch2/catch_test_macros.hpp>

#include <vector>

using namespace heptachord;

TEST_CASE ("Dur auf C: 14 Grade", "[scales]")
{
    const std::vector<int> expected { 0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21, 23 };
    for (int degree = 0; degree < static_cast<int> (expected.size()); ++degree)
        REQUIRE (degreeToSemitone (degree, Scale::Dur) == expected[static_cast<std::size_t> (degree)]);
}

TEST_CASE ("Negative Grade", "[scales]")
{
    REQUIRE (degreeToSemitone (-1, Scale::Dur) == -1);
    REQUIRE (degreeToSemitone (-7, Scale::Dur) == -12);
    REQUIRE (degreeToSemitone (-8, Scale::Dur) == -13);
}

TEST_CASE ("Oktavperiodizitaet", "[scales]")
{
    for (int degree = -14; degree <= 14; ++degree)
        REQUIRE (degreeToSemitone (degree + 7, Scale::Dur) == degreeToSemitone (degree, Scale::Dur) + 12);
}

TEST_CASE ("Acht Skalentabellen", "[scales]")
{
    REQUIRE (scaleTables().size() == kNumScales);
    for (const auto& table : scaleTables())
    {
        REQUIRE (table.front() == 0);
        for (std::size_t i = 1; i < table.size(); ++i)
            REQUIRE (table[i] > table[i - 1]); // streng aufsteigend
        REQUIRE (table.back() < 12);
    }
}

TEST_CASE ("Klemmung auf 0...127", "[scales]")
{
    REQUIRE (degreeToNote (0, -1000, Scale::Dur) == 0);
    REQUIRE (degreeToNote (127, 1000, Scale::Dur) == 127);
    REQUIRE (degreeToNote (60, 0, Scale::Dur) == 60);
}

TEST_CASE ("Rastung", "[scales]")
{
    REQUIRE (snapToScale (60, Scale::Dur, 0) == 60);
    REQUIRE (snapToScale (62, Scale::Dur, 0) == 62);
    REQUIRE (snapToScale (61, Scale::Dur, 0) == 60);   // Gleichstand -> tieferer Ton
    REQUIRE (snapToScale (61, Scale::Lydisch, 0) == 60);
    REQUIRE (snapToScale (63, Scale::Dur, 0) == 62);   // ebenfalls Gleichstand 62/64
    REQUIRE (snapToScale (64, Scale::Dur, 0) == 64);
    REQUIRE (snapToScale (0, Scale::Dur, 0) == 0);
    REQUIRE (snapToScale (127, Scale::Dur, 0) == 127);
}

TEST_CASE ("Rastung ist am Grundton verankert", "[scales]")
{
    // Cis-Dur: C#(1), D#(3), E#(5), F(6), G#(8), A#(10), H#(11).
    REQUIRE (snapToScale (61, Scale::Dur, 1) == 61);   // Cis liegt in der Skala
    REQUIRE (snapToScale (63, Scale::Dur, 1) == 63);   // Dis liegt in der Skala
    REQUIRE (snapToScale (60, Scale::Dur, 1) == 60);   // H# liegt in der Skala
    REQUIRE (snapToScale (59, Scale::Dur, 1) == 58);   // H -> Gis (Gleichstand, tiefer)

    // Gleicher Ton, anderer Grundton: das Ergebnis verschiebt sich mit.
    REQUIRE (snapToScale (61, Scale::Dur, 0) == 60);   // Cis -> D in C-Dur (Gleichstand)
}

TEST_CASE ("Basisgrad", "[scales]")
{
    // Der Grad zaehlt vom tiefsten Vorkommen des Grundtons (MIDI root ... root+11).
    REQUIRE (baseDegree (60, 0, Scale::Dur) == 35);   // C4  = 5 Oktaven ueber C0
    REQUIRE (baseDegree (62, 0, Scale::Dur) == 36);   // D4
    REQUIRE (baseDegree (64, 0, Scale::Dur) == 37);   // E4
    REQUIRE (baseDegree (72, 0, Scale::Dur) == 42);   // C5  = 6 Oktaven
    REQUIRE (baseDegree (59, 0, Scale::Dur) == 34);   // H3
    REQUIRE (baseDegree (61, 0, Scale::Dur) == 35);   // Es rastet bei Gleichstand auf C

    // In Cis-Dur ist C die siebte Stufe der naechsten Oktave.
    REQUIRE (baseDegree (60, 1, Scale::Dur) == 34);
}

TEST_CASE ("Jede Stimme bleibt in der Skala", "[scales][property]")
{
    for (int scaleIndex = 0; scaleIndex < kNumScales; ++scaleIndex)
    {
        const auto scale = static_cast<Scale> (scaleIndex);
        const auto& table = scaleTable (scale);

        for (int degree = -14; degree <= 14; ++degree)
        {
            const int note = degreeToNote (0, degree, scale);
            const int pitchClass = note % 12;

            bool inScale = false;
            for (const int semitone : table)
                if (pitchClass == semitone)
                    inScale = true;

            REQUIRE (inScale);
        }
    }
}
