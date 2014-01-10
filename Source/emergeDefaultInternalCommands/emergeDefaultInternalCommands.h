/*!
  @file emergeDefaultInternalCommands.h
  @brief export header for the emergeDefaultInternalCommands plugin
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2013  The Emerge Desktop Development Team

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

#ifndef __GUARD_1538cb48_b84e_4a89_b598_5ef4f197c502
#define __GUARD_1538cb48_b84e_4a89_b598_5ef4f197c502

/*!
  @def UNICODE
  @brief Define to use UNICODE versions of functions
  */
#define UNICODE 1

#ifdef EMERGEDEFAULTINTERNALCOMMANDS_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#ifndef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif
#endif

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#include <windows.h>
#include <string>
#include <vector>
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeOSLib.h"
#include "../emergeLib/emergeUtilityLib.h"
#include "../emergeLib/emergeWindowLib.h"
#include "../emergeLib/emergeFileRegistryLib/EmergeInternalCommandInterface.h"

#ifdef __cplusplus
extern "C" {
#endif

DLL_EXPORT bool AboutCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool ActivateCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool AliasEditorCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool CoreSettingsCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool DisconnectCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool EmptyBinCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool HaltCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool HelpCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool HibernateCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool HideCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool HomepageCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool LaunchEditorCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool LockCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool LogoffCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool MidDeskMenuCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool QuitCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool RebootCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool RightDeskMenuCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool RunCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool ShellChangerCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool ShowCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool ShowAppletCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool ShowDesktopCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool ShutdownCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool SuspendCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool ThemeManagerCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool TutorialCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWM1CommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWM2CommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWM3CommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWM4CommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWM5CommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWM6CommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWM7CommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWM8CommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWM9CommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWMUpCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWMDownCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWMLeftCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWMRightCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWMGatherCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWMPrevCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VWMNextCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VolumeDownCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VolumeMuteCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool VolumeUpCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool WelcomeCommandHandler(std::vector<std::wstring> args);
DLL_EXPORT bool WorkspaceSettingsCommandHandler(std::vector<std::wstring> args);

#ifdef __cplusplus
}
#endif

#endif
