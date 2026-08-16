#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class DirtyBassAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit DirtyBassAudioProcessorEditor(DirtyBassAudioProcessor&);
    ~DirtyBassAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DirtyBassAudioProcessor& processor;
    std::array<std::unique_ptr<juce::Slider>, 6> sliders;
    std::array<std::unique_ptr<juce::Label>, 6> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 6> attachments;
    const std::array<juce::String, 6> ids { "DRIVE", "GRIT", "BODY", "BITE", "MIX", "OUTPUT" };
    const std::array<juce::String, 6> names { "DRIVE", "GRIT", "BODY", "BITE", "MIX", "OUTPUT" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirtyBassAudioProcessorEditor)
};
