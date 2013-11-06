/*!
  @file Core.h
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

#ifndef __GUARD_6752f525_8457_4568_b404_94beb7b28fd7
#define __GUARD_6752f525_8457_4568_b404_94beb7b28fd7

#define UNICODE 1

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include <process.h>
#include <map>
#include <set>
#include <string>
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeOSLib.h"
#include "Config.h"
#include "Desktop.h"
#include "InternalCommandHandler.h"
#include "LaunchEditor.h"
#include "MessageControl.h"
#include "Settings.h"
#include "Shell.h"
#include "ShellChanger.h"
#include "ThemeSelector.h"
#include "Welcome.h"

typedef std::set<HWND> WindowSet;
typedef std::map< std::wstring, WindowSet > LaunchMap;

static const UINT TASKBAR_CREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));

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
  void CheckLaunchItem(LaunchMap* launchMap, std::wstring item);
  void StartExplorer();
  void EnableExplorerDesktop();
  std::wstring xmlFile;
  void ShowWelcome();
  HANDLE explorerThread;
  static DWORD WINAPI EnableExplorerThreadProc(LPVOID lpParameter);

public:
  Core(HINSTANCE hInstance);
  ~Core();
  void About();
  void ShowConfig(UINT startPage);
  bool Initialize(WCHAR* commandLine);
  LRESULT DoWTSSessionChange(UINT message);
  LRESULT DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT DoCopyData(COPYDATASTRUCT* cds);
};

#endif
