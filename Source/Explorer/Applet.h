//----  --------------------------------------------------------------------------------------------------------
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
//----  --------------------------------------------------------------------------------------------------------

#ifndef __APPLET_H
#define __APPLET_H

#define UNICODE 1

#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeOSLib.h"
#include "../emergeLib/emergeUtilityLib.h"
#include "../emergeLib/emergeWindowLib.h"
#include "../emergeAppletEngine/emergeAppletEngine.h"
#include "ShellDesktopTray.h"

typedef void *(WINAPI *SHCREATEDESKTOP)(void *);
typedef bool (WINAPI *SHDESKTOPMESSAGELOOP)(void *);

class Applet
{
private:
  HWND mainWnd;
  HINSTANCE mainInst;
	HANDLE m_hThread, showThread;
	DWORD m_dwThreadID;
  void ShellServicesInit();
  void ShellServicesTerminate();
  void ToggleThread(LPARAM lParam);
  static DWORD WINAPI ShowDesktopThreadProc(LPVOID lpParameter);

public:
  Applet(HINSTANCE hInstance);
  ~Applet();
  UINT Initialize();
  static LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
	static DWORD WINAPI ThreadFunc(LPVOID pvParam);
	LRESULT DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif
