//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2013  The Emerge Desktop Development Team
//
//  Emerge Desktop is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  Emerge Desktop is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//----  --------------------------------------------------------------------------------------------------------

#include "InternalCommandHandler.h"

void SetupDefaultEmergeInternalCommands()
{
  ELRegisterInternalCommand(TEXT("About"), CORE_ABOUT, (COMMANDHANDLERPROC)AboutCommandHandler);
  ELRegisterInternalCommand(TEXT("CoreAbout"), CORE_ABOUT, (COMMANDHANDLERPROC)AboutCommandHandler); //Alias for About
  ELRegisterInternalCommand(TEXT("Activate"), CORE_ACTIVATE, (COMMANDHANDLERPROC)ActivateCommandHandler);
  ELRegisterInternalCommand(TEXT("AliasEditor"), CORE_ALIAS, (COMMANDHANDLERPROC)AliasEditorCommandHandler);
  ELRegisterInternalCommand(TEXT("CoreSettings"), CORE_CONFIGURE, (COMMANDHANDLERPROC)CoreSettingsCommandHandler);
  ELRegisterInternalCommand(TEXT("Disconnect"), CORE_DISCONNECT, (COMMANDHANDLERPROC)DisconnectCommandHandler);
  ELRegisterInternalCommand(TEXT("EmptyBin"), CORE_EMPTYBIN, (COMMANDHANDLERPROC)EmptyBinCommandHandler);
  ELRegisterInternalCommand(TEXT("Halt"), CORE_HALT, (COMMANDHANDLERPROC)HaltCommandHandler);
  ELRegisterInternalCommand(TEXT("Help"), UNDEFINED_INTERNALCOMMAND_VALUE, (COMMANDHANDLERPROC)HelpCommandHandler);
  ELRegisterInternalCommand(TEXT("Hibernate"), CORE_HIBERNATE, (COMMANDHANDLERPROC)HibernateCommandHandler);
  ELRegisterInternalCommand(TEXT("Hide"), CORE_HIDE, (COMMANDHANDLERPROC)HideCommandHandler);
  ELRegisterInternalCommand(TEXT("Homepage"), UNDEFINED_INTERNALCOMMAND_VALUE, (COMMANDHANDLERPROC)HomepageCommandHandler);
  ELRegisterInternalCommand(TEXT("LaunchEditor"), CORE_LAUNCH, (COMMANDHANDLERPROC)LaunchEditorCommandHandler);
  ELRegisterInternalCommand(TEXT("CoreLaunchEditor"), CORE_LAUNCH, (COMMANDHANDLERPROC)LaunchEditorCommandHandler); //alias for LaunchEditor
  ELRegisterInternalCommand(TEXT("Lock"), CORE_LOCK, (COMMANDHANDLERPROC)LockCommandHandler);
  ELRegisterInternalCommand(TEXT("Logoff"), CORE_LOGOFF, (COMMANDHANDLERPROC)LogoffCommandHandler);
  ELRegisterInternalCommand(TEXT("MidDeskMenu"), CORE_MIDMENU, (COMMANDHANDLERPROC)MidDeskMenuCommandHandler);
  ELRegisterInternalCommand(TEXT("Quit"), CORE_QUIT, (COMMANDHANDLERPROC)QuitCommandHandler);
  ELRegisterInternalCommand(TEXT("Reboot"), CORE_REBOOT, (COMMANDHANDLERPROC)RebootCommandHandler);
  ELRegisterInternalCommand(TEXT("RightDeskMenu"), CORE_RIGHTMENU, (COMMANDHANDLERPROC)RightDeskMenuCommandHandler);
  ELRegisterInternalCommand(TEXT("Run"), CORE_RUN, (COMMANDHANDLERPROC)RunCommandHandler);
  ELRegisterInternalCommand(TEXT("ShellChanger"), CORE_SHELL, (COMMANDHANDLERPROC)ShellChangerCommandHandler);
  ELRegisterInternalCommand(TEXT("CoreShellChanger"), CORE_SHELL, (COMMANDHANDLERPROC)ShellChangerCommandHandler); //alias for ShellChanger
  ELRegisterInternalCommand(TEXT("Show"), CORE_SHOW, (COMMANDHANDLERPROC)ShowCommandHandler);
  ELRegisterInternalCommand(TEXT("ShowApplet"), CORE_SHOWAPPLET, (COMMANDHANDLERPROC)ShowAppletCommandHandler);
  ELRegisterInternalCommand(TEXT("ShowDesktop"), CORE_DESKTOP, (COMMANDHANDLERPROC)ShowDesktopCommandHandler);
  ELRegisterInternalCommand(TEXT("Suspend"), CORE_SUSPEND, (COMMANDHANDLERPROC)SuspendCommandHandler);
  ELRegisterInternalCommand(TEXT("Shutdown"), CORE_SHUTDOWN, (COMMANDHANDLERPROC)ShutdownCommandHandler);
  ELRegisterInternalCommand(TEXT("ThemeManager"), CORE_THEMESELECT, (COMMANDHANDLERPROC)ThemeManagerCommandHandler);
  ELRegisterInternalCommand(TEXT("CoreThemeSelector"), CORE_THEMESELECT, (COMMANDHANDLERPROC)ThemeManagerCommandHandler); //alias for ThemeManager
  ELRegisterInternalCommand(TEXT("Tutorial"), UNDEFINED_INTERNALCOMMAND_VALUE, (COMMANDHANDLERPROC)TutorialCommandHandler);
  ELRegisterInternalCommand(TEXT("VolumeUp"), UNDEFINED_INTERNALCOMMAND_VALUE, (COMMANDHANDLERPROC)VolumeUpCommandHandler);
  ELRegisterInternalCommand(TEXT("VolumeDown"), UNDEFINED_INTERNALCOMMAND_VALUE, (COMMANDHANDLERPROC)VolumeDownCommandHandler);
  ELRegisterInternalCommand(TEXT("VolumeMute"), UNDEFINED_INTERNALCOMMAND_VALUE, (COMMANDHANDLERPROC)VolumeMuteCommandHandler);
  ELRegisterInternalCommand(TEXT("VWM_1"), VWM_1, (COMMANDHANDLERPROC)VWM1CommandHandler);
  ELRegisterInternalCommand(TEXT("VWM_2"), VWM_2, (COMMANDHANDLERPROC)VWM2CommandHandler);
  ELRegisterInternalCommand(TEXT("VWM_3"), VWM_3, (COMMANDHANDLERPROC)VWM3CommandHandler);
  ELRegisterInternalCommand(TEXT("VWM_4"), VWM_4, (COMMANDHANDLERPROC)VWM4CommandHandler);
  ELRegisterInternalCommand(TEXT("VWM_5"), VWM_5, (COMMANDHANDLERPROC)VWM5CommandHandler);
  ELRegisterInternalCommand(TEXT("VWM_6"), VWM_6, (COMMANDHANDLERPROC)VWM6CommandHandler);
  ELRegisterInternalCommand(TEXT("VWM_7"), VWM_7, (COMMANDHANDLERPROC)VWM7CommandHandler);
  ELRegisterInternalCommand(TEXT("VWM_8"), VWM_8, (COMMANDHANDLERPROC)VWM8CommandHandler);
  ELRegisterInternalCommand(TEXT("VWM_9"), VWM_9, (COMMANDHANDLERPROC)VWM9CommandHandler);
  ELRegisterInternalCommand(TEXT("VWMDown"), VWM_DOWN, (COMMANDHANDLERPROC)VWMDownCommandHandler);
  ELRegisterInternalCommand(TEXT("VWMGather"), VWM_GATHER, (COMMANDHANDLERPROC)VWMGatherCommandHandler);
  ELRegisterInternalCommand(TEXT("VWMLeft"), VWM_LEFT, (COMMANDHANDLERPROC)VWMLeftCommandHandler);
  ELRegisterInternalCommand(TEXT("VWMRight"), VWM_RIGHT, (COMMANDHANDLERPROC)VWMRightCommandHandler);
  ELRegisterInternalCommand(TEXT("VWMUp"), VWM_UP, (COMMANDHANDLERPROC)VWMUpCommandHandler);
  ELRegisterInternalCommand(TEXT("VWMPrev"), VWM_PREV, (COMMANDHANDLERPROC)VWMPrevCommandHandler);
  ELRegisterInternalCommand(TEXT("VWMNext"), VWM_NEXT, (COMMANDHANDLERPROC)VWMNextCommandHandler);
  ELRegisterInternalCommand(TEXT("Welcome"), CORE_WELCOME, (COMMANDHANDLERPROC)WelcomeCommandHandler);
  ELRegisterInternalCommand(TEXT("WorkspaceSettings"), CORE_SETTINGS, (COMMANDHANDLERPROC)WorkspaceSettingsCommandHandler);
}

