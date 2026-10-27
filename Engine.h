#pragma once
#include <juce_dsp/juce_dsp.h>
#include <cmath>

struct EngineParams
{
    float inGainDb = 0, outGainDb = 0;
    bool  compOn = true;   float compAmt = 4, compLevelDb = 0; int attack = 1;      // 0 SOFT, 1 MID, 2 HARD
    bool  toneOn = true;   float lowDb = 0, midDb = 0, hiDb = 0, enhance = 0, toneLevelDb = 0;
    bool  wahOn = false;   float wahPos = 5, wahRes = 5; int wahMode = 1;           // 0 AUTO, 1 PEDAL
    bool  chorusOn = true; float chSpeed = 0.8f, chWidth = 5, chPre = 8;
    bool  reverbOn = true; float rvMix = 18; int rvType = 1;                         // 0 PLATE, 1 ROOM, 2 STAGE, 3 HALL
};

// Cadena: Entrada -> Comp/Limiter -> Wah -> Tone Shaper -> Chorus estereo -> Reverb -> Salida (+ limitador de seguridad)
class GtrEngine
{
public:
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        for (auto* s : { &inGain, &outGain, &compMakeup, &toneGain })
            s->reset (sr, 0.03);
        for (auto* s : { &compMix, &toneMix, &wahMix, &chorusMix, &reverbMix })
            s->reset (sr, 0.015);
        chPreSm.reset (sr, 0.08);
        chDepthSm.reset (sr, 0.08);

        chBuf.setSize (2, (int) (0.07 * sr) + 8);
        scratch.setSize (2, kChunk);
        reverb.setSampleRate (sr);

        juce::dsp::ProcessSpec spec { sr, (juce::uint32) kChunk, 2 };
        limiter.prepare (spec);
        limiter.setThreshold (-0.5f);
        limiter.setRelease (60.0f);
        reset();
    }

    void reset()
    {
        chBuf.clear(); chPos = 0; lfo = 0.0f;
        compEnv = 0.0f; wEnv = 0.0f; fcSm = 600.0f;
        for (int c = 0; c < 2; ++c)
        {
            lowF[c].clear(); midF[c].clear(); hiF[c].clear();
            exLp[c] = 0.0f; s1[c] = s2[c] = 0.0f;
        }
        reverb.reset(); reverbWasActive = false;
        limiter.reset();
        first = true;
        lastLow = lastMid = lastHi = 1.0e9f;
        lastRvType = -1;
    }

    void process (juce::AudioBuffer<float>& buffer, const EngineParams& p)
    {
        if (buffer.getNumChannels() < 2) return;
        const int total = buffer.getNumSamples();
        if (first) { snap (p); first = false; }

        float* L = buffer.getWritePointer (0);
        float* R = buffer.getWritePointer (1);
        juce::dsp::AudioBlock<float> whole (buffer);

        for (int start = 0; start < total; start += kChunk)
        {
            const int n = juce::jmin (kChunk, total - start);
            processChunk (L + start, R + start, n, p);
            auto sub = whole.getSubBlock ((size_t) start, (size_t) n);
            limiter.process (juce::dsp::ProcessContextReplacing<float> (sub));
        }
    }

