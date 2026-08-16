#include "PluginEditor.h"

DirtyBassAudioProcessorEditor::DirtyBassAudioProcessorEditor(DirtyBassAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(620, 260);
    for (size_t i = 0; i < sliders.size(); ++i)
    {
        sliders[i] = std::make_unique<juce::Slider>();
        sliders[i]->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        sliders[i]->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
        addAndMakeVisible(*sliders[i]);

        labels[i] = std::make_unique<juce::Label>();
        labels[i]->setText(names[i], juce::dontSendNotification);
        labels[i]->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*labels[i]);

        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.apvts, ids[i], *sliders[i]);
    }
}

void DirtyBassAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(26.0f, juce::Font::bold));
    g.drawFittedText("DIRTY BASS", 20, 14, getWidth() - 40, 36, juce::Justification::centred, 1);
    g.setFont(12.0f);
    g.drawFittedText("dynamic harmonic bass saturation  /  v0.1", 20, 45, getWidth() - 40, 20,
                     juce::Justification::centred, 1);
}

void DirtyBassAudioProcessorEditor::resized()
{
    const int top = 72;
    const int w = getWidth() / 6;
    for (size_t i = 0; i < sliders.size(); ++i)
    {
        sliders[i]->setBounds((int)i * w + 5, top + 22, w - 10, 130);
        labels[i]->setBounds((int)i * w + 5, top, w - 10, 24);
    }
}
