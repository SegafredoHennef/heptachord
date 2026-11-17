#include "core/ParameterTable.h"

#include <catch2/catch_test_macros.hpp>

#include <cstring>
#include <fstream>
#include <string>
#include <vector>

using namespace heptachord;

TEST_CASE ("Golden File stimmt mit der Tabelle ueberein", "[params]")
{
    const std::string path = std::string (HEPTACHORD_SOURCE_DIR) + "/tests/parameter_ids.txt";
    std::ifstream file (path);
    REQUIRE (file.good());

    std::vector<std::string> ids;
    for (std::string line; std::getline (file, line);)
        if (! line.empty())
            ids.push_back (line);

    REQUIRE (ids.size() == static_cast<std::size_t> (params::kNumParameters));

    for (int i = 0; i < params::kNumParameters; ++i)
        REQUIRE (ids[static_cast<std::size_t> (i)] == params::kParams[static_cast<std::size_t> (i)].id);
}

TEST_CASE ("IDs sind eindeutig und auffindbar", "[params]")
{
    for (const auto& p : params::kParams)
    {
        const int index = params::indexOf (p.id);
        REQUIRE (index >= 0);
        REQUIRE (&params::kParams[static_cast<std::size_t> (index)] == &p);
    }
}

TEST_CASE ("Auswahlparameter haben gueltige Defaults", "[params]")
{
    for (const auto& p : params::kParams)
    {
        if (p.type != params::Type::Choice)
            continue;

        REQUIRE (p.numChoices > 1);
        REQUIRE (p.choices != nullptr);
        REQUIRE (p.defaultValue >= 0.0f);
        REQUIRE (p.defaultValue < static_cast<float> (p.numChoices));
    }
}

TEST_CASE ("Bereiche sind plausibel", "[params]")
{
    for (const auto& p : params::kParams)
    {
        if (p.type != params::Type::Int && p.type != params::Type::Float)
            continue;

        REQUIRE (p.min < p.max);
        REQUIRE (p.defaultValue >= p.min);
        REQUIRE (p.defaultValue <= p.max);
    }
}

TEST_CASE ("Bezeichnungsmodus ist als Choice mit drei Eintraegen vorhanden", "[params]")
{
    REQUIRE (params::kNumParameters == 117);
    REQUIRE (params::id_names_mode == 9);

    const auto& p = params::kParams[static_cast<std::size_t> (params::id_names_mode)];
    REQUIRE (std::string (p.id) == "names.mode");
    REQUIRE (p.type == params::Type::Choice);
    REQUIRE (p.numChoices == 3);
    REQUIRE (std::string (p.choices[0]) == "Deutsch");
    REQUIRE (std::string (p.choices[1]) == "International (b)");
    REQUIRE (std::string (p.choices[2]) == "International (#)");
    REQUIRE (p.defaultValue == 0.0f);
}

TEST_CASE ("Grundton-Choices enden auf Ais und H (deutsche Konvention)", "[params]")
{
    const auto& root = params::kParams[static_cast<std::size_t> (params::id_root)];
    REQUIRE (root.numChoices == 12);
    REQUIRE (std::string (root.choices[0]) == "C");
    REQUIRE (std::string (root.choices[1]) == "Cis");
    REQUIRE (std::string (root.choices[10]) == "Ais");
    REQUIRE (std::string (root.choices[11]) == "H");
}
