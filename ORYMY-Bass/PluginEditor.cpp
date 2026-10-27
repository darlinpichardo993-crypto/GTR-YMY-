#include "PluginEditor.h"
#include "Presets.h"

namespace {
const juce::Colour paper(0xfff7f7f4), ink(0xff17191b), soft(0xffe7e7e2), grey(0xff6e7274), accent(0xffb8a37a);
const std::array<const char*,12> noteNames {{ "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" }};
class BassLook final : public juce::LookAndFeel_V4 {
public:
    BassLook() {
        setColour(juce::Slider::rotarySliderFillColourId,ink);
        setColour(juce::Slider::rotarySliderOutlineColourId,soft);
        setColour(juce::Slider::thumbColourId,accent);
        setColour(juce::Slider::trackColourId,ink);
        setColour(juce::Slider::backgroundColourId,soft);
        setColour(juce::ComboBox::backgroundColourId,paper);
        setColour(juce::ComboBox::textColourId,ink);
        setColour(juce::ComboBox::outlineColourId,soft);
    }
    void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&) override {
        auto r=juce::Rectangle<float>((float)x+5,(float)y+5,(float)w-10,(float)h-10).reduced(4);
        const float d=juce::jmin(r.getWidth(),r.getHeight());
        r.setSize(d,d); r.setCentre((float)x+w*0.5f,(float)y+h*0.5f);
        g.setColour(soft); g.fillEllipse(r);
        g.setColour(ink); g.fillEllipse(r.reduced(4));
        g.setColour(juce::Colour(0xff45484a)); g.drawEllipse(r.reduced(2),1.0f);
        juce::Path arc; arc.addCentredArc(r.getCentreX(),r.getCentreY(),d*0.43f,d*0.43f,0,start,start+(end-start)*pos,true);
        g.setColour(accent); g.strokePath(arc,juce::PathStrokeType(3.0f));
        const float a=start+(end-start)*pos;
        juce::Point<float> p(r.getCentreX()+std::cos(a)*d*0.28f,r.getCentreY()+std::sin(a)*d*0.28f);
        g.setColour(paper); g.drawLine(r.getCentreX(),r.getCentreY(),p.x,p.y,2.2f);
        g.setColour(accent); g.fillEllipse(r.getCentreX()-2.5f,r.getCentreY()-2.5f,5,5);
    }
};
void text(juce::Graphics& g,juce::String s,juce::Rectangle<int> r,float size,juce::Colour c=ink,bool bold=false,juce::Justification j=juce::Justification::centredLeft) {
    g.setColour(c); g.setFont(juce::Font(juce::FontOptions(size, bold?juce::Font::bold:juce::Font::plain))); g.drawText(s,r,j,false);
}
}
ORYMYBassAudioProcessorEditor::ORYMYBassAudioProcessorEditor(ORYMYBassAudioProcessor& p)
: AudioProcessorEditor(&p), audioProcessor(p) {
    ownedLookAndFeel = std::make_unique<BassLook>();
    setLookAndFeel(ownedLookAndFeel.get());
    setSize(1120,760);
    brandLabel.setText("ORYMY BASS",juce::dontSendNotification);
    brandLabel.setFont(juce::Font(juce::FontOptions(27.0f,juce::Font::bold)));
    brandLabel.setColour(juce::Label::textColourId,ink); addAndMakeVisible(brandLabel);
    subtitleLabel.setText("YMY SOUNDS  /  DOMINICAN BASS AMPLIFIER",juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font(juce::FontOptions(10.0f))); subtitleLabel.setColour(juce::Label::textColourId,grey); addAndMakeVisible(subtitleLabel);
    presetBox.addSectionHeading("SIGNATURE PRESETS");
    for(int i=0;i<(int)OrymyBass::presets.size();++i) presetBox.addItem(OrymyBass::presets[(size_t)i].name,i+1);
    presetBox.setSelectedId(1,juce::dontSendNotification);
    presetBox.onChange=[this]{applySelectedPreset();}; addAndMakeVisible(presetBox);
    scaleBox.addItem("C MAJOR",1); scaleBox.addItem("A MINOR",2); scaleBox.addItem("D MAJOR",3);
    scaleBox.addItem("E MINOR",4); scaleBox.addItem("G MAJOR",5); scaleBox.addItem("B MINOR",6);
    scaleBox.addItem("A MAJOR",7); scaleBox.addItem("F# MINOR",8); scaleBox.setSelectedId(1); addAndMakeVisible(scaleBox);
    scaleLabel.setText("SCALE GUIDE",juce::dontSendNotification); scaleLabel.setFont(juce::Font(juce::FontOptions(10.0f,juce::Font::bold))); scaleLabel.setColour(juce::Label::textColourId,grey); addAndMakeVisible(scaleLabel);
    noteLabel.setText("NOTE MONITOR  —   PLAY BASS",juce::dontSendNotification); noteLabel.setFont(juce::Font(juce::FontOptions(12.0f,juce::Font::bold))); noteLabel.setColour(juce::Label::textColourId,ink); addAndMakeVisible(noteLabel);
    for(int i=0;i<10;++i) {
        setupKnob(knobs[(size_t)i],knobNames[(size_t)i]);
        addAndMakeVisible(knobLabels[(size_t)i]);
        knobAttachments[(size_t)i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,knobIds[(size_t)i],knobs[(size_t)i]);
        eqSliders[(size_t)i].setSliderStyle(juce::Slider::LinearVertical);
        eqSliders[(size_t)i].setTextBoxStyle(juce::Slider::TextBoxBelow,false,48,16);
        eqSliders[(size_t)i].setRange(-12.0,12.0,0.1);
        eqSliders[(size_t)i].setColour(juce::Slider::trackColourId,accent);
        eqSliders[(size_t)i].setColour(juce::Slider::thumbColourId,ink);
        addAndMakeVisible(eqSliders[(size_t)i]);
        eqAttachments[(size_t)i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,"eq"+juce::String(i),eqSliders[(size_t)i]);
        eqLabels[(size_t)i].setText(juce::String((int)std::array<int,10>{{31,62,125,250,500,1000,2000,4000,8000,16000}}[(size_t)i]),juce::dontSendNotification);
        eqLabels[(size_t)i].setFont(juce::Font(juce::FontOptions(9.0f))); eqLabels[(size_t)i].setJustificationType(juce::Justification::centred); eqLabels[(size_t)i].setColour(juce::Label::textColourId,grey); addAndMakeVisible(eqLabels[(size_t)i]);
    }
    startTimerHz(12);
    OrymyBass::applyPreset(audioProcessor.parameters,0);
}
ORYMYBassAudioProcessorEditor::~ORYMYBassAudioProcessorEditor(){stopTimer();setLookAndFeel(nullptr);ownedLookAndFeel.reset();}
void ORYMYBassAudioProcessorEditor::setupKnob(juce::Slider& s,const juce::String& label) {
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,56,16);
    s.setName(label); addAndMakeVisible(s);
}
void ORYMYBassAudioProcessorEditor::applySelectedPreset(){const int id=presetBox.getSelectedId();if(id>0)OrymyBass::applyPreset(audioProcessor.parameters,id-1);}
void ORYMYBassAudioProcessorEditor::timerCallback() {
    const int n=audioProcessor.getCurrentMidiNote();
    if(n<0) noteLabel.setText("NOTE MONITOR  —   PLAY A NOTE",juce::dontSendNotification);
    else noteLabel.setText("NOTE MONITOR  /  "+juce::String(noteNames[(size_t)(n%12)])+juce::String(n/12-1)+"   •   "+scaleBox.getText(),juce::dontSendNotification);
}
void ORYMYBassAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(paper);
    g.setColour(ink); g.fillRect(0,0,getWidth(),5);
    g.setColour(soft); g.drawLine(22,78,1098,78,1.0f);
    g.setColour(soft); g.fillRoundedRectangle(20,96,430,338,10);
    g.setColour(ink); g.drawRoundedRectangle(20,96,430,338,10,1.0f);
    text(g,"SIGNATURE AMP",juce::Rectangle<int>(36,108,180,18),10,grey,true);
    // Original vector illustration: musician, bass and amplifier, rendered as clean editorial line art.
    auto amp=juce::Rectangle<float>(44,260,130,105);
    g.setColour(juce::Colour(0xff26282a)); g.fillRoundedRectangle(amp,5);
    g.setColour(juce::Colour(0xff55585a)); g.drawRoundedRectangle(amp,5,1.2f);
    for(int row=0;row<8;++row) for(int col=0;col<10;++col) {
        g.setColour(juce::Colour(0xff55585a)); g.fillEllipse(50+col*11.5f,270+row*11.0f,2.5f,2.5f);
    }
    g.setColour(accent); g.fillRoundedRectangle(55,246,108,18,3);
    text(g,"YMY",juce::Rectangle<int>(72,247,72,15),11,paper,true,juce::Justification::centred);
    // Jacket and body silhouette
    juce::Path body; body.startNewSubPath(210,190); body.lineTo(184,204); body.lineTo(163,253); body.lineTo(186,271);
    body.lineTo(180,324); body.lineTo(242,324); body.lineTo(255,270); body.lineTo(279,244); body.lineTo(253,206); body.lineTo(230,190); body.closeSubPath();
    g.setColour(ink); g.fillPath(body);
    g.setColour(juce::Colour(0xff45484a)); g.strokePath(body,juce::PathStrokeType(1.2f));
    // Neck and head
    g.setColour(juce::Colour(0xffbd8868)); g.fillRoundedRectangle(210,169,25,31,7);
    g.setColour(juce::Colour(0xffd0a080)); g.fillEllipse(199,133,48,51);
    g.setColour(juce::Colour(0xffd4c7a9)); g.fillEllipse(196,126,54,28);
    g.setColour(ink); g.fillEllipse(202,124,9,8); g.fillEllipse(219,122,7,6); g.fillEllipse(234,127,8,6);
    // Face shadow, glasses, and chain
    g.setColour(ink); g.fillRoundedRectangle(200,149,22,9,3); g.fillRoundedRectangle(226,149,19,9,3);
    g.setColour(accent); g.drawLine(222,153,226,153,1.5f); g.drawLine(218,196,225,217,1.3f);
    g.setColour(juce::Colour(0xffb6b7b3)); g.drawLine(214,207,226,234,1.4f); g.drawLine(226,234,237,207,1.4f);
    // Black bass guitar silhouette, body and neck
    g.setColour(juce::Colour(0xff090a0b)); g.fillEllipse(188,250,74,83); g.fillEllipse(208,237,52,57);
    juce::Path neck; neck.startNewSubPath(226,270); neck.lineTo(382,220); neck.lineTo(389,227); neck.lineTo(235,283); neck.closeSubPath(); g.fillPath(neck);
    g.setColour(juce::Colour(0xffc3c5c2)); for(int i=0;i<4;++i) g.drawLine(221.0f+i*2.0f,272.0f,386.0f,222.0f+i*1.5f,0.8f);
    g.setColour(accent); g.fillRoundedRectangle(211,270,20,5,2); g.fillRoundedRectangle(219,283,20,5,2);
    g.setColour(juce::Colour(0xffc5c6c0)); g.drawLine(218,291,236,303,1.5f);
    // Forearm and plucking hand
    g.setColour(juce::Colour(0xffc99675)); juce::Path arm; arm.startNewSubPath(250,218); arm.lineTo(277,238); arm.lineTo(258,266); arm.lineTo(242,258); arm.lineTo(253,241); arm.closeSubPath(); g.fillPath(arm);
    g.setColour(juce::Colour(0xffc99675)); g.fillEllipse(243,254,20,15);
    text(g,"ORYMY",juce::Rectangle<int>(285,310,128,32),22,ink,true,juce::Justification::centred);
    text(g,"BASS / DOMINICAN TONE CRAFT",juce::Rectangle<int>(280,340,136,18),8,grey,true,juce::Justification::centred);
    g.setColour(soft); g.drawLine(36,386,434,386,1.0f);
    text(g,"TONE GUIDE",juce::Rectangle<int>(36,394,110,16),9,grey,true);
    text(g,"Low-end weight  /  String attack  /  Mix clarity",juce::Rectangle<int>(36,410,400,16),10,ink,false);
    g.setColour(soft); g.fillRoundedRectangle(465,96,635,338,10);
    g.setColour(ink); g.drawRoundedRectangle(465,96,635,338,10,1.0f);
    text(g,"10-BAND GRAPHIC EQUALIZER",juce::Rectangle<int>(484,108,300,18),11,ink,true);
    text(g,"±12 dB",juce::Rectangle<int>(1014,108,65,18),10,grey,false,juce::Justification::centredRight);
    for(int i=0;i<10;++i) {
        const int x=485+i*60;
        g.setColour(juce::Colour(0xffd9d9d3)); g.drawLine((float)x+22,153,(float)x+22,332,1.0f);
        g.setColour(juce::Colour(0xffd0d0ca)); g.fillEllipse((float)x+17,240,10,10);
    }
    // Amp panel and meter / guide section
    g.setColour(soft); g.fillRoundedRectangle(465,445,635,52,8);
    text(g,"AMP",juce::Rectangle<int>(480,453,40,15),9,grey,true);
    text(g,"MODERN CLEAN  •  4×10 CAB  •  DYNAMIC MIC",juce::Rectangle<int>(520,453,400,15),10,ink,true);
    text(g,"INPUT → AMP → EQ → COMP → MASTER",juce::Rectangle<int>(480,474,450,14),9,grey,false);
    text(g,"TONE SHAPED FOR BACHATA & MERENGUE",juce::Rectangle<int>(850,474,230,14),8,grey,true,juce::Justification::centredRight);
    g.setColour(soft); g.drawLine(22,511,1098,511,1.0f);
    text(g,"AMPLIFIER CONTROLS",juce::Rectangle<int>(25,518,230,16),10,grey,true);
    text(g,"LIVE PARAMETERS  /  AUTOMATION READY",juce::Rectangle<int>(790,518,300,16),9,grey,false,juce::Justification::centredRight);
    g.setColour(soft); g.drawLine(22,741,1098,741,1.0f);
    text(g,"ORYMY BASS    •    YMY SOUNDS VST    •    ORIGINAL TONE, BUILT FOR THE RHYTHM",juce::Rectangle<int>(25,742,700,16),8,grey,false);
}
void ORYMYBassAudioProcessorEditor::resized() {
    brandLabel.setBounds(24,14,230,35); subtitleLabel.setBounds(27,49,350,18);
    presetBox.setBounds(470,22,410,34);
    scaleLabel.setBounds(890,14,120,14); scaleBox.setBounds(890,29,120,28); noteLabel.setBounds(470,59,600,15);
    for(int i=0;i<10;++i) {
        const int x=485+i*60;
        eqLabels[(size_t)i].setBounds(x,130,48,16);
        eqSliders[(size_t)i].setBounds(x,150,48,194);
    }
    for(int i=0;i<10;++i) {
        const int col=i%5, row=i/5;
        const int x=34+col*218, y=536+row*100;
        knobLabels[(size_t)i].setText(knobNames[(size_t)i],juce::dontSendNotification);
        knobLabels[(size_t)i].setFont(juce::Font(juce::FontOptions(9.0f,juce::Font::bold)));
        knobLabels[(size_t)i].setJustificationType(juce::Justification::centred);
        knobLabels[(size_t)i].setColour(juce::Label::textColourId,grey);
        knobLabels[(size_t)i].setBounds(x,y,100,14);
        knobs[(size_t)i].setBounds(x+12,y+13,76,82);
    }
}
