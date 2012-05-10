; -------------------------------------------------------------------
; help >>> Modern UI defines...
; -------------------------------------------------------------------
!include LogicLib.nsh
!include MUI2.nsh
!include x64.nsh

; -------------------------------------------------------------------
; help >>> General NAME and Versioning
; -------------------------------------------------------------------
Name "Emerge Desktop 6.0 RC1"
!define FIELD1 $R1
!define FIELD2 $R2
!define FIELD3 $R3
!define FIELD4 $R4
!define EMERGERUNNING $R8
!define BINDIR "..\Source\bin64"
!define MUI_CUSTOMFUNCTION_ABORT ".onCustomAbort"

Var Dialog
Var Label1
Var Label2
Var Label3
Var CheckBox1
Var CheckBox2
Var CheckBox3
Var CheckBox4

; -------------------------------------------------------------------
; help >>> Set compression
; -------------------------------------------------------------------
SetCompressor lzma

; -------------------------------------------------------------------
; help >>> installer pages we're gonna use
; -------------------------------------------------------------------
!define MUI_ABORTWARNING
!define MUI_UNABORTWARNING
!define MUI_HEADERBITMAP_RIGHT
!define MUI_CUSTOMPAGECOMMANDS
!define MUI_UNCUSTOMPAGECOMMANDS

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "license.txt"
Page custom nsDialogOptions nsDialogOptionsLeave
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
UninstPage custom un.nsDialogOptions un.nsDialogOptionsLeave
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

; -------------------------------------------------------------------
; help >>> when installation finishes, allow user to launch docs.
; -------------------------------------------------------------------
Icon "emerge.ico"
UninstallIcon "unemerge.ico"
OutFile "EmergeDesktop64-6.0-RC1.exe"

; -------------------------------------------------------------------
; help >>> Branding the installer makes it nice an unique... ;)
; -------------------------------------------------------------------
BrandingText "©2004 - 2012 Emerge Desktop Development Team"

; -------------------------------------------------------------------
; help >>> NSIS 2.0 modern ui with XP style please...
; -------------------------------------------------------------------
XPStyle on

; -------------------------------------------------------------------
; help >>> Good setting for all installers, overwrites older files
;          which is quite cool if you're updating later!
; -------------------------------------------------------------------
SetOverwrite ifnewer

; -------------------------------------------------------------------
; help >>> Where do we want our app to go today? Aah, the install dir!
; -------------------------------------------------------------------
InstallDir "$PROGRAMFILES64\Emerge Desktop"

; -------------------------------------------------------------------
; help >>> FILES TO INSTALL STUFF
; -------------------------------------------------------------------
Section "emergeCore" SecemergeCore
SectionIn RO
SetOutPath "$INSTDIR"
File "${BINDIR}\emergeCore.exe"
SetOutPath "$APPDATA\Emerge Desktop\scripts"
IfFileExists "$APPDATA\Emerge Desktop\scripts\hide.pl" +2
File "..\Source\scripts\hide.pl"
IfFileExists "$APPDATA\Emerge Desktop\scripts\Perl_HowTo.txt" +2
File "..\Source\scripts\Perl_HowTo.txt"
IfFileExists "$APPDATA\Emerge Desktop\scripts\hide.pyw" +2
File "..\Source\scripts\hide.pyw"
IfFileExists "$APPDATA\Emerge Desktop\scripts\Python_HowTo.txt" +2
File "..\Source\scripts\Python_HowTo.txt"
IfFileExists "$APPDATA\Emerge Desktop\scripts\hide.rb" +2
File "..\Source\scripts\hide.rb"
Push $R0
${If} ${FIELD1} == ${BST_CHECKED}
  System::Call "kernel32::GetCurrentProcess() i .s"
  System::Call "kernel32::IsWow64Process(i s, *i .r0)"
  StrCmp $0 "0" +2
  SetRegView 64
  ReadRegStr $R0 HKCU "Software\Microsoft\Windows NT\CurrentVersion\Winlogon" "Shell"
  Push $R0
  Call GetInQuotes
  Pop $R0
  StrCmp $R0 "$INSTDIR\emergeCore.exe" CORE_END
  IfFileExists "$INSTDIR\emergeCore.exe" 0 CORE_END
  WriteRegStr HKCU "Software\Microsoft\Windows NT\CurrentVersion\Winlogon" "Shell" "$INSTDIR\emergeCore.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows NT\CurrentVersion\Winlogon" "Shell" "explorer.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows NT\CurrentVersion\IniFileMapping\system.ini\boot" "Shell" "USR:Software\Microsoft\Windows NT\CurrentVersion\Winlogon"
  WriteRegDword HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer" "DesktopProcess" 1
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer" "BrowseNewProcess" "yes"
  SetRebootFlag true
