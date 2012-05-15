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

#ifndef __ECM_COMMAND_H
#define __ECM_COMMAND_H

#undef _WIN32_IE
#define _WIN32_IE 0x0600 // Enables all the needed define's in ShellAPI.h

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#define MOUSE_TIMER 0
#define MOUSE_POLL_TIME 250

#include "Settings.h"
#include "CustomSource.h"
#include "CommandSource.h"
#include "../emergeGraphics/emergeGraphics.h"
#include <windows.h>
#include <time.h>
#include <commctrl.h>
#include <process.h>
#include <shlobj.h>
#include <shldisp.h>

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#define MAX_RUN_STRING 1024
#define FOCUS_TIMER 1

#ifndef SHACF_DEFAULT
#define SHACF_DEFAULT 0x00
#endif

#ifndef ACO_UPDOWNKEYDROPSLIST
#define ACO_UPDOWNKEYDROPSLIST 0x20
#endif

#ifndef ACO_AUTOAPPEND
#define ACO_AUTOAPPEND 0x02
#endif

#ifndef ACO_USETAB
#define ACO_USETAB 0x10
#endif

class Command
{
public:
  Command(HWND mainWnd, HINSTANCE hInstance, std::tr1::shared_ptr<Settings> pSettings);
  ~Command();
  bool Init();
  void UpdateGUI(GUIINFO guiInfo);
  HWND GetCommandWnd();
  WNDPROC GetWndProc();
  void UpdateEdit(GUIINFO, int width, int height);
  void ShowTextWindow();
  WCHAR *GetZPosition();
  LRESULT DoSysCommand(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  void Show();
  void AddElement(WCHAR *element);
  void SetHidden(bool appletHidden);
  void ShowAppletWindow();

private:
  bool appletHidden;
  std::tr1::shared_ptr<Settings> pSettings;
  IAutoComplete *pac;
  IAutoComplete2 *pac2;
  IObjMgr *pom;
  IUnknown *fileSource;
  IEnumString *historySource, *commandSource;
  CustomSource *history;
  CommandSource *command;
  WCHAR fontName[MAX_RUN_STRING];
  int  dragBorder;
  HWND hText, commandWnd, mainWnd;
  HFONT textFont;
  WNDPROC wpOld;
  HINSTANCE mainInst;
  bool registered;
  static LRESULT CALLBACK CommandProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK EditProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif

