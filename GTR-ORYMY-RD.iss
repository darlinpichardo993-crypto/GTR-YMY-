[Setup]
AppName=GTR ORYMY RD
AppVersion=1.0.0
AppPublisher=ORYMY | YMY Records
DefaultDirName={autopf}\GTR ORYMY RD
DefaultGroupName=GTR ORYMY RD
OutputDir=Output
OutputBaseFilename=GTR-ORYMY-RD-Setup
Compression=lzma2
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
DisableProgramGroupPage=yes

[Files]
; Plugin VST3 (carpeta estandar que escanea FL Studio)
Source: "..\build\GtrOrymyRd_artefacts\Release\VST3\GTR ORYMY RD.vst3\*"; DestDir: "{commoncf64}\VST3\GTR ORYMY RD.vst3"; Flags: recursesubdirs ignoreversion
; Version independiente
Source: "..\build\GtrOrymyRd_artefacts\Release\Standalone\GTR ORYMY RD.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\GTR ORYMY RD"; Filename: "{app}\GTR ORYMY RD.exe"

[Run]
Filename: "{app}\GTR ORYMY RD.exe"; Description: "Abrir GTR ORYMY RD"; Flags: nowait postinstall skipifsilent unchecked
