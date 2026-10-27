#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
    {
        using FA = juce::AudioParameterFloatAttributes;
        using NR = juce::NormalisableRange<float>;
        juce::AudioProcessorValueTreeState::ParameterLayout l;

        auto addF = [&l] (const char* id, const char* name, NR range, float def, const juce::String& unit, int decimals)
        {
            l.add (std::make_unique<juce::AudioParameterFloat> (
                juce::ParameterID { id, 1 }, name, range, def,
                FA().withStringFromValueFunction ([unit, decimals] (float v, int) { return juce::String (v, decimals) + unit; })));
        };
        auto addB = [&l] (const char* id, const char* name, bool def)
        {
            l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { id, 1 }, name, def));
        };
        auto addC = [&l] (const char* id, const char* name, const juce::StringArray& items, int def)
        {
            l.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { id, 1 }, name, items, def));
        };

        // El orden debe coincidir con gtr::paramIds()
        addF ("inGain",    "Master Input", NR (-24.0f, 12.0f, 0.1f), 0.0f, " dB", 1);
        addB ("compOn",    "Comp On", true);
        addF ("compAmt",   "Compression", NR (0.0f, 10.0f, 0.1f), 4.0f, "", 1);
        addF ("compLevel", "Comp Level", NR (-6.0f, 12.0f, 0.1f), 2.0f, " dB", 1);
        addC ("attack",    "Attack", { "SOFT", "MID", "HARD" }, 1);
        addB ("toneOn",    "Tone On", true);
        addF ("low",       "Low", NR (-12.0f, 12.0f, 0.1f), 0.0f, " dB", 1);
        addF ("mid",       "Mid", NR (-12.0f, 12.0f, 0.1f), 0.0f, " dB", 1);
        addF ("hi",        "Hi", NR (-12.0f, 12.0f, 0.1f), 1.5f, " dB", 1);
        addF ("enhance",   "Enhance", NR (-10.0f, 10.0f, 0.1f), 2.0f, "", 1);
        addF ("toneLevel", "Tone Level", NR (-12.0f, 12.0f, 0.1f), 0.0f, " dB", 1);
        addB ("wahOn",     "Wah On", false);
        addF ("wahPos",    "Wah", NR (0.0f, 10.0f, 0.1f), 5.0f, "", 1);
        addF ("wahRes",    "Wah Resonance", NR (0.0f, 10.0f, 0.1f), 5.0f, "", 1);
        addC ("wahMode",   "Wah Mode", { "AUTO", "PEDAL" }, 1);
        addB ("chorusOn",  "Chorus On", true);
        addF ("chSpeed",   "Chorus Speed", NR (0.1f, 8.0f, 0.01f, 0.4f), 0.8f, " Hz", 2);
        addF ("chWidth",   "Chorus Width", NR (0.0f, 10.0f, 0.1f), 5.0f, "", 1);
        addF ("chPre",     "Pre-Delay", NR (0.0f, 20.0f, 0.1f), 8.0f, " ms", 1);
        addB ("reverbOn",  "Reverb On", true);
        addF ("rvMix",     "Reverb Mix", NR (0.0f, 50.0f, 0.5f), 18.0f, " %", 1);
        addC ("rvType",    "Reverb Type", { "PLATE", "ROOM", "STAGE", "HALL" }, 1);
        addF ("outGain",   "Master Output", NR (-24.0f, 12.0f, 0.1f), 0.0f, " dB", 1);
        return l;
    }
}

GtrProcessor::GtrProcessor()
    : AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", makeLayout())
{
    for (int i = 0; i < gtr::kNumParams; ++i)
        raw[(size_t) i] = apvts.getRawParameterValue (gtr::paramIds()[i]);
}

void GtrProcessor::prepareToPlay (double sampleRate, int)
{
    engine.prepare (sampleRate);
    keyTracker.prepare (sampleRate);
}

bool GtrProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

EngineParams GtrProcessor::snapshot() const
{
    auto v = [this] (int i) { return raw[(size_t) i]->load(); };
    EngineParams p;
    p.inGainDb = v (0);
    p.compOn = v (1) > 0.5f;  p.compAmt = v (2);  p.compLevelDb = v (3);  p.attack = (int) std::lround (v (4));
    p.toneOn = v (5) > 0.5f;  p.lowDb = v (6);  p.midDb = v (7);  p.hiDb = v (8);  p.enhance = v (9);  p.toneLevelDb = v (10);
    p.wahOn = v (11) > 0.5f;  p.wahPos = v (12);  p.wahRes = v (13);  p.wahMode = (int) std::lround (v (14));
    p.chorusOn = v (15) > 0.5f;  p.chSpeed = v (16);  p.chWidth = v (17);  p.chPre = v (18);
    p.reverbOn = v (19) > 0.5f;  p.rvMix = v (20);  p.rvType = (int) std::lround (v (21));
    p.outGainDb = v (22);
    return p;
}

void GtrProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    if (buffer.getNumChannels() < 2) return;

    // alimenta el detector de tonalidad con la senal de entrada (mono)
    const float* l = buffer.getReadPointer (0);
    const float* r = buffer.getReadPointer (1);
    const int n = buffer.getNumSamples();
    const int cap = (int) monoTmp.size();
    for (int start = 0; start < n; start += cap)
    {
        const int m = juce::jmin (cap, n - start);
        for (int i = 0; i < m; ++i) monoTmp[(size_t) i] = 0.5f * (l[start + i] + r[start + i]);
        keyTracker.push (monoTmp.data(), m);
    }

    engine.process (buffer, snapshot());
}

void GtrProcessor::setCurrentProgram (int index)
{
    index = juce::jlimit (0, (int) gtr::presets().size() - 1, index);
    currentProgram = index;
    apvts.state.setProperty ("program", index, nullptr);

    const auto& pr = gtr::presets()[(size_t) index];
    for (int i = 0; i < gtr::kNumParams; ++i)
        if (auto* prm = apvts.getParameter (gtr::paramIds()[i]))
            prm->setValueNotifyingHost (prm->convertTo0to1 (pr.v[i]));
}

const juce::String GtrProcessor::getProgramName (int index)
{
    if (index < 0 || index >= (int) gtr::presets().size()) return {};
    return gtr::presets()[(size_t) index].name;
}

void GtrProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void GtrProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
            currentProgram = juce::jlimit (0, (int) gtr::presets().size() - 1, (int) apvts.state.getProperty ("program", 0));
        }
}

juce::AudioProcessorEditor* GtrProcessor::createEditor() { return new GtrEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GtrProcessor(); }
