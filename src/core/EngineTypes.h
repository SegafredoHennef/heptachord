#pragma once

#include "core/Scales.h"

#include <array>
#include <cstdint>

namespace heptachord
{

// --- Enums ------------------------------------------------------------------

enum class Direction : int
{
    Forward = 0,
    Backward,
    PendelI,      // klassisches PingPong (Enden nicht wiederholt)
    PendelII,     // PingPong mit Enden-Wiederholung
    Brownian,     // Zufallswalk (+-1)
    Random        // zufaelliger aktiver Step
};

enum class StepMode : int
{
    Play = 0,
    Hold,
    Rest,
    Ratchet
};

enum class OutputMode : int
{
    Multi = 0,
    Single
};

// --- Groessen ----------------------------------------------------------------

inline constexpr int kNumVoices = 4;            // Thru, H1, H2, H3
inline constexpr int kNumHarmonyVoices = 3;      // H1, H2, H3
inline constexpr int kNumSteps = 8;              // s1 ... s8
inline constexpr int kVoiceThru = 0;             // Index von Thru in den Masken

// Bitmasken der Stimmenmaske (sN.v1 ... sN.v4).
inline constexpr std::uint8_t kMaskThru = 1u << 0;
inline constexpr std::uint8_t kMaskH1 = 1u << 1;
inline constexpr std::uint8_t kMaskH2 = 1u << 2;
inline constexpr std::uint8_t kMaskH3 = 1u << 3;
inline constexpr std::uint8_t kMaskAll = kMaskThru | kMaskH1 | kMaskH2 | kMaskH3;

// --- Parametergruppen (flache Structs, keine JUCE-Typen) --------------------

struct ThruSettings
{
    bool enable = true;
    bool snap = false;
    float velocity = 100.0f;     // Prozent, 0 ... 200
    int channel = 1;             // 1 ... 16
};

struct HarmonySettings
{
    bool enable = true;
    int degree = 0;              // -14 ... +14, diatonisch
    int octave = 0;              // -3 ... +3, chromatisch
    float velocity = 100.0f;     // Prozent, 0 ... 200
    bool follow = true;          // an = velocity wird ignoriert
    float gate = 50.0f;          // Prozent der Step-Laenge, 5 ... 100
    int channel = 2;             // 1 ... 16
};

struct StepSettings
{
    bool active = true;
    StepMode mode = StepMode::Play;
    int offset = 0;              // -7 ... +7, diatonisch, nur H1 ... H3
    float velocity = 100.0f;     // Prozent, 0 ... 200 (Akzent)
    float probability = 100.0f;  // Prozent, 0 ... 100
    int ratchet = 2;             // 2 ... 8
    std::uint8_t voiceMask = kMaskAll;
};

struct EngineSettings
{
    int root = 0;                    // 0 ... 11, C = 0
    Scale scale = Scale::Dur;
    float rate = 0.125f;             // 1/16 ... 8; 1.0 = ein Takt
    Direction direction = Direction::Forward;
    float swing = 0.0f;              // Prozent, 0 ... 75
    float slackMs = 0.0f;            // Millisekunden, -100 ... +100
    bool latch = false;
    OutputMode outputMode = OutputMode::Multi;

    float harmVelocity = 100.0f;      // Prozent, 0 ... 200 (H1 ... H3)
    float harmSpread = 0.3f;          // 0 ... 1

    ThruSettings thru {};
    std::array<HarmonySettings, kNumHarmonyVoices> harmony {};
    std::array<StepSettings, kNumSteps> steps {};
};

} // namespace heptachord
