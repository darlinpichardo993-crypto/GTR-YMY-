#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

namespace OrymyBass {
struct Preset {
    const char* name;
    float input, bass, mid, treble, presence, master, gate, attack, comp, drive;
    std::array<float,10> eq;
};
inline const std::array<Preset,16> presets {{
 {"Bachata - Antony Santos", 4.6f,6.8f,5.1f,5.4f,6.2f,7.0f,-52.0f,16.0f,5.8f,2.0f, {{2,1.5f,1,0.5f,0,1,2,2,1,0.5f}}},
 {"Bachata - Raulin", 4.2f,6.2f,6.2f,5.8f,5.4f,7.1f,-54.0f,20.0f,5.0f,1.7f, {{1.5f,1,0.5f,1,1.5f,2,1.5f,1,0.5f,0}}},
 {"Bachata - Romeo", 4.8f,6.1f,5.8f,6.4f,6.7f,6.8f,-55.0f,12.0f,5.4f,2.5f, {{1,1,0.5f,0,0.5f,1,1.5f,2,2,1.5f}}},
 {"Max Santos Bass", 5.0f,7.0f,6.8f,5.6f,5.8f,6.8f,-50.0f,8.0f,6.4f,3.0f, {{2.5f,2,1.5f,1,1.5f,2,1,0.5f,0,-0.5f}}},
 {"Slap Bachata", 5.6f,6.4f,6.9f,6.5f,7.1f,6.4f,-48.0f,4.0f,4.8f,4.8f, {{1.5f,2,1.5f,0,1,2,2.5f,3,2,1}}},
 {"Mambo Clarity", 4.8f,6.1f,7.4f,6.6f,7.2f,6.5f,-50.0f,5.0f,6.0f,2.8f, {{1.5f,1,0,1,2,2.5f,2,2,1,0.5f}}},
 {"Merengue Clasico", 4.1f,6.7f,6.0f,5.1f,5.2f,7.2f,-54.0f,18.0f,5.6f,1.4f, {{2,1.5f,1,0.5f,0.5f,1,1,0.5f,0,-0.5f}}},
 {"Merengue Moderno", 4.8f,6.0f,7.0f,6.6f,7.0f,6.5f,-50.0f,6.0f,6.0f,2.8f, {{1,1,0.5f,1.5f,2,2,2.5f,2,1.5f,1}}},
 {"Tipico Bajo", 4.6f,7.2f,7.0f,5.0f,5.0f,6.8f,-52.0f,10.0f,5.2f,2.0f, {{2.5f,2,1,1,1.5f,2,1,0,-1,-1}}},
 {"Bajo Profundo", 4.2f,8.2f,4.6f,4.0f,4.0f,6.8f,-58.0f,24.0f,5.8f,1.2f, {{3,2.5f,2,1,0,-1,-1.5f,-2,-2,-2}}},
 {"Bajo Suave", 3.8f,6.5f,5.2f,4.7f,4.6f,7.2f,-58.0f,28.0f,4.0f,0.7f, {{1.5f,1,1,0.5f,0,-0.5f,-1,-1,-1,-1}}},
 {"Bajo Duro", 5.8f,6.6f,7.0f,6.1f,6.4f,6.0f,-46.0f,3.0f,7.2f,4.0f, {{1,2,2,1,2,2,1,0.5f,0,-0.5f}}},
 {"Bajo de Estudio", 4.0f,6.0f,5.8f,5.4f,5.2f,7.0f,-56.0f,22.0f,4.8f,0.8f, {{0.5f,0.5f,0.5f,0,0,0.5f,0.5f,0.5f,0,0}}},
 {"Bachata - Cuerda y Cuerpo", 4.5f,7.0f,6.2f,5.2f,5.7f,6.8f,-52.0f,12.0f,5.8f,1.8f, {{2,2,1.5f,1,0.5f,1,1,0.5f,0,-0.5f}}},
 {"Mambo - Ataque Preciso", 5.2f,6.0f,7.4f,6.4f,7.5f,6.3f,-48.0f,2.0f,6.5f,2.6f, {{1,1,0.5f,1,2,2.5f,2,2,1,1}}},
 {"Custom - ORYMY", 4.5f,6.0f,6.0f,6.0f,6.0f,7.0f,-52.0f,12.0f,5.0f,1.5f, {{0,0,0,0,0,0,0,0,0,0}}}
}};
inline void applyPreset(juce::AudioProcessorValueTreeState& state, int index) {
    const auto& p = presets[(size_t)juce::jlimit(0, (int)presets.size()-1, index)];
    auto set = [&state](const juce::String& id, float value) {
        if (auto* param = state.getParameter(id)) param->setValueNotifyingHost(param->convertTo0to1(value));
    };
    set("input",p.input); set("bass",p.bass); set("mid",p.mid); set("treble",p.treble);
    set("presence",p.presence); set("master",p.master); set("gate",p.gate); set("attack",p.attack);
    set("comp",p.comp); set("drive",p.drive);
    for (int i=0;i<10;++i) set("eq"+juce::String(i),p.eq[(size_t)i]);
}
}
