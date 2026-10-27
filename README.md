# GTR ORYMY RD

Pedal de guitarra de bachata en formato VST3 y aplicación Standalone para Windows.

## Compilar y descargar

Cada push a `main` y cada ejecución manual compilan el proyecto con GitHub Actions.

1. Abre la pestaña **Actions** del repositorio.
2. Selecciona **Build GTR ORYMY RD (Windows)** y abre la ejecución más reciente.
3. Espera a que termine correctamente.
4. En **Artifacts**, descarga `GTR-ORYMY-RD-Setup`.
5. Descomprime el ZIP descargado y ejecuta `GTR-ORYMY-RD-Setup.exe`.

El instalador incluye el plugin VST3 y la aplicación independiente. Usa una identidad de instalación propia y no necesita sobrescribir otro plugin. Cierra FL Studio antes de instalar y vuelve a escanear los plugins desde **Options → Manage plugins → Find installed plugins**.

## Efectos

- Compresor/limitador, wah, tone shaper, chorus estéreo y reverb.
- Controles Master Input y Master Output.
- 13 presets y detector de tonalidad/escala.

## Compilación local

Requiere CMake 3.22+, Visual Studio 2022 C++ build tools y acceso a Internet para descargar JUCE 8.0.4.
