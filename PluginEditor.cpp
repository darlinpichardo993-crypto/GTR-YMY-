#include "PluginEditor.h"

namespace
{
    constexpr int kKnobW = 100, kKnobH = 122;       // perilla + valor
    constexpr int kRowA = 124, kRowB = 284;

    void styleLcdLabel (juce::Label& l)
    {
        l.setFont (pedal::mono (17.0f));
        l.setColour (juce::Label::textColourId, pedal::lcdTxt);
        l.setJustificationType (juce::Justification::centredLeft);
    }

    juce::Point<float> knobCentre (int cx, int yTop) { return { (float) cx, (float) yTop + (float) (kKnobH - 18) * 0.5f }; }
}

GtrEditor::GtrEditor (GtrProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    addAndMakeVisible (content);
    content.onPaint = [this] (juce::Graphics& g) { paintContent (g); };

    // ---- perillas
    setupKnob (inKnob,    "inGain");
    setupKnob (outKnob,   "outGain");
    setupKnob (levelKnob, "compLevel");
    setupKnob (compKnob,  "compAmt");
    setupKnob (wahKnob,   "wahPos");
    setupKnob (resKnob,   "wahRes");
    setupKnob (speedKnob, "chSpeed");
    setupKnob (widthKnob, "chWidth");
    setupKnob (preKnob,   "chPre");
    setupKnob (mixKnob,   "rvMix");

    // ---- sliders
    setupVSlider (attackSl, "attack",    false, 3, false);
    setupVSlider (lowSl,    "low",       true,  0, true);
    setupVSlider (midSl,    "mid",       true,  0, true);
    setupVSlider (hiSl,     "hi",        true,  0, true);
    setupVSlider (enhSl,    "enhance",   true,  0, true);
    setupVSlider (toneLvSl, "toneLevel", true,  0, true);
    setupVSlider (modeSl,   "wahMode",   false, 2, false);
    setupVSlider (typeSl,   "rvType",    false, 4, false);

    // ---- pedales de pie
    content.addAndMakeVisible (compBtn);   buttonAtt.push_back (std::make_unique<ButtonAtt> (proc.apvts, "compOn",   compBtn));
    content.addAndMakeVisible (toneBtn);   buttonAtt.push_back (std::make_unique<ButtonAtt> (proc.apvts, "toneOn",   toneBtn));
    content.addAndMakeVisible (wahBtn);    buttonAtt.push_back (std::make_unique<ButtonAtt> (proc.apvts, "wahOn",    wahBtn));
    content.addAndMakeVisible (chorusBtn); buttonAtt.push_back (std::make_unique<ButtonAtt> (proc.apvts, "chorusOn", chorusBtn));
    content.addAndMakeVisible (reverbBtn); buttonAtt.push_back (std::make_unique<ButtonAtt> (proc.apvts, "reverbOn", reverbBtn));

    // ---- presets
    presetBox.setLookAndFeel (&laf);
    presetBox.setColour (juce::ComboBox::textColourId, pedal::lcdTxt);
    for (int i = 0; i < (int) gtr::presets().size(); ++i)
        presetBox.addItem (gtr::presets()[(size_t) i].name, i + 1);
    presetBox.onChange = [this] { proc.setCurrentProgram (presetBox.getSelectedId() - 1); };
    content.addAndMakeVisible (presetBox);

    prevBtn.setLookAndFeel (&laf);
    nextBtn.setLookAndFeel (&laf);
    resetBtn.setLookAndFeel (&laf);
    prevBtn.onClick = [this] { const int n = proc.getNumPrograms(); proc.setCurrentProgram ((proc.getCurrentProgram() + n - 1) % n); };
    nextBtn.onClick = [this] { const int n = proc.getNumPrograms(); proc.setCurrentProgram ((proc.getCurrentProgram() + 1) % n); };
    resetBtn.onClick = [this] { proc.getKeyTracker().reset(); };
    content.addAndMakeVisible (prevBtn);
    content.addAndMakeVisible (nextBtn);
    content.addAndMakeVisible (resetBtn);

    // ---- tonalidad / escala
    styleLcdLabel (keyLine);
    styleLcdLabel (scaleLine);
    styleLcdLabel (noteLine);
    keyLine.setText ("TONALIDAD: toca la guitarra...", juce::dontSendNotification);
    scaleLine.setText ("ESCALA: --", juce::dontSendNotification);
    noteLine.setText ("NOTA: --", juce::dontSendNotification);
    content.addAndMakeVisible (keyLine);
    content.addAndMakeVisible (scaleLine);
    content.addAndMakeVisible (noteLine);
    content.addAndMakeVisible (keyboard);

    setResizable (true, true);
    setResizeLimits (680, 400, 2040, 1200);
    getConstrainer()->setFixedAspectRatio ((double) kW / (double) kH);
    setSize (kW, kH);
    startTimerHz (20);
}

