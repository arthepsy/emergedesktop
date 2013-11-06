/*!
  @file BaseApplet.h
  @brief header for emergeBaseClasses
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

#ifndef __GUARD_d12f0cd8_ef59_49fd_8c72_a77cdf71cc5b
#define __GUARD_d12f0cd8_ef59_49fd_8c72_a77cdf71cc5b

#define UNICODE 1

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#ifdef EMERGEBASECLASSES_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#define MIN_APPLET_SIZE 10
#define MOUSE_TIMER 0
#define MOUSE_POLL_TIME 250

#define FULLSCREEN_WAIT_TIME    100
#define DISPLAYCHANGE_WAIT_TIME 500

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include <vector>
#include "../emergeGraphics/emergeGraphics.h"
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeOSLib.h"
#include "../emergeLib/emergeUtilityLib.h"
#include "../emergeLib/emergeWindowLib.h"
#include "../emergeStyleEngine/emergeStyleEngine.h"
#include "BaseAppletMenu.h"
#include "BaseSettings.h"

class DLL_EXPORT BaseApplet
{
public:
  BaseApplet(HINSTANCE hInstance, const WCHAR* appletName, bool allowAutoSize, bool allowMultipleInstances);
  virtual ~BaseApplet();
  UINT Initialize(WNDPROC WindowProcedure, LPVOID lpParam, std::tr1::shared_ptr<BaseSettings> pSettings);
  LRESULT DoMoving(HWND hwnd, RECT* lpRect);
  LRESULT DoEnterSizeMove(HWND hwnd);
  LRESULT DoExitSizeMove(HWND hwnd);
  LRESULT DoWindowPosChanging(WINDOWPOS* windowPos);
  LRESULT DoMove();
  LRESULT DoSize(DWORD width, DWORD height);
  LRESULT DoSizing(HWND hwnd, UINT edge, LPRECT rect);
  LRESULT DoDisplayChange(HWND hwnd UNUSED);
  LRESULT DoNCLButtonDown();
  LRESULT DoNCLButtonUp();
  LRESULT DoSetCursor();
  LRESULT DoTimer(UINT_PTR timerID);
  LRESULT DoNCRButtonUp();
  LRESULT DoSysColorChange();
  LRESULT DoSysCommand(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT DoHitTest(LPARAM lParam);
  LRESULT DoCopyData(COPYDATASTRUCT* cds);
  void UpdateGUI(std::wstring styleFile = TEXT(""));
  void AdjustRect(RECT* wndRect);
  bool ClickThrough();
  std::tr1::shared_ptr<BaseSettings> pBaseSettings;
  HDC activeBackgroundDC, inactiveBackgroundDC;
  HGDIOBJ activeBackgroundObj, inactiveBackgroundObj;
  HBITMAP activeBackgroundBMP, inactiveBackgroundBMP;
  LRESULT DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  void SetFullScreen(bool value);
  bool GetFullScreen();
  HWND GetMainWnd();
  void HideApplet(bool hide, bool* variable, bool force);
  WCHAR* GetInstanceName();

private:
  int appletCount;
  bool allowAutoSize, allowMultipleInstances;
  RECT referenceRect;
  HANDLE fullScreenThread, multiInstanceLock;
  HANDLE instanceManagementMutex;
  static DWORD WINAPI FullScreenThreadProc(LPVOID lpParameter);
  int ReadAppletCount(int defaultValue = -1);
  bool WriteAppletCount(int value, bool forceCreate = true);
  void RenameSettingsFiles();
  bool SpawnInstance();
  bool firstRun;
  inline bool MonitorCheck(HWND app);

protected:
  WCHAR appletName[MAX_LINE_LENGTH], baseAppletName[MAX_LINE_LENGTH];
  HWND mainWnd, toolWnd;
  HINSTANCE mainInst;
  bool mouseOver, appletHidden, fullScreen;
  UINT ShellMessage;
  POINT anchor;
  GUIINFO guiInfo;
  RECT oldrt;
  std::tr1::shared_ptr<BaseAppletMenu> pBaseAppletMenu;

  void DrawAlphaBlend();
  virtual LRESULT PaintContent(HDC hdc, RECT clientrt);
  virtual void AppletUpdate();
  virtual void AdjustContentRect(LPRECT contentRect);
  virtual void Activate();
  virtual void ShowConfig();
  virtual bool PaintItem(HDC hdc, size_t index, int x, int y, RECT rect);
  virtual size_t GetVisibleIconCount();
  virtual size_t GetIconCount();
};

#endif
