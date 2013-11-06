/*!
  @file Shutdown.h
  @brief internal header for emergeLib
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

#ifndef __GUARD_8735e727_2f88_4a35_99e7_517af0ee2e5a
#define __GUARD_8735e727_2f88_4a35_99e7_517af0ee2e5a

#define UNICODE 1

#undef _WIN32_IE
#define _WIN32_IE	0x501

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x600

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#include <windows.h>
#include "../../emergeIcons/resource.h"
#include "../emergeOSLib.h"
#include "../emergeWindowLib.h"
#include "../resource.h"

#ifndef _W64
#include <WtsApi32.h>
#endif

class Shutdown
{
public:
  Shutdown(HINSTANCE hInstance, HWND mainWnd);
  ~Shutdown();
  int Show();
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  BOOL DoNotify(HWND hwndDlg, LPARAM lParam);

private:
  HINSTANCE hInstance;
  HMODULE hIconsDLL;
  HWND mainWnd, toolWnd;
  HBITMAP logoBMP;
  bool enableDisconnect;
  static BOOL CALLBACK ShutdownDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
  bool DoShutdown(HWND hwndDlg);
};

#endif
