#pragma once
#include <JuceHeader.h>
#include <array>

class ORYMYBassAudioProcessor : public juce::AudioProcessor {
public:
    ORYMYBassAudioProcessor();
    ~ORYMYBassAudioProcessor() override = default;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "ORYMY Bass"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState parameters;
    int getCurrentMidiNote() const { return currentMidiNote.load(); }
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
private:
    std::array<std::array<std::unique_ptr<juce::dsp::IIR::Filter<float>>,2>,10> eqFilters;
    std::array<std::array<std::unique_ptr<juce::dsp::IIR::Filter<float>>,2>,3> toneFilters;
    juce::dsp::Compressor<float> compressor;
    juce::dsp::ProcessSpec spec {};
    std::atomic<int> currentMidiNote {-1};
    float gateEnvelope = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ORYMYBassAudioProcessor)
};