GtrEditor::~GtrEditor()
{
    for (auto* c : std::initializer_list<juce::Component*> {
             &inKnob, &outKnob, &levelKnob, &compKnob, &wahKnob, &resKnob, &speedKnob, &widthKnob, &preKnob, &mixKnob,
             &attackSl, &lowSl, &midSl, &hiSl, &enhSl, &toneLvSl, &modeSl, &typeSl,
             &presetBox, &prevBtn, &nextBtn, &resetBtn })
        c->setLookAndFeel (nullptr);
}

void GtrEditor::setupKnob (juce::Slider& s, const char* id)
{
    s.setLookAndFeel (&laf);
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 84, 18);
    s.setRotaryParameters (-2.4f, 2.4f, true);
    s.setMouseDragSensitivity (200);
    s.setColour (juce::Slider::textBoxTextColourId, pedal::lcdTxt);
    s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    s.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    content.addAndMakeVisible (s);
    sliderAtt.push_back (std::make_unique<SliderAtt> (proc.apvts, id, s));
}

void GtrEditor::setupVSlider (juce::Slider& s, const char* id, bool valueText, int ticks, bool bipolar)
{
    s.setLookAndFeel (&laf);
    s.setSliderStyle (juce::Slider::LinearVertical);
    s.setTextBoxStyle (valueText ? juce::Slider::TextBoxBelow : juce::Slider::NoTextBox, false, 64, 18);
    s.getProperties().set ("ticks", ticks);
    s.getProperties().set ("bipolar", bipolar ? 1 : 0);
    s.setColour (juce::Slider::textBoxTextColourId, pedal::lcdTxt);
    s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    s.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    content.addAndMakeVisible (s);
    sliderAtt.push_back (std::make_unique<SliderAtt> (proc.apvts, id, s));
}

void GtrEditor::timerCallback()
{
    const int prog = proc.getCurrentProgram();
    if (prog != lastProgram) { lastProgram = prog; presetBox.setSelectedId (prog + 1, juce::dontSendNotification); }

    const auto r = proc.getKeyTracker().get();
    bool mask[12] = {};
    if (r.hasKey)
    {
        const auto names = gtrkey::spellScale (r.tonic, r.minor, r.harmonic, mask);
        const int rel = r.minor ? (r.tonic + 3) % 12 : (r.tonic + 9) % 12;
        keyLine.setText ("TONALIDAD: " + gtrkey::keyName (r.tonic, r.minor) + "  [" + juce::String (juce::roundToInt (r.confidence * 100.0f)) + "%]",
                         juce::dontSendNotification);
        scaleLine.setText ("ESCALA: " + juce::String (r.minor ? (r.harmonic ? "menor armonica" : "menor natural") : "mayor")
                           + "  " + names.joinIntoString (" "), juce::dontSendNotification);
        noteLine.setText ("NOTA: " + (r.noteMidi >= 0 ? gtrkey::pcName (r.noteMidi) + juce::String (r.noteMidi / 12 - 1) : juce::String ("--"))
                          + "     RELATIVA: " + gtrkey::keyName (rel, ! r.minor), juce::dontSendNotification);
        keyboard.set (mask, r.tonic, r.noteMidi >= 0 ? r.noteMidi % 12 : -1);
    }
    else
    {
        keyLine.setText ("TONALIDAD: toca la guitarra...", juce::dontSendNotification);
        scaleLine.setText ("ESCALA: --", juce::dontSendNotification);
        noteLine.setText ("NOTA: " + (r.noteMidi >= 0 ? gtrkey::pcName (r.noteMidi) + juce::String (r.noteMidi / 12 - 1) : juce::String ("--")),
                          juce::dontSendNotification);
        keyboard.set (nullptr, -1, r.noteMidi >= 0 ? r.noteMidi % 12 : -1);
    }
}

