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

#ifndef __BASEAPPLET_H
#define __BASEAPPLET_H

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#define UNICODE 1

#include "../emergeLib/emergeLib.h"
#include "../emergeGraphics/emergeGraphics.h"
#include "../emergeSchemeEngine/emergeSchemeEngine.h"
#include "BaseSettings.h"
#include "BaseAppletMenu.h"
#include <tr1/memory>
#include <tr1/shared_ptr.h>

#ifdef EMERGEBASECLASSES_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif

#define MIN_APPLET_SIZE 10
#define MOUSE_TIMER 0
#define MOUSE_POLL_TIME 250

#define FULLSCREEN_TIMER      1
#define FULLSCREEN_POLL_TIME  100

class DLL_EXPORT BaseApplet
{
public:
  BaseApplet(HINSTANCE hInstance, WCHAR *appletName, bool allowAutoSize);
  virtual ~BaseApplet();
  UINT Initialize(WNDPROC WindowProcedure, LPVOID lpParam, std::tr1::shared_ptr<BaseSettings> pSettings);
  LRESULT DoMoving(HWND hwnd, RECT *lpRect);
  LRESULT DoEnterSizeMove(HWND hwnd);
  LRESULT DoExitSizeMove(HWND hwnd);
  LRESULT DoWindowPosChanging(WINDOWPOS *windowPos);
  LRESULT DoMove();
  LRESULT DoSize(DWORD width, DWORD height);
  LRESULT DoSizing(HWND hwnd, UINT edge, LPRECT rect);
  LRESULT DoDisplayChange(HWND hwnd UNUSED);
  LRESULT DoNCLButtonDown();
  LRESULT DoNCLButtonUp();
  LRESULT DoSetCursor();
  LRESULT DoTimer(UINT_PTR timerID);
  LRESULT DoNCRButtonUp();
  LRESULT DoEmergeNotify(UINT messageClass, UINT message);
  LRESULT DoSysColorChange();
  LRESULT DoSysCommand(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT DoHitTest(LPARAM lParam);
  LRESULT DoCopyData(COPYDATASTRUCT *cds);
  void UpdateGUI(WCHAR *schemeFile = NULL);
  void AdjustRect(RECT *wndRect);
  bool ClickThrough();
  std::tr1::shared_ptr<BaseSettings> pBaseSettings;
  HDC activeBackgroundDC, inactiveBackgroundDC;
  LRESULT DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  void SetFullScreen(bool value);
  bool GetFullScreen();

private:
  WCHAR appletName[MAX_LINE_LENGTH];
  static VOID CALLBACK FullScreenTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
  bool fullScreen, allowAutoSize;
  RECT referenceRect;

protected:
  HWND mainWnd, toolWnd;
  HINSTANCE mainInst;
  bool mouseOver, appletHidden;
  UINT ShellMessage;
  POINT anchor;
  GUIINFO guiInfo;
  RECT oldrt;
  std::tr1::shared_ptr<BaseAppletMenu> pBaseAppletMenu;

  void DrawAlphaBlend();
  virtual LRESULT PaintContent(HDC hdc, RECT clientrt);
  virtual void AppletUpdate();
  virtual void ShowConfig();
  virtual bool PaintItem(HDC hdc, UINT index, int x, int y, RECT rect);
  virtual size_t GetVisibleIconCount();
  virtual size_t GetIconCount();
};

#endif
