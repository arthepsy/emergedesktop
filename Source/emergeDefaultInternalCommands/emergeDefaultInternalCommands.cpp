#include "emergeDefaultInternalCommands.h"

struct ImplementCommandProvider :cppcomponents::implement_runtime_class<ImplementCommandProvider, CommandProvider_t>
{
ImplementCommandProvider(){}


std::vector<Command>GetEmergeInternalCommands()
{
  std::vector<Command> commandList;
  commandList.push_back(std::make_tuple(TEXT("About"), CORE_ABOUT, TEXT("AboutCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("CoreAbout"), CORE_ABOUT, TEXT("AboutCommandHandler"))); //Alias for About
  commandList.push_back(std::make_tuple(TEXT("Activate"), CORE_ACTIVATE, TEXT("ActivateCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("AliasEditor"), CORE_ALIAS, TEXT("AliasEditorCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("CoreSettings"), CORE_CONFIGURE, TEXT("CoreSettingsCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("Disconnect"), CORE_DISCONNECT, TEXT("DisconnectCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("EmptyBin"), CORE_EMPTYBIN, TEXT("EmptyBinCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("Halt"), CORE_HALT, TEXT("HaltCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("Help"), UNDEFINED_INTERNALCOMMAND_VALUE, TEXT("HelpCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("Hibernate"), CORE_HIBERNATE, TEXT("HibernateCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("Hide"), CORE_HIDE, TEXT("HideCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("Homepage"), UNDEFINED_INTERNALCOMMAND_VALUE, TEXT("HomepageCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("LaunchEditor"), CORE_LAUNCH, TEXT("LaunchEditorCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("CoreLaunchEditor"), CORE_LAUNCH, TEXT("LaunchEditorCommandHandler"))); //alias for LaunchEditor
  commandList.push_back(std::make_tuple(TEXT("Lock"), CORE_LOCK, TEXT("LockCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("Logoff"), CORE_LOGOFF, TEXT("LogoffCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("MidDeskMenu"), CORE_MIDMENU, TEXT("MidDeskMenuCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("Quit"), CORE_QUIT, TEXT("QuitCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("Reboot"), CORE_REBOOT, TEXT("RebootCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("RightDeskMenu"), CORE_RIGHTMENU, TEXT("RightDeskMenuCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("Run"), CORE_RUN, TEXT("RunCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("ShellChanger"), CORE_SHELL, TEXT("ShellChangerCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("CoreShellChanger"), CORE_SHELL, TEXT("ShellChangerCommandHandler"))); //alias for ShellChanger
  commandList.push_back(std::make_tuple(TEXT("Show"), CORE_SHOW, TEXT("ShowCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("ShowApplet"), CORE_SHOWAPPLET, TEXT("ShowAppletCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("ShowDesktop"), CORE_DESKTOP, TEXT("ShowDesktopCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("Suspend"), CORE_SUSPEND, TEXT("SuspendCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("Shutdown"), CORE_SHUTDOWN, TEXT("ShutdownCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("ThemeManager"), CORE_THEMESELECT, TEXT("ThemeManagerCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("CoreThemeSelector"), CORE_THEMESELECT, TEXT("ThemeManagerCommandHandler"))); //alias for ThemeManager
  commandList.push_back(std::make_tuple(TEXT("Tutorial"), UNDEFINED_INTERNALCOMMAND_VALUE, TEXT("TutorialCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VolumeUp"), UNDEFINED_INTERNALCOMMAND_VALUE, TEXT("VolumeUpCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VolumeDown"), UNDEFINED_INTERNALCOMMAND_VALUE, TEXT("VolumeDownCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VolumeMute"), UNDEFINED_INTERNALCOMMAND_VALUE, TEXT("VolumeMuteCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWM_1"), VWM_1, TEXT("VWM1CommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWM_2"), VWM_2, TEXT("VWM2CommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWM_3"), VWM_3, TEXT("VWM3CommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWM_4"), VWM_4, TEXT("VWM4CommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWM_5"), VWM_5, TEXT("VWM5CommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWM_6"), VWM_6, TEXT("VWM6CommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWM_7"), VWM_7, TEXT("VWM7CommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWM_8"), VWM_8, TEXT("VWM8CommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWM_9"), VWM_9, TEXT("VWM9CommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWMDown"), VWM_DOWN, TEXT("VWMDownCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWMGather"), VWM_GATHER, TEXT("VWMGatherCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWMLeft"), VWM_LEFT, TEXT("VWMLeftCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWMRight"), VWM_RIGHT, TEXT("VWMRightCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWMUp"), VWM_UP, TEXT("VWMUpCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWMPrev"), VWM_PREV, TEXT("VWMPrevCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("VWMNext"), VWM_NEXT, TEXT("VWMNextCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("Welcome"), CORE_WELCOME, TEXT("WelcomeCommandHandler")));
  commandList.push_back(std::make_tuple(TEXT("WorkspaceSettings"), CORE_SETTINGS, TEXT("WorkspaceSettingsCommandHandler")));

  return commandList;
}


};

CPPCOMPONENTS_REGISTER(ImplementCommandProvider)
CPPCOMPONENTS_DEFINE_FACTORY()


bool AboutCommandHandler(std::vector<std::wstring> args UNUSED)
{
  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_CORE, CORE_ABOUT);

  return true;
}

bool ActivateCommandHandler(std::vector<std::wstring> args)
{
  std::wstring applicationPath, appletName;

  if (args.size() == 0)
  {
    return false;
  }

  if (args.at(0).empty())
  {
    return false;
  }

  appletName = args.at(0);

  applicationPath = args.at(0);
  if (ELToLower(ELGetFileExtension(applicationPath)) != TEXT(".exe"))
  {
    applicationPath = applicationPath + TEXT(".exe");
  }

  if (ELPathIsRelative(applicationPath))
  {
    applicationPath = ELGetAbsolutePath(applicationPath);
  }

  if ((!ELIsAppletRunning(applicationPath)) && (ELFileExists(applicationPath)))
  {
    ELExecuteFileOrCommand(applicationPath);
    Sleep(500); //wait half a second for the applet to start
  }

  ELSwitchToThisWindow(ELGetCoreWindow());
  ELDispatchCoreMessage(EMERGE_CORE, CORE_ACTIVATE, appletName);

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
  ELExecuteFileOrCommand(TEXT("https://github.com/arthepsy/emergedesktop/wiki"));

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
  std::wstring instanceName = TEXT("");

  ELSwitchToThisWindow(ELGetCoreWindow());

  if ((args.size() > 0) && (!args.at(0).empty()))
  {
    instanceName = args.at(0);
  }

  ELDispatchCoreMessage(EMERGE_CORE, CORE_SHOWAPPLET, instanceName);

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
  ELExecuteFileOrCommand(TEXT("https://github.com/arthepsy/emergedesktop/wiki/The-Basics"));

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