${EndIf}
CORE_END:
Pop $R0
SectionEnd

SubSection "Core Applets" SecCoreApplets

Section "emergeWorkspace" SecemergeWorkspace
SectionIn RO
SetOutPath "$INSTDIR"
File "${BINDIR}\emergeWorkspace.exe"
Delete "$INSTDIR\emergeDesktop.exe"
SectionEnd

Section "emergeTasks" SecemergeTasks
SetOutPath "$INSTDIR"
File "${BINDIR}\emergeTasks.exe"
SectionEnd

Section "emergeTray" SecemergeTray
SetOutPath "$INSTDIR"
File "${BINDIR}\emergeTray.exe"
SectionEnd

SubSectionEnd

SubSection "Additional Applets" SecAdditionalApplets

Section "emergeCommand" SecemergeCommand
SetOutPath "$INSTDIR"
File "${BINDIR}\emergeCommand.exe"
CreateDirectory "$APPDATA\Emerge Desktop\files"
IfFileExists "$APPDATA\Emerge Desktop\files\cmd.txt" +3
SetOutPath "$APPDATA\Emerge Desktop\files"
File "..\Source\files\cmd.txt"
SectionEnd

Section "emergeHotkeys" SecemergeHotkeys
SetOutPath "$INSTDIR"
File "${BINDIR}\emergeHotkeys.exe"
SectionEnd

Section "emergeLauncher" SecemergeLauncher
SetOutPath "$INSTDIR"
File "${BINDIR}\emergeLauncher.exe"
SectionEnd

Section "emergeVWM" SecemergeVWM
SetOutPath "$INSTDIR"
File "${BINDIR}\emergeVWM.exe"
SectionEnd

Section "emergePower" SecemergePower
SetOutPath "$INSTDIR"
File "${BINDIR}\emergePower.exe"
SectionEnd

Section "emergeSysMon" SecemergeSysMon
SetOutPath "$INSTDIR"
File "${BINDIR}\emergeSysMon.exe"
SectionEnd

SubSectionEnd

SubSection "Utilities" SecUtilities

Section "emerge" Secemerge
SetOutPath "$INSTDIR"
File "${BINDIR}\emerge.exe"
SectionEnd

SubSectionEnd

Section "-Libraries"
SetOutPath "$INSTDIR"
File "${BINDIR}\emergeLib.dll"
File "${BINDIR}\libgcc_s_sjlj-1.dll"
File "${BINDIR}\libstdc++-6.dll"
File "${BINDIR}\emergeIcons.dll"
File "${BINDIR}\emergeGraphics.dll"
File "${BINDIR}\emergeAppletEngine.dll"
File "${BINDIR}\emergeStyleEngine.dll"
File "${BINDIR}\emergeBaseClasses.dll"
Delete "$INSTDIR\emergeSchemeEngine.dll"
SectionEnd

Section "-Themes"
CreateDirectory "$APPDATA\Emerge Desktop\themes\GBRY"
SetOutPath "$APPDATA\Emerge Desktop\themes\GBRY"
File /r "..\Source\themes\GBRY\*"
SetOutPath "$APPDATA\Emerge Desktop"
File "..\Source\themes\theme.xml"
SectionEnd

; -------------------------------------------------------------------
; help >>> add all files needed... here: documentation
; -------------------------------------------------------------------
Section "Documentation" SecDocumentation
SetOutPath "$INSTDIR\documentation\"
File "..\Documentation\Emerge Desktop.chm"
SectionEnd

; -------------------------------------------------------------------
; help >>> STARTMENU STUFF
; -------------------------------------------------------------------

Section "Start Menu Shortcuts" SecShortcuts
  SetShellVarContext current
  CreateDirectory "$SMPROGRAMS\Emerge Desktop"

