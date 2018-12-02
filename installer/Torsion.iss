; Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
; This file is subject to the terms and conditions defined in
; file 'LICENSE.txt', which is part of this source code package.

[Setup]
AppName=Torsion
AppVerName=Torsion 1.1.4091
AppPublisher= Sickhead Games, LLC
AppPublisherURL=http://www.sickheadgames.com/
AppSupportURL=https://github.com/SickheadGames/Torsion/issues
AppUpdatesURL=https://github.com/SickheadGames/Torsion
AppCopyright=Copyright (C) Sickhead Games, LLC
AppMutex=Torsion.SickheadGames
UsePreviousAppDir=yes
DefaultGroupName=Torsion
DefaultDirName={pf}\Torsion
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE.txt
InfoAfterFile=PostInstall.txt
Compression=lzma/ultra
SolidCompression=yes
OutputDir=..\
OutputBaseFilename=InstallTorsion
EnableDirDoesntExistWarning=no
DirExistsWarning=no
AppendDefaultDirName=no
ChangesAssociations=yes
SetupIconFile=..\code\Torsion\icons\torsion.ico
WizardImageFile=installerimage.bmp
WizardSmallImageFile=installerimagesmall_a.bmp

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\media\torsion_p.exe"; DestName: "torsion.exe";  DestDir: "{app}"; Flags: ignoreversion
Source: "..\media\msvcr71.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\media\msvcp71.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\media\torsion.chm"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\media\torsion_exports.cs"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\media\torsion_precompile.cs"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE.txt"; DestName: "Torsion EULA.txt"; DestDir: "{app}"; Flags: ignoreversion

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[INI]
Filename: "{app}\Torsion Product Page.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.garagegames.com/products/106"
Filename: "{app}\Torsion Forums.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.garagegames.com/mg/forums/result.area.php?qa=55"

[Icons]
Name: "{group}\Torsion"; Filename: "{app}\torsion.exe"; WorkingDir: "{app}"; Comment: "Torsion TorqueScript IDE"
Name: "{group}\Torsion Help"; Filename: "{app}\torsion.chm"; WorkingDir: "{app}"; Comment: "Torsion Help"
Name: "{group}\Torsion Product Page"; Filename: "{app}\Torsion Product Page.url"; Comment: "Torsion Product Page"
Name: "{group}\Torsion Forums"; Filename: "{app}\Torsion Forums.url"; Comment: "Torsion Forums"
Name: "{group}\{cm:UninstallProgram,Torsion}"; Filename: "{uninstallexe}"; Comment: "Uninstall Torsion"
Name: "{userdesktop}\Torsion"; Filename: "{app}\torsion.exe"; Tasks: desktopicon; Comment: "Torsion TorqueScript IDE"

[Run]
Filename: "{app}\torsion.exe"; Description: "{cm:LaunchProgram,Torsion}"; Flags: nowait postinstall skipifsilent

; Let Torsion register the script extensions itself.
Filename: "{app}\torsion.exe"; WorkingDir: "{app}"; Parameters: "-exts"; Flags: waituntilterminated

[Registry]

; Add Torsion to the App Paths key.
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\torsion.exe"; ValueType: string; ValueData: "{app}\torsion.exe"; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\torsion.exe"; ValueName: "Path"; ValueType: string; ValueData: "{app}\"; Flags: uninsdeletevalue uninsdeletekeyifempty

; Associate .torsion to torsion.exe
Root: HKCR; Subkey: ".torsion"; ValueType: string; ValueData: "TorsionProject";
Root: HKCR; Subkey: "TorsionProject"; ValueType: string; ValueData: "Torsion Project File";
Root: HKCR; Subkey: "TorsionProject\DefaultIcon"; ValueType: string; ValueData: "torsion.exe,1"
Root: HKCR; Subkey: "TorsionProject\shell\open\command"; ValueType: string; ValueData: "torsion.exe ""%1"""

; Torsion itself does the associations which point script extensions
; (.cs,.gui,.mis,etc...) to TorsionTorqueScript.
Root: HKCR; Subkey: "TorsionTorqueScript"; ValueType: string; ValueData: "TorqueScript File";
Root: HKCR; Subkey: "TorsionTorqueScript\DefaultIcon"; ValueType: string; ValueData: "torsion.exe,2"
Root: HKCR; Subkey: "TorsionTorqueScript\shell\open\command"; ValueType: string; ValueData: "torsion.exe ""%1"""

[UninstallDelete]
Type: files; Name: "{app}\Torsion Product Page.url"
Type: files; Name: "{app}\Torsion Forums.url"
Type: files; Name: "{app}\preferences.xml"
Type: files; Name: "{app}\ignition.dat"

[UninstallRun]
 
; Let Torsion unregister the script extensions.
Filename: "{app}\torsion.exe"; WorkingDir: "{app}"; Parameters: "-unexts"; Flags: waituntilterminated


