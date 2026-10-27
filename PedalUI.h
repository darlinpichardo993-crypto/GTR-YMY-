#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>

// Componentes visuales del pedal analogo (todo dibujado por codigo)
namespace pedal
{
    const juce::Colour lime   (0xff7dff6a);
    const juce::Colour cream  (0xfff1f5e9);
    const juce::Colour ink    (0xff0b0f0c);
    const juce::Colour orange (0xffffa23a);
    const juce::Colour lcdBg  (0xff06120a);
    const juce::Colour lcdTxt (0xff8dff7a);

    inline juce::Font serif  (float h, int style = juce::Font::bold) { return juce::Font (juce::FontOptions ("Georgia", h, style)); }
    inline juce::Font mono   (float h, int style = juce::Font::bold) { return juce::Font (juce::FontOptions ("Courier New", h, style)); }
    inline juce::Font script (float h)                               { return juce::Font (juce::FontOptions ("Segoe Script", h, juce::Font::bold)); }
    inline juce::Font sans   (float h, int style = juce::Font::bold) { return juce::Font (juce::FontOptions ("Segoe UI", h, style)); }

    //==========================================================================
    struct PedalLookAndFeel : public juce::LookAndFeel_V4
    {
        PedalLookAndFeel()
        {
            setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff0a1a0f));
            setColour (juce::PopupMenu::textColourId, lcdTxt);
            setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff1f6b2c));
            setColour (juce::PopupMenu::highlightedTextColourId, cream);
        }

        int getSliderThumbRadius (juce::Slider&) override { return 10; }

        juce::Label* createSliderTextBox (juce::Slider& s) override
        {
            auto* l = juce::LookAndFeel_V4::createSliderTextBox (s);
            l->setFont (mono (12.5f));
            l->setJustificationType (juce::Justification::centred);
            return l;
        }

        void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                               float startA, float endA, juce::Slider&) override
        {
            auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (6.0f);
            const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
            const auto c = b.getCentre();
            const float angle = startA + pos * (endA - startA);

            // marcas alrededor
            for (int i = 0; i <= 10; ++i)
            {
                const float a = startA + (float) i / 10.0f * (endA - startA);
                const bool big = (i == 0 || i == 10 || i == 5);
                g.setColour (cream.withAlpha (big ? 0.95f : 0.65f));
                g.drawLine (c.x + std::sin (a) * (r * 0.80f), c.y - std::cos (a) * (r * 0.80f),
                            c.x + std::sin (a) * (r * (big ? 1.0f : 0.94f)), c.y - std::cos (a) * (r * (big ? 1.0f : 0.94f)), big ? 2.0f : 1.4f);
            }

            const float rk = r * 0.68f;
            g.setColour (juce::Colours::black.withAlpha (0.55f));
            g.fillEllipse (c.x - rk + 1.5f, c.y - rk + 3.5f, rk * 2.0f, rk * 2.0f);
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a4f4b), c.x - rk * 0.5f, c.y - rk * 0.8f,
                                                     juce::Colour (0xff080a08), c.x + rk * 0.5f, c.y + rk, true));
            g.fillEllipse (c.x - rk, c.y - rk, rk * 2.0f, rk * 2.0f);
            g.setColour (juce::Colour (0xff1c201d));
            g.drawEllipse (c.x - rk, c.y - rk, rk * 2.0f, rk * 2.0f, 1.5f);
            g.setColour (juce::Colours::white.withAlpha (0.08f));
            g.drawEllipse (c.x - rk * 0.78f, c.y - rk * 0.78f, rk * 1.56f, rk * 1.56f, 1.0f);

            juce::Path ptr;
            ptr.addRoundedRectangle (-2.4f, -rk + 3.0f, 4.8f, rk * 0.52f, 2.0f);
            g.setColour (lime);
            g.fillPath (ptr, juce::AffineTransform::rotation (angle).translated (c.x, c.y));
        }

        void drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float sliderPos, float, float,
                               juce::Slider::SliderStyle style, juce::Slider& s) override
        {
            if (style != juce::Slider::LinearVertical)
            {
                juce::LookAndFeel_V4::drawLinearSlider (g, x, y, w, h, sliderPos, 0.0f, 0.0f, style, s);
                return;
            }
            const float cx = (float) x + (float) w * 0.5f;
            const float top = (float) y, bottom = (float) (y + h);
            const int ticks = (int) s.getProperties().getWithDefault ("ticks", 0);
            const bool bipolar = (int) s.getProperties().getWithDefault ("bipolar", 0) != 0;

            // ranura
            g.setColour (juce::Colours::black.withAlpha (0.8f));
            g.fillRoundedRectangle (cx - 4.5f, top - 4.0f, 9.0f, bottom - top + 8.0f, 4.5f);
            g.setColour (juce::Colour (0xff2a2f2b));
            g.drawRoundedRectangle (cx - 4.5f, top - 4.0f, 9.0f, bottom - top + 8.0f, 4.5f, 1.0f);

            // marcas
            g.setColour (cream.withAlpha (0.75f));
            if (ticks > 1)
                for (int i = 0; i < ticks; ++i)
                {
                    const float ty = top + (bottom - top) * (float) i / (float) (ticks - 1);
                    g.drawLine (cx - 16.0f, ty, cx - 9.0f, ty, 1.6f);
                }
            else if (bipolar)
            {
                const float mid = (top + bottom) * 0.5f;
                g.drawLine (cx - 18.0f, mid, cx - 9.0f, mid, 1.8f);
                g.drawLine (cx + 9.0f, mid, cx + 18.0f, mid, 1.8f);
                g.setColour (cream.withAlpha (0.4f));
                for (int i : { 1, 3 })
                {
                    const float ty = top + (bottom - top) * (float) i / 4.0f;
                    g.drawLine (cx - 14.0f, ty, cx - 9.0f, ty, 1.0f);
                    g.drawLine (cx + 9.0f, ty, cx + 14.0f, ty, 1.0f);
                }
            }

            // tapon metalico
            const juce::Rectangle<float> cap (cx - 17.0f, sliderPos - 9.0f, 34.0f, 18.0f);
            g.setColour (juce::Colours::black.withAlpha (0.55f));
            g.fillRoundedRectangle (cap.translated (0.0f, 2.5f), 4.0f);
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff2f2ee), cap.getX(), cap.getY(),
                                                     juce::Colour (0xff8d918c), cap.getX(), cap.getBottom(), false));
            g.fillRoundedRectangle (cap, 4.0f);
            g.setColour (juce::Colours::black.withAlpha (0.6f));
            g.drawRoundedRectangle (cap, 4.0f, 1.2f);
            g.setColour (juce::Colour (0xff2b2f2c));
            g.drawLine (cap.getX() + 5.0f, cap.getCentreY(), cap.getRight() - 5.0f, cap.getCentreY(), 2.0f);
        }

        void drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&) override
        {
            auto b = juce::Rectangle<float> (0.0f, 0.0f, (float) w, (float) h).reduced (1.5f);
            g.setColour (lcdBg);
            g.fillRoundedRectangle (b, 6.0f);
            g.setColour (juce::Colour (0xff2f7a3a));
            g.drawRoundedRectangle (b, 6.0f, 1.6f);
            juce::Path arrow;
            const float ax = (float) w - 20.0f, ay = (float) h * 0.5f;
            arrow.addTriangle (ax - 6.0f, ay - 3.5f, ax + 6.0f, ay - 3.5f, ax, ay + 4.5f);
            g.setColour (lime);
            g.fillPath (arrow);
        }

        juce::Font getComboBoxFont (juce::ComboBox&) override { return mono (16.0f); }

        void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
        {
            label.setBounds (10, 1, box.getWidth() - 36, box.getHeight() - 2);
            label.setFont (getComboBoxFont (box));
        }

        void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                   bool highlighted, bool down) override
        {
            auto b = button.getLocalBounds().toFloat().reduced (2.0f);
            juce::Colour base (0xff1f6b2c);
            if (down) base = base.darker (0.3f); else if (highlighted) base = base.brighter (0.15f);
            g.setColour (juce::Colours::black.withAlpha (0.5f));
            g.fillRoundedRectangle (b.translated (0.0f, down ? 1.0f : 2.5f), 7.0f);
            g.setGradientFill (juce::ColourGradient (base.brighter (0.3f), b.getX(), b.getY(),
                                                     base.darker (0.35f), b.getX(), b.getBottom(), false));
            g.fillRoundedRectangle (b, 7.0f);
            g.setColour (juce::Colours::black.withAlpha (0.7f));
            g.drawRoundedRectangle (b, 7.0f, 1.3f);
        }

        void drawButtonText (juce::Graphics& g, juce::TextButton& button, bool, bool) override
        {
            g.setFont (serif (15.0f));
            g.setColour (cream);
            g.drawText (button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
        }
    };

    //==========================================================================
    // Pedal de pie: interruptor con LED y goma estriada
    struct PedalButton : public juce::Button
    {
        explicit PedalButton (const juce::String& label) : juce::Button (label), caption (label)
        {
            setClickingTogglesState (true);
            setMouseCursor (juce::MouseCursor::PointingHandCursor);
        }

        void paintButton (juce::Graphics& g, bool over, bool down) override
        {
            auto b = getLocalBounds().toFloat();
            const bool on = getToggleState();

            g.setFont (serif (17.0f));
            g.setColour (juce::Colours::black.withAlpha (0.55f));
            g.drawText (caption, b.removeFromTop (26.0f).translated (1.0f, 1.5f), juce::Justification::centred);
            g.setColour (cream);
            g.drawText (caption, juce::Rectangle<float> (0.0f, 0.0f, b.getWidth(), 26.0f), juce::Justification::centred);

            auto ledArea = b.removeFromTop (30.0f);
            const float lx = ledArea.getCentreX(), ly = ledArea.getCentreY() + 2.0f;
            if (on)
            {
                g.setGradientFill (juce::ColourGradient (lime.withAlpha (0.7f), lx, ly, juce::Colours::transparentBlack, lx + 22.0f, ly, true));
                g.fillEllipse (lx - 22.0f, ly - 22.0f, 44.0f, 44.0f);
            }
            g.setColour (on ? lime : juce::Colour (0xff0e2a14));
            g.fillEllipse (lx - 8.0f, ly - 8.0f, 16.0f, 16.0f);
            g.setColour (juce::Colours::black.withAlpha (0.8f));
            g.drawEllipse (lx - 8.0f, ly - 8.0f, 16.0f, 16.0f, 1.5f);
            g.setColour (juce::Colours::white.withAlpha (on ? 0.7f : 0.2f));
            g.fillEllipse (lx - 4.0f, ly - 5.0f, 5.0f, 4.0f);

            // base y goma
            auto pad = b.reduced (8.0f, 4.0f);
            g.setColour (juce::Colours::black.withAlpha (0.55f));
            g.fillRoundedRectangle (pad.translated (0.0f, 4.0f), 14.0f);
            const float dy = down ? 3.0f : 0.0f;
            pad = pad.translated (0.0f, dy);
            g.setGradientFill (juce::ColourGradient (juce::Colour (over ? 0xff3a3f3b : 0xff2c302d), pad.getX(), pad.getY(),
                                                     juce::Colour (0xff101311), pad.getX(), pad.getBottom(), false));
            g.fillRoundedRectangle (pad, 14.0f);
            g.setColour (juce::Colour (0xff4a4f4b));
            g.drawRoundedRectangle (pad, 14.0f, 1.6f);
            g.setColour (juce::Colours::white.withAlpha (0.07f));
            g.drawRoundedRectangle (pad.reduced (5.0f), 10.0f, 1.0f);

            // estrias de goma
            const float ry0 = pad.getBottom() - 66.0f;
            for (int i = 0; i < 7; ++i)
            {
                const float ry = ry0 + (float) i * 8.0f;
                g.setColour (juce::Colours::black.withAlpha (0.9f));
                g.fillRoundedRectangle (pad.getCentreX() - 44.0f, ry, 88.0f, 3.5f, 1.5f);
                g.setColour (juce::Colours::white.withAlpha (0.10f));
                g.drawLine (pad.getCentreX() - 42.0f, ry + 4.4f, pad.getCentreX() + 42.0f, ry + 4.4f, 1.0f);
            }
        }

        juce::String caption;
    };

    //==========================================================================
    // Teclado de una octava: muestra la escala detectada y la nota que suena
    struct KeyboardView : public juce::Component
    {
        void set (const bool* mask12, int tonicPc, int currentPc)
        {
            bool changed = (tonic != tonicPc) || (current != currentPc);
            for (int i = 0; i < 12; ++i)
            {
                const bool v = mask12 != nullptr && mask12[i];
                if (v != mask[i]) { mask[i] = v; changed = true; }
            }
            tonic = tonicPc; current = currentPc;
            if (changed) repaint();
        }

        void paint (juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat().reduced (2.0f);
            const float kw = b.getWidth() / 7.0f, bw = kw * 0.58f, bh = b.getHeight() * 0.60f;
            static const int whitePc[7] = { 0, 2, 4, 5, 7, 9, 11 };
            static const char* names[7] = { "Do", "Re", "Mi", "Fa", "Sol", "La", "Si" };
            static const int blackPc[5] = { 1, 3, 6, 8, 10 };
            static const int blackAfter[5] = { 0, 1, 3, 4, 5 };

            auto fillFor = [this] (int pc, juce::Colour normal, juce::Colour inScale)
            {
                if (pc == tonic)  return juce::Colour (0xffffd34d);
                if (mask[pc])     return inScale;
                return normal;
            };

            for (int i = 0; i < 7; ++i)
            {
                const int pc = whitePc[i];
                auto k = juce::Rectangle<float> (b.getX() + (float) i * kw, b.getY(), kw, b.getHeight()).reduced (1.5f, 0.0f);
                g.setColour (fillFor (pc, juce::Colour (0xffeeeee6), juce::Colour (0xff6dea5c)));
                g.fillRoundedRectangle (k, 4.0f);
                g.setColour (juce::Colours::black.withAlpha (0.7f));
                g.drawRoundedRectangle (k, 4.0f, 1.2f);
                g.setFont (serif (12.0f));
                g.setColour (pedal::ink);
                g.drawText (names[i], k.removeFromBottom (18.0f), juce::Justification::centred);
            }
            for (int i = 0; i < 5; ++i)
            {
                const int pc = blackPc[i];
                const float x = b.getX() + (float) (blackAfter[i] + 1) * kw - bw * 0.5f;
                auto k = juce::Rectangle<float> (x, b.getY(), bw, bh);
                g.setColour (fillFor (pc, juce::Colour (0xff151815), juce::Colour (0xff2fa03a)));
                g.fillRoundedRectangle (k, 3.0f);
                g.setColour (juce::Colours::black);
                g.drawRoundedRectangle (k, 3.0f, 1.2f);
            }

            // nota que suena ahora
            if (current >= 0)
            {
                float cx = 0.0f, cy = 0.0f;
                for (int i = 0; i < 7; ++i)
                    if (whitePc[i] == current) { cx = b.getX() + ((float) i + 0.5f) * kw; cy = b.getBottom() - 30.0f; }
                for (int i = 0; i < 5; ++i)
                    if (blackPc[i] == current) { cx = b.getX() + (float) (blackAfter[i] + 1) * kw; cy = b.getY() + bh - 14.0f; }
                g.setGradientFill (juce::ColourGradient (orange.withAlpha (0.8f), cx, cy, juce::Colours::transparentBlack, cx + 16.0f, cy, true));
                g.fillEllipse (cx - 16.0f, cy - 16.0f, 32.0f, 32.0f);
                g.setColour (orange);
                g.fillEllipse (cx - 7.0f, cy - 7.0f, 14.0f, 14.0f);
                g.setColour (juce::Colours::black.withAlpha (0.8f));
                g.drawEllipse (cx - 7.0f, cy - 7.0f, 14.0f, 14.0f, 1.3f);
            }
        }

    private:
        bool mask[12] = {};
        int tonic = -1, current = -1;
    };
}
