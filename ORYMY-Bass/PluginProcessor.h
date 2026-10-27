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
    int getCurrentMidiNote() const { const int midi=currentMidiNote.load(); return midi >= 0 ? midi : detectedAudioNote.load(); }
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
private:
    std::array<std::array<std::unique_ptr<juce::dsp::IIR::Filter<float>>,2>,10> eqFilters;
    std::array<std::array<std::unique_ptr<juce::dsp::IIR::Filter<float>>,2>,3> toneFilters;
    juce::dsp::Compressor<float> compressor;
    juce::dsp::ProcessSpec spec {};
    std::atomic<int> currentMidiNote {-1};
    std::atomic<int> detectedAudioNote {-1};
    std::array<float,4096> pitchHistory {};
    int pitchWriteIndex = 0;
    int pitchSamples = 0;
    int pitchAnalysisCountdown = 0;
    void updateAudioPitch(const juce::AudioBuffer<float>& buffer);
    std::array<float,2> gateEnvelope {{0.0f,0.0f}};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ORYMYBassAudioProcessor)
};
