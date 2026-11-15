#include "plugin/PluginProcessor.h"

#include "plugin/ParameterLayout.h"
#include "plugin/PluginEditor.h"

namespace heptachord
{

Processor::Processor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

Processor::~Processor() = default;

void Processor::prepareToPlay (double, int)
{
}

void Processor::releaseResources()
{
}

bool Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet().isDisabled()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void Processor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Kein Audio: der Bus ist still, damit Live/Cubase das Instrument laden.
    buffer.clear();

    // M0: Noten unveraendert durchreichen (Identitaet). M1 ersetzt das durch
    // Step-Engine und Stimmenausgabe.
    juce::ignoreUnused (midiMessages);
}

juce::AudioProcessorEditor* Processor::createEditor()
{
    return new Editor (*this);
}

void Processor::getStateInformation (juce::MemoryBlock& destData)
{
    if (const auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void Processor::setStateInformation (const void* data, int sizeInBytes)
{
    if (const auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

} // namespace heptachord

// Der Wrapper erzeugt die Instanz ueber diese Factory.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new heptachord::Processor();
}
