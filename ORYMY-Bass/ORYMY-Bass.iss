#define MyAppName "ORYMY Bass"
#define MyAppVersion "1.0.0"
#define MyPublisher "YMY Sounds"
[Setup]
AppId={{A21D6A34-8A9C-4B7A-BASS-ORYMY000001}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyPublisher}
DefaultDirName={autopf}\YMY Sounds\ORYMY Bass
DefaultGroupName=YMY Sounds\ORYMY Bass
OutputDir=build\installer\Output
OutputBaseFilename=ORYMY-Bass-Setup
SetupIconFile=assets\orymy-eye.ico
WizardImageFile=assets\wizard-yellow.bmp
WizardSmallImageFile=assets\wizard-small-yellow.bmp
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\orymy-eye.ico
[Dirs]
Name: "{commoncf}\VST3"
[Files]
Source: "build\ORYMYBass_artefacts\Release\VST3\ORYMY Bass.vst3\*"; DestDir: "{commoncf}\VST3\ORYMY Bass.vst3"; Flags: recursesubdirs ignoreversion
Source: "build\ORYMYBass_artefacts\Release\Standalone\ORYMY Bass.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "assets\orymy-eye.ico"; DestDir: "{app}"; Flags: ignoreversion
[Icons]
Name: "{autoprograms}\ORYMY Bass"; Filename: "{app}\ORYMY Bass.exe"; IconFilename: "{app}\orymy-eye.ico"
Name: "{autodesktop}\ORYMY Bass"; Filename: "{app}\ORYMY Bass.exe"; IconFilename: "{app}\orymy-eye.ico"; Tasks: desktopicon
[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional shortcuts:"
[Run]
Filename: "{app}\ORYMY Bass.exe"; Description: "Launch ORYMY Bass"; Flags: postinstall nowait skipifsilent
