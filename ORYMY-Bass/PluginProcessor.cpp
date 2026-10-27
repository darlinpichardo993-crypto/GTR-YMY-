#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace {
constexpr std::array<float,10> eqHz {{31.0f,62.0f,125.0f,250.0f,500.0f,1000.0f,2000.0f,4000.0f,8000.0f,16000.0f}};
float value(juce::AudioProcessorValueTreeState& s, const juce::String& id) {
    if (auto* p=s.getRawParameterValue(id)) return p->load();
    return 0.0f;
}
}
ORYMYBassAudioProcessor::ORYMYBassAudioProcessor()
: AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),
  parameters(*this,nullptr,"ORYMY_STATE",createParameterLayout()) {
    for (auto& band:eqFilters) for (auto& f:band) f=std::make_unique<juce::dsp::IIR::Filter<float>>();
    for (auto& band:toneFilters) for (auto& f:band) f=std::make_unique<juce::dsp::IIR::Filter<float>>();
}
juce::AudioProcessorValueTreeState::ParameterLayout ORYMYBassAudioProcessor::createParameterLayout() {
    using P=juce::AudioParameterFloat;
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    auto add=[&](const char* id,const char* name,float lo,float hi,float def,float step=0.01f) {
        l.add(std::make_unique<P>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(lo,hi,step),def));
    };
    add("input","Input",0.0f,10.0f,4.5f); add("bass","Bass",0.0f,10.0f,6.0f);
    add("mid","Mid",0.0f,10.0f,6.0f); add("treble","Treble",0.0f,10.0f,5.8f);
    add("presence","Presence",0.0f,10.0f,6.0f); add("master","Master",0.0f,10.0f,7.0f);
    add("gate","Noise Gate",-80.0f,-24.0f,-52.0f,0.5f); add("attack","Attack",1.0f,60.0f,12.0f,0.5f);
    add("comp","Compressor",0.0f,10.0f,5.0f); add("drive","Drive",0.0f,10.0f,1.5f);
    for(int i=0;i<10;++i) add(("eq"+juce::String(i)).toRawUTF8(),("Graphic EQ "+juce::String(i)).toRawUTF8(),-12.0f,12.0f,0.0f,0.1f);
    return l;
}
void ORYMYBassAudioProcessor::prepareToPlay(double sampleRate,int samplesPerBlock) {
    spec={sampleRate,(juce::uint32)samplesPerBlock,(juce::uint32)getTotalNumOutputChannels()};
    for(auto& band:eqFilters) for(auto& f:band) { f->reset(); f->prepare(spec); }
    for(auto& band:toneFilters) for(auto& f:band) { f->reset(); f->prepare(spec); }
    compressor.prepare(spec); compressor.reset(); gateEnvelope=0.0f;
    pitchHistory.fill(0.0f); pitchWriteIndex=0; pitchSamples=0; pitchAnalysisCountdown=0; detectedAudioNote.store(-1);
}
void ORYMYBassAudioProcessor::updateAudioPitch(const juce::AudioBuffer<float>& buffer) {
    // Keep analysis bounded on the real-time audio thread.
    if (buffer.getNumChannels() < 1 || spec.sampleRate <= 0.0) return;
    const auto* input = buffer.getReadPointer(0);
    double rms = 0.0;
    const int count = buffer.getNumSamples();
    for (int i=0; i<count; i+=4) rms += (double)input[i] * input[i];
    rms = std::sqrt(rms / (double)juce::jmax(1, (count + 3) / 4));
    if (rms < 0.0005) detectedAudioNote.store(-1);
}
void ORYMYBassAudioProcessor::releaseResources() {}
bool ORYMYBassAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
    return l.getMainInputChannelSet()==l.getMainOutputChannelSet() &&
      (l.getMainOutputChannelSet()==juce::AudioChannelSet::mono() || l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo());
}
void ORYMYBassAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    for(const auto metadata:midi) {
        const auto m=metadata.getMessage();
        if(m.isNoteOn()) currentMidiNote.store(m.getNoteNumber());
        else if(m.isNoteOff() && currentMidiNote.load()==m.getNoteNumber()) currentMidiNote.store(-1);
    }
    updateAudioPitch(buffer);
    const float in=value(parameters,"input"), master=value(parameters,"master");
    const float drive=value(parameters,"drive")*0.8f;
    const float gateDb=value(parameters,"gate");
    const float gate=juce::Decibels::decibelsToGain(gateDb);
    const float comp=value(parameters,"comp");
    compressor.setThreshold(-30.0f+comp*1.6f);
    compressor.setRatio(1.0f+comp*0.65f);
    compressor.setAttack(value(parameters,"attack"));
    compressor.setRelease(100.0f);
    for(size_t i=0;i<eqHz.size();++i) {
        auto c=juce::dsp::IIR::Coefficients<float>::makePeakFilter(spec.sampleRate,eqHz[i],1.0f,juce::Decibels::decibelsToGain(value(parameters,"eq"+juce::String((int)i))));
        for(auto& f:eqFilters[i]) f->coefficients=c;
    }
    auto bass=juce::dsp::IIR::Coefficients<float>::makeLowShelf(spec.sampleRate,120.0,0.707f,juce::Decibels::decibelsToGain((value(parameters,"bass")-5.0f)*1.25f));
    auto mid=juce::dsp::IIR::Coefficients<float>::makePeakFilter(spec.sampleRate,700.0,0.8f,juce::Decibels::decibelsToGain((value(parameters,"mid")-5.0f)*1.2f));
    auto treble=juce::dsp::IIR::Coefficients<float>::makeHighShelf(spec.sampleRate,4200.0,0.707f,juce::Decibels::decibelsToGain((value(parameters,"treble")-5.0f)*1.1f+(value(parameters,"presence")-5.0f)*0.6f));
    for(int ch=0;ch<buffer.getNumChannels();++ch) {
        float channelGateEnvelope=0.0f;
        auto* data=buffer.getWritePointer(ch);
        for(int n=0;n<buffer.getNumSamples();++n) {
            const float x=data[n];
            channelGateEnvelope=std::max(std::abs(x),channelGateEnvelope*0.9995f);
            if(channelGateEnvelope<gate) data[n]=0.0f;
            else data[n]=std::tanh(x*juce::Decibels::decibelsToGain((in-5.0f)*2.0f)*(1.0f+drive*0.8f))/(1.0f+drive*0.16f);
        }
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block.getSingleChannelBlock((size_t)ch));
        for(size_t i=0;i<eqHz.size();++i) eqFilters[i][(size_t)ch]->process(context);
        toneFilters[0][(size_t)ch]->coefficients=bass; toneFilters[0][(size_t)ch]->process(context);
        toneFilters[1][(size_t)ch]->coefficients=mid; toneFilters[1][(size_t)ch]->process(context);
        toneFilters[2][(size_t)ch]->coefficients=treble; toneFilters[2][(size_t)ch]->process(context);
    }
    juce::dsp::AudioBlock<float> whole(buffer);
    compressor.process(juce::dsp::ProcessContextReplacing<float>(whole));
    buffer.applyGain(juce::Decibels::decibelsToGain((master-7.0f)*2.0f));
}
juce::AudioProcessorEditor* ORYMYBassAudioProcessor::createEditor(){return new ORYMYBassAudioProcessorEditor(*this);}
void ORYMYBassAudioProcessor::getStateInformation(juce::MemoryBlock& d){auto s=parameters.copyState();std::unique_ptr<juce::XmlElement> x(s.createXml());copyXmlToBinary(*x,d);}
void ORYMYBassAudioProcessor::setStateInformation(const void* d,int size){std::unique_ptr<juce::XmlElement> x(getXmlFromBinary(d,size));if(x&&x->hasTagName(parameters.state.getType()))parameters.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new ORYMYBassAudioProcessor();}