; -------------------------------------------------------------------
; help >>> add more shortcuts as needed.
; -------------------------------------------------------------------
  CreateShortCut "$SMPROGRAMS\Emerge Desktop\Emerge Desktop Documentation.lnk" "$INSTDIR\documentation\Emerge Desktop.chm" "" "$INSTDIR\documentation\Emerge Desktop.chm" 0
  CreateShortCut "$SMPROGRAMS\Emerge Desktop\Emerge Desktop Shell Changer.lnk" "$INSTDIR\emergeCore.exe" "/shellchanger" "$INSTDIR\emergeCore.exe" 0
  IfFileExists "$INSTDIR\reg2xml.exe" +1 +2
  CreateShortCut "$SMPROGRAMS\Emerge Desktop\Registry to XML converter.lnk" "$INSTDIR\reg2xml.exe" "" "$INSTDIR\emergeCore.exe" 0
  CreateShortCut "$SMPROGRAMS\Emerge Desktop\Uninstall.lnk" "$INSTDIR\uninst.exe" "" "$INSTDIR\uninst.exe" 0
SectionEnd

; -------------------------------------------------------------------
; help >>> UNINSTALL STUFF
; -------------------------------------------------------------------
Section Uninstall
  System::Call "kernel32::GetCurrentProcess() i .s"
  System::Call "kernel32::IsWow64Process(i s, *i .r0)"
  StrCmp $0 "0" +2
  SetRegView 64
  DeleteRegValue HKCU "Software\Microsoft\Windows NT\CurrentVersion\Winlogon" "Shell"
  WriteRegStr HKLM "Software\Microsoft\Windows NT\CurrentVersion\Winlogon" "Shell" "explorer.exe"
  SetRebootFlag true

; -------------------------------------------------------------------
; help >>> remove scripts, if desired
; -------------------------------------------------------------------
  ${If} ${FIELD2} == ${BST_UNCHECKED}
    Delete "$APPDATA\Emerge Desktop\scripts\*"
    RMDir "$APPDATA\Emerge Desktop\scripts"
  ${EndIf}

; -------------------------------------------------------------------
; help >>> remove files, if desired
; -------------------------------------------------------------------
  ${If} ${FIELD3} == ${BST_UNCHECKED}
    Delete "$APPDATA\Emerge Desktop\files\*"
    RMDir "$APPDATA\Emerge Desktop\files"
  ${EndIf}

; -------------------------------------------------------------------
; help >>> remove themes, if desired
; -------------------------------------------------------------------
  ${If} ${FIELD4} == ${BST_UNCHECKED}
    RMDir /r "$APPDATA\Emerge Desktop\themes"
    Delete "$APPDATA\Emerge Desktop\theme.xml"
  ${EndIf}

; -------------------------------------------------------------------
; help >>> uninstall Emerge Desktop
; -------------------------------------------------------------------
  Call un.CloseCore
  SetShellVarContext current
  Delete "$SMPROGRAMS\Emerge Desktop\*.lnk"
  RMDir "$SMPROGRAMS\Emerge Desktop"
  Delete "$INSTDIR\documentation\Emerge Desktop.chm"
  RMDir "$INSTDIR\documentation"
  Delete "$INSTDIR\emerge.exe"
  Delete "$INSTDIR\emergeCommand.exe"
  Delete "$INSTDIR\emergeWorkspace.exe"
  Delete "$INSTDIR\emergeHotkeys.exe"
  Delete "$INSTDIR\emergeLauncher.exe"
  Delete "$INSTDIR\emergeTasks.exe"
  Delete "$INSTDIR\emergeTray.exe"
  Delete "$INSTDIR\emergeCore.exe"
  Delete "$INSTDIR\emergeVWM.exe"
  Delete "$INSTDIR\emergePower.exe"
  Delete "$INSTDIR\emergeSysMon.exe"
  Delete "$INSTDIR\emergeLib.dll"
  Delete "$INSTDIR\libgcc_s_sjlj-1.dll"
  Delete "$INSTDIR\libstdc++-6.dll"
  Delete "$INSTDIR\emergeIcons.dll"
  Delete "$INSTDIR\emergeAppletEngine.dll"
  Delete "$INSTDIR\emergeStyleEngine.dll"
  Delete "$INSTDIR\emergeGraphics.dll"
  Delete "$INSTDIR\emergeBaseClasses.dll"
  Delete "$INSTDIR\uninst.exe"
  RMDir "$INSTDIR"
  RMDir "$APPDATA\Emerge Desktop"
  DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Emerge Desktop"
  DeleteRegKey HKCU "SOFTWARE\Emerge Desktop"
SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${Secemerge} "emerge is a command line utility that can be used to execute Emerge Desktop Internal Commands by other non-Emerge Desktop applications."
!insertmacro MUI_DESCRIPTION_TEXT ${SecemergeCore} "emergeCore is the core of Emerge Desktop.  It is required for setting Emerge Desktop as the default shell."
!insertmacro MUI_DESCRIPTION_TEXT ${SecemergeCommand} "Provides a clock as well as method of entering commands to be executed"
!insertmacro MUI_DESCRIPTION_TEXT ${SecemergeHotkeys} "'Hotkey' application laucher as well as interfacing to Emerge Desktop itself"
!insertmacro MUI_DESCRIPTION_TEXT ${SecemergeTasks} "Provides an interface for managing the tasks on the system"
!insertmacro MUI_DESCRIPTION_TEXT ${SecemergeTray} "A 'System Tray' for Emerge Desktop"
!insertmacro MUI_DESCRIPTION_TEXT ${SecemergeVWM} "A 'Virtual Window Manager' for Emerge Desktop"
!insertmacro MUI_DESCRIPTION_TEXT ${SecemergePower} "A battery charge monitor for Emerge Desktop"
!insertmacro MUI_DESCRIPTION_TEXT ${SecemergeSysMon} "A CPU and Memory monitor for Emerge Desktop"
!insertmacro MUI_DESCRIPTION_TEXT ${SecDocumentation} "All the documentation for the Emerge Desktop applets and applications"
!insertmacro MUI_DESCRIPTION_TEXT ${SecShortcuts} "Provides a shortcut in the start menu to the Emerge Desktop documentation"
!insertmacro MUI_DESCRIPTION_TEXT ${SecCoreApplets} "Emerge Desktop Core applets"
!insertmacro MUI_DESCRIPTION_TEXT ${SecAdditionalApplets} "Additional Emerge Desktop applets"
!insertmacro MUI_DESCRIPTION_TEXT ${SecUtilities} "Utilities to assist in using Emerge Desktop"
!insertmacro MUI_DESCRIPTION_TEXT ${SecemergeLauncher} "Application Launcher"
!insertmacro MUI_DESCRIPTION_TEXT ${SecemergeWorkspace} "Provides the desktop menus"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; -------------------------------------------------------------------
; help >>> Language Strings
; -------------------------------------------------------------------

LangString PAGE_TITLE ${LANG_ENGLISH} "Desktop Shell"
LangString PAGE_SUBTITLE ${LANG_ENGLISH} "Set the desktop shell"

; -------------------------------------------------------------------
; help >>> Functions
; -------------------------------------------------------------------

; .onInit
;
; Read the install opions and versify the user has Windows 2000 or above
Function .onInit
${IfNot} ${RunningX64}
  MessageBox MB_OK|MB_SETFOREGROUND|MB_ICONSTOP "This installer is meant for machines running a 64-bit OS."
  Abort
${EndIf}
Call CheckWindowsVersion
StrCpy ${EMERGERUNNING} "0"
Push $R0
FindWindow $R0 "emergeCoreClass"
IntCmp $R0 0 +1 +2 +2
FindWindow $R0 "EmergeDesktopCore"
IntCmp $R0 0 SKIP_INSTDIR
Pop $R0
StrCpy ${EMERGERUNNING} "1"
MessageBox MB_OKCANCEL|MB_SETFOREGROUND|MB_ICONQUESTION "Emerge Desktop is currently running.  Would you like the$\r$\n installer to quit Emerge Desktop so the install can continue?" IDCANCEL +1 IDOK +2
Abort
Call CloseCore
Push $1
ReadRegStr $1 HKCU "Software\Microsoft\Windows NT\CurrentVersion\Winlogon" "Shell"
Push $1
Call GetInQuotes
Call GetPath
Pop $INSTDIR
StrCmp $INSTDIR "" +1 +2
StrCpy $INSTDIR "$PROGRAMFILES64\Emerge Desktop"
Pop $1
SKIP_INSTDIR:
FunctionEnd

;Function un.onUninstSuccess
;IfRebootFlag +1 SKIP_REBOOT
;MessageBox MB_YESNO "The system must be rebooted for the changes to take effect.  Do you want to reboot now?" IDYES +1 IDNO SKIP_REBOOT
;Reboot
;SKIP_REBOOT:
;FunctionEnd

