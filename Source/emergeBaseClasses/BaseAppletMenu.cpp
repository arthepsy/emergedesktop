// vim: tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2007  The Emerge Desktop Development Team
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
#include "../emergeSchemeEngine/emergeSchemeEngine.h"
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

BaseAppletMenu::BaseAppletMenu(HWND mainWnd, HINSTANCE hInstance, WCHAR *appletName)
{
  (*this).hInstance = hInstance;

  (*this).mainWnd = mainWnd;
  menuHook = NULL;
  wcscpy((*this).appletName, appletName);
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
  // Clear the menu hook
  if (menuHook)
    UnhookWindowsHookEx(menuHook);

  globalMenuAlpha = menuAlpha;

  // Hook the menus (for transparency)
  menuHook = SetWindowsHookEx(WH_CALLWNDPROC, HookCallWndProc, 0, GetWindowThreadProcessId(mainWnd, 0));
}

void BaseAppletMenu::BuildMenu()
{
  AppendMenu(appletMenu, MF_STRING, EBC_ABOUT, TEXT("About"));
  AppendMenu(appletMenu, MF_SEPARATOR, 0, NULL);
  AppendMenu(appletMenu, MF_STRING, EBC_LOADSCHEME, TEXT("Load Scheme"));
  AppendMenu(appletMenu, MF_STRING, EBC_RELOADSCHEME, TEXT("Reload Current Scheme"));
  AppendMenu(appletMenu, MF_SEPARATOR, 0, NULL);
  AppendMenu(appletMenu, MF_STRING, EBC_CONFIGURE, TEXT("Configure"));
  AppendMenu(appletMenu, MF_SEPARATOR, 0, NULL);
  AppendMenu(appletMenu, MF_STRING, EBC_EXIT, TEXT("Exit"));
}

DWORD BaseAppletMenu::ActivateMenu(int x, int y, WCHAR *schemeFile)
{
  UINT ret = TrackPopupMenuEx(appletMenu, TPM_RETURNCMD, x, y, mainWnd, NULL);
  WCHAR file[MAX_PATH];
  OPENFILENAME ofn;
  std::wstring themePath = TEXT("%ThemeDir%"), schemePath = ESEGetScheme();
  themePath = ELExpandVars(themePath);
  schemePath = ELExpandVars(schemePath);

  switch (ret)
    {
    case EBC_ABOUT:
      WCHAR tmp[MAX_LINE_LENGTH];
      VERSIONINFO versionInfo;

      if (ELAppletVersionInfo(mainWnd, &versionInfo))
        {
          swprintf(tmp, TEXT("%s\n\nVersion: %s\n\nCurrent Scheme: %s\n\nAuthor: %s"),
                   versionInfo.Description,
                   versionInfo.Version,
                   schemePath.c_str(),
                   versionInfo.Author);

          ELMessageBox(GetDesktopWindow(), tmp, appletName, ELMB_ICONQUESTION|ELMB_OK|ELMB_MODAL);
        }
      else
        ret = 0;
      break;

    case EBC_EXIT:
      PostQuitMessage(0);
      break;

    case EBC_LOADSCHEME:
      ZeroMemory(file, MAX_PATH);
      ZeroMemory(&ofn, sizeof(ofn));

      ofn.lStructSize = sizeof(ofn);
      ofn.hwndOwner = mainWnd;
      ofn.lpstrFilter = TEXT("Scheme (*.eds)\0*.eds\0");
      ofn.lpstrFile = file;
      ofn.nMaxFile = MAX_PATH;
      schemePath = themePath + TEXT("\\Schemes");
      if (PathIsDirectory(schemePath.c_str()))
        themePath = schemePath;
      ofn.lpstrInitialDir = themePath.c_str();
      ofn.lpstrTitle = TEXT("Load Scheme");
      ofn.lpstrDefExt = NULL;
      ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS;

      if (GetOpenFileName(&ofn))
        {
          ELUnExpandVars(file);
          wcscpy(schemeFile, file);
          break;
        }
      else
        ret = 0;
      break;
    }

  return ret;
}
