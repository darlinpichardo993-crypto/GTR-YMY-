#define MyAppName "GTR ORYMY RD"
#define MyAppVersion "1.0.1"
#define MyAppPublisher "ORYMY | YMY Records"

[Setup]
AppId={{8E17A45B-6CF0-4D4C-8B21-6A5B1C0A1F72}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\GTR ORYMY RD
DefaultGroupName={#MyAppName}
OutputDir=build\installer\Output
OutputBaseFilename=GTR-ORYMY-RD-Setup
Compression=lzma2
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
DisableProgramGroupPage=yes
WizardStyle=modern
SetupIconFile=assets\gtr-eye.ico
WizardImageFile=assets\wizard-yellow.bmp
WizardSmallImageFile=assets\wizard-small-yellow.bmp
UninstallDisplayIcon={app}\GTR ORYMY RD.exe

[Files]
Source: "build\GtrOrymyRd_artefacts\Release\VST3\GTR ORYMY RD.vst3\*"; DestDir: "{commoncf}\VST3\GTR ORYMY RD.vst3"; Flags: recursesubdirs ignoreversion
Source: "build\GtrOrymyRd_artefacts\Release\Standalone\GTR ORYMY RD.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "assets\gtr-eye.ico"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\GTR ORYMY RD"; Filename: "{app}\GTR ORYMY RD.exe"; IconFilename: "{app}\gtr-eye.ico"
Name: "{autodesktop}\GTR ORYMY RD"; Filename: "{app}\GTR ORYMY RD.exe"; IconFilename: "{app}\gtr-eye.ico"

[Run]
Filename: "{app}\GTR ORYMY RD.exe"; Description: "Abrir GTR ORYMY RD"; Flags: nowait postinstall skipifsilent unchecked
