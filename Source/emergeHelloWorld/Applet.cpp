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

#include "Applet.h"

WCHAR myName[ ] = TEXT("emergeHelloWorld");

//----  --------------------------------------------------------------------------------------------------------
// Function:	WindowProcedure
// Required:	HWND hwnd - window handle that message was sent to
// 		UINT message - action to handle
// 		WPARAM wParam - dependant on message
// 		LPARAM lParam - dependant on message
// Returns:	LRESULT
// Purpose:	Handles messages sent from DispatchMessage
//----  --------------------------------------------------------------------------------------------------------
LRESULT CALLBACK Applet::WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  CREATESTRUCT *cs;
  static Applet *pApplet = NULL;

  // As part of the CreateWindowEx function, an Applet class pointer is passed, which is recieved
  // in the WM_CREATE message.  A static class pointer is created to store the passed class pointer.
  if (message == WM_CREATE)
    {
      // Register to recieve the specified Emerge Desktop messages
      PostMessage(ELGetCoreWindow(), EMERGE_REGISTER, (WPARAM)hwnd, (LPARAM)EMERGE_CORE);

      cs = (CREATESTRUCT*)lParam;
      pApplet = reinterpret_cast<Applet*>(cs->lpCreateParams);
      return DefWindowProc(hwnd, message, wParam, lParam);
    }

  // If the class pointer is NULL (other than for the WM_CREATE message), simply return the default
  // action for that message so as to not cause an access violation.
  if (pApplet == NULL)
    return DefWindowProc(hwnd, message, wParam, lParam);

  switch (message)
    {
      // The WM_COPYDATA message is used to notify the applet of a change to the ThemeDir variable.
    case WM_COPYDATA:
      return pApplet->DoCopyData((COPYDATASTRUCT *)lParam);

      // Needed to handle changing the system colors.  It forces
      // a repaint of the window as well as the frame.
    case WM_SYSCOLORCHANGE:
      return pApplet->DoSysColorChange();

      // Allow for window dragging via Ctrl - Left - Click dragging
    case WM_NCLBUTTONDOWN:
      pApplet->DoNCLButtonDown();
      return DefWindowProc(hwnd, message, wParam, lParam);

      // Display the mainMenu via Ctrl - Right - Click
    case WM_NCRBUTTONUP:
      return pApplet->DoNCRButtonUp();

      // Reset the cursor back to the standard arrow after dragging
    case WM_NCLBUTTONUP:
      pApplet->DoNCLButtonUp();
      return DefWindowProc(hwnd, message, wParam, lParam);

      // The WM_SETCURSOR message is used to determine when the Applet should switch to 'mouse over'
      // state.
    case WM_SETCURSOR:
      pApplet->DoSetCursor();
      return DefWindowProc(hwnd, message, wParam, lParam);

      // If the display changes size, let the application now.
    case WM_DISPLAYCHANGE:
      return pApplet->DoDisplayChange(hwnd);

      // Handles the resizing of the window
    case WM_NCHITTEST:
      return pApplet->DoHitTest(lParam);

      // Repaint the icons as the window size is changing
    case WM_WINDOWPOSCHANGING:
      return pApplet->DoWindowPosChanging((WINDOWPOS *)lParam);

      // The WM_MOVING message is used to fascilitate Snap Move.
    case WM_MOVING:
      return pApplet->DoMoving(hwnd, (RECT*)lParam);

      // WM_ENTERSIZEMOVE is captured to determine the an anchor point that is used to assist
      // Snap Move.
    case WM_ENTERSIZEMOVE:
      return pApplet->DoEnterSizeMove(hwnd);

      // Capture WM_EXITSIZEMOVE messages so as to save the size and the position of the applet.
    case WM_EXITSIZEMOVE:
      return pApplet->DoExitSizeMove(hwnd);

      // WM_SIZING is used to enable Snap Size.
    case WM_SIZING:
      return pApplet->DoSizing(hwnd, (UINT)wParam, (LPRECT)lParam);

      // Hande WM_SYSCOMMAND messages so the applet does not close on Alt+F4 / maximize / minimize.
    case WM_SYSCOMMAND:
      return pApplet->DoSysCommand(hwnd, message, wParam, lParam);

      // WM_TIMER messages captured to handle the mouse leaving the applet.
    case WM_TIMER:
      return pApplet->DoTimer((UINT)wParam);

      // When the window is destroyed, send a '0' as the quit value.
    case WM_DESTROY:
    case WM_NCDESTROY:
      // Unregister the specified Emerge Desktop messages
      PostMessage(ELGetCoreWindow(), EMERGE_UNREGISTER, (WPARAM)hwnd, (LPARAM)EMERGE_CORE);

      PostQuitMessage(0);
      break;

      // If not handled just forward the message on
    default:
      return pApplet->DoDefault(hwnd, message, wParam, lParam);
    }

  return 0;
}

