//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2011  The Emerge Desktop Development Team
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

#ifndef __ETR_APPLET_H
#define __ETR_APPLET_H

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

// Define required for menu animation
#undef WINVER
#define WINVER 0x0501

/**< Required to support NIF_INFO */
#undef _WIN32_IE
#define _WIN32_IE 0x0700

#define MOUSE_TIMER 0
#define MOUSE_POLL_TIME 250

#define APPBAR_MESSAGE          0
#define TRAY_MESSAGE            1
#define ICONIDENTIFIER_MESSAGE  3

#include <vector>
#include <process.h>
#include "AppBar.h"
#include <docobj.h>
#include <shlguid.h>
#include "../emergeBaseClasses/BaseApplet.h"
#include "Settings.h"
#include "Config.h"
#include "TrayIcon.h"

#ifndef NIN_POPUPOPEN
#define NIN_POPUPOPEN WM_USER+6
#endif

#ifndef NIN_POPUPCLOSE
#define NIN_POPUPCLOSE WM_USER+7
#endif

#ifndef NIF_SHOWTIP
#define NIF_SHOWTIP 0x80
#endif

#ifndef ABM_SETSTATE
#define ABM_SETSTATE  10
#endif

//====================
// this is the versions of NOTIFYICONDATA
typedef struct
{
  DWORD cbSize;
  HWND hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  CHAR szTip[64];
}
NID_4A;

typedef struct
{
  DWORD cbSize;
  HWND hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  WCHAR szTip[64];
}
NID_4W;

typedef struct
{
  DWORD cbSize;
  HWND hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  CHAR szTip[128];
  DWORD dwState;
  DWORD dwStateMask;
  CHAR szInfo[256];
  union
  {
    UINT uTimeout;
    UINT uVersion;
  } DUMMYUNIONNAME;
  CHAR szInfoTitle[64];
  DWORD dwInfoFlags;
}
NID_5A;

typedef struct
{
  DWORD cbSize;
  HWND hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  WCHAR szTip[128];
  DWORD dwState;
  DWORD dwStateMask;
  WCHAR szInfo[256];
  union
  {
    UINT uTimeout;
    UINT uVersion;
  } DUMMYUNIONNAME;
  WCHAR szInfoTitle[64];
  DWORD dwInfoFlags;
}
NID_5W;

typedef struct
{
  DWORD cbSize;
  HWND hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  CHAR szTip[128];
  DWORD dwState;
  DWORD dwStateMask;
  CHAR szInfo[256];
  union
  {
    UINT uTimeout;
    UINT uVersion;
  } DUMMYUNIONNAME;
  CHAR szInfoTitle[64];
  DWORD dwInfoFlags;
  GUID guidItem;
  HICON32 hBallonIcon;
}
NID_7A;

typedef struct
{
  DWORD cbSize;
  HWND hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  WCHAR szTip[128];
  DWORD dwState;
  DWORD dwStateMask;
  WCHAR szInfo[256];
  union
  {
    UINT uTimeout;
    UINT uVersion;
  } DUMMYUNIONNAME;
  WCHAR szInfoTitle[64];
  DWORD dwInfoFlags;
  GUID guidItem;
  HICON32 hBallonIcon;
}
NID_7W;

typedef struct
{
  DWORD cbSize;
  HWND hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  CHAR szTip[128];
  DWORD dwState;
  DWORD dwStateMask;
  CHAR szInfo[256];
  union
  {
    UINT uTimeout;
    UINT uVersion;
  } DUMMYUNIONNAME;
  CHAR szInfoTitle[64];
  DWORD dwInfoFlags;
  GUID guidItem;
}
NID_6A;

typedef struct
{
  DWORD cbSize;
  HWND hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  WCHAR szTip[128];
  DWORD dwState;
  DWORD dwStateMask;
  WCHAR szInfo[256];
  union
  {
    UINT uTimeout;
    UINT uVersion;
  } DUMMYUNIONNAME;
  WCHAR szInfoTitle[64];
  DWORD dwInfoFlags;
  GUID guidItem;
}
NID_6W;

