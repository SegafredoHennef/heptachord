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
    REQUIRE (snapToScale (60, Scale::Dur) == 60);
    REQUIRE (snapToScale (62, Scale::Dur) == 62);
    REQUIRE (snapToScale (61, Scale::Dur) == 60);   // Gleichstand -> tieferer Ton
    REQUIRE (snapToScale (61, Scale::Lydisch) == 60);
    REQUIRE (snapToScale (63, Scale::Dur) == 64);
    REQUIRE (snapToScale (0, Scale::Dur) == 0);
    REQUIRE (snapToScale (127, Scale::Dur) == 127);
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
