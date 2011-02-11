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

#ifndef __ED_MENUBUILDER_H
#define __ED_MENUBUILDER_H

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

// Define required for menu animation
#undef WINVER
#define WINVER 0x0501

#include <map>
#include <process.h>
#include <time.h>
#include "MenuEditor.h"
#include "ItemEditor.h"
#include "MenuListItem.h"
#include "Config.h"
#include "../emergeGraphics/emergeGraphics.h"
#include "../emergeAppletEngine/emergeAppletEngine.h"
#include "CustomDropSource.h"
#include "CustomDropTarget.h"
#include "CustomDataObject.h"

// Define Menu Types
#define MENU_RIGHT           0x01
#define MENU_MID             0x02

// Define DisplayRegMenu Types
#define DRM_DELETE          0x100
#define DRM_EDIT            0x200
#define DRM_ADD             0x300
#define DRM_RUNAS           0x400

#ifndef MND_CONTINUE
#define MND_CONTINUE 0
#endif

#ifndef MND_ENDMENU
#define MND_ENDMENU 1
#endif

// Define BuildSettingsMenu options
typedef enum _BUILDSETTINGSMENU {
  BSM_SEPARATOR = 0,
  BSM_CONFIGURE,
  BSM_LAUNCH,
  BSM_SHELL,
  BSM_ALIAS,
  BSM_SELECTTHEME,
  BSM_SAVETHEME,
  BSM_QUIT
} BUILDSETTINGSMENU;

// Define BuildHelpMenu options
typedef enum _BUILDHELPMENU {
  BHM_SEPARATOR = 0,
  BHM_OFFLINE,
  BHM_TUTORIAL,
  BHM_WEBSITE,
  BHM_ABOUT
} BUILDHELPMENU;

typedef std::map< HMENU,std::tr1::shared_ptr<MenuListItem> > MenuMap;

class MenuBuilder
{
private:
  std::tr1::shared_ptr<MenuEditor> pMenuEditor;
  std::tr1::shared_ptr<ItemEditor> pItemEditor;
  std::tr1::shared_ptr<Settings> pSettings;
  HWND menuWnd;
  HINSTANCE mainInst;
  HMENU rootMenu, taskMenu;
  bool MButtonDown;
  UINT SelectedMenuType, SelectedMenuIndex;
  HMENU SelectedMenu;
  WORD SelectedItem;
  WORD SelectedItemType;
  UINT ShellMessage;
  void BuildMenu(MenuMap::iterator iter);
  void BuildXMLMenu(MenuMap::iterator iter);
  void BuildFileMenu(MenuMap::iterator iter);
  void BuildFileMenuFromString(MenuMap::iterator iter, WCHAR *parsedValue);
  void BuildDefaultMenu(MenuMap::iterator iter);
  void BuildSettingsMenu(MenuMap::iterator iter);
  void BuildHelpMenu(MenuMap::iterator iter);
  bool NoPrefixString(WCHAR *source);
  MenuMap menuMap;
  HHOOK menuHook;
  void ExecuteXMLMenuItem(UINT type, WCHAR *value, WCHAR *workingDir);
  void ExecuteFileMenuItem(const WCHAR *value);
  void ExecuteTaskMenuItem(HWND task);
  void ExecuteSettingsMenuItem(UINT index);
  void ExecuteHelpMenuItem(UINT index);
  void UpdateMenuHook();
  void ExpandEmergeVar(LPTSTR value, LPTSTR var);
  void ClearMenu(MenuMap::iterator iter);
  void AddSettingsItem(MenuMap::iterator iter, WCHAR* text, UINT id);
  bool GetPos(MenuMap::iterator iter, WCHAR *input, bool directory, UINT *pos, UINT *itemID, ULONG_PTR *itemData);
  static LRESULT CALLBACK HookCallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
  static BOOL CALLBACK BuildTasksMenu(HWND hwnd, LPARAM lParam);
  static LRESULT CALLBACK MenuProcedure (HWND, UINT, WPARAM, LPARAM);
  static BOOL CALLBACK SetMonitorArea(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
  MenuMap::iterator GetMenuIterID(POINT pt, int *index);
  int DisplayRegContext(POINT pt, int type);
  bool registered;
  bool AddMenuItem(MenuMap::iterator iter, int index);
  bool EditMenuItem(MenuMap::iterator iter, int index);
  void ElevatedExecute(MenuItem *menuItem);
  float winVersion;
//    CustomDropTarget *customDropTarget;
//    IDropTarget *dropTarget;

public:
  MenuBuilder(HINSTANCE desktopInst);
  ~MenuBuilder();
  bool Initialize();
  LRESULT DoButtonDown(UINT button);
  LRESULT DoMenuDrag(HWND hwnd, HMENU menu);
  LRESULT DoMenuGetObject(HWND hwnd, MENUGETOBJECTINFO *mgoInfo);
  LRESULT DoInitMenu(HMENU menu);
  LRESULT ExecuteMenuItem(UINT itemID);
  LRESULT DoContextMenu(POINT pt);
  LRESULT DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT DoCopyData(COPYDATASTRUCT *cds);
  void AddTaskItem(HWND hwnd);
  BOOL DoDrawItem(LPDRAWITEMSTRUCT lpDrawItem);
  BOOL DoMeasureItem(LPMEASUREITEMSTRUCT lpMeasureItem);
  LRESULT DoMenuSelect(HMENU menu, WORD itemType, WORD item);
  void ClearAllMenus();
  HWND GetWnd();
  void SetWorkArea();
  void RenameConfigFile();
};

#endif