typedef struct
{
  DWORD cbSize;
  HWND32 hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  CHAR szTip[64];
}
NID_4A_WOW32;

typedef struct
{
  DWORD cbSize;
  HWND32 hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  WCHAR szTip[64];
}
NID_4W_WOW32;

typedef struct
{
  DWORD cbSize;
  HWND32 hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  CHAR szTip[128];
  DWORD dwState;
  DWORD dwStateMask;
  CHAR szInfo[256];
  union
  {
    UINT uTimeout;
    UINT uVersion;
  } DUMMYUNIONNAME;
  CHAR szInfoTitle[64];
  DWORD dwInfoFlags;
}
NID_5A_WOW32;

typedef struct
{
  DWORD cbSize;
  HWND32 hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  WCHAR szTip[128];
  DWORD dwState;
  DWORD dwStateMask;
  WCHAR szInfo[256];
  union
  {
    UINT uTimeout;
    UINT uVersion;
  } DUMMYUNIONNAME;
  WCHAR szInfoTitle[64];
  DWORD dwInfoFlags;
}
NID_5W_WOW32;

typedef struct
{
  DWORD cbSize;
  HWND32 hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  CHAR szTip[128];
  DWORD dwState;
  DWORD dwStateMask;
  CHAR szInfo[256];
  union
  {
    UINT uTimeout;
    UINT uVersion;
  } DUMMYUNIONNAME;
  CHAR szInfoTitle[64];
  DWORD dwInfoFlags;
  GUID guidItem;
  HICON32 hBallonIcon;
}
NID_7A_WOW32;

typedef struct
{
  DWORD cbSize;
  HWND32 hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  WCHAR szTip[128];
  DWORD dwState;
  DWORD dwStateMask;
  WCHAR szInfo[256];
  union
  {
    UINT uTimeout;
    UINT uVersion;
  } DUMMYUNIONNAME;
  WCHAR szInfoTitle[64];
  DWORD dwInfoFlags;
  GUID guidItem;
  HICON32 hBallonIcon;
}
NID_7W_WOW32;

typedef struct
{
  DWORD cbSize;
  HWND32 hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  CHAR szTip[128];
  DWORD dwState;
  DWORD dwStateMask;
  CHAR szInfo[256];
  union
  {
    UINT uTimeout;
    UINT uVersion;
  } DUMMYUNIONNAME;
  CHAR szInfoTitle[64];
  DWORD dwInfoFlags;
  GUID guidItem;
}
NID_6A_WOW32;

typedef struct
{
  DWORD cbSize;
  HWND32 hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
  WCHAR szTip[128];
  DWORD dwState;
  DWORD dwStateMask;
  WCHAR szInfo[256];
  union
  {
    UINT uTimeout;
    UINT uVersion;
  } DUMMYUNIONNAME;
  WCHAR szInfoTitle[64];
  DWORD dwInfoFlags;
  GUID guidItem;
}
NID_6W_WOW32;

typedef struct
{
  DWORD cbSize;
  HWND32 hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
}
NID_XX_WOW32;

typedef struct
{
  DWORD cbSize;
  HWND hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON32 hIcon;
}
NID_XX;

//====================
// Used to get the icon data
typedef struct SHELLTRAYDATA
{
  DWORD dwUnknown;
  DWORD dwMessage;
  NID_XX iconData;
}
SHELLTRAYDATA, *PSHELLTRAYDATA;

typedef struct SHELLTRAYDATAWOW32
{
  DWORD dwUnknown;
  DWORD dwMessage;
  NID_XX_WOW32 iconData;
}
SHELLTRAYDATAWOW32, *PSHELLTRAYDATAWOW32;