bool AboutCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_CORE, CORE_ABOUT);

  return true;
}

bool ActivateCommandHandler(std::vector<std::wstring> args)
{
  std::wstring application;

  if (args.size() == 0)
  {
    return false;
  }

  if (args.at(0).empty())
  {
    return false;
  }

  application = args.at(0);
  if (ELToLower(ELGetFileExtension(application)) != TEXT(".exe"))
  {
    application = application + TEXT(".exe");
  }

  if (ELPathIsRelative(application))
  {
    application = ELGetAbsolutePath(application);
  }

  if ((!ELIsAppletRunning(application)) && (ELFileExists(application)))
  {
    ELExecuteFileOrCommand(application);
    Sleep(500); //wait half a second for the applet to start
  }

  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_CORE, CORE_ACTIVATE, application.c_str());

  return true;
}

bool AliasEditorCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_CORE, CORE_ALIAS);

  return true;
}

bool CoreSettingsCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_CORE, CORE_CONFIGURE);

  return true;
}

bool DisconnectCommandHandler(std::vector<std::wstring> args)
{
  size_t counter;
  bool confirm = true;

  for (counter = 0; counter < args.size(); counter++)
  {
    if (ELToLower(args.at(counter)) == TEXT("/silent"))
    {
      confirm = false;
    }
  }

  ELExit(EMERGE_DISCONNECT, confirm);
  return true;
}

