/*!
  @file LaunchEditor.h
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

#ifndef __GUARD_ed5f7e08_c054_4513_9f5c_7579a04de7de
#define __GUARD_ed5f7e08_c054_4513_9f5c_7579a04de7de

#define UNICODE 1

#undef _WIN32_IE
#define _WIN32_IE	0x501

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#define MAX_LINE_LENGTH 4096

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include <windows.h>
#include <commctrl.h>
#include <psapi.h>
#include <process.h>
#include <shlwapi.h>
#include <stdio.h>
#include <map>
#include <string>
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeOSLib.h"
#include "../emergeLib/emergeUtilityLib.h"
#include "../emergeLib/emergeWindowLib.h"
#include "../tinyxml/tinyxml.h"
#include "resource.h"

class LaunchEditor
{
public:
  LaunchEditor(HINSTANCE hInstance, HWND mainWnd);
  ~LaunchEditor();
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoLaunchCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  BOOL DoNotify(HWND hwndDlg, LPARAM lParam);
  bool UpdateLaunch(HWND hwndDlg);
  bool GetLaunchAppletName(int index, WCHAR* applet);
  std::wstring GetSelectedApplet();
  static INT_PTR CALLBACK LaunchDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
  bool CheckFields(HWND hwndDlg);
  bool PopulateList(HWND listWnd);
  bool DoLaunchAddEdit(HWND listWnd);
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
  void InsertListViewItem(HWND listWnd, int index, const WCHAR* item);
  BOOL DoRightClick(HWND hwndDlg, int index);
  BOOL PopulateFields(HWND hwndDlg, int index);
  WCHAR* GetLaunchItemState(WCHAR* launchItem);
  UINT saveCount, deleteCount;
  HINSTANCE hInstance;
  HWND mainWnd, toolWnd, dlgWnd;
  HICON addIcon, delIcon, upIcon, downIcon, browseIcon, saveIcon, abortIcon, startIcon, stopIcon;
  HICON infoIcon, gatherIcon, configIcon, editIcon;
  static BOOL CALLBACK AppletCheck(HWND hwnd, LPARAM lParam);
  static BOOL CALLBACK GatherApplet(HWND hwnd, LPARAM lParam);
  std::wstring xmlFile;
  std::wstring selectedApplet;
  bool FindListSubItem(HWND listWnd, int subItem, WCHAR* searchString);
  bool edit;
  int selectedItem;
};

#endif
