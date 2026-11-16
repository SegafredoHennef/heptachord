#pragma once

#include "core/EngineTypes.h"

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace heptachord
{

// Zufallsquelle fuer Richtung "Zufall" und die Step-Wahrscheinlichkeit.
// Injizierbar, damit Tests reproduzierbar sind. Liefert Werte in [0, 1).
using RandomSource = std::function<double()>;

// Bestimmt die Reihenfolge, in der die Steps durchlaufen werden.
//
// Der Zyklus besteht aus den *aktiven* Steps (sN.active = true) in
// Rasterreihenfolge; inaktive Steps fallen ganz heraus (Entscheidung 36).
// Steps im Modus "Rest" bleiben im Zyklus - sie sind nur stumm.
class StepSequencer
{
public:
    void setSteps (const std::array<StepSettings, kNumSteps>& steps) noexcept;
    void setDirection (Direction direction) noexcept;
    void setRandomSource (RandomSource source);

    // Setzt den Cursor auf den Anfang der Traversal-Folge.
    void reset() noexcept;

    // Naechster Schritt der Traversal-Folge. false, wenn kein aktiver Step
    // vorhanden ist (dann wird stepIndex auf -1 gesetzt).
    bool nextStep (int& stepIndex) noexcept;

    int activeCount() const noexcept;
    const std::vector<int>& activeSteps() const noexcept { return m_active; }

private:
    void rebuildActive() noexcept;

    std::vector<int> m_active;      // Indizes der aktiven Steps (0 ... 7)
    std::array<StepSettings, kNumSteps> m_steps {};
    bool m_initialized = false;
    Direction m_direction = Direction::Forward;
    RandomSource m_random;
    int m_position = -1;            // Position in m_active
    int m_pingPongDirection = 1;     // +1 / -1
};

} // namespace heptachord
