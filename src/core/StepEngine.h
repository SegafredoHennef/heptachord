#pragma once

#include "core/EngineTypes.h"
#include "core/NoteTracker.h"
#include "core/StepClock.h"
#include "core/StepPlanner.h"
#include "core/StepSequencer.h"
#include "core/TimedEvent.h"

#include <cstdint>
#include <random>
#include <vector>

namespace heptachord
{

// Zeithinweis des Hosts fuer einen Block. ppqPosition ist die Position in
// Viertelnoten am Blockanfang; positionValid = false bedeutet, dass der Host
// keine Position liefert (dann laeuft die Uhr intern weiter).
struct HostTimeline
{
    double sampleRate = 44100.0;
    int numSamples = 0;
    double bpm = 120.0;
    bool isPlaying = false;
    bool positionValid = false;
    double ppqPosition = 0.0;
};

// Ereignisschicht: uebersetzt Slots, Gate-Enden und Ratchet-Treffer in
// MIDI-Ereignisse mit Sample-Offset.
//
// Zeitmodell (Entscheidung 64): Die Slot-Position kommt aus der Host-ppq
// (n = floor(ppq / Slotlaenge) in Viertelnoten); innerhalb des Blocks wird
// sample-genau fortgeschrieben. Springt die ppq (Locate, Loop-Sprung), werden
// klingende Noten per All-Notes-Off beendet und die Traversal-Folge beginnt
// neu beim ersten aktiven Step.
class StepEngine
{
public:
    StepEngine();

    void setSettings (const EngineSettings& settings);
    const EngineSettings& settings() const noexcept { return m_settings; }

    // Injizierbar fuer reproduzierbare Tests; ohne Quelle laeuft ein
    // std::mt19937 mit Zufallsseed.
    void setRandomSource (RandomSource source);

    // Setzt Clock, Sequencer, Noten und Stimmen zurueck.
    void reset() noexcept;

    // Ein Block. input enthaelt Note-On/Note-Off mit Offset im Block;
    // out wird geleert und gefuellt.
    void processBlock (const HostTimeline& timeline,
                       const std::vector<TimedEvent>& input,
                       EventList& out);

    // reset-Parameter: der naechste Step-Rand beginnt beim ersten aktiven Step.
    void requestReset() noexcept { m_resetPending = true; }

    // Preset-Wechsel, Editor schliessen: Noten beenden.
    void requestAllNotesOff() noexcept { m_allNotesOffPending = true; }

    const NoteTracker& noteTracker() const noexcept { return m_notes; }
    std::int64_t nextSlotIndex() const noexcept { return m_nextSlot; }
    bool isRunning() const noexcept { return m_wasPlaying; }

private:
    struct PendingOff
    {
        bool active = false;
        int pitch = 0;
        int channel = 1;
        double samplesLeft = 0.0;   // relativ zum Blockanfang
    };

    struct SoundingNote
    {
        bool active = false;
        int pitch = 0;
        int channel = 1;
    };

    // Ein Slot kann laenger dauern als ein Block (Ratchet-Treffer); deshalb
    // wird er ueber Blockgrenzen hinweg weitergegeben.
    struct ActiveSlot
    {
        bool active = false;
        StepPlan plan {};
        int nextHit = 0;
        double firstHitOffset = 0.0;
        double spacingSamples = 0.0;
    };

    void updateComponents();
    void beginBlock (const HostTimeline& timeline, EventList& out);
    void applyInputEvents (const std::vector<TimedEvent>& input);
    void applyInput (const TimedEvent& event);

    void startSlot (int offsetSamples);
    void fireHit (int offsetSamples, EventList& out);
    void emitAllNotesOff (EventList& out, int offsetSamples);
    void emitPendingOffs (int offsetSamples, EventList& out);
    void clearVoices() noexcept;
    bool hasVoices() const noexcept;

    double slotTimeQn (std::int64_t slotIndex) const noexcept;
    std::int64_t slotAtOrAfter (double ppq) const noexcept;
    int nextHitOffset() const noexcept;
    int nextPendingOffOffset() const noexcept;
    double random01();

    EngineSettings m_settings {};
    StepClock m_clock {};
    StepSequencer m_sequencer {};
    StepPlanner m_planner {};
    NoteTracker m_notes {};

    std::array<PendingOff, kNumVoices> m_pending {};
    std::array<SoundingNote, kNumVoices> m_sounding {};
    std::array<VoiceState, kNumVoices> m_voiceState {};
    ActiveSlot m_slot {};

    std::mt19937 m_rng;
    RandomSource m_randomSource {};

    bool m_wasPlaying = false;
    bool m_haveAnchor = false;
    bool m_resetPending = false;
    bool m_allNotesOffPending = false;

    double m_bpm = 120.0;
    double m_sampleRate = 44100.0;
    double m_blockPpq = 0.0;
    double m_blockSpq = 22050.0;
    std::int64_t m_nextSlot = 0;
    double m_nextSlotOffset = 0.0;   // relativ zum Blockanfang
    double m_lastPpq = 0.0;
    double m_lastSpq = 22050.0;
    int m_lastNumSamples = 0;
};

} // namespace heptachord
