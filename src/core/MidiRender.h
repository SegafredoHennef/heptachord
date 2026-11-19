// Deterministischer Render eines Zyklus in MIDI-Ticks (Decision 84, M1).
// Nutzt exakt dieselbe Planung wie die Live-Engine (StepPlanner, StepSequencer,
// applyStep), aber ohne JUCE, ohne Host und ohne Audio-Thread (ADR 0007).
// Feste Zufallsquelle (Seed 1) -> bitgleicher Export bei identischen Parametern.
#pragma once

#include "core/EngineTypes.h"
#include "core/SettingsFromValues.h"
#include "core/StepClock.h"

#include <cstdint>
#include <vector>

namespace heptachord::midirender
{

// PPQ des exportierten MIDI-Files (Entscheidung 86).
inline constexpr int kPpq = 480;

// Letzte Eingangsnote, wenn noch keine gespielt wurde (Entscheidung 86).
inline constexpr int kFallbackNote = 60;  // C4
inline constexpr int kFallbackVelocity = 100;

// Ein Notenereignis im exportierten Zyklus. tick = absolute Zeit in PPQ
// (1 Viertel = 480 Ticks); track = 0 Thru, 1..3 H1..H3; step = 1-basierte
// Step-Nummer (1..8) des ausloesenden Steps. Note-Offs tragen denselben Step
// wie das zugehoerige Note-On (auch Gate-Enden/Hold-Offs).
struct RenderedNote
{
    std::int64_t tick = 0;
    int pitch = 0;       // 0 ... 127
    int velocity = 0;    // 1 ... 127
    int channel = 1;     // 1 ... 16
    int track = 0;       // 0 ... 3
    bool noteOn = true;
    int step = 0;        // 1 ... 8 (0 = unbekannt)

    bool operator== (const RenderedNote&) const = default;
};

struct RenderOptions
{
    EngineSettings settings {};
    double bpm = 120.0;
    int inputNote = kFallbackNote;      // 0 = Fallback, wenn keine Note
    int inputVelocity = kFallbackVelocity;
    std::uint32_t seed = 1;
};

// Rendert genau einen Zyklus (alle aktiven Steps einmal, Entscheidung 86).
// Die Traversal-Folge (Forward/Backward/PingPong/Random) kommt aus
// StepSequencer; Wahrscheinlichkeit und Richtung "Zufall" werden aus dem
// festen Seed gewuerfelt. Rueckgabe ist nach (tick, track, noteOn, pitch)
// sortiert, damit der MIDI-Schreiber sie direkt uebernehmen kann.
std::vector<RenderedNote> renderCycle (const RenderOptions& options) noexcept;

} // namespace heptachord::midirender
