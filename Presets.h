#pragma once
#include <array>

// Orden de los parametros (igual en todos los presets)
namespace gtr
{
    constexpr int kNumParams = 23;

    inline const char* const* paramIds()
    {
        static const char* const ids[kNumParams] = {
            "inGain", "compOn", "compAmt", "compLevel", "attack",
            "toneOn", "low", "mid", "hi", "enhance", "toneLevel",
            "wahOn", "wahPos", "wahRes", "wahMode",
            "chorusOn", "chSpeed", "chWidth", "chPre",
            "reverbOn", "rvMix", "rvType",
            "outGain" };
        return ids;
    }

    struct Preset { const char* name; float v[kNumParams]; };

    // attack: 0=SOFT 1=MID 2=HARD | wahMode: 0=AUTO 1=PEDAL | rvType: 0=PLATE 1=ROOM 2=STAGE 3=HALL
    inline const std::array<Preset, 13>& presets()
    {
        static const std::array<Preset, 13> p = {{
        //                     in   cOn cAmt cLvl atk tOn  low  mid   hi  enh tLvl wOn wPos wRes wMode chOn chSpd chW chPre rvOn rvMix rvT out
        { "GTR ORYMY - Fluye",         { 0,  1, 4.0f, 2, 1,  1,   0,   0, 1.5f, 2,  0,  0, 5, 5, 1,  1, 0.8f, 5, 8,   1, 18, 1,  0 } },
        { "Raulin - Requinto Clasico", { 0,  1, 5.0f, 2, 2,  1,  -3,   2,   4,  4,  0,  0, 5, 5, 1,  1, 0.6f, 3, 5,   1, 14, 1,  0 } },
        { "Raulin - Requinto Brillante",{0,  1, 6.0f, 2, 2,  1,  -4,   1,   6,  6, -1,  0, 5, 5, 1,  1, 1.0f, 4, 4,   1, 20, 0, -1 } },
        { "Antony Santos - Guitarra Calida",{0,1,3.0f,2, 1,  1,   3,   1,  -1, -2,  0,  0, 5, 5, 1,  1, 0.5f, 6, 10,  1, 22, 2,  0 } },
        { "Antony Santos - Bachata Vieja",  {0,1,2.0f,3, 1,  1,   4,   0,  -3, -5,  0,  0, 5, 5, 1,  1, 0.4f, 7, 12,  1, 26, 3,  0 } },
        { "Romeo - Aventura Cristal",  { 0,  1, 4.0f, 2, 1,  1,  -1,   1,   3,  3,  0,  0, 5, 5, 1,  1, 1.2f, 6, 8,   1, 20, 2,  0 } },
        { "Romeo - Acustica Ancha",    { 0,  1, 3.0f, 2, 1,  1,   1,   0,   2,  1,  0,  0, 5, 5, 1,  1, 0.9f, 8, 14,  1, 25, 3,  0 } },
        { "Lenny - Lead Cremoso",      { 0,  1, 7.0f, 3, 0,  1,  -2,   3,   2,  2,  1,  0, 5, 5, 1,  1, 1.5f, 5, 6,   1, 22, 0, -1 } },
        { "Lenny - Requinto Dulce",    { 0,  1, 5.0f, 2, 1,  1,  -2,   2,   1,  0,  0,  0, 5, 5, 1,  1, 0.7f, 6, 9,   1, 28, 3,  0 } },
        { "Myke Suenala - Urbano Limpio",{0, 1, 4.0f, 2, 1,  1,   0,   1,   2,  2,  0,  0, 5, 5, 1,  1, 0.3f, 2, 3,   1, 12, 1,  0 } },
        { "Myke Suenala - Ambiental",  { 0,  1, 3.0f, 2, 1,  1,   1,   0,   1,  1,  0,  0, 5, 5, 1,  1, 0.6f, 9, 16,  1, 38, 3,  0 } },
        { "Wah Bachatero",             { 0,  1, 4.0f, 2, 1,  1,   0,   1,   2,  1,  0,  1, 6, 6, 1,  1, 0.8f, 4, 6,   1, 16, 1,  0 } },
        { "Wah Auto - Sensual",        { 0,  1, 4.0f, 2, 1,  1,   1,   0,   1,  0,  0,  1, 6, 5, 0,  1, 0.7f, 5, 8,   1, 22, 2,  0 } },
        }};
        return p;
    }
}
