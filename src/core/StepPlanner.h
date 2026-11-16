#pragma once

#include "core/EngineTypes.h"
#include "core/StepClock.h"

#include <array>
#include <cstdint>

namespace heptachord
{

// Was an einer Step-Kante fuer eine Stimme ansteht.
struct VoiceTrigger
{
    bool selected = false;     // maskiert und aktiviert
    int pitch = 0;             // MIDI-Note 0 ... 127
    int velocity = 0;          // 1 ... 127
    double gateSeconds = 0.0;  // Gate-Laenge, nie laenger als der Step
    int channel = 1;           // 1 ... 16
};

// Das Ergebnis der Planung fuer einen Slot.
struct StepPlan
{
    bool valid = false;          // ein Schritt laeuft an dieser Stelle
    int stepIndex = -1;          // 0 ... 7
    std::int64_t slotIndex = 0;
    double startSeconds = 0.0;   // Zeitpunkt der Step-Kante
    StepMode mode = StepMode::Play;
    int hits = 0;                // 0 = kein Trigger (Rest oder keine Note)
    double hitSpacingSeconds = 0.0;
    std::array<VoiceTrigger, kNumVoices> voices {};
};

// Zustand einer Stimme im Core.
struct VoiceState
{
    bool sounding = false;   // Note ist gerade an
    bool held = false;       // Note laeuft in einer Hold-Kette
};

// Note-On/Note-Off, die an einer Step-Kante anfallen.
struct StepActions
{
    std::array<bool, kNumVoices> noteOff {};
    std::array<bool, kNumVoices> noteOn {};
};

// Rechnet aus den Einstellungen und der Eingangsnote aus, was an einer
// Step-Kante passiert. Kennt keine Hosts, keine Threads, keine Zeit.
class StepPlanner
{
public:
    void setSettings (const EngineSettings& settings);
    void setClock (const StepClock& clock);

    // Eingangsseite: gehaltene Note und deren Velocity. Ohne Note entstehen
    // keine Gates.
    void setNote (int note, int velocity);
    void clearNote();
    bool hasNote() const noexcept { return m_noteActive; }

    // Plan fuer den Slot slotIndex mit dem Step stepIndex.
    StepPlan planSlot (int stepIndex, std::int64_t slotIndex) const;

private:
    int voicePitch (int voiceIndex, const StepSettings& step, int baseGrad) const;
    int voiceVelocity (int voiceIndex, const StepSettings& step) const;
    double voiceGate (int voiceIndex, double stepSeconds, int hits) const;
    int voiceChannel (int voiceIndex) const;

    EngineSettings m_settings {};
    StepClock m_clock {};
    int m_note = 60;
    int m_inputVelocity = 100;
    bool m_noteActive = false;
};

// Wuerfel fuer die Step-Wahrscheinlichkeit: 0 % trifft nie, 100 % immer.
// random01 muss in [0, 1) liegen.
bool rollProbability (float percent, double random01) noexcept;

// Wendet einen Step auf den Zustand der Stimmen an. "hit" sagt je Stimme,
// ob der Trigger tatsaechlich stattgefunden hat (nach dem Wurf).
// Hold-Ketten: eine gehaltene Note wird nicht neu angeschlagen und endet
// erst, wenn ein Step kommt, in dem die Stimme nicht maskiert ist oder der
// Modus Play/Ratchet gilt. Rest-Schritte beenden nichts.
StepActions applyStep (const StepPlan& plan,
                        const std::array<bool, kNumVoices>& hit,
                        std::array<VoiceState, kNumVoices>& state);

} // namespace heptachord