private:
    static constexpr int kChunk = 256;

    struct Coef { float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0; };
    struct Biquad
    {
        Coef c; float z1 = 0, z2 = 0;
        float process (float x) { const float y = c.b0 * x + z1; z1 = c.b1 * x - c.a1 * y + z2; z2 = c.b2 * x - c.a2 * y; return y; }
        void clear() { z1 = z2 = 0.0f; }
    };

    static Coef shelf (bool high, double sr, double f0, double q, double db)
    {
        const double A = std::pow (10.0, db / 40.0), w = 2.0 * juce::MathConstants<double>::pi * f0 / sr;
        const double c = std::cos (w), al = std::sin (w) / (2.0 * q), sA = std::sqrt (A);
        double b0, b1, b2, a0, a1, a2;
        if (! high)
        {
            b0 = A * ((A + 1) - (A - 1) * c + 2 * sA * al);  b1 = 2 * A * ((A - 1) - (A + 1) * c);  b2 = A * ((A + 1) - (A - 1) * c - 2 * sA * al);
            a0 = (A + 1) + (A - 1) * c + 2 * sA * al;        a1 = -2 * ((A - 1) + (A + 1) * c);     a2 = (A + 1) + (A - 1) * c - 2 * sA * al;
        }
        else
        {
            b0 = A * ((A + 1) + (A - 1) * c + 2 * sA * al);  b1 = -2 * A * ((A - 1) + (A + 1) * c); b2 = A * ((A + 1) + (A - 1) * c - 2 * sA * al);
            a0 = (A + 1) - (A - 1) * c + 2 * sA * al;        a1 = 2 * ((A - 1) - (A + 1) * c);      a2 = (A + 1) - (A - 1) * c - 2 * sA * al;
        }
        return { (float) (b0 / a0), (float) (b1 / a0), (float) (b2 / a0), (float) (a1 / a0), (float) (a2 / a0) };
    }

    static Coef peak (double sr, double f0, double q, double db)
    {
        const double A = std::pow (10.0, db / 40.0), w = 2.0 * juce::MathConstants<double>::pi * f0 / sr;
        const double c = std::cos (w), al = std::sin (w) / (2.0 * q);
        const double a0 = 1 + al / A;
        return { (float) ((1 + al * A) / a0), (float) (-2 * c / a0), (float) ((1 - al * A) / a0),
                 (float) (-2 * c / a0), (float) ((1 - al / A) / a0) };
    }

    static float dbToGain (float db) { return std::pow (10.0f, db / 20.0f); }

    void snap (const EngineParams& p)
    {
        inGain.setCurrentAndTargetValue (dbToGain (p.inGainDb));
        outGain.setCurrentAndTargetValue (dbToGain (p.outGainDb));
        toneGain.setCurrentAndTargetValue (dbToGain (p.toneLevelDb));
        compMakeup.setCurrentAndTargetValue (1.0f);
        compMix.setCurrentAndTargetValue (p.compOn ? 1.0f : 0.0f);
        toneMix.setCurrentAndTargetValue (p.toneOn ? 1.0f : 0.0f);
        wahMix.setCurrentAndTargetValue (p.wahOn ? 1.0f : 0.0f);
        chorusMix.setCurrentAndTargetValue (p.chorusOn ? 1.0f : 0.0f);
        reverbMix.setCurrentAndTargetValue (p.reverbOn ? p.rvMix / 100.0f : 0.0f);
        chPreSm.setCurrentAndTargetValue (p.chPre);
        chDepthSm.setCurrentAndTargetValue (p.chWidth * 0.6f);
    }

    void applyReverbType (int t)
    {
        juce::Reverb::Parameters rp;
        rp.dryLevel = 0.0f; rp.wetLevel = 0.33f; rp.width = 1.0f; rp.freezeMode = 0.0f;
        switch (t)
        {
            case 0:  rp.roomSize = 0.62f; rp.damping = 0.15f; break;   // PLATE: brillante
            case 1:  rp.roomSize = 0.38f; rp.damping = 0.55f; break;   // ROOM: corta
            case 2:  rp.roomSize = 0.70f; rp.damping = 0.45f; break;   // STAGE
            default: rp.roomSize = 0.88f; rp.damping = 0.35f; break;   // HALL: larga
        }
        reverb.setParameters (rp);
    }

    float tap (const float* b, int size, double rp) const
    {
        while (rp < 0.0) rp += size;
        const int i0 = (int) rp;
        const float f = (float) (rp - i0);
        int i1 = i0 + 1; if (i1 >= size) i1 -= size;
        return b[i0 % size] * (1.0f - f) + b[i1] * f;
    }

    void processChunk (float* L, float* R, int n, const EngineParams& p)
    {
        const float pi = juce::MathConstants<float>::pi;

        // ---- objetivos de los suavizadores
        inGain.setTargetValue (dbToGain (p.inGainDb));
        outGain.setTargetValue (dbToGain (p.outGainDb));
        toneGain.setTargetValue (dbToGain (p.toneLevelDb));
        compMix.setTargetValue (p.compOn ? 1.0f : 0.0f);
        toneMix.setTargetValue (p.toneOn ? 1.0f : 0.0f);
        wahMix.setTargetValue (p.wahOn ? 1.0f : 0.0f);
        chorusMix.setTargetValue (p.chorusOn ? 1.0f : 0.0f);
        reverbMix.setTargetValue (p.reverbOn ? p.rvMix / 100.0f : 0.0f);
        chPreSm.setTargetValue (p.chPre);
        chDepthSm.setTargetValue (p.chWidth * 0.6f);

        // ---- compresor
        const float thresholdDb = -6.0f - p.compAmt * 3.0f;
        const float ratio = 1.5f + p.compAmt * 0.65f;
        const float slope = 1.0f / ratio - 1.0f;
        const float atkMs = p.attack == 2 ? 30.0f : (p.attack == 1 ? 12.0f : 2.0f);
        const float atkCoef = std::exp (-1.0f / (atkMs * 0.001f * (float) sr));
        const float relCoef = std::exp (-1.0f / (0.15f * (float) sr));
        compMakeup.setTargetValue (dbToGain (-thresholdDb * (1.0f - 1.0f / ratio) * 0.45f + p.compLevelDb));

        // ---- tone shaper (coeficientes solo si cambian)
        const float hiEff = p.hiDb + juce::jmin (0.0f, p.enhance) * 0.6f;
        if (p.lowDb != lastLow) { lastLow = p.lowDb; const auto c = shelf (false, sr, 110.0, 0.707, p.lowDb); lowF[0].c = lowF[1].c = c; }
        if (p.midDb != lastMid) { lastMid = p.midDb; const auto c = peak (sr, 900.0, 0.8, p.midDb);            midF[0].c = midF[1].c = c; }
        if (hiEff != lastHi)    { lastHi = hiEff;    const auto c = shelf (true, sr, 4500.0, 0.707, hiEff);    hiF[0].c  = hiF[1].c  = c; }
        const float exAmt = juce::jmax (0.0f, p.enhance) / 10.0f;
        const float exA = 1.0f - std::exp (-2.0f * pi * 3500.0f / (float) sr);

        // ---- wah
        const float q = 1.5f + p.wahRes * 0.75f, k = 1.0f / q;
        const float wAtk = std::exp (-1.0f / (0.003f * (float) sr)), wRel = std::exp (-1.0f / (0.10f * (float) sr));

        // ---- chorus
        const float lfoInc = p.chSpeed / (float) sr;
        const int cSize = chBuf.getNumSamples();
        float* cb[2] = { chBuf.getWritePointer (0), chBuf.getWritePointer (1) };

        for (int i = 0; i < n; ++i)
        {
            const float ig = inGain.getNextValue(), mk = compMakeup.getNextValue(), tg = toneGain.getNextValue();
            const float mC = compMix.getNextValue(), mT = toneMix.getNextValue(), mW = wahMix.getNextValue(), mH = chorusMix.getNextValue();
            float x[2] = { L[i] * ig, R[i] * ig };

            // COMP / LIMITER
            {
                const float lvl = std::max (std::abs (x[0]), std::abs (x[1]));
                const float c = lvl > compEnv ? atkCoef : relCoef;
                compEnv = c * compEnv + (1.0f - c) * lvl;
                if (mC > 0.0001f)
                {
                    const float over = 20.0f * std::log10 (std::max (compEnv, 1.0e-6f)) - thresholdDb;
                    float red = 0.0f;
                    if (2.0f * over > 6.0f)       red = slope * over;
                    else if (2.0f * over > -6.0f) red = slope * (over + 3.0f) * (over + 3.0f) / 12.0f;
                    const float g = std::pow (10.0f, red / 20.0f) * mk;
                    for (int ch = 0; ch < 2; ++ch) x[ch] += mC * (x[ch] * g - x[ch]);
                }
            }

            // WAH (pedal manual o automatico por envolvente)
            {
                const float lvl = std::max (std::abs (x[0]), std::abs (x[1]));
                const float c = lvl > wEnv ? wAtk : wRel;
                wEnv = c * wEnv + (1.0f - c) * lvl;
                if (mW > 0.0001f)
                {
                    float pos;
                    if (p.wahMode == 1) pos = p.wahPos / 10.0f;
                    else                pos = std::pow (juce::jlimit (0.0f, 1.0f, wEnv * (4.0f + p.wahPos * 4.0f)), 0.7f);
                    const float fc = 380.0f * std::pow (2400.0f / 380.0f, pos);
                    fcSm += 0.003f * (fc - fcSm);
                    const float g = std::tan (pi * juce::jmin (fcSm, (float) sr * 0.4f) / (float) sr);
                    const float a1 = 1.0f / (1.0f + g * (g + k)), a2 = g * a1, a3 = g * a2;
                    for (int ch = 0; ch < 2; ++ch)
                    {
                        const float v3 = x[ch] - s2[ch];
                        const float v1 = a1 * s1[ch] + a2 * v3;
                        const float v2 = s2[ch] + a2 * s1[ch] + a3 * v3;
                        s1[ch] = 2.0f * v1 - s1[ch];
                        s2[ch] = 2.0f * v2 - s2[ch];
                        const float wet = v1 * k * 2.2f + 0.12f * x[ch];
                        x[ch] += mW * (wet - x[ch]);
                    }
                }
            }

            // TONE SHAPER
            if (mT > 0.0001f)
            {
                for (int ch = 0; ch < 2; ++ch)
                {
                    float y = hiF[ch].process (midF[ch].process (lowF[ch].process (x[ch])));
                    if (exAmt > 0.0f)
                    {
                        exLp[ch] += exA * (y - exLp[ch]);
                        const float hp = y - exLp[ch];
                        y += exAmt * 0.4f * std::tanh (hp * 3.0f);
                    }
                    y *= tg;
                    x[ch] += mT * (y - x[ch]);
                }
            }

            // STEREO CHORUS (2 voces por canal, LFO en cuadratura entre izquierda y derecha)
            {
                const float pre = chPreSm.getNextValue(), depth = chDepthSm.getNextValue();
                for (int ch = 0; ch < 2; ++ch) cb[ch][chPos] = x[ch];
                if (mH > 0.0001f)
                {
                    const double base = 8.0 + (double) pre;
                    for (int ch = 0; ch < 2; ++ch)
                    {
                        float wet = 0.0f;
                        for (int v = 0; v < 2; ++v)
                        {
                            float ph = lfo + (ch == 0 ? (v == 0 ? 0.0f : 0.5f) : (v == 0 ? 0.25f : 0.75f));
                            ph -= std::floor (ph);
                            const double dMs = base + (double) depth * std::sin (2.0 * juce::MathConstants<double>::pi * (double) ph);
                            wet += tap (cb[ch], cSize, (double) chPos - dMs * 0.001 * sr);
                        }
                        const float y = 0.7f * x[ch] + 0.35f * wet;
                        x[ch] += mH * (y - x[ch]);
                    }
                }
                lfo += lfoInc; lfo -= std::floor (lfo);
                if (++chPos >= cSize) chPos = 0;
            }

            L[i] = x[0];
            R[i] = x[1];
        }

        // ---- reverb digital (por bloque)
        if (p.rvType != lastRvType) { lastRvType = p.rvType; applyReverbType (p.rvType); }
        const bool active = p.reverbOn || reverbMix.isSmoothing() || reverbMix.getCurrentValue() > 0.0005f;
        if (active)
        {
            float* s0 = scratch.getWritePointer (0);
            float* s1p = scratch.getWritePointer (1);
            std::copy (L, L + n, s0);
            std::copy (R, R + n, s1p);
            reverb.processStereo (s0, s1p, n);
            for (int i = 0; i < n; ++i)
            {
                const float m = reverbMix.getNextValue();
                L[i] = L[i] * (1.0f - m) + s0[i] * m;
                R[i] = R[i] * (1.0f - m) + s1p[i] * m;
            }
            reverbWasActive = true;
        }
        else if (reverbWasActive) { reverb.reset(); reverbWasActive = false; }

        // ---- salida
        for (int i = 0; i < n; ++i)
        {
            const float g = outGain.getNextValue();
            L[i] *= g; R[i] *= g;
        }
    }

    double sr = 44100.0;
    bool first = true;

    juce::SmoothedValue<float> inGain { 1.0f }, outGain { 1.0f }, compMakeup { 1.0f }, toneGain { 1.0f };
    juce::SmoothedValue<float> compMix { 0.0f }, toneMix { 0.0f }, wahMix { 0.0f }, chorusMix { 0.0f }, reverbMix { 0.0f };
    juce::SmoothedValue<float> chPreSm { 8.0f }, chDepthSm { 3.0f };

    float compEnv = 0.0f, wEnv = 0.0f, fcSm = 600.0f;
    Biquad lowF[2], midF[2], hiF[2];
    float exLp[2] = { 0.0f, 0.0f };
    float lastLow = 1.0e9f, lastMid = 1.0e9f, lastHi = 1.0e9f;
    float s1[2] = { 0.0f, 0.0f }, s2[2] = { 0.0f, 0.0f };

    juce::AudioBuffer<float> chBuf, scratch;
    int chPos = 0;
    float lfo = 0.0f;

    juce::Reverb reverb;
    bool reverbWasActive = false;
    int lastRvType = -1;
    juce::dsp::Limiter<float> limiter;
};
