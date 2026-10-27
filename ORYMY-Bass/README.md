# ORYMY Bass — YMY Sounds VST

A dedicated JUCE VST3 / Standalone bass amplifier for Dominican bachata, merengue, típico, slap and studio bass. This project lives in its own `ORYMY-Bass/` directory and does not change the GTR ORYMY RD source files.

## First signature preset bank
- Bachata — Antony Santos
- Bachata — Raulin
- Bachata — Romeo
- Max Santos Bass
- Slap Bachata
- Mambo Clarity
- Merengue Clásico / Moderno
- Típico Bajo
- Bajo Profundo / Suave / Duro
- Bajo de Estudio
- Bachata — Cuerda y Cuerpo
- Mambo — Ataque Preciso
- Custom — ORYMY

These are original tonal starting points inspired by genre and performance approaches, not official presets or endorsements from the named artists.

## Audio features
Input drive and amp saturation, 10-band graphic EQ, bass/mid/treble/presence voicing, noise gate, compressor with attack control, output level, saved host automation/state, MIDI note monitor, and scale guide.

## Build on Windows
GitHub Actions runs CMake + JUCE 8.0.4, builds the VST3 and Standalone targets, packages the VST3 and Standalone app in an Inno Setup installer, and publishes the installer as a workflow artifact. Open **Actions → Build ORYMY Bass → latest successful run → Artifacts**.

## Local build
```powershell
cmake -S ORYMY-Bass -B ORYMY-Bass/build -A x64
cmake --build ORYMY-Bass/build --config Release
```

## Brand
ORYMY Bass / YMY Sounds VST • Dominican Republic
