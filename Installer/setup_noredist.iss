; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "BorderlessImageViewer"
#define MyAppVersion "20230417"
#define MyAppURL "https://github.com/Helios-vmg/Borderless"
#define MyAppExeName "Borderless.exe"
#define SourceBasePath ".."

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{F86CCAD1-7B42-4869-8A0B-3B32A0717A0B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DisableProgramGroupPage=yes
LicenseFile={#SourceBasePath}\COPYING.txt
OutputDir={#SourceBasePath}\bin64\
OutputBaseFilename=Borderless_setup_noredist
SetupIconFile=custom\icons\icon.ico
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#SourceBasePath}\bin64\{#MyAppExeName}"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#SourceBasePath}\COPYING.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "custom\qt.conf"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "custom\bin\*"; DestDir: "{app}\bin"; Flags: ignoreversion recursesubdirs

[Registry]
; Set up Applications subkey
Root: HKCR; Subkey: "Applications\{#MyAppExeName}"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Applications\{#MyAppExeName}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppName}"
Root: HKCR; Subkey: "Applications\{#MyAppExeName}\SupportedTypes"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Applications\{#MyAppExeName}\shell\open\command"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Applications\{#MyAppExeName}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\{#MyAppExeName}"" ""%1"""

#include "registry.iss"

[Icons]
Name: "{commonprograms}\{#MyAppName}"; Filename: "{app}\bin\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\bin\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
