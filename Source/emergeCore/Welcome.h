//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2011  The Emerge Desktop Development Team
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

#ifndef __WELCOME_H
#define __WELCOME_H

#undef _WIN32_IE
#define _WIN32_IE	0x501

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x600

#include "../emergeLib/emergeLib.h"
#include "../emergeIcons/resource.h"
#include "resource.h"
#include "Settings.h"

class Welcome
{
public:
  Welcome(HINSTANCE hInstance, HWND mainWnd, std::tr1::shared_ptr<Settings> pSettings);
  ~Welcome();
  int Show();
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  BOOL DoNotify(HWND hwndDlg, LPARAM lParam);

private:
  std::tr1::shared_ptr<Settings> pSettings;
  HINSTANCE hInstance;
  HWND mainWnd, toolWnd;
  HBITMAP logoBMP;
  HMODULE hIconsDLL;
  static BOOL CALLBACK WelcomeDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
  bool UpdateSettings(HWND hwndDlg);
};

#endif

