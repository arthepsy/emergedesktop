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

#ifndef __EC_LAUNCHEDITOR_H
#define __EC_LAUNCHEDITOR_H

#undef _WIN32_IE
#define _WIN32_IE	0x501

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#define UNICODE 1

#include <shlwapi.h>
#include <commctrl.h>
#include "resource.h"
#include <psapi.h>
#include <map>
#include <process.h>
#include <stdio.h>
#include "../emergeLib/emergeLib.h"

class LaunchEditor
{
public:
  LaunchEditor(HINSTANCE hInstance, HWND mainWnd);
  ~LaunchEditor();
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoLaunchCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  BOOL DoNotify(HWND hwndDlg, LPARAM lParam);
  bool UpdateLaunch(HWND hwndDlg);
  bool GetLaunchAppletName(int index, WCHAR *applet);
  std::wstring GetSelectedApplet();
  static INT_PTR CALLBACK LaunchDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
  bool CheckFields(HWND hwndDlg);
  bool PopulateList(HWND listWnd);
  bool DoLaunchAdd(HWND listWnd);
  bool DoLaunchSave(HWND listWnd);
  bool DoLaunchAbort(HWND listWnd);
  bool DoLaunchDelete(HWND listWnd);
  bool DoLaunchBrowse(HWND listWnd);
  bool DoLaunchStop(HWND listWnd, int index);
  bool DoLaunchStart(HWND listWnd, int index);
  BOOL DoMultiStart(HWND hwndDlg);
  BOOL DoMultiStop(HWND hwndDlg);
  BOOL DoMultiGather(HWND hwndDlg);
  BOOL DoMultiConfig(HWND hwndDlg);
  BOOL DoMultiInfo(HWND hwndDlg);
  bool DoLaunchMove(HWND listWnd, bool up);
  bool CheckSaveCount(HWND hwndDlg);
  void InsertListViewItem(HWND listWnd, int index, const WCHAR *item);
  BOOL DoRightClick(HWND hwndDlg, int index);
  BOOL PopulateFields(HWND hwndDlg, int index);
  WCHAR *GetLaunchItemState(WCHAR *launchItem);
  UINT saveCount, deleteCount;
  HINSTANCE hInstance;
  HWND mainWnd, toolWnd, dlgWnd;
  HICON addIcon, delIcon, upIcon, downIcon, browseIcon, saveIcon, abortIcon, startIcon, stopIcon;
  HICON infoIcon, gatherIcon, configIcon;
  static BOOL CALLBACK AppletCheck(HWND hwnd, LPARAM lParam);
  static BOOL CALLBACK GatherApplet(HWND hwnd, LPARAM lParam);
  std::wstring xmlFile;
  std::wstring selectedApplet;
  bool FindListSubItem(HWND listWnd, int subItem, WCHAR *searchString);
};

#endif