// In SDK 7.0A the structure NOTIFYICONIDENTIFIER is defined in ShellAPI.h
#ifndef NTDDI_WIN7
typedef struct _NOTIFYICONIDENTIFIER
{
  DWORD cbSize;
  HWND hWnd;
  UINT uID;
  GUID guidItem;
} NOTIFYICONIDENTIFIER, *PNOTIFYICONIDENTIFIER;
#endif

typedef struct ICONIDENTIFIERDATA
{
  DWORD dwMagic;
  DWORD dwMessage;
  NOTIFYICONIDENTIFIER niid;
}
ICONIDENTIFIERDATA, *PICONIDENTIFIERDATA;

class Applet: public BaseApplet
{
private:
  std::tr1::shared_ptr<Settings> pSettings;
  std::vector< std::tr1::shared_ptr<AppBar> > barList;
  std::vector< std::tr1::shared_ptr<TrayIcon> > trayIconList;
  std::vector<IOleCommandTarget*> ssoIconList;
  bool movesizeinprogress;
  HWND trayWnd, notifyWnd;
  bool baseClassRegistered, trayClassRegistered, notifyClassRegistered;
  RECT HoverRect;
  void UpdateIcons();
  static LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
  static LRESULT CALLBACK TrayProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  bool IsIconVisible(TrayIcon *pTrayIcon);
  TrayIcon *activeIcon;
  bool SetAutoHideEdge(UINT edge);
  bool ClearAutoHideEdge(UINT edge);
  bool autoHideLeft, autoHideRight, autoHideTop, autoHideBottom;

public:
  Applet(HINSTANCE hInstance);
  ~Applet();
  UINT Initialize();
  TrayIcon *GetTrayIconListItem(UINT index);
  size_t GetTrayIconListSize();
  LRESULT DoTimer(UINT timerID);
  LRESULT DoSetCursor();
  LRESULT MySize();
  LRESULT MyMove();
  LRESULT DoSizing(HWND hwnd, UINT edge, LPRECT rect);
  LRESULT DoEnterSizeMove(HWND hwnd);
  void AdjustIcons();
  void AppletUpdate();
  void UnloadSSO();
  void LoadSSO();
  void CleanTray();
  void ShowConfig();
  TrayIcon* FindTrayIcon(HWND hwnd, UINT uID);
  AppBar* FindAppBar(APPBARDATA abd);
  bool RemoveAppBar(AppBar *pAppBar);
  bool RemoveTrayIconListItem(TrayIcon *pTrayIcon);
  LRESULT RemoveTrayIcon(HWND hwnd, UINT uID);
  LRESULT ModifyTrayIcon(HWND hwnd, UINT uID, UINT uFlags, UINT uCallbackMessage,
                         HICON icon, LPTSTR newTip, LPTSTR newInfo,
                         LPTSTR newInfoTitle, DWORD newInfoFlags, bool hidden,
                         bool shared);
  LRESULT SetTrayIconVersion(HWND hwnd, UINT uID, UINT iconVersion);
  LRESULT AddTrayIcon(HWND hwnd, UINT uID, UINT uFlags, UINT uCallbackMessage,
                      HICON icon, LPTSTR szTip, LPTSTR szInfo,
                      LPTSTR szInfoTitle, DWORD dwInfoFlags, bool hidden,
                      bool shared);
  bool TrayMouseEvent(UINT message, LPARAM lParam);
  LRESULT TrayIconEvent(COPYDATASTRUCT *cpData);
  LRESULT AppBarEvent(COPYDATASTRUCT *cpData);
  LRESULT IconIdentifierEvent(COPYDATASTRUCT *cpData);
  void ShowHiddenIcons(bool cmd, bool force);
  void SortIcons();
  bool PaintItem(HDC hdc, UINT index, int x, int y, RECT rect);
  size_t GetVisibleIconCount();
  size_t GetIconCount();
};

#endif
