/*!
  @file Welcome.h
  @brief header for emergeCore
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

#ifndef __GUARD_8cd8601f_4bbd_4887_a00e_7d9df067c37f
#define __GUARD_8cd8601f_4bbd_4887_a00e_7d9df067c37f

#define UNICODE 1

#undef _WIN32_IE
#define _WIN32_IE	0x501

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x600

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include <windows.h>
#include "../emergeIcons/resource.h"
#include "../emergeLib/emergeOSLib.h"
#include "../emergeLib/emergeWindowLib.h"
#include "hyperlink.h"
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
  std::tr1::shared_ptr<CHyperLink> pForumLink, pWikiLink, pTutorialLink, pOfflineLink;
  HINSTANCE hInstance;
  HWND mainWnd, toolWnd;
  HBITMAP logoBMP;
  HMODULE hIconsDLL;
  static BOOL CALLBACK WelcomeDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK LinkProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  bool UpdateSettings(HWND hwndDlg);
};

#endif
