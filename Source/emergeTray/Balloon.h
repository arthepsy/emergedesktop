/*!
  @file Balloon.h
  @brief header for emergeTray
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

#ifndef __GUARD_e19a7b6f_522d_479e_acf5_24a2ea558786
#define __GUARD_e19a7b6f_522d_479e_acf5_24a2ea558786

#define UNICODE 1

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1 //suppress warnings about old versions of wcscpy, wcscat, etc.
#endif

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

// Define required for menu animation
#undef WINVER
#define WINVER 0x0501

#undef _WIN32_IE
#define _WIN32_IE 0x0501

#define OEMRESOURCE 1

#include "../emergeAppletEngine/emergeAppletEngine.h"
#include "../emergeGraphics/emergeGraphics.h"
#include "Settings.h"

#define BALLOON_TIMER_ID        1
#define START_BALLOON_TIMER_ID  2

#ifndef NIIF_USER
#define NIIF_USER 0x4
#endif

#ifndef NIIF_LARGE_ICON
#define NIIF_LARGE_ICON 0x20
#endif

#ifndef NIIF_RESPECT_QUIET_TIME
#define NIIF_RESPECT_QUIET_TIME 0x80
#endif

class TrayIcon;

class Balloon
{
public:
  Balloon(HINSTANCE hInstance, TrayIcon* pTrayIcon, Settings* pSettings);
  ~Balloon();
  bool Initialize();
  bool Show(POINT showPt);
  LRESULT Hide();
  LRESULT DoLButtonDown();
  bool SetInfo(WCHAR* info);
  bool SetInfoTitle(WCHAR* info);
  bool SetInfoFlags(DWORD infoFlags, HICON infoIcon);
  bool DrawAlphaBlend();
  BOOL SendMessage(LPARAM lParam);
  void SetIconVersion(UINT iconVersion);
  void SetCallbackMessage(UINT callbackMessage);
  void SetIconRect(RECT rect);
  bool ShowBalloon();
  void TimeoutBalloon();

private:
  static LRESULT CALLBACK BalloonProcedure (HWND, UINT, WPARAM, LPARAM);
  HINSTANCE mainInst;
  HWND balloonWnd;
  DWORD infoFlags;
  WCHAR info[TIP_SIZE];
  WCHAR infoTitle[TIP_SIZE];
  RECT titleRect, infoRect;
  HICON icon;
  Settings* pSettings;
  int iconHeight;
  int iconWidth;
  UINT trayIconID;
  UINT trayIconCallbackMessage;
  UINT trayIconVersion;
  RECT trayIconRect;
  HWND trayIconWnd;
  HANDLE showThread;
  DWORD showID;
  POINT showPt;
  static DWORD WINAPI ShowThreadProc(LPVOID lpParameter);
};

#endif
