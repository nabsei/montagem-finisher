#include "FinisherProcessor.h"
#include "PluginEditor.h"

FinisherProcessor::FinisherProcessor()
    : AudioProcessor(BusesProperties()
                          .withInput("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "STATE", createParameterLayout())
{
    saturator.functionToUse = [](float x) { return std::tanh(x); };
}

juce::AudioProcessorValueTreeState::ParameterLayout FinisherProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // The single knob the user touches. Everything else derives from it.
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "amount", "Amount", juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    return { params.begin(), params.end() };
}

void FinisherProcessor::prepareToPlay(double sampleRate, int)
{
    lastSampleRate = sampleRate;

    lowpassCoef = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * 1200.0f / (float)sampleRate);

    amountSmoothed.reset(sampleRate, 0.03);
    amountSmoothed.setCurrentAndTargetValue(*apvts.getRawParameterValue("amount"));

    updateFromAmount(amountSmoothed.getCurrentValue());
}

void FinisherProcessor::updateFromAmount(float amount01)
{
    // Amount maps to drive and output limiting -- one macro instead of separate
    // knobs, matching the "single button" product design. The exact curve shape
    // shipped in the production plugin is tuned against reference material and
    // is not reproduced in this public version.
    const float driveDb = juce::jmap(amount01, 0.0f, 1.0f, 0.0f, 12.0f);
    const float driveGain = juce::Decibels::decibelsToGain(driveDb);
    saturator.functionToUse = [driveGain](float x) { return std::tanh(x * driveGain) / std::tanh(driveGain); };
}

void FinisherProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    amountSmoothed.setTargetValue(*apvts.getRawParameterValue("amount"));
    const float amount01 = amountSmoothed.skip(buffer.getNumSamples());
    updateFromAmount(amount01);

    const float hfBoost = amount01 * 0.3f;
    const float limThresholdLin = juce::Decibels::decibelsToGain(juce::jmap(amount01, 0.0f, 1.0f, -1.0f, -6.0f));
    const float limAttackCoef = std::exp(-1.0f / (float)(lastSampleRate * 0.002));
    const float limReleaseCoef = std::exp(-1.0f / (float)(lastSampleRate * 0.060));

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* samples = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            // Drive
            float x = saturator.functionToUse(samples[i]);

            // Simple tone tilt
            lowpassState[ch] += lowpassCoef * (x - lowpassState[ch]);
            x = x + hfBoost * (x - lowpassState[ch]);

            // Simple output limiter
            const float ax = std::abs(x);
            const float coef = ax > limiterEnvelope[ch] ? limAttackCoef : limReleaseCoef;
            limiterEnvelope[ch] = coef * limiterEnvelope[ch] + (1.0f - coef) * ax;
            const float gainReduction = limiterEnvelope[ch] > limThresholdLin
                                             ? limThresholdLin / limiterEnvelope[ch]
                                             : 1.0f;
            samples[i] = x * gainReduction;
        }
    }
}

juce::AudioProcessorEditor* FinisherProcessor::createEditor()
{
    return new FinisherEditor(*this);
}

void FinisherProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
        if (auto xml = state.createXml())
            copyXmlToBinary(*xml, destData);
}

void FinisherProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}
