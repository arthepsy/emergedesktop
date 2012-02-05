// vim:tags+=../emergeLib/tags
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

#ifndef __EC_CORE_H
#define __EC_CORE_H

#include <process.h>
#include <string>
#include <map>
#include <set>
#include "Shell.h"
#include "MessageControl.h"
#include "ShellChanger.h"
#include "Settings.h"
#include "LaunchEditor.h"
#include "Desktop.h"
#include "ThemeSelector.h"
#include "Config.h"
#include "Welcome.h"

typedef std::set<HWND> WindowSet;
typedef std::map< std::wstring, WindowSet > LaunchMap;

class Core
{
private:
  std::tr1::shared_ptr<ShellChanger> pShellChanger;
  std::tr1::shared_ptr<Settings> pSettings;
  std::tr1::shared_ptr<Shell> pShell;
  std::tr1::shared_ptr<MessageControl> pMessageControl;
  std::tr1::shared_ptr<Desktop> pDesktop;
  std::tr1::shared_ptr<LaunchEditor> pLaunchEditor;
  std::tr1::shared_ptr<ThemeSelector> pThemeSelector;
  HINSTANCE mainInst;
  HWND mainWnd;
  bool registered;
  static LRESULT CALLBACK CoreProcedure (HWND, UINT, WPARAM, LPARAM);
  static BOOL CALLBACK LaunchMapEnum(HWND hwnd, LPARAM lParam);
  bool RunLaunchItems();
  void ConvertTheme();
  bool CheckLaunchList();
  bool BuildLaunchList();
  void CheckLaunchItem(LaunchMap *launchMap, const WCHAR *item);
  void StartExplorer(bool showDesktop);
  std::wstring xmlFile;
  void ShowWelcome();

public:
  Core(HINSTANCE hInstance);
  ~Core();
  void About();
  void ShowConfig(UINT startPage);
  bool Initialize(WCHAR *commandLine);
  LRESULT DoWTSSessionChange(UINT message);
  LRESULT DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT DoCopyData(COPYDATASTRUCT *cds);
};

#endif

