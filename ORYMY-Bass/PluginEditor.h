#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>
#include <memory>

class ORYMYBassAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit ORYMYBassAudioProcessorEditor(ORYMYBassAudioProcessor&);
    ~ORYMYBassAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    void setupKnob(juce::Slider&, const juce::String&);
    void applySelectedPreset();
    ORYMYBassAudioProcessor& audioProcessor;
    juce::ComboBox presetBox, scaleBox;
    juce::Label noteLabel, scaleLabel, brandLabel, subtitleLabel;
    std::array<juce::Slider,10> knobs;
    std::array<juce::Label,10> knobLabels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,10> knobAttachments;
    std::array<juce::Slider,10> eqSliders;
    std::array<juce::Label,10> eqLabels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,10> eqAttachments;
    std::array<juce::String,10> knobIds {{ "input","bass","mid","treble","presence","master","gate","attack","comp","drive" }};
    std::array<juce::String,10> knobNames {{ "INPUT","BASS","MID","TREBLE","PRESENCE","MASTER","NOISE GATE","ATTACK","COMPRESSOR","DRIVE" }};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ORYMYBassAudioProcessorEditor)
};
