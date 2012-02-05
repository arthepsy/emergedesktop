//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2012  The Emerge Desktop Development Team
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
//---

#ifndef __ETR_TRAYICON_H
#define __ETR_TRAYICON_H

// Defines required for tooltip
#undef _WIN32_IE
#define _WIN32_IE 0x0600

#ifndef NIN_BALLOONSHOW
#define NIN_BALLOONSHOW WM_USER+2
#endif

#ifndef NIN_BALLOONHIDE
#define NIN_BALLOONHIDE WM_USER+3
#endif

#ifndef NIN_BALLOONTIMEOUT
#define NIN_BALLOONTIMEOUT WM_USER+4
#endif

#ifndef NIN_BALLOONUSERCLICK
#define NIN_BALLOONUSERCLICK WM_USER+5
#endif

#ifndef NOTIFYICON_VERSION_4
#define NOTIFYICON_VERSION_4  4
#endif

//-----
//
// Note: for MSVC users, put the AggressiveOptimize.h header file (available from
// http://www.nopcode.com) in your \include directory.  It cuts down on executable
// filesize.
//
//-----
#include "../emergeLib/emergeLib.h"
#include "../emergeGraphics/emergeGraphics.h"
#include "Balloon.h"
#include <commctrl.h>

// Define icon and tip settings
#define ICON_SIZE 16
#define TIP_SIZE 256

//====================
// The TrayIcon Class
class TrayIcon
{
public:
  TrayIcon(HINSTANCE appInstance, HWND wnd, UINT id, HWND mainWnd, HWND toolWnd, Settings *pSettings);
  ~TrayIcon();

  void UpdateTip();
  void DeleteTip();
  void DeleteBalloon();
  HWND GetWnd();
  HICON GetIcon();
  UINT GetID();
  UINT GetCallback();
  WCHAR *GetTip();
  UINT GetFlags();
  RECT *GetRect();
  bool GetHidden();
  bool GetShared();
  UINT GetIconVersion();
  void SetIconVersion(UINT iconVersion);
  bool SetIcon(HICON icon);
  bool SetCallback(UINT callbackMessage);
  bool SetTip(WCHAR *tip);
  void SetFlags(UINT flags);
  void SetRect(RECT rect);
  void SetHidden(bool hidden);
  void SetShared(bool shared);
  void CreateNewIcon(BYTE foregroundAlpha, BYTE backgroundAlpha);
  void UpdateIcon();
  BOOL SendMessage(LPARAM lParam);
  void ShowBalloon(WCHAR *infoTitle, WCHAR *info, DWORD infoFlags, HICON icon);
  void HideBalloon();

private:
  HWND wnd, mainWnd, toolWnd;
  HINSTANCE appInstance;
  HICON origIcon, newIcon;
  UINT id;
  UINT callbackMessage;
  UINT iconVersion;
  WCHAR tip[TIP_SIZE];
  UINT flags;
  RECT rect;
  bool hidden;
  bool shared;
  bool convertIcon;
  std::tr1::shared_ptr<Balloon> pBalloon;
  Settings *pSettings;
  bool lbuttonDown;

  // Holds refrence to original icon handler
  // (since we do not create this icon we should not destroy it either)
  HICON origIconSource;
};

//====================

#endif
