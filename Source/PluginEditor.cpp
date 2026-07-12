#include "PluginEditor.h"

namespace
{
    // Deterministic pseudo-waveform bars used as pure decoration on each side
    // of the knob -- ties the plugin UI back to the "before/after waveform"
    // visual identity used in the TikTok clips and avatar, without needing to
    // bundle an image asset via BinaryData.
    std::vector<float> makeBarHeights(int count, float seed)
    {
        std::vector<float> heights;
        heights.reserve((size_t)count);
        for (int i = 0; i < count; ++i)
        {
            const float t = (float)i / (float)juce::jmax(1, count - 1);
            const float envelope = std::pow(1.0f - t, 1.6f);
            const float wobble = 0.55f + 0.45f * std::sin(seed + t * 27.0f);
            heights.push_back(juce::jlimit(0.06f, 1.0f, envelope * wobble));
        }
        return heights;
    }
}

FinisherEditor::FinisherEditor(FinisherProcessor& p)
    : juce::AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&lookAndFeel);

    titleLabel.setText("MONTAGEM FINISHER", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setFont(juce::Font(juce::FontOptions(26.0f, juce::Font::bold)));
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("DRIVE & LOUDNESS  /  ONE-KNOB MASTER", juce::dontSendNotification);
    subtitleLabel.setJustificationType(juce::Justification::centred);
    subtitleLabel.setColour(juce::Label::textColourId, FinisherLookAndFeel::textDim);
    subtitleLabel.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::plain)));
    addAndMakeVisible(subtitleLabel);

    amountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    amountSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    amountSlider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,
                                      juce::MathConstants<float>::pi * 2.8f, true);
    addAndMakeVisible(amountSlider);

    amountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "amount", amountSlider);

    amountValueLabel.setJustificationType(juce::Justification::centred);
    amountValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    amountValueLabel.setFont(juce::Font(juce::FontOptions(20.0f, juce::Font::bold)));
    addAndMakeVisible(amountValueLabel);

    footerLabel.setText("AMOUNT", juce::dontSendNotification);
    footerLabel.setJustificationType(juce::Justification::centred);
    footerLabel.setColour(juce::Label::textColourId, FinisherLookAndFeel::textDim);
    footerLabel.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    addAndMakeVisible(footerLabel);

    amountSlider.onValueChange = [this] { updateValueLabel(); repaint(); };
    updateValueLabel();

    brandLabel.setText("@montagem.finisher", juce::dontSendNotification);
    brandLabel.setJustificationType(juce::Justification::centredRight);
    brandLabel.setColour(juce::Label::textColourId, FinisherLookAndFeel::textDim.withAlpha(0.5f));
    brandLabel.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::plain)));
    addAndMakeVisible(brandLabel);

    setResizable(false, false);
    setSize(480, 360);
}

FinisherEditor::~FinisherEditor()
{
    setLookAndFeel(nullptr);
}

void FinisherEditor::updateValueLabel()
{
    const int pct = (int)std::round(amountSlider.getValue() * 100.0);
    amountValueLabel.setText(juce::String(pct) + "%", juce::dontSendNotification);
}

void FinisherEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient bgGradient(FinisherLookAndFeel::bg.brighter(0.03f), bounds.getCentre(),
                                     FinisherLookAndFeel::bg.darker(0.15f), bounds.getBottomLeft(), true);
    g.setGradientFill(bgGradient);
    g.fillAll();

    const float amount = (float)amountSlider.getValue();

    auto knobArea = amountSlider.getBounds().toFloat();
    const float barAreaWidth = 70.0f;
    const int barCount = 14;
    const float barWidth = 3.5f;
    const float baseY = knobArea.getCentreY();
    const float maxBarHalfHeight = knobArea.getHeight() * 0.42f;

    // red bars, decaying outward to the left of the knob
    {
        auto heights = makeBarHeights(barCount, 1.7f);
        const float startX = knobArea.getX() - 18.0f;
        for (int i = 0; i < barCount; ++i)
        {
            const float h = heights[(size_t)i] * maxBarHalfHeight * (0.4f + 0.6f * amount);
            const float x = startX - (float)i * (barWidth + 3.0f) - barWidth;
            g.setColour(FinisherLookAndFeel::red.withAlpha(0.85f));
            g.fillRoundedRectangle(x, baseY - h, barWidth, h * 2.0f, barWidth * 0.5f);
            if (x < barAreaWidth) break;
        }
    }

    // green bars, growing outward to the right of the knob
    {
        auto heights = makeBarHeights(barCount, 4.2f);
        const float startX = knobArea.getRight() + 18.0f;
        for (int i = 0; i < barCount; ++i)
        {
            const float h = heights[(size_t)i] * maxBarHalfHeight * (0.5f + 0.5f * amount);
            const float x = startX + (float)i * (barWidth + 3.0f);
            g.setColour(FinisherLookAndFeel::green.withAlpha(0.85f));
            g.fillRoundedRectangle(x, baseY - h, barWidth, h * 2.0f, barWidth * 0.5f);
            if (x > bounds.getWidth() - barAreaWidth) break;
        }
    }
}

void FinisherEditor::resized()
{
    auto area = getLocalBounds().reduced(16);

    titleLabel.setBounds(area.removeFromTop(36));
    subtitleLabel.setBounds(area.removeFromTop(20));

    area.removeFromTop(8);
    brandLabel.setBounds(area.removeFromBottom(14));
    footerLabel.setBounds(area.removeFromBottom(18));
    amountValueLabel.setBounds(area.removeFromBottom(28));

    const int knobSize = 180;
    juce::Rectangle<int> knobArea(0, 0, knobSize, knobSize);
    knobArea.setCentre(area.getCentre());
    amountSlider.setBounds(knobArea);
}
