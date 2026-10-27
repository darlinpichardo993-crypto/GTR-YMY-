#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <cmath>

// Detecta la tonalidad, la escala y la nota que tocas (hilo propio, no toca el audio).
//  - Tonalidad: croma por saliencia armonica + perfiles de Krumhansl.
//  - Nota actual: algoritmo YIN.
namespace gtrkey
{
    inline juce::String solfege (char letter)
    {
        switch (letter)
        {
            case 'C': return "Do";  case 'D': return "Re";  case 'E': return "Mi";
            case 'F': return "Fa";  case 'G': return "Sol"; case 'A': return "La";
            default:  return "Si";
        }
    }

    // nombre de una nota (con sostenidos) en solfeo, ej. 1 -> "Do#"
    inline juce::String pcName (int pc)
    {
        static const char* n[12] = { "Do", "Do#", "Re", "Re#", "Mi", "Fa", "Fa#", "Sol", "Sol#", "La", "La#", "Si" };
        return n[((pc % 12) + 12) % 12];
    }

    // Escritura correcta de la escala: cada letra una vez (Re Mi Fa Sol La Sib Do ...)
    inline juce::StringArray spellScale (int tonicPc, bool minor, bool harmonic, bool* pcMask12 = nullptr)
    {
        static const char* maj[12] = { "C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };
        static const char* mnr[12] = { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "G#", "A", "Bb", "B" };
        static const int stepsMaj[7]  = { 0, 2, 4, 5, 7, 9, 11 };
        static const int stepsMin[7]  = { 0, 2, 3, 5, 7, 8, 10 };
        static const int stepsHarm[7] = { 0, 2, 3, 5, 7, 8, 11 };
        static const int naturalPc[7] = { 0, 2, 4, 5, 7, 9, 11 };
        const juce::String letters ("CDEFGAB");

        const juce::String tn = minor ? mnr[tonicPc % 12] : maj[tonicPc % 12];
        const int li = letters.indexOfChar (tn[0]);
        const int* steps = minor ? (harmonic ? stepsHarm : stepsMin) : stepsMaj;

        juce::StringArray out;
        for (int d = 0; d < 7; ++d)
        {
            const int idx = (li + d) % 7;
            const int target = (tonicPc + steps[d]) % 12;
            const int diff = ((target - naturalPc[idx] + 18) % 12) - 6;
            juce::String acc;
            if (diff == 1) acc = "#"; else if (diff == -1) acc = "b";
            else if (diff == 2) acc = "x"; else if (diff == -2) acc = "bb";
            out.add (solfege (letters[idx]) + acc);
            if (pcMask12 != nullptr) pcMask12[target] = true;
        }
        return out;
    }

    inline juce::String keyName (int tonicPc, bool minor)
    {
        static const char* maj[12] = { "C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };
        static const char* mnr[12] = { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "G#", "A", "Bb", "B" };
        const juce::String code = minor ? mnr[tonicPc % 12] : maj[tonicPc % 12];
        const juce::String solf = solfege (code[0]) + code.substring (1).replace ("#", "#").replace ("b", "b");
        return solf + (minor ? " menor" : " mayor") + "  (" + code + (minor ? "m)" : ")");
    }
}

class KeyTracker : private juce::Thread
{
public:
    struct Result
    {
        bool  hasKey = false;
        int   tonic = 0;
        bool  minor = false;
        bool  harmonic = false;
        float confidence = 0.0f;     // 0..1
        int   noteMidi = -1;         // nota que suena ahora (-1 = ninguna)
        float chroma[12] = {};
    };

    KeyTracker() : juce::Thread ("GTR key tracker"), fifo (1 << 15), ring (1 << 15, 0.0f) {}
    ~KeyTracker() override { stopThread (2000); }

    void prepare (double sampleRate)
    {
        sr = sampleRate;
        decim = juce::jmax (1, (int) std::round (sampleRate / 11000.0));
        srD = sampleRate / (double) decim;
        if (! isThreadRunning()) startThread();
    }

    void push (const float* mono, int n)            // hilo de audio
    {
        int s1, z1, s2, z2;
        fifo.prepareToWrite (n, s1, z1, s2, z2);
        if (z1 > 0) std::copy (mono, mono + z1, ring.begin() + s1);
        if (z2 > 0) std::copy (mono + z1, mono + z1 + z2, ring.begin() + s2);
        fifo.finishedWrite (z1 + z2);
    }

    void reset() { resetRequested = true; }

    Result get() const { const juce::ScopedLock l (lock); return result; }

private:
    void run() override
    {
        std::vector<float> in, dec;                  // dec = senal decimada (ventana deslizante)
        in.reserve (1 << 15);
        dec.reserve (16384);
        std::vector<float> fftData (2 * kN, 0.0f);
        juce::dsp::FFT fft (13);
        std::vector<float> window (kN);
        for (int i = 0; i < kN; ++i)
            window[(size_t) i] = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * (float) i / (float) (kN - 1));

        double hist[12] = {};
        int sinceChroma = 0, sinceYin = 0;
        float decAcc = 0.0f; int decCount = 0;
        bool keyValid = false; int curTonic = 0; bool curMinor = false; float curCorr = 0.0f;
        int lastNote = -1, noteStable = 0;
        float chromaView[12] = {};

        while (! threadShouldExit())
        {
            wait (25);
            if (resetRequested.exchange (false))
            {
                for (double& h : hist) h = 0.0;
                keyValid = false; lastNote = -1; noteStable = 0;
                const juce::ScopedLock l (lock);
                result = Result();
            }

            const int avail = fifo.getNumReady();
            if (avail <= 0) continue;
            in.resize ((size_t) avail);
            int s1, z1, s2, z2;
            fifo.prepareToRead (avail, s1, z1, s2, z2);
            if (z1 > 0) std::copy (ring.begin() + s1, ring.begin() + s1 + z1, in.begin());
            if (z2 > 0) std::copy (ring.begin() + s2, ring.begin() + s2 + z2, in.begin() + z1);
            fifo.finishedRead (z1 + z2);

            for (float x : in)                       // decimacion (promedio de D muestras)
            {
                decAcc += x;
                if (++decCount >= decim) { dec.push_back (decAcc / (float) decim); decAcc = 0.0f; decCount = 0; ++sinceChroma; ++sinceYin; }
            }
            if (dec.size() > 20000) dec.erase (dec.begin(), dec.end() - 12000);
            if ((int) dec.size() < kN) continue;

            const float* tail = dec.data() + dec.size() - (size_t) kN;

            // ---------------- nota actual (YIN) cada ~46 ms
            if (sinceYin >= 512)
            {
                sinceYin = 0;
                float rms = 0.0f;
                for (int i = 0; i < 1024; ++i) { const float v = tail[kN - 1024 + i]; rms += v * v; }
                rms = std::sqrt (rms / 1024.0f);
                int note = -1;
                if (rms > 0.004f)
                {
                    const double f = yin (dec.data(), (int) dec.size());
                    if (f > 60.0 && f < 1300.0)
                        note = (int) std::round (69.0 + 12.0 * std::log2 (f / 440.0));
                }
                if (note == lastNote && note >= 0) ++noteStable; else noteStable = 0;
                lastNote = note;
                const juce::ScopedLock l (lock);
                result.noteMidi = (note >= 0 && noteStable >= 1) ? note : -1;
            }

            // ---------------- croma y tonalidad cada ~190 ms
            if (sinceChroma >= 2048)
            {
                sinceChroma = 0;
                float rms = 0.0f;
                for (int i = 0; i < kN; ++i) rms += tail[i] * tail[i];
                rms = std::sqrt (rms / (float) kN);
                if (rms > 0.004f)
                {
                    for (int i = 0; i < kN; ++i) fftData[(size_t) i] = tail[i] * window[(size_t) i];
                    std::fill (fftData.begin() + kN, fftData.end(), 0.0f);
                    fft.performFrequencyOnlyForwardTransform (fftData.data());

                    float c[12] = {};
                    const double df = srD / (double) kN;
                    float sal[kMaxMidi + 2] = {};
                    for (int m = kMinMidi - 1; m <= kMaxMidi + 1; ++m)
                    {
                        const double f0 = 440.0 * std::pow (2.0, (m - 69) / 12.0);
                        float s = 0.0f, w = 1.0f;
                        for (int h = 1; h <= 5; ++h, w *= 0.75f)
                        {
                            const double f = f0 * h;
                            if (f > srD * 0.5 - 50.0) break;
                            const int k = (int) std::lround (f / df);
                            s += w * juce::jmax (fftData[(size_t) juce::jmax (0, k - 1)], fftData[(size_t) k], fftData[(size_t) (k + 1)]);
                        }
                        sal[m - (kMinMidi - 1)] = s;
                    }
                    for (int m = kMinMidi; m <= kMaxMidi; ++m)
                    {
                        const float a = sal[m - (kMinMidi - 1)];
                        if (a > sal[m - 1 - (kMinMidi - 1)] && a >= sal[m + 1 - (kMinMidi - 1)])
                            c[m % 12] += a;
                    }
                    float sum = 0.0f;
                    for (float v : c) sum += v;
                    if (sum > 1.0e-9f)
                    {
                        for (int i = 0; i < 12; ++i) { c[i] /= sum; chromaView[i] = c[i]; }
                        for (int i = 0; i < 12; ++i) hist[i] = hist[i] * 0.99 + (double) c[i];
                    }
                }

                double total = 0.0;
                for (double h : hist) total += h;
                if (total > 2.0)                                   // ~ medio segundo de audio con tono
                {
                    static const double maj[12] = { 6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88 };
                    static const double mnr[12] = { 6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17 };
                    double bestC = -2.0; int bestT = 0; bool bestMin = false;
                    double hm = 0.0; for (double h : hist) hm += h / 12.0;
                    for (int t = 0; t < 12; ++t)
                        for (int mode = 0; mode < 2; ++mode)
                        {
                            const double* prof = mode == 0 ? maj : mnr;
                            double pm = 0.0; for (int i = 0; i < 12; ++i) pm += prof[i] / 12.0;
                            double sxy = 0.0, sxx = 0.0, syy = 0.0;
                            for (int i = 0; i < 12; ++i)
                            {
                                const double x = hist[(i + t) % 12] - hm, y = prof[i] - pm;
                                sxy += x * y; sxx += x * x; syy += y * y;
                            }
                            const double corr = sxy / std::sqrt (juce::jmax (1.0e-12, sxx * syy));
                            if (corr > bestC) { bestC = corr; bestT = t; bestMin = (mode == 1); }
                        }

                    // histeresis: solo cambia si la nueva tonalidad gana con claridad
                    if (! keyValid || (bestT == curTonic && bestMin == curMinor))
                    { curTonic = bestT; curMinor = bestMin; curCorr = (float) bestC; keyValid = true; }
                    else
                    {
                        double curC = -2.0;
                        {
                            const double* prof = curMinor ? mnr : maj;
                            double pm = 0.0; for (int i = 0; i < 12; ++i) pm += prof[i] / 12.0;
                            double sxy = 0.0, sxx = 0.0, syy = 0.0;
                            for (int i = 0; i < 12; ++i)
                            {
                                const double x = hist[(i + curTonic) % 12] - hm, y = prof[i] - pm;
                                sxy += x * y; sxx += x * x; syy += y * y;
                            }
                            curC = sxy / std::sqrt (juce::jmax (1.0e-12, sxx * syy));
                        }
                        if (bestC > curC + 0.04) { curTonic = bestT; curMinor = bestMin; curCorr = (float) bestC; }
                        else curCorr = (float) curC;
                    }

                    const bool harmonic = curMinor && hist[(curTonic + 11) % 12] > hist[(curTonic + 10) % 12]
                                          && hist[(curTonic + 11) % 12] > 0.04 * total;
                    const juce::ScopedLock l (lock);
                    result.hasKey = true; result.tonic = curTonic; result.minor = curMinor;
                    result.harmonic = harmonic; result.confidence = juce::jlimit (0.0f, 1.0f, curCorr);
                    for (int i = 0; i < 12; ++i) result.chroma[i] = chromaView[i];
                }
            }
        }
    }

    // YIN sobre la senal decimada. Devuelve la frecuencia en Hz o 0.
    double yin (const float* x, int n) const
    {
        constexpr int W = 1024;
        const int tauMax = (int) (srD / 65.0) + 2, tauMin = juce::jmax (2, (int) (srD / 1250.0));
        if (n < W + tauMax + 2) return 0.0;
        const float* seg = x + (n - (W + tauMax));
        const float* a = seg + tauMax;
        std::vector<double> cm ((size_t) tauMax + 2, 1.0);
        double run = 0.0;
        for (int tau = 1; tau <= tauMax; ++tau)
        {
            const float* b = seg + tauMax - tau;
            double d = 0.0;
            for (int j = 0; j < W; ++j) { const double df = (double) a[j] - (double) b[j]; d += df * df; }
            run += d;
            cm[(size_t) tau] = run > 0.0 ? d * tau / run : 1.0;
        }
        int tau = -1;
        for (int t = tauMin; t < tauMax; ++t)
            if (cm[(size_t) t] < 0.15)
            {
                while (t + 1 < tauMax && cm[(size_t) t + 1] < cm[(size_t) t]) ++t;
                tau = t; break;
            }
        if (tau < 0) return 0.0;
        double tf = (double) tau;
        if (tau > 1 && tau < tauMax - 1)
        {
            const double s0 = cm[(size_t) tau - 1], s1 = cm[(size_t) tau], s2 = cm[(size_t) tau + 1];
            const double den = 2.0 * (2.0 * s1 - s2 - s0);
            if (std::abs (den) > 1.0e-12) tf = tau + (s2 - s0) / den;
        }
        return srD / tf;
    }

    static constexpr int kN = 8192, kMinMidi = 40, kMaxMidi = 84;

    juce::AbstractFifo fifo;
    std::vector<float> ring;
    double sr = 44100.0, srD = 11025.0;
    int decim = 4;
    std::atomic<bool> resetRequested { false };
    mutable juce::CriticalSection lock;
    Result result;
};
