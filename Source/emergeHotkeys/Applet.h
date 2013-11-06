/*!
  @file Applet.h
  @brief header for emergeHotkeys
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

#ifndef __GUARD_301207f0_b6ec_49a6_b414_d57ff3ed7f88
#define __GUARD_301207f0_b6ec_49a6_b414_d57ff3ed7f88

#define EXECUTE_WAIT_TIME 50

#include <process.h>
#include "../emergeAppletEngine/emergeAppletEngine.h"
#include "../emergeBaseClasses/BaseApplet.h"
#include "Actions.h"

class Applet: public BaseApplet
{
private:
  HHOOK keyHook;
  std::tr1::shared_ptr<Settings> pSettings;
  std::tr1::shared_ptr<Actions> pActions;
  UINT hotkeyCount;

public:
  Applet(HINSTANCE hInstance);
  ~Applet();
  UINT Initialize();
  void ExecuteAction(UINT index);
  LRESULT DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
  static DWORD WINAPI ExecuteThreadProc(LPVOID lpParameter);
  static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
  void ShowConfig();
  void Activate();
};

#endif
