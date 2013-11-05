/*!
  @file emergeLib.h
  @brief export header for emergeLib
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

#ifndef __EMERGECORELIB_H
#define __EMERGECORELIB_H

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#ifdef EMERGELIB_EXPORTS
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
#include <psapi.h>
#include <string>

typedef struct _NOTIFYINFO
{
  DWORD Type;
  DWORD Message;
  WCHAR InstanceName[MAX_PATH]; /*!< Path to the executable defined by the shortcut */
}
NOTIFYINFO, *LPNOTIFYINFO;

typedef struct _THEMEINFO
{
  std::wstring theme;
  std::wstring path;
  std::wstring themePath;
  std::wstring userPath;
}
THEMEINFO, *LPTHEMEINFO;

/*!
  @struct _VERSIONINFO
  @brief structure that contains applet version information
  */
typedef struct _VERSIONINFO
{
  WCHAR Version[MAX_PATH];
  WCHAR Author[MAX_PATH];
  WCHAR Description[MAX_PATH];
}
VERSIONINFO, *LPVERSIONINFO;

typedef enum _PORTABLEMODE
{
  PM_INSTALLED = 0,
  PM_PORTABLE,
  PM_CUSTOM
}
PORTABLEMODE;

typedef enum _MESSAGETYPE
{
  EMERGE_CORE = 1,
  EMERGE_VWM = 2
} MESSAGETYPE;

// EMERGE_CORE Messages
typedef enum _COREMESSAGES {
  CORE_QUIT = 100,
  CORE_SHOW,
  CORE_HIDE,
  CORE_LAUNCH,
  CORE_SETTINGS,
  CORE_MENU,
  CORE_SHELL,
  CORE_RIGHTMENU,
  CORE_MIDMENU,
  CORE_RUN,
  CORE_SHUTDOWN,
  CORE_EMPTYBIN,
  CORE_LOGOFF,
  CORE_REBOOT,
  CORE_HALT,
  CORE_SUSPEND,
  CORE_HIBERNATE,
  CORE_LOCK,
  CORE_DESKTOP,
  CORE_REFRESH,
  CORE_DISCONNECT,
  CORE_ABOUT,
  CORE_RECONFIGURE,
  CORE_THEMESELECT,
  CORE_WRITESETTINGS,
  CORE_LEFTMENU,
  CORE_REPOSITION,
  CORE_CONFIGURE,
  CORE_ALIAS,
  CORE_SHOWCONFIG,
  CORE_SHOWAPPLET,
  CORE_ACTIVATE,
  CORE_WELCOME,
  CORE_FULLSTART,
  CORE_FULLSTOP
} COREMESSAGES;

// EMERGE_VWM Messages
typedef enum _VWMMESSAGES
{
  VWM_UP = 200,
  VWM_DOWN,
  VWM_LEFT,
  VWM_RIGHT,
  VWM_GATHER,
  VWM_PREV,
  VWM_NEXT,
  VWM_1 = 211,
  VWM_2,
  VWM_3,
  VWM_4,
  VWM_5,
  VWM_6,
  VWM_7,
  VWM_8,
  VWM_9
} VWMMESSAGES;

typedef enum _EXPLORERMESSAGES
{
  EXPLORER_ENABLE = 1,
  EXPLORER_SHOW
}
EXPLORERMESSAGES;

typedef enum _VOLUMEFLAGS
{
  ELAV_MUTE = 0,
  ELAV_VOLUMEUP,
  ELAV_VOLUMEDOWN
}
VOLUMEFLAGS;

static const UINT EMERGE_REGISTER = RegisterWindowMessage(TEXT("EmergeRegister"));
static const UINT EMERGE_UNREGISTER = RegisterWindowMessage(TEXT("EmergeUnregister"));
static const UINT EMERGE_NOTIFY = RegisterWindowMessage(TEXT("EmergeNotify"));
static const UINT EMERGE_DISPATCH = RegisterWindowMessage(TEXT("EmergeDispatch"));
static const UINT EMERGE_MESSAGE = RegisterWindowMessage(TEXT("EmergeMessage"));

//Environment.h
DLL_EXPORT std::wstring ELExpandVars(std::wstring value);
DLL_EXPORT std::wstring ELUnExpandVars(std::wstring value);

DLL_EXPORT std::wstring ELGetCurrentPath();
DLL_EXPORT std::wstring ELGetUserDataPath();

DLL_EXPORT PORTABLEMODE ELGetPortableMode();

DLL_EXPORT bool ELIsWow64();

DLL_EXPORT void ELGetThemeInfo(LPTHEMEINFO themeInfo);

DLL_EXPORT bool ELSetEmergeVars();
DLL_EXPORT void ELSetEnvironmentVars(bool showErrors);
DLL_EXPORT void ELClearEmergeVars();

DLL_EXPORT bool ELIsExplorerShell();
DLL_EXPORT bool ELIsEmergeShell();

//MiscOps.h
DLL_EXPORT void ELDispatchCoreMessage(DWORD type, DWORD message, std::wstring instanceName = NULL);

DLL_EXPORT bool ELAppletVersionInfo(HWND appWnd, LPVERSIONINFO versionInfo);
DLL_EXPORT bool ELAppletFileVersion(std::wstring applet, LPVERSIONINFO versionInfo);

DLL_EXPORT bool ELIsAppletRunning(std::wstring applet);

DLL_EXPORT std::wstring ELGetThemeName();
DLL_EXPORT bool ELSetTheme(std::wstring theme);
DLL_EXPORT bool ELIsModifiedTheme(std::wstring theme);
DLL_EXPORT bool ELSetModifiedTheme(std::wstring theme);
DLL_EXPORT std::wstring ELStripModifiedTheme(std::wstring theme);

DLL_EXPORT bool ELAdjustVolume(VOLUMEFLAGS command);
DLL_EXPORT bool ELPlaySound(std::wstring sound);

#endif //__EMERGECORELIB_H
