#pragma once
#include "FinisherLookAndFeel.h"
#include "FinisherProcessor.h"

class FinisherEditor : public juce::AudioProcessorEditor
{
public:
    explicit FinisherEditor(FinisherProcessor&);
    ~FinisherEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    FinisherProcessor& processorRef;
    FinisherLookAndFeel lookAndFeel;

    juce::Slider amountSlider;
    juce::Label amountValueLabel;
    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label footerLabel;
    juce::Label brandLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttachment;

    void updateValueLabel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FinisherEditor)
};