; .onInstSuccess
;
; Write uninstaller after successful installation
Function .onInstSuccess
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\Emerge Desktop" "DisplayName" "Emerge Desktop, replacement shell (remove only)"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\Emerge Desktop" "UninstallString" "$INSTDIR\uninst.exe"
WriteUnInstaller "$INSTDIR\uninst.exe"
IfRebootFlag +2
call RestartShell
FunctionEnd

Function .onCustomAbort
call RestartShell
FunctionEnd

Function .onInstFailed
call RestartShell
FunctionEnd


Function GetInQuotes
Exch $R0
Push $R1
Push $R2
Push $R3
Push $R4

StrCpy $R4 $R0

StrCpy $R2 -1
IntOp $R2 $R2 + 1
StrCpy $R3 $R0 1 $R2
StrCmp $R3 "" 0 +3
StrCpy $R0 $R4
Goto GetInQuotesDone
StrCmp $R3 '"' 0 -5

IntOp $R2 $R2 + 1
StrCpy $R0 $R0 "" $R2

StrCpy $R2 0
IntOp $R2 $R2 + 1
StrCpy $R3 $R0 1 $R2
StrCmp $R3 "" 0 +3
StrCpy $R0 $R4
Goto GetInQuotesDone
StrCmp $R3 '"' 0 -5

StrCpy $R0 $R0 $R2

GetInQuotesDone:
Pop $R4
Pop $R3
Pop $R2
Pop $R1
Exch $R0
FunctionEnd

Function GetPath
; This function takes a file name and returns the base name (no extension)
; Input is from the top of the stack
; Usage example:
; push (file name)
; call GetPath
; pop (file name)

Exch $1  ; Initial value
Push $2  ; Pointer variable
Push $3  ; single character (temp)
StrLen $2 $1
StartBaseLoop:
  IntOp $2 $2 - 1
  StrCpy $3 $1 1 $2
  strCmp $3 "\" ExitCopy
  StrCmp $3 "" ExitBaseLoop
  Goto StartBaseLoop
ExitCopy:
StrCpy $1 $1 $2
ExitBaseLoop:
Pop $3
Pop $2
Exch $1
FunctionEnd

Function RestartShell
Push $1
Push $2
ReadRegStr $1 HKCU "Software\Microsoft\Windows NT\CurrentVersion\Winlogon" "Shell"
StrCpy $2 $1
Push $2
Call GetInQuotes
Pop $2
IfFileExists "$2" StartShell
ReadRegStr $1 HKLM "Software\Microsoft\Windows NT\CurrentVersion\Winlogon" "Shell"
StrCpy $2 $1
Push $2
Call GetInQuotes
Pop $2
IfFileExists "$2" StartShell
StrCpy $1 "explorer.exe"
StartShell:
MessageBox MB_OK "The installer will now restart the desktop shell.  Please note that it is possible not all tray icons will reappear."
Exec $1
Pop $2
Pop $1
FunctionEnd

; CheckWindowsVersion
;
; Based on GetWindowsVersion
;
; Usage:
;   Call CheckWindowsVersion
Function CheckWindowsVersion
Push $R0
ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion
StrCmp $R0 "" 0 WINNT
; we are not NT.
Goto ERROR

WINNT:
StrCmp $R0 '3' ERROR
StrCmp $R0 '4' ERROR
StrCmp $R0 '5' ERROR
Goto DONE

ERROR:
MessageBox MB_OK|MB_ICONEXCLAMATION "Emerge Desktop Requires Windows XP or higher"
Abort

DONE:
Pop $R0
FunctionEnd

Function CloseCore
  Push $R1
  System::Call "user32::RegisterWindowMessage(t 'EmergeDispatch') isR1"
  Push $R0
LOOP1:
  FindWindow $R0 "emergeCoreClass"
  IntCmp $R0 0 +1 +2 +2
  FindWindow $R0 "EmergeDesktopCore"
  IntCmp $R0 0 +4
    SendMessage $R0 130 0 0
    Sleep 100
    Goto LOOP1
LOOP2:
  FindWindow $R0 "EmergeDesktopApplet"
  IntCmp $R0 0 +4
    SendMessage $R0 130 0 0
    Sleep 100
    Goto LOOP2
