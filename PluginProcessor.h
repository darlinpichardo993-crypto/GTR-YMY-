#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include "Engine.h"
#include "KeyTracker.h"
#include "Presets.h"

class GtrProcessor : public juce::AudioProcessor
{
public:
    GtrProcessor();
    ~GtrProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "GTR ORYMY RD"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override { return (int) gtr::presets().size(); }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    KeyTracker& getKeyTracker() { return keyTracker; }

    juce::AudioProcessorValueTreeState apvts;

private:
    EngineParams snapshot() const;

    GtrEngine engine;
    KeyTracker keyTracker;
    std::array<std::atomic<float>*, gtr::kNumParams> raw {};
    int currentProgram = 0;
    std::array<float, 2048> monoTmp {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GtrProcessor)
};