bool EmptyBinCommandHandler(std::vector<std::wstring> args)
{
  size_t counter;
  bool confirm = true;
  SHQUERYRBINFO binInfo;

  binInfo.cbSize = sizeof(binInfo);

  for (counter = 0; counter < args.size(); counter++)
  {
    if (ELToLower(args.at(counter)) == TEXT("/silent"))
    {
      confirm = false;
    }
  }

  DWORD emptyFlags = 0;
  if (!confirm)
  {
    emptyFlags = SHERB_NOCONFIRMATION | SHERB_NOPROGRESSUI | SHERB_NOSOUND;
  }

  if (SUCCEEDED(SHQueryRecycleBin(NULL, &binInfo)))
  {
    if (binInfo.i64NumItems > 0)
    {
      return (SHEmptyRecycleBin(GetDesktopWindow(), NULL, emptyFlags) == S_OK);
    }
    else
    {
      if (confirm)
      {
        ELMessageBox(GetDesktopWindow(), TEXT("Recycle Bin is empty."), TEXT("Emerge Desktop"), ELMB_OK | ELMB_ICONINFORMATION);
      }

      return true;
    }
  }

  return false;
}

bool HaltCommandHandler(std::vector<std::wstring> args)
{
  size_t counter;
  bool confirm = true;

  for (counter = 0; counter < args.size(); counter++)
  {
    if (ELToLower(args.at(counter)) == TEXT("/silent"))
    {
      confirm = false;
    }
  }

  ELExit(EMERGE_HALT, confirm);

  return true;
}

bool HelpCommandHandler(std::vector<std::wstring> args UNUSED)
{
  return ELExecuteFileOrCommand(TEXT("%AppletDir%\\Documentation\\Emerge Desktop.chm"));
}

bool HibernateCommandHandler(std::vector<std::wstring> args)
{
  size_t counter;
  bool confirm = true;

  for (counter = 0; counter < args.size(); counter++)
  {
    if (ELToLower(args.at(counter)) == TEXT("/silent"))
    {
      confirm = false;
    }
  }

  ELExit(EMERGE_HIBERNATE, confirm);

  return true;
}

bool HideCommandHandler(std::vector<std::wstring> args)
{
  if (args.size() == 0)
  {
    return false;
  }

  if (args.at(0).empty())
  {
    return false;
  }

  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_CORE, CORE_HIDE, args.at(0).c_str());

  return true;
}

bool HomepageCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELExecuteFileOrCommand(TEXT("http://emergedesktop.org"));

  return true;
}

bool LaunchEditorCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_CORE, CORE_LAUNCH);

  return true;
}

bool LockCommandHandler(std::vector<std::wstring> args UNUSED)
{
  return (LockWorkStation() == TRUE);
}

bool LogoffCommandHandler(std::vector<std::wstring> args)
{
  size_t counter;
  bool confirm = true;

  for (counter = 0; counter < args.size(); counter++)
  {
    if (ELToLower(args.at(counter)) == TEXT("/silent"))
    {
      confirm = false;
    }
  }

  ELExit(EMERGE_LOGOFF, confirm);
  return true;
}

