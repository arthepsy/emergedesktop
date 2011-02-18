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

#ifndef __APPLET_H
#define __APPLET_H

#include "ShellDesktopTray.h"
#include "../emergeAppletEngine/emergeAppletEngine.h"

typedef void *(WINAPI *SHCREATEDESKTOP)(void *);
typedef bool (WINAPI *SHDESKTOPMESSAGELOOP)(void *);

class Applet
{
private:
  HWND mainWnd;
  HINSTANCE mainInst;
	HANDLE m_hThread;
	DWORD m_dwThreadID;
  void ShellServicesInit();
  void ShellServicesTerminate();

public:
  Applet(HINSTANCE hInstance);
  ~Applet();
  UINT Initialize();
  static LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
	static DWORD WINAPI ThreadFunc(LPVOID pvParam);
};

#endif