Applet::Applet(HINSTANCE hInstance)
  :BaseApplet(hInstance, myName, false, true)
{
  mainInst = hInstance;
  wcscpy(commandText, TEXT("\0"));
  mainFont = NULL;
}

Applet::~Applet()
{
  if (mainFont)
    DeleteObject(mainFont);
}

UINT Applet::Initialize()
{
  pSettings = std::tr1::shared_ptr<Settings>(new Settings());
  UINT ret = BaseApplet::Initialize(WindowProcedure, this, pSettings);
  if (ret == 0)
    return ret;

  // Set the window transparency
  UpdateGUI();

  return 1;
}

LRESULT Applet::PaintContent(HDC hdc, RECT clientrt)
{
  CLIENTINFO clientInfo;
  FORMATINFO formatInfo;

  if (_wcsicmp(pSettings->GetTextAlign(), TEXT("center")) == 0)
    formatInfo.horizontalAlignment = EGDAT_HCENTER;
  else if (_wcsicmp(pSettings->GetTextAlign(), TEXT("right")) == 0)
    formatInfo.horizontalAlignment = EGDAT_RIGHT;
  else
    formatInfo.horizontalAlignment = EGDAT_LEFT;
  if (_wcsicmp(pSettings->GetVerticalAlign(), TEXT("center")) == 0)
    formatInfo.verticalAlignment = EGDAT_VCENTER;
  else if (_wcsicmp(pSettings->GetVerticalAlign(), TEXT("bottom")) == 0)
    formatInfo.verticalAlignment = EGDAT_BOTTOM;
  else
    formatInfo.verticalAlignment = EGDAT_TOP;
  formatInfo.font = mainFont;
  formatInfo.color = guiInfo.colorFont;
  formatInfo.flags = 0;

  clientInfo.hdc = hdc;
  CopyRect(&clientInfo.rt, &clientrt);
  clientInfo.bgAlpha = guiInfo.alphaBackground;

  EGDrawAlphaText(guiInfo.alphaText, clientInfo, formatInfo, pSettings->GetDisplayTextFormat());
  UpdateTip(pSettings->GetDisplayTipFormat());

  return 0;
}

void Applet::ShowConfig()
{
  Config config(mainInst, mainWnd, pSettings, myName);
  if (config.Show() == IDOK)
    UpdateGUI();
}

void Applet::UpdateTip(WCHAR *tip)
{
  TOOLINFO ti;
  ZeroMemory(&ti, sizeof(TOOLINFO));
  RECT rect;
  bool exists;

  GetClientRect(mainWnd, &rect);

  // fill in the TOOLINFO structure
  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = mainWnd;
  ti.uId = (ULONG_PTR)toolWnd;

  // Check to see if the tooltip exists
  exists = SendMessage(toolWnd, TTM_GETTOOLINFO, 0,(LPARAM) (LPTOOLINFO) &ti) ? true : false;

  //  complete the rest of the TOOLINFO structure
  ti.hinst = mainInst;
  ti.lpszText = tip;
  ti.rect = rect;

  // If it exists, modify the tooltip, if not add it
  if (exists)
    SendMessage(toolWnd, TTM_SETTOOLINFO, 0, (LPARAM)(LPTOOLINFO)&ti);
  else
    SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
}

void Applet::AppletUpdate()
{

  if (mainFont != NULL)
    DeleteObject(mainFont);
  mainFont = CreateFontIndirect(pSettings->GetFont());
}