bool MidDeskMenuCommandHandler(std::vector<std::wstring> args UNUSED)
{
  /// TODO (Chris#1#): Find better implementation that doesn't rely on finding the desktop window
  ELSwitchToThisWindow(FindWindow(TEXT("EmergeDesktopMenuBuilder"), NULL));/**< Needed to address keyboard focus issue with Launcher */
  ELDispatchCoreMessage(EMERGE_CORE, CORE_MIDMENU);

  return true;
}

bool QuitCommandHandler(std::vector<std::wstring> args)
{
  size_t counter;
  bool confirm = true;

  for (counter = 0; counter < args.size(); counter++)
  {
    if (ELToLower(args.at(counter)) == TEXT("/silent"))
    {
      confirm = false;
    }
  }

  ELQuit(confirm);

  return true;
}

bool RebootCommandHandler(std::vector<std::wstring> args)
{
  size_t counter;
  bool confirm = true;

  for (counter = 0; counter < args.size(); counter++)
  {
    if (ELToLower(args.at(counter)) == TEXT("/silent"))
    {
      confirm = false;
    }
  }

  ELExit(EMERGE_REBOOT, confirm);

  return true;
}

bool RightDeskMenuCommandHandler(std::vector<std::wstring> args UNUSED)
{
  /// TODO (Chris#1#): Find better implementation that doesn't rely on finding the desktop window
  ELSwitchToThisWindow(FindWindow(TEXT("EmergeDesktopMenuBuilder"), NULL));/**< Needed to address keyboard focus issue with Launcher */
  ELDispatchCoreMessage(EMERGE_CORE, CORE_RIGHTMENU);

  return true;
}

bool RunCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELDisplayRunDialog();

  return true;
}

bool ShellChangerCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_CORE, CORE_SHELL);

  return true;
}

bool ShowCommandHandler(std::vector<std::wstring> args)
{
  if (args.size() == 0)
  {
    return false;
  }

  if (args.at(0).empty())
  {
    return false;
  }

  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_CORE, CORE_SHOW, args.at(0).c_str());

  return true;
}

bool ShowDesktopCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_CORE, CORE_DESKTOP);

  return true;
}

bool ShowAppletCommandHandler(std::vector<std::wstring> args)
{
  if (args.size() == 0)
  {
    return false;
  }

  if (args.at(0).empty())
  {
    return false;
  }

  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_CORE, CORE_SHOWAPPLET, args.at(0).c_str());

  return true;
}

bool ShutdownCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELDisplayShutdownDialog(ELGetCoreWindow());

  return true;
}

bool SuspendCommandHandler(std::vector<std::wstring> args)
{
  size_t counter;
  bool confirm = true;

  for (counter = 0; counter < args.size(); counter++)
  {
    if (ELToLower(args.at(counter)) == TEXT("/silent"))
    {
      confirm = false;
    }
  }

  ELExit(EMERGE_SUSPEND, confirm);

  return true;
}

bool ThemeManagerCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_CORE, CORE_THEMESELECT);

  return true;
}

bool TutorialCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELExecuteFileOrCommand(TEXT("http://www.emergedesktop.org/?page_id=220"));

  return true;
}

bool VWM1CommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_1);

  return true;
}

bool VWM2CommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_2);

  return true;
}

bool VWM3CommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_3);

  return true;
}

bool VWM4CommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_4);

  return true;
}

bool VWM5CommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_5);

  return true;
}

bool VWM6CommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_6);

  return true;
}

bool VWM7CommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_7);

  return true;
}

bool VWM8CommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_8);

  return true;
}

bool VWM9CommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_9);

  return true;
}

bool VWMUpCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_UP);

  return true;
}

bool VWMDownCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_DOWN);

  return true;
}

bool VWMLeftCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_LEFT);

  return true;
}

bool VWMRightCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_RIGHT);

  return true;
}

bool VWMGatherCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_GATHER);

  return true;
}

bool VWMPrevCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_PREV);

  return true;
}

bool VWMNextCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_VWM, VWM_NEXT);

  return true;
}

bool VolumeDownCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELAdjustVolume(ELAV_VOLUMEDOWN);

  return true;
}

bool VolumeMuteCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELAdjustVolume(ELAV_MUTE);

  return true;
}

bool VolumeUpCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELAdjustVolume(ELAV_VOLUMEUP);

  return true;
}

bool WelcomeCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_CORE, CORE_WELCOME);

  return true;
}

bool WorkspaceSettingsCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_CORE, CORE_SETTINGS);

  return true;
}
