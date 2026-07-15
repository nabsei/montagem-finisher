#include "PluginEditor.h"

namespace
{
    // Deterministic pseudo-waveform bars used as pure decoration on each side
    // of the knob, mirrored (same heights, same colour) so the two sides
    // always match -- without needing to bundle an image asset via BinaryData.
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

    setResizable(true, true);
    setResizeLimits(360, 280, 900, 700);
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

    // Both sides now share one height pattern and one colour (interpolated
    // red -> green with amount, matching the knob) instead of a fixed
    // red-left/green-right split with two different random seeds and two
    // different scaling formulas. The old asymmetry (different shape, and
    // colour that never moved with amount) read as a bug, not a design choice.
    const float amount = (float)amountSlider.getValue();
    const auto colour = FinisherLookAndFeel::red.interpolatedWith(FinisherLookAndFeel::green, amount);

    auto knobArea = amountSlider.getBounds().toFloat();
    const float barAreaWidth = 70.0f;
    const int barCount = 14;
    const float barWidth = 3.5f;
    const float baseY = knobArea.getCentreY();
    const float maxBarHalfHeight = knobArea.getHeight() * 0.42f;
    const float scale = 0.4f + 0.6f * amount;
    auto heights = makeBarHeights(barCount, 1.7f);

    // left side
    {
        const float startX = knobArea.getX() - 18.0f;
        for (int i = 0; i < barCount; ++i)
        {
            const float h = heights[(size_t)i] * maxBarHalfHeight * scale;
            const float x = startX - (float)i * (barWidth + 3.0f) - barWidth;
            g.setColour(colour.withAlpha(0.85f));
            g.fillRoundedRectangle(x, baseY - h, barWidth, h * 2.0f, barWidth * 0.5f);
            if (x < barAreaWidth) break;
        }
    }

    // right side -- mirror of the left, same heights/colour/scale
    {
        const float startX = knobArea.getRight() + 18.0f;
        for (int i = 0; i < barCount; ++i)
        {
            const float h = heights[(size_t)i] * maxBarHalfHeight * scale;
            const float x = startX + (float)i * (barWidth + 3.0f);
            g.setColour(colour.withAlpha(0.85f));
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

    // Scale with the available area (clamped) instead of a fixed size, so
    // resizing the window actually grows/shrinks the knob rather than just
    // leaving dead space around a fixed-size control.
    const int knobSize = juce::jlimit(120, 260, juce::jmin(area.getWidth(), area.getHeight()) - 40);
    juce::Rectangle<int> knobArea(0, 0, knobSize, knobSize);
    knobArea.setCentre(area.getCentre());
    amountSlider.setBounds(knobArea);
}