void GtrEditor::paintContent (juce::Graphics& g)
{
    using namespace pedal;
    const float W = (float) kW, H = (float) kH;

    // ---- carcasa verde del pedal
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff35a84a), 0.0f, 0.0f, juce::Colour (0xff0f4a1e), 0.0f, H, false));
    g.fillAll();
    juce::Random rnd (7);
    for (float x = 0.0f; x < W; x += 2.0f)
    {
        g.setColour ((rnd.nextBool() ? juce::Colours::black : juce::Colours::white).withAlpha (0.02f + rnd.nextFloat() * 0.04f));
        g.drawVerticalLine ((int) x, 0.0f, H);
    }
    g.setColour (juce::Colours::white.withAlpha (0.28f));
    g.drawRoundedRectangle (4.0f, 4.0f, W - 8.0f, H - 8.0f, 16.0f, 2.0f);
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.drawRoundedRectangle (9.0f, 9.0f, W - 18.0f, H - 18.0f, 13.0f, 2.0f);

    const juce::Point<float> screws[] = { { 22.0f, 22.0f }, { W - 22.0f, 22.0f }, { 22.0f, H - 22.0f }, { W - 22.0f, H - 22.0f } };
    for (auto s : screws)
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe0e0e0), s.x - 2.0f, s.y - 2.0f, juce::Colour (0xff4a4a4a), s.x + 4.0f, s.y + 4.0f, true));
        g.fillEllipse (s.x - 6.0f, s.y - 6.0f, 12.0f, 12.0f);
        g.setColour (juce::Colours::black.withAlpha (0.7f));
        g.drawLine (s.x - 4.0f, s.y + 2.0f, s.x + 4.0f, s.y - 2.0f, 1.5f);
    }

    // ---- titulo
    g.setFont (serif (44.0f, juce::Font::bold | juce::Font::italic));
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawText ("GTR ORYMY RD", juce::Rectangle<int> (46, 18, 520, 52), juce::Justification::centredLeft);
    g.setColour (cream);
    g.drawText ("GTR ORYMY RD", juce::Rectangle<int> (44, 16, 520, 52), juce::Justification::centredLeft);
    g.setFont (script (26.0f));
    g.setColour (lime);
    g.drawText ("Fluye", juce::Rectangle<int> (440, 26, 200, 40), juce::Justification::centredLeft);

    // ---- placa frontal
    const juce::Rectangle<float> plate (22.0f, 80.0f, 1316.0f, 384.0f);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (plate.translated (0.0f, 5.0f), 18.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff111612), 0.0f, plate.getY(), juce::Colour (0xff060907), 0.0f, plate.getBottom(), false));
    g.fillRoundedRectangle (plate, 18.0f);
    g.setColour (juce::Colour (0xff59e04a));
    g.drawRoundedRectangle (plate.reduced (1.5f), 18.0f, 3.0f);
    g.setColour (lime.withAlpha (0.25f));
    g.drawRoundedRectangle (plate.reduced (7.0f), 14.0f, 1.2f);

    // separadores de seccion
    g.setColour (lime.withAlpha (0.22f));
    for (int x : { 262, 632, 862, 1182 })
        g.drawVerticalLine (x, plate.getY() + 14.0f, plate.getBottom() - 14.0f);

    // cajas de master (como en un pedal real)
    for (int cx : { 90, 1260 })
    {
        const juce::Rectangle<float> box ((float) cx - 56.0f, 94.0f, 112.0f, 162.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a8a3e), box.getX(), box.getY(), juce::Colour (0xff145a26), box.getX(), box.getBottom(), false));
        g.fillRoundedRectangle (box, 8.0f);
        g.setColour (cream.withAlpha (0.55f));
        g.drawRoundedRectangle (box, 8.0f, 1.5f);
    }

    // ---- textos
    auto text = [&g] (const juce::String& t, int cx, int y, int w, float size, juce::Colour c, int style = juce::Font::bold)
    {
        g.setFont (serif (size, style));
        g.setColour (c);
        g.drawText (t, juce::Rectangle<int> (cx - w / 2, y, w, 18), juce::Justification::centred);
    };

    text ("COMP/LIMITER", 152, 84, 140, 13.0f, lime);
    text ("TONE SHAPER", 445, 84, 140, 13.0f, lime);
    text ("WAH", 732, 84, 100, 13.0f, lime);
    text ("CHORUS", 950, 84, 80, 13.0f, orange);
    text ("REVERB", 1045, 84, 80, 13.0f, lime);

    text ("MASTER INPUT", 90, 104, 110, 11.5f, cream);
    text ("MASTER OUTPUT", 1260, 104, 112, 11.0f, cream);
    text ("LEVEL", 215, 104, 90, 11.5f, cream);
    text ("COMPRESSION", 215, 264, 100, 11.0f, cream);
    text ("ATTACK", 90, 264, 90, 11.5f, cream);
    text ("LOW", 300, 104, 60, 11.5f, cream);
    text ("MID", 358, 104, 60, 11.5f, cream);
    text ("HI", 416, 104, 60, 11.5f, cream);
    text ("ENHANCE", 474, 104, 62, 10.5f, cream);
    text ("LEVEL", 590, 104, 60, 11.5f, cream);
    text ("WAH", 690, 104, 90, 11.5f, cream);
    text ("RESONANCE", 690, 264, 100, 11.0f, cream);
    text ("MODE", 775, 264, 70, 11.5f, cream);
    text ("SPEED", 900, 104, 90, 11.5f, cream);
    text ("WIDTH", 900, 264, 90, 11.5f, cream);
    text ("PRE-DELAY", 1000, 104, 100, 11.5f, orange);
    text ("REVERB MIX", 1000, 264, 100, 11.5f, cream);
    text ("TYPE", 1090, 130, 70, 11.5f, cream);

    // escala BOOST / 0 / CUT
    {
        const int top = toneLvSl.getY() + 10, bottom = toneLvSl.getBottom() - 18 - 10;
        text ("BOOST", 530, top - 8, 60, 11.0f, cream);
        text ("0", 530, (top + bottom) / 2 - 8, 60, 11.0f, cream);
        text ("CUT", 530, bottom - 8, 60, 11.0f, cream);
    }

    // etiquetas de los sliders discretos (de arriba hacia abajo)
    auto sliderLabels = [&g] (const juce::Slider& s, const juce::StringArray& labels)
    {
        const float top = (float) s.getY() + 10.0f, bottom = (float) s.getBottom() - 10.0f;
        g.setFont (pedal::serif (11.5f));
        g.setColour (pedal::cream);
        for (int i = 0; i < labels.size(); ++i)
        {
            const float y = top + (bottom - top) * (float) i / (float) juce::jmax (1, labels.size() - 1);
            g.drawText (labels[i], juce::Rectangle<float> ((float) s.getRight() - 14.0f, y - 8.0f, 56.0f, 16.0f), juce::Justification::centredLeft);
        }
    };
    sliderLabels (attackSl, { "HARD", "MID", "SOFT" });
    sliderLabels (modeSl, { "PEDAL", "AUTO" });
    sliderLabels (typeSl, { "HALL", "STAGE", "ROOM", "PLATE" });

    // extremos de perillas
    auto ends = [&g] (int cx, int yTop, const char* lo, const char* hi)
    {
        const auto c = knobCentre (cx, yTop);
        g.setFont (pedal::serif (10.5f));
        g.setColour (pedal::cream.withAlpha (0.85f));
        g.drawText (lo, juce::Rectangle<float> (c.x - 62.0f, c.y + 30.0f, 40.0f, 14.0f), juce::Justification::centred);
        g.drawText (hi, juce::Rectangle<float> (c.x + 22.0f, c.y + 30.0f, 40.0f, 14.0f), juce::Justification::centred);
    };
    ends (1000, kRowA, "0", "20");
    ends (1000, kRowB, "0%", "50%");
    ends (900, kRowA, "0.1", "8Hz");
    ends (900, kRowB, "0", "10");

    // ---- panel LCD de tonalidad y escala
    const juce::Rectangle<float> lcd (40.0f, 476.0f, 710.0f, 92.0f), kb (762.0f, 472.0f, 576.0f, 100.0f);
    for (auto r : { lcd, kb })
    {
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillRoundedRectangle (r.translated (0.0f, 3.0f), 12.0f);
        g.setColour (lcdBg);
        g.fillRoundedRectangle (r, 12.0f);
        g.setColour (juce::Colour (0xff2f7a3a));
        g.drawRoundedRectangle (r, 12.0f, 2.0f);
    }

    // ---- pie
    g.setFont (serif (14.0f, juce::Font::italic));
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawText ("Made By ORYMY | YMY Records Song", juce::Rectangle<int> (1, kH - 24, kW, 20), juce::Justification::centred);
    g.setColour (cream);
    g.drawText ("Made By ORYMY | YMY Records Song", juce::Rectangle<int> (0, kH - 25, kW, 20), juce::Justification::centred);
}

void GtrEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void GtrEditor::resized()
{
    const float s = juce::jmin ((float) getWidth() / (float) kW, (float) getHeight() / (float) kH);
    content.setBounds (0, 0, kW, kH);
    content.setTransform (juce::AffineTransform::scale (s));

    auto knob = [] (juce::Slider& sl, int cx, int yTop) { sl.setBounds (cx - kKnobW / 2, yTop, kKnobW, kKnobH); };
    knob (inKnob, 90, kRowA);       knob (outKnob, 1260, kRowA);
    knob (levelKnob, 215, kRowA);   knob (compKnob, 215, kRowB);
    knob (wahKnob, 690, kRowA);     knob (resKnob, 690, kRowB);
    knob (speedKnob, 900, kRowA);   knob (widthKnob, 900, kRowB);
    knob (preKnob, 1000, kRowA);    knob (mixKnob, 1000, kRowB);

    attackSl.setBounds (90 - 32, 300, 64, 100);
    lowSl.setBounds (300 - 32, 124, 64, 290);
    midSl.setBounds (358 - 32, 124, 64, 290);
    hiSl.setBounds (416 - 32, 124, 64, 290);
    enhSl.setBounds (474 - 32, 124, 64, 290);
    toneLvSl.setBounds (590 - 32, 124, 64, 290);
    modeSl.setBounds (775 - 32, 300, 64, 100);
    typeSl.setBounds (1090 - 32, 150, 64, 220);

    compBtn.setBounds (100, 578, 200, 200);
    toneBtn.setBounds (340, 578, 200, 200);
    wahBtn.setBounds (580, 578, 200, 200);
    chorusBtn.setBounds (820, 578, 200, 200);
    reverbBtn.setBounds (1060, 578, 200, 200);

    prevBtn.setBounds (880, 26, 40, 34);
    presetBox.setBounds (926, 26, 364, 34);
    nextBtn.setBounds (1296, 26, 40, 34);

    keyLine.setBounds (54, 482, 590, 26);
    scaleLine.setBounds (54, 508, 690, 26);
    noteLine.setBounds (54, 534, 690, 26);
    resetBtn.setBounds (656, 484, 82, 28);
    keyboard.setBounds (770, 478, 560, 88);
}
