#include "core/StepPlanner.h"

#include <algorithm>
#include <cmath>

namespace heptachord
{
namespace
{
int clampVelocity (double value) noexcept
{
    return std::clamp (static_cast<int> (std::lround (value)), 1, 127);
}
} // namespace

void StepPlanner::setSettings (const EngineSettings& settings)
{
    m_settings = settings;
}

void StepPlanner::setClock (const StepClock& clock)
{
    m_clock = clock;
}

void StepPlanner::setNote (int note, int velocity)
{
    m_note = std::clamp (note, 0, 127);
    m_inputVelocity = std::clamp (velocity, 1, 127);
    m_noteActive = true;
}

void StepPlanner::clearNote()
{
    m_noteActive = false;
}

int StepPlanner::voicePitch (int voiceIndex, const StepSettings& step, int baseGrad) const
{
    if (voiceIndex == kVoiceThru)
    {
        if (! m_settings.thru.snap)
            return m_note;

        return snapToScale (m_note, m_settings.scale, m_settings.root);
    }

    const auto& voice = m_settings.harmony[static_cast<std::size_t> (voiceIndex - 1)];

    // Grad = Basisgrad + Voice-Grad + Step-Offset (alle diatonisch).
    const int grade = baseGrad + voice.degree + step.offset;
    const int semitone = degreeToSemitone (grade, m_settings.scale);
    const int pitch = m_settings.root + semitone + 12 * voice.octave;

    return std::clamp (pitch, 0, 127);
}

int StepPlanner::voiceVelocity (int voiceIndex, const StepSettings& step) const
{
    const double stepFactor = static_cast<double> (step.velocity) / 100.0;

    if (voiceIndex == kVoiceThru)
    {
        const double thruFactor = static_cast<double> (m_settings.thru.velocity) / 100.0;
        return clampVelocity (static_cast<double> (m_inputVelocity) * thruFactor * stepFactor);
    }

    const auto& voice = m_settings.harmony[static_cast<std::size_t> (voiceIndex - 1)];
    const double harmFactor = static_cast<double> (m_settings.harmVelocity) / 100.0;
    const double voiceFactor = voice.follow ? 1.0 : static_cast<double> (voice.velocity) / 100.0;

    // spread_i = 1 - spread * (i - 1) / 2, i = 1 ... 3
    const double spreadFactor = 1.0 - static_cast<double> (m_settings.harmSpread) * (voiceIndex - 1) / 2.0;

    return clampVelocity (static_cast<double> (m_inputVelocity) * harmFactor * voiceFactor
                          * spreadFactor * stepFactor);
}

double StepPlanner::voiceGate (int voiceIndex, double stepSeconds, int hits) const
{
    double gate = stepSeconds;

    if (voiceIndex != kVoiceThru)
    {
        const auto& voice = m_settings.harmony[static_cast<std::size_t> (voiceIndex - 1)];
        gate = std::clamp (static_cast<double> (voice.gate), 0.0, 100.0) / 100.0 * stepSeconds;
    }

    // Bei Ratchet darf das Gate nicht in den naechsten Treffer laufen.
    if (hits > 1)
        gate = std::min (gate, stepSeconds / hits);

    return gate;
}

int StepPlanner::voiceChannel (int voiceIndex) const
{
    if (m_settings.outputMode == OutputMode::Single)
        return m_settings.thru.channel;

    if (voiceIndex == kVoiceThru)
        return m_settings.thru.channel;

    return m_settings.harmony[static_cast<std::size_t> (voiceIndex - 1)].channel;
}

StepPlan StepPlanner::planSlot (int stepIndex, std::int64_t slotIndex) const
{
    StepPlan plan;

    if (stepIndex < 0 || stepIndex >= kNumSteps)
        return plan;

    const auto& step = m_settings.steps[static_cast<std::size_t> (stepIndex)];
    if (! step.active)
        return plan;

    plan.valid = true;
    plan.stepIndex = stepIndex;
    plan.slotIndex = slotIndex;
    plan.startSeconds = m_clock.slotTimeSeconds (slotIndex);
    plan.mode = step.mode;

    if (step.mode == StepMode::Rest || ! m_noteActive)
    {
        plan.hits = 0;
        return plan;
    }

    const double stepSeconds = m_clock.stepSeconds();
    plan.hits = (step.mode == StepMode::Ratchet) ? std::clamp (step.ratchet, 2, 8) : 1;
    plan.hitSpacingSeconds = (plan.hits > 1) ? stepSeconds / plan.hits : stepSeconds;

    const int baseGrad = baseDegree (m_note, m_settings.root, m_settings.scale);

    for (int voiceIndex = 0; voiceIndex < kNumVoices; ++voiceIndex)
    {
        auto& trigger = plan.voices[static_cast<std::size_t> (voiceIndex)];

        const bool masked = (step.voiceMask & (1u << voiceIndex)) != 0;
        const bool enabled = (voiceIndex == kVoiceThru)
                                 ? m_settings.thru.enable
                                 : m_settings.harmony[static_cast<std::size_t> (voiceIndex - 1)].enable;

        trigger.selected = masked && enabled;
        if (! trigger.selected)
            continue;

        trigger.pitch = voicePitch (voiceIndex, step, baseGrad);
        trigger.velocity = voiceVelocity (voiceIndex, step);
        trigger.gateSeconds = voiceGate (voiceIndex, stepSeconds, plan.hits);
        trigger.channel = voiceChannel (voiceIndex);
    }

    return plan;
}

bool rollProbability (float percent, double random01) noexcept
{
    const double probability = std::clamp (static_cast<double> (percent), 0.0, 100.0) / 100.0;
    return random01 < probability;
}

StepActions applyStep (const StepPlan& plan,
                        const std::array<bool, kNumVoices>& hit,
                        std::array<VoiceState, kNumVoices>& state)
{
    StepActions actions;

    if (! plan.valid || plan.hits <= 0)
        return actions;   // Rest oder keine Note: nichts aendern

    for (int voiceIndex = 0; voiceIndex < kNumVoices; ++voiceIndex)
    {
        const auto index = static_cast<std::size_t> (voiceIndex);
        auto& voiceState = state[index];
        const bool selected = plan.voices[index].selected;

        if (! selected)
        {
            // Stimme ist in diesem Step nicht maskiert: eine laufende Note
            // endet hier.
            if (voiceState.sounding)
            {
                actions.noteOff[index] = true;
                voiceState.sounding = false;
                voiceState.held = false;
            }
            continue;
        }

        if (! hit[index])
            continue;   // Wurf hat den Trigger verhindert

        if (plan.mode == StepMode::Hold)
        {
            // Hold: nur einmal anschlagen, dann weiter halten.
            if (! voiceState.sounding)
            {
                actions.noteOn[index] = true;
                voiceState.sounding = true;
            }
            voiceState.held = true;
            continue;
        }

        // Play und Ratchet: alte Note beenden, neue anschlagen.
        if (voiceState.sounding)
            actions.noteOff[index] = true;

        actions.noteOn[index] = true;
        voiceState.sounding = true;
        voiceState.held = false;
    }

    return actions;
}

} // namespace heptachord
