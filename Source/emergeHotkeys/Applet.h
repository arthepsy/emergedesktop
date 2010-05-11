//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2010  The Emerge Desktop Development Team
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

#ifndef __EH_APPLET_H
#define __EH_APPLET_H

#include "Actions.h"
#include <process.h>
#include "../emergeAppletEngine/emergeAppletEngine.h"

class Applet
{
private:
  std::tr1::shared_ptr<Settings> pSettings;
  std::tr1::shared_ptr<Actions> pActions;
  HINSTANCE mainInst;
  HWND mainWnd;
  HMENU mainMenu;
  UINT hotkeyCount;
  UINT lwinID, rwinID;

public:
  Applet(HINSTANCE hInstance);
  ~Applet();
  UINT Initialize();
  void ExecuteAction(UINT index);
  LRESULT DoTimer(UINT index);
  LRESULT DoCopyData(COPYDATASTRUCT *cds);
  static LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
};

#endif