LOOP3:
  FindWindow $R0 "EmergeDesktopExplorer"
  IntCmp $R0 0 +4
    SendMessage $R0 130 0 0
    Sleep 100
    Goto LOOP3
LOOP4:
  FindWindow $R0 "EmergeDesktopMenuBuilder"
  IntCmp $R0 0 +4
    SendMessage $R0 130 0 0
    Sleep 100
    Goto LOOP4
  Pop $R0
  Pop $R1
FunctionEnd

Function un.CloseCore
  Push $R1
  System::Call "user32::RegisterWindowMessage(t 'EmergeDispatch') isR1"
  Push $R0
LOOP1:
  FindWindow $R0 "emergeCoreClass"
  IntCmp $R0 0 +1 +2 +2
  FindWindow $R0 "EmergeDesktopCore"
  IntCmp $R0 0 +5
  SetRebootFlag true
    SendMessage $R0 130 0 0
    Sleep 100
    Goto LOOP1
LOOP2:
  FindWindow $R0 "EmergeDesktopApplet"
  IntCmp $R0 0 +4
    SendMessage $R0 130 0 0
    Sleep 100
    Goto LOOP2
LOOP3:
  FindWindow $R0 "EmergeDesktopExplorer"
  IntCmp $R0 0 +4
    SendMessage $R0 130 0 0
    Sleep 100
    Goto LOOP3
LOOP4:
  FindWindow $R0 "EmergeDesktopMenuBuilder"
  IntCmp $R0 0 +4
    SendMessage $R0 130 0 0
    Sleep 100
    Goto LOOP4
  Pop $R0
  Pop $R1
FunctionEnd

Function nsDialogOptions
  !insertmacro MUI_HEADER_TEXT $(PAGE_TITLE) $(PAGE_SUBTITLE)

  nsDialogs::Create /NOUNLOAD 1018
  Pop $Dialog

  ${If} $Dialog == error
    Abort
  ${EndIf}

  ${NSD_CreateLabel} 0 10u 100% 20u "Emerge Desktop has the ability to replace the default windows desktop.  Would you like to replace the default windows desktop?"
  Pop $Label1

  ${NSD_CreateCheckBox} 0 40u 100% 12u "Set Emerge Desktop as the default shell for the current user"
  Pop $CheckBox1
  StrCmp ${EMERGERUNNING} "1" +1 +3
  ${NSD_Check} $CheckBox1
  EnableWindow $CheckBox1 0

  ${NSD_CreateLabel} 0 70u 100% 40u "Warning: Some anti-virus / firewall / anti-spyware programs interfere with emergeCore and emergeWorkspace.  It is recommended that they be added to your 'trusted' list before rebooting."
  Pop $Label2
  CreateFont $R2 "MS Shell Dlg" 10 700
  SendMessage $Label2 ${WM_SETFONT} $R2 0

  nsDialogs::Show
FunctionEnd

Function nsDialogOptionsLeave
  ${NSD_GetState} $CheckBox1 ${FIELD1}
FunctionEnd

Function un.nsDialogOptions
  nsDialogs::Create /NOUNLOAD 1018
  Pop $Dialog

  ${If} $Dialog == error
    Abort
  ${EndIf}

  ${NSD_CreateLabel} 0 5u 100% 25u "The uninstallation process will remove both the Emerge Desktop core as well as all the applets.  It will also restore the default Windows desktop."
  Pop $Label1

  ${NSD_CreateLabel} 0 35u 100% 25u "The uninstaller by default also removes all scripts, files (including configuration files) and themes.  If you would like to save your scripts, files or themes for later use, please check the appropriate box below:"
  Pop $Label3

  ${NSD_CreateCheckBox} 0 70u 100u 12u "Save Scripts"
  Pop $CheckBox2

  ${NSD_CreateCheckBox} 100u 70u 100u 12u "Save Files"
  Pop $CheckBox3

  ${NSD_CreateCheckBox} 200u 70u 100u 12u "Save Themes"
  Pop $CheckBox4

  nsDialogs::Show
FunctionEnd

Function un.nsDialogOptionsLeave
  ${NSD_GetState} $CheckBox1 ${FIELD1}
  ${NSD_GetState} $CheckBox2 ${FIELD2}
  ${NSD_GetState} $CheckBox3 ${FIELD3}
  ${NSD_GetState} $CheckBox4 ${FIELD4}
FunctionEnd

