/*!
  @file InternalCommandHandler.h
  @brief header for emergeCore
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2012  The Emerge Desktop Development Team

  @attention Emerge Desktop is free software; you can redistribute it and/or
  modify  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  @attention Emerge Desktop is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  @attention You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */

#ifndef __INTERNALCOMMANDHANDLER_H
#define __INTERNALCOMMANDHANDLER_H

#define UNICODE 1

#include <windows.h>
#include <string>
#include <vector>
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeOSLib.h"
#include "../emergeLib/emergeUtilityLib.h"
#include "../emergeLib/emergeWindowLib.h"

//Default Emerge internal command setup
void SetupDefaultEmergeInternalCommands();

bool AboutCommandHandler(std::vector<std::wstring> args);
bool ActivateCommandHandler(std::vector<std::wstring> args);
bool AliasEditorCommandHandler(std::vector<std::wstring> args);
bool CoreSettingsCommandHandler(std::vector<std::wstring> args);
bool DisconnectCommandHandler(std::vector<std::wstring> args);
bool EmptyBinCommandHandler(std::vector<std::wstring> args);
bool HaltCommandHandler(std::vector<std::wstring> args);
bool HelpCommandHandler(std::vector<std::wstring> args);
bool HibernateCommandHandler(std::vector<std::wstring> args);
bool HideCommandHandler(std::vector<std::wstring> args);
bool HomepageCommandHandler(std::vector<std::wstring> args);
bool LaunchEditorCommandHandler(std::vector<std::wstring> args);
bool LockCommandHandler(std::vector<std::wstring> args);
bool LogoffCommandHandler(std::vector<std::wstring> args);
bool MidDeskMenuCommandHandler(std::vector<std::wstring> args);
bool QuitCommandHandler(std::vector<std::wstring> args);
bool RebootCommandHandler(std::vector<std::wstring> args);
bool RightDeskMenuCommandHandler(std::vector<std::wstring> args);
bool RunCommandHandler(std::vector<std::wstring> args);
bool ShellChangerCommandHandler(std::vector<std::wstring> args);
bool ShowCommandHandler(std::vector<std::wstring> args);
bool ShowAppletCommandHandler(std::vector<std::wstring> args);
bool ShowDesktopCommandHandler(std::vector<std::wstring> args);
bool ShutdownCommandHandler(std::vector<std::wstring> args);
bool SuspendCommandHandler(std::vector<std::wstring> args);
bool ThemeManagerCommandHandler(std::vector<std::wstring> args);
bool TutorialCommandHandler(std::vector<std::wstring> args);
bool VWM1CommandHandler(std::vector<std::wstring> args);
bool VWM2CommandHandler(std::vector<std::wstring> args);
bool VWM3CommandHandler(std::vector<std::wstring> args);
bool VWM4CommandHandler(std::vector<std::wstring> args);
bool VWM5CommandHandler(std::vector<std::wstring> args);
bool VWM6CommandHandler(std::vector<std::wstring> args);
bool VWM7CommandHandler(std::vector<std::wstring> args);
bool VWM8CommandHandler(std::vector<std::wstring> args);
bool VWM9CommandHandler(std::vector<std::wstring> args);
bool VWMUpCommandHandler(std::vector<std::wstring> args);
bool VWMDownCommandHandler(std::vector<std::wstring> args);
bool VWMLeftCommandHandler(std::vector<std::wstring> args);
bool VWMRightCommandHandler(std::vector<std::wstring> args);
bool VWMGatherCommandHandler(std::vector<std::wstring> args);
bool VWMPrevCommandHandler(std::vector<std::wstring> args);
bool VWMNextCommandHandler(std::vector<std::wstring> args);
bool VolumeDownCommandHandler(std::vector<std::wstring> args);
bool VolumeMuteCommandHandler(std::vector<std::wstring> args);
bool VolumeUpCommandHandler(std::vector<std::wstring> args);
bool WelcomeCommandHandler(std::vector<std::wstring> args);
bool WorkspaceSettingsCommandHandler(std::vector<std::wstring> args);

#endif //__INTERNALCOMMANDHANDLER_H
