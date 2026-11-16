#include "core/StepEngine.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace heptachord
{
namespace
{
constexpr int kNoEvent = std::numeric_limits<int>::max();

// Toleranz fuer die Fortschreibung des Slot-Offsets. Weicht der mitgefuehrte
// Wert um mehr als ein Sample von der Host-ppq ab, gilt die Host-Zeit.
constexpr double kOffsetToleranceSamples = 1.0;

// Ab dieser Abweichung gilt die Transportposition als Sprung (Locate).
constexpr double kJumpToleranceSlots = 0.25;
} // namespace

StepEngine::StepEngine()
    : m_rng (std::random_device {}())
{
}

void StepEngine::setSettings (const EngineSettings& settings)
{
    m_settings = settings;
    m_notes.setLatch (settings.latch);
    updateComponents();
}

void StepEngine::setRandomSource (RandomSource source)
{
    m_randomSource = std::move (source);
}

void StepEngine::reset() noexcept
{
    m_sequencer.reset();
    m_notes.reset();
    m_planner.clearNote();
    m_slot = {};
    m_pending = {};
    m_sounding = {};
    m_voiceState = {};
    m_wasPlaying = false;
    m_haveAnchor = false;
    m_resetPending = false;
    m_allNotesOffPending = false;
    m_nextSlot = 0;
    m_nextSlotOffset = 0.0;
    m_lastPpq = 0.0;
    m_lastNumSamples = 0;
}

void StepEngine::updateComponents()
{
    m_clock.setBpm (m_bpm);
    m_clock.setRate (m_settings.rate);
    m_clock.setSwing (m_settings.swing);
    m_clock.setSlackMs (m_settings.slackMs);

    m_sequencer.setDirection (m_settings.direction);
    m_sequencer.setSteps (m_settings.steps);

    m_planner.setSettings (m_settings);
    m_planner.setClock (m_clock);
    m_notes.setLatch (m_settings.latch);
}

// --- Block -------------------------------------------------------------------

void StepEngine::processBlock (const HostTimeline& timeline,
                               const std::vector<TimedEvent>& input,
                               EventList& out)
{
    out.clear();

    if (timeline.numSamples <= 0 || timeline.sampleRate <= 0.0)
    {
        applyInputEvents (input);
        return;
    }

    m_sampleRate = timeline.sampleRate;
    m_bpm = std::clamp (timeline.bpm, 20.0, 400.0);
    updateComponents();

    if (m_allNotesOffPending)
    {
        emitAllNotesOff (out, 0);
        clearVoices();
        m_allNotesOffPending = false;
    }

    if (! timeline.isPlaying)
    {
        if (m_wasPlaying)
        {
            emitAllNotesOff (out, 0);
            clearVoices();
        }

        m_wasPlaying = false;
        m_haveAnchor = false;
        m_resetPending = false;
        m_slot = {};
        applyInputEvents (input);
        return;
    }

    if (! m_wasPlaying)
    {
        // Transport-Start: nichts klingt, Traversal beginnt neu.
        m_wasPlaying = true;
        m_haveAnchor = false;
        m_slot = {};
    }

    beginBlock (timeline, out);

    std::size_t inputIndex = 0;

    while (true)
    {
        const int tOff = nextPendingOffOffset();
        const int tHit = nextHitOffset();
        const int tSlot = (m_nextSlotOffset < static_cast<double> (timeline.numSamples))
                              ? std::max (0, static_cast<int> (std::llround (m_nextSlotOffset)))
                              : kNoEvent;
        const int tIn = (inputIndex < input.size())
                            ? std::clamp (input[inputIndex].offsetSamples, 0, timeline.numSamples - 1)
                            : kNoEvent;

        const int next = std::min (std::min (tOff, tIn), std::min (tHit, tSlot));
        if (next == kNoEvent || next >= timeline.numSamples)
            break;

        // Reihenfolge bei gleichem Sample: Gate-Ende, Eingabe, Treffer,
        // Slot-Beginn. Note-Off steht damit immer vor dem zugehoerigen
        // Note-On (Spezifikation 8.1).
        emitPendingOffs (next, out);

        while (inputIndex < input.size()
               && std::clamp (input[inputIndex].offsetSamples, 0, timeline.numSamples - 1) <= next)
            applyInput (input[inputIndex++]);

        while (m_slot.active && nextHitOffset() <= next)
            fireHit (nextHitOffset(), out);

        if (tSlot == next)
        {
            startSlot (next);

            while (m_slot.active && nextHitOffset() <= next)
                fireHit (nextHitOffset(), out);
        }
    }

    while (inputIndex < input.size())
        applyInput (input[inputIndex++]);

    // Offsets in den naechsten Block weitertragen.
    for (auto& pending : m_pending)
        if (pending.active)
            pending.samplesLeft -= static_cast<double> (timeline.numSamples);

    if (m_slot.active)
        m_slot.firstHitOffset -= static_cast<double> (timeline.numSamples);

    m_nextSlotOffset -= static_cast<double> (timeline.numSamples);

    m_lastPpq = m_blockPpq;
    m_lastSpq = m_blockSpq;
    m_lastNumSamples = timeline.numSamples;
}

void StepEngine::beginBlock (const HostTimeline& timeline, EventList& out)
{
    m_blockSpq = 60.0 / m_bpm * m_sampleRate;

    bool discontinuity = ! m_haveAnchor;
    m_blockPpq = 0.0;

    if (timeline.positionValid)
    {
        m_blockPpq = timeline.ppqPosition;

        if (m_haveAnchor)
        {
            const double expected = m_lastPpq + static_cast<double> (m_lastNumSamples) / m_lastSpq;
            const double slotQn = m_clock.stepSeconds() * m_bpm / 60.0;
            if (std::abs (m_blockPpq - expected) > kJumpToleranceSlots * slotQn)
                discontinuity = true;
        }
    }
    else if (m_haveAnchor)
    {
        m_blockPpq = m_lastPpq + static_cast<double> (m_lastNumSamples) / m_lastSpq;
    }

    if (discontinuity)
    {
        if (hasVoices())
        {
            emitAllNotesOff (out, 0);
            clearVoices();
        }

        m_sequencer.reset();
        m_slot = {};
        m_nextSlot = slotAtOrAfter (m_blockPpq);
        m_nextSlotOffset = (slotTimeQn (m_nextSlot) - m_blockPpq) * m_blockSpq;
        m_haveAnchor = true;
        return;
    }

    // Fortschreibung gegen die Host-ppq pruefen; bei Abweichung gilt der Host.
    const double recomputed = (slotTimeQn (m_nextSlot) - m_blockPpq) * m_blockSpq;
    if (std::abs (recomputed - m_nextSlotOffset) > kOffsetToleranceSamples)
    {
        m_nextSlotOffset = recomputed;

        // Verpasste Kanten aufholen (z. B. Tempoaenderung).
        while (m_nextSlotOffset < -kOffsetToleranceSamples)
        {
            ++m_nextSlot;
            m_nextSlotOffset = (slotTimeQn (m_nextSlot) - m_blockPpq) * m_blockSpq;
        }
    }

    if (m_nextSlotOffset < 0.0)
        m_nextSlotOffset = 0.0;
}

void StepEngine::applyInputEvents (const std::vector<TimedEvent>& input)
{
    for (const auto& event : input)
        applyInput (event);
}

void StepEngine::applyInput (const TimedEvent& event)
{
    if (event.type == EventType::AllNotesOff)
    {
        m_notes.releaseAll();
    }
    else if (event.type == EventType::NoteOn && event.velocity > 0)
    {
        m_notes.noteOn (event.pitch, event.velocity);
    }
    else
    {
        m_notes.noteOff (event.pitch);
    }

    if (m_notes.active())
        m_planner.setNote (m_notes.pitch(), m_notes.velocity());
    else
        m_planner.clearNote();
}

// --- Slots -------------------------------------------------------------------

void StepEngine::startSlot (int offsetSamples)
{
    if (m_resetPending)
    {
        m_sequencer.reset();
        m_resetPending = false;
    }

    const std::int64_t slotIndex = m_nextSlot;
    ++m_nextSlot;
    m_nextSlotOffset = (slotTimeQn (m_nextSlot) - m_blockPpq) * m_blockSpq;

    m_slot = {};

    int stepIndex = -1;
    if (! m_sequencer.nextStep (stepIndex))
        return;

    const StepPlan plan = m_planner.planSlot (stepIndex, slotIndex);
    if (! plan.valid || plan.hits <= 0)
        return;

    m_slot.active = true;
    m_slot.plan = plan;
    m_slot.nextHit = 0;
    m_slot.firstHitOffset = static_cast<double> (offsetSamples);
    m_slot.spacingSamples = plan.hitSpacingSeconds * m_sampleRate;
}

void StepEngine::fireHit (int offsetSamples, EventList& out)
{
    const StepPlan& plan = m_slot.plan;
    const auto& step = m_settings.steps[static_cast<std::size_t> (plan.stepIndex)];
    const int offset = std::max (0, offsetSamples);

    // Die Wahrscheinlichkeit wird pro Stimme und Treffer gewuerfelt
    // (Entscheidung 65).
    std::array<bool, kNumVoices> hit {};
    for (int voice = 0; voice < kNumVoices; ++voice)
        hit[static_cast<std::size_t> (voice)] = rollProbability (step.probability, random01());

    const StepActions actions = applyStep (plan, hit, m_voiceState);

    for (int voice = 0; voice < kNumVoices; ++voice)
    {
        const auto index = static_cast<std::size_t> (voice);

        if (actions.noteOff[index] && m_sounding[index].active)
        {
            out.add (TimedEvent { offset, EventType::NoteOff, m_sounding[index].pitch, 0,
                                  m_sounding[index].channel });
            m_sounding[index].active = false;
            m_pending[index].active = false;
        }

        if (! actions.noteOn[index])
            continue;

        const auto& trigger = plan.voices[index];
        out.add (TimedEvent { offset, EventType::NoteOn, trigger.pitch, trigger.velocity, trigger.channel });

        m_sounding[index] = { true, trigger.pitch, trigger.channel };
        m_pending[index] = {};

        if (plan.mode != StepMode::Hold)
        {
            m_pending[index].active = true;
            m_pending[index].pitch = trigger.pitch;
            m_pending[index].channel = trigger.channel;
            m_pending[index].samplesLeft = static_cast<double> (offset)
                                           + trigger.gateSeconds * m_sampleRate;
        }
    }

    ++m_slot.nextHit;

    if (m_slot.nextHit >= plan.hits)
        m_slot.active = false;
}

void StepEngine::emitPendingOffs (int offsetSamples, EventList& out)
{
    for (int voice = 0; voice < kNumVoices; ++voice)
    {
        const auto index = static_cast<std::size_t> (voice);
        auto& pending = m_pending[index];

        if (! pending.active)
            continue;

        const int due = std::max (0, static_cast<int> (std::llround (pending.samplesLeft)));
        if (due > offsetSamples)
            continue;

        if (m_sounding[index].active)
        {
            out.add (TimedEvent { offsetSamples, EventType::NoteOff, m_sounding[index].pitch, 0,
                                  m_sounding[index].channel });
            m_sounding[index].active = false;
        }

        // Das Gate-Ende beendet die Note auch im Zustandsautomaten; sonst
        // wuerde applyStep beim naechsten Treffer ein zweites Note-Off planen.
        m_voiceState[index].sounding = false;
        m_voiceState[index].held = false;
        pending.active = false;
    }
}

void StepEngine::emitAllNotesOff (EventList& out, int offsetSamples)
{
    std::array<bool, 16> used {};

    const auto mark = [&used] (int channel)
    {
        if (channel >= 1 && channel <= 16)
            used[static_cast<std::size_t> (channel - 1)] = true;
    };

    mark (m_settings.thru.channel);

    if (m_settings.outputMode == OutputMode::Multi)
        for (const auto& voice : m_settings.harmony)
            mark (voice.channel);

    for (int channel = 0; channel < 16; ++channel)
        if (used[static_cast<std::size_t> (channel)])
            out.add (TimedEvent { offsetSamples, EventType::AllNotesOff, 0, 0, channel + 1 });
}

void StepEngine::clearVoices() noexcept
{
    m_sounding = {};
    m_pending = {};
    m_voiceState = {};
}

bool StepEngine::hasVoices() const noexcept
{
    for (int voice = 0; voice < kNumVoices; ++voice)
    {
        const auto index = static_cast<std::size_t> (voice);
        if (m_sounding[index].active || m_pending[index].active || m_voiceState[index].sounding)
            return true;
    }

    return false;
}

// --- Zeit --------------------------------------------------------------------

double StepEngine::slotTimeQn (std::int64_t slotIndex) const noexcept
{
    return m_clock.slotTimeSeconds (slotIndex) * m_bpm / 60.0;
}

std::int64_t StepEngine::slotAtOrAfter (double ppq) const noexcept
{
    const double slotQn = m_clock.stepSeconds() * m_bpm / 60.0;
    if (slotQn <= 0.0)
        return 0;

    const double slackQn = m_clock.slackSeconds() * m_bpm / 60.0;
    const double swingQn = m_clock.swingSeconds() * m_bpm / 60.0;
    const double eps = 1e-9;

    const auto timeQn = [slotQn, swingQn, slackQn] (std::int64_t n)
    {
        const double base = static_cast<double> (n) * slotQn + slackQn;
        return (n % 2 != 0) ? base + swingQn : base;
    };

    auto n = static_cast<std::int64_t> (std::floor ((ppq - slackQn) / slotQn));
    n = std::max<std::int64_t> (n, -4);

    while (n > -4 && timeQn (n) > ppq + eps)
        --n;

    while (timeQn (n + 1) <= ppq + eps)
        ++n;

    // n ist der Slot, der ppq enthaelt. Beginnt er vor ppq, folgt der naechste.
    if (timeQn (n) < ppq - eps)
        ++n;

    return std::max<std::int64_t> (n, 0);
}

int StepEngine::nextHitOffset() const noexcept
{
    if (! m_slot.active)
        return kNoEvent;

    const double offset = m_slot.firstHitOffset
                          + static_cast<double> (m_slot.nextHit) * m_slot.spacingSamples;

    return static_cast<int> (std::llround (std::max (0.0, offset)));
}

int StepEngine::nextPendingOffOffset() const noexcept
{
    int best = kNoEvent;

    for (const auto& pending : m_pending)
        if (pending.active)
            best = std::min (best, std::max (0, static_cast<int> (std::llround (pending.samplesLeft))));

    return best;
}

double StepEngine::random01()
{
    if (m_randomSource)
        return m_randomSource();

    std::uniform_real_distribution<double> distribution (0.0, 1.0);
    return distribution (m_rng);
}

} // namespace heptachord
