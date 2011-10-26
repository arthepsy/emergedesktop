// vim: tags+=../emergeLib/tags
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

#include "BaseAppletMenu.h"
#include "../emergeStyleEngine/emergeStyleEngine.h"
#include <stdio.h>
#include <shlwapi.h>

BYTE globalMenuAlpha;

//----  --------------------------------------------------------------------------------------------------------
// Function:	HookCallWndProc
// Requires:	int nCode - action
// 		wParam - not used
// 		lParam - message data
// Returns:	LRESULT
// Purpose:	Hooks the menu windows of a given application
//----  --------------------------------------------------------------------------------------------------------
LRESULT CALLBACK BaseAppletMenu::HookCallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  CWPSTRUCT cwps;

  if ( nCode == HC_ACTION )
    {
      CopyMemory(&cwps, (LPVOID)lParam, sizeof(CWPSTRUCT));

      switch (cwps.message)
        {
        case WM_CREATE:
        {
          WCHAR szClass[128];
          GetClassName(cwps.hwnd, szClass, 127);
          if (_wcsicmp(szClass, TEXT("#32768"))==0)
            {
              SetWindowLongPtr(cwps.hwnd,
                               GWL_EXSTYLE,
                               GetWindowLongPtr(cwps.hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
              SetLayeredWindowAttributes(cwps.hwnd, 0, globalMenuAlpha, LWA_ALPHA);
            }
        }
        break;
        }
    }

  return CallNextHookEx((HHOOK)WH_CALLWNDPROC, nCode, wParam, lParam);
}

BaseAppletMenu::BaseAppletMenu(HWND mainWnd, HINSTANCE hInstance, WCHAR *appletName, bool allowMultipleInstances)
{
  (*this).hInstance = hInstance;

  (*this).mainWnd = mainWnd;
  menuHook = NULL;
  wcscpy((*this).appletName, appletName);
  (*this).allowMultipleInstances = allowMultipleInstances;

  // Hook the menus (for transparency)
  menuHook = SetWindowsHookEx(WH_CALLWNDPROC, HookCallWndProc, 0, GetWindowThreadProcessId(mainWnd, 0));
}

BaseAppletMenu::~BaseAppletMenu()
{
  DestroyMenu(appletMenu);

  // Clear the menu hook
  if (menuHook)
    UnhookWindowsHookEx(menuHook);
}

void BaseAppletMenu::Initialize()
{
  // create the mainMenu
  appletMenu = CreatePopupMenu();

  // Populate The Menu
  BuildMenu();
}

void BaseAppletMenu::UpdateHook(DWORD menuAlpha)
{
  globalMenuAlpha = menuAlpha;
}

void BaseAppletMenu::BuildMenu()
{
  AppendMenu(appletMenu, MF_STRING, EBC_ABOUT, TEXT("About"));
  AppendMenu(appletMenu, MF_SEPARATOR, 0, NULL);
  AppendMenu(appletMenu, MF_STRING, EBC_LOADSTYLE, TEXT("Load Style"));
  AppendMenu(appletMenu, MF_STRING, EBC_RELOADSTYLE, TEXT("Reload Current Style"));
  AppendMenu(appletMenu, MF_SEPARATOR, 0, NULL);
  AppendMenu(appletMenu, MF_STRING, EBC_CONFIGURE, TEXT("Configure"));
  AppendMenu(appletMenu, MF_SEPARATOR, 0, NULL);
  if ((*this).allowMultipleInstances)
  {
    AppendMenu(appletMenu, MF_STRING, EBC_NEWINSTANCE, TEXT("New Instance"));
    AppendMenu(appletMenu, MF_STRING, EBC_DELETEINSTANCE, TEXT("Delete Instance"));
    AppendMenu(appletMenu, MF_SEPARATOR, 0, NULL);
  }
  AppendMenu(appletMenu, MF_STRING, EBC_EXIT, TEXT("Exit"));
}

DWORD BaseAppletMenu::ActivateMenu(int x, int y, WCHAR *styleFile)
{
  UINT ret = TrackPopupMenuEx(appletMenu, TPM_RETURNCMD, x, y, mainWnd, NULL);
  WCHAR file[MAX_PATH];
  OPENFILENAME ofn;
  std::wstring themePath = TEXT("%ThemeDir%"), stylePath = ESEGetStyle();
  themePath = ELExpandVars(themePath);
  stylePath = ELExpandVars(stylePath);

  switch (ret)
    {
    case EBC_ABOUT:
      WCHAR tmp[MAX_LINE_LENGTH];
      VERSIONINFO versionInfo;

      if (ELAppletVersionInfo(mainWnd, &versionInfo))
        {
          swprintf(tmp, TEXT("%s\n\nVersion: %s\n\nCurrent Style: %s\n\nAuthor: %s"),
                   versionInfo.Description,
                   versionInfo.Version,
                   stylePath.c_str(),
                   versionInfo.Author);

          ELMessageBox(GetDesktopWindow(), tmp, appletName, ELMB_ICONQUESTION|ELMB_OK|ELMB_MODAL);
        }
      else
        ret = 0;
      break;

    case EBC_EXIT:
      PostQuitMessage(0);
      break;

    case EBC_LOADSTYLE:
      ZeroMemory(file, MAX_PATH);
      ZeroMemory(&ofn, sizeof(ofn));

      ofn.lStructSize = sizeof(ofn);
      ofn.hwndOwner = mainWnd;
      ofn.lpstrFilter = TEXT("Style (*.eds)\0*.eds\0");
      ofn.lpstrFile = file;
      ofn.nMaxFile = MAX_PATH;
      stylePath = themePath + TEXT("\\Styles");
      if (ELPathIsDirectory(stylePath.c_str()))
        themePath = stylePath;
      ofn.lpstrInitialDir = themePath.c_str();
      ofn.lpstrTitle = TEXT("Load Style");
      ofn.lpstrDefExt = NULL;
      ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS;

      if (GetOpenFileName(&ofn))
        {
          ELUnExpandVars(file);
          ELRelativePathFromAbsPath(file, MAX_PATH);
          wcscpy(styleFile, file);
          break;
        }
      else
        ret = 0;
      break;
    }

  return ret;
}
