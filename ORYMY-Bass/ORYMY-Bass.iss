#define MyAppName "ORYMY Bass"
#define MyAppVersion "1.0.1"
#define MyPublisher "YMY Sounds"
[Setup]
AppId={{A21D6A34-8A9C-4B7A-BA55-0A0B00000001}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
UninstallDisplayName=ORYMY Bass by YMY Sounds
AppPublisher={#MyPublisher}
AppComments=Professional Dominican bass tone suite by YMY Sounds
AppContact=https://github.com/darlinpichardo993-crypto/GTR-YMY-
VersionInfoVersion=1.0.1.0
VersionInfoCompany={#MyPublisher}
VersionInfoDescription=ORYMY Bass Installer
VersionInfoProductName=ORYMY Bass
DefaultDirName={autopf}\YMY Sounds\ORYMY Bass
DefaultGroupName=YMY Sounds\ORYMY Bass
DisableProgramGroupPage=yes
DisableWelcomePage=no
DisableDirPage=no
DisableReadyPage=no
UninstallDisplayName=ORYMY Bass by YMY Sounds
CloseApplications=yes
RestartApplications=no
OutputDir=build\installer\Output
OutputBaseFilename=ORYMY-Bass-Setup
SetupIconFile=assets\orymy-eye.ico
WizardImageFile=assets\wizard-premium.bmp
WizardSmallImageFile=assets\wizard-small-premium.bmp
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
Name: "{autodesktop}\ORYMY Bass"; Filename: "{app}\ORYMY Bass.exe"; WorkingDir: "{app}"; IconFilename: "{app}\orymy-eye.ico"; Comment: "ORYMY Bass by YMY Sounds"
[Run]
Filename: "{app}\ORYMY Bass.exe"; Description: "Launch ORYMY Bass now"; WorkingDir: "{app}"; Flags: postinstall nowait skipifsilent
