/*!
  @file Applet.h
  @brief header for emergeLauncher
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

#ifndef __GUARD_fd579399_5236_44ff_bf78_55155d45ad47
#define __GUARD_fd579399_5236_44ff_bf78_55155d45ad47

#define UNICODE 1

// Defines required for tooltip
#undef _WIN32_IE
#define _WIN32_IE 0x0600

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

// Define required for menu animation
#undef WINVER
#define WINVER 0x0501

#define MOUSE_TIMER 0
#define MOUSE_POLL_TIME 250

#define ACTIVE_DELAY_TIME 500

#include <process.h>
#include <vector>
#include "../emergeBaseClasses/BaseApplet.h"
#include "Config.h"
#include "Item.h"
#include "Settings.h"

typedef std::map<DWORD, std::wstring> LiveFolderMap;

class Applet: public BaseApplet
{
private:
  std::tr1::shared_ptr<Config> pConfig;
  std::tr1::shared_ptr<Settings> pSettings;
  HWND activeWnd;
  int iconSize;
  void UpdateIcons();
  static LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
  static BOOL CALLBACK EnumFullScreenWindows(HWND hwnd, LPARAM lParam);
  static VOID CALLBACK ActiveTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
  static DWORD WINAPI LiveFolderThreadProc(LPVOID lpParameter);
  HANDLE liveFolderThread;
  LiveFolderMap liveFolderMap;

public:
  Applet(HINSTANCE hInstance);
  ~Applet();
  UINT Initialize();
  LRESULT DoSizing(HWND hwnd, UINT edge, LPRECT rect);
  LRESULT ItemMouseEvent(UINT message, LPARAM lParam);
  bool PaintItem(HDC hdc, size_t index, int x, int y, RECT rect);
  void ShowConfig();
  void CreateBrushes();
  void DestroyBrushes();
  void AppletUpdate();
  void UpdateTip(UINT index);
  void DeleteTip(UINT index);
  size_t GetIconCount();
  size_t GetVisibleIconCount();
  void ClearActive(UINT index);
  std::wstring GetFolderForThreadId(DWORD threadID);
  void InitLiveFolderMap();
};

#endif
