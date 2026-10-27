#pragma once
#include "PluginProcessor.h"
#include "PedalUI.h"
#include <functional>
#include <vector>

class GtrEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit GtrEditor (GtrProcessor&);
    ~GtrEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    static constexpr int kW = 1360, kH = 800;      // tamano de diseno; la ventana escala desde aqui

    struct Content : public juce::Component
    {
        std::function<void (juce::Graphics&)> onPaint;
        void paint (juce::Graphics& g) override { if (onPaint) onPaint (g); }
    };

    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void timerCallback() override;
    void paintContent (juce::Graphics&);
    void setupKnob (juce::Slider&, const char* paramId);
    void setupVSlider (juce::Slider&, const char* paramId, bool valueText, int ticks, bool bipolar);

    GtrProcessor& proc;

    pedal::PedalLookAndFeel laf;                    // se declara antes que los componentes que lo usan
    Content content;

    juce::Slider inKnob, outKnob, levelKnob, compKnob, wahKnob, resKnob, speedKnob, widthKnob, preKnob, mixKnob;
    juce::Slider attackSl, lowSl, midSl, hiSl, enhSl, toneLvSl, modeSl, typeSl;
    std::vector<std::unique_ptr<SliderAtt>> sliderAtt;
    std::vector<std::unique_ptr<ButtonAtt>> buttonAtt;

    pedal::PedalButton compBtn { "COMP/LIMITER" }, toneBtn { "TONE SHAPER" }, wahBtn { "WAH" },
                       chorusBtn { "STEREO CHORUS" }, reverbBtn { "DIGITAL REVERB" };

    juce::ComboBox presetBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, resetBtn { "RESET" };
    juce::Label keyLine, scaleLine, noteLine;
    pedal::KeyboardView keyboard;

    int lastProgram = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GtrEditor)
};
