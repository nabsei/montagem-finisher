#pragma once
#include <functional>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

// One-button "genre finisher": a single macro parameter drives a small signal
// chain (drive -> tone shaping -> output limiting) end to end.
//
// NOTE: this is the public / portfolio version of the processor. The tuned
// DSP internals (exact filter topology, envelope-follower time constants,
// makeup-gain calibration) that make the shipped plugin sound "finished" are
// intentionally simplified/omitted here -- this file demonstrates the JUCE
// architecture (parameters, editor, state save/load), not the production
// audio algorithm.
class FinisherProcessor : public juce::AudioProcessor
{
public:
    FinisherProcessor();
    ~FinisherProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Montagem Finisher"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::dsp::WaveShaper<float, std::function<float(float)>> saturator;

    // Simple one-pole tone-tilt state (illustrative -- see class comment above).
    float lowpassState[2] = { 0.0f, 0.0f };
    float lowpassCoef = 0.0f;

    // Simple envelope-follower limiter (illustrative -- see class comment above).
    float limiterEnvelope[2] = { 0.0f, 0.0f };

    double lastSampleRate = 44100.0;

    // Smooths the "amount" parameter itself so downstream coefficients ramp over
    // ~30ms instead of jumping instantly block-to-block -- moving the knob live,
    // or automating it, would otherwise cause an audible click at the block boundary.
    juce::SmoothedValue<float> amountSmoothed;

    void updateFromAmount(float amount01);
};
