#include "plugin/ParameterBinding.h"

namespace heptachord
{

void ParameterBinding::connect (juce::AudioProcessorValueTreeState& apvts)
{
    for (int i = 0; i < params::kNumParameters; ++i)
        m_values[static_cast<std::size_t> (i)] = apvts.getRawParameterValue (params::kParams[i].id);

    m_connected = true;
}

ParameterValues ParameterBinding::current() const noexcept
{
    ParameterValues values = defaultParameterValues();

    for (int i = 0; i < params::kNumParameters; ++i)
        if (const auto* value = m_values[static_cast<std::size_t> (i)])
            values[static_cast<std::size_t> (i)] = value->load (std::memory_order_relaxed);

    return values;
}

float ParameterBinding::get (int index) const noexcept
{
    if (index < 0 || index >= params::kNumParameters)
        return 0.0f;

    if (const auto* value = m_values[static_cast<std::size_t> (index)])
        return value->load (std::memory_order_relaxed);

    return params::kParams[static_cast<std::size_t> (index)].defaultValue;
}

} // namespace heptachord
