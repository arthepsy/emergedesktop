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

WCHAR myName[] = TEXT("emergeLang");

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

  if (message == WM_CREATE)
    {
      // Register to recieve the specified Emerge Desktop messages
      PostMessage(ELGetCoreWindow(), EMERGE_REGISTER, (WPARAM)hwnd, (LPARAM)EMERGE_CORE);

      cs = (CREATESTRUCT*)lParam;
      pApplet = reinterpret_cast<Applet*>(cs->lpCreateParams);
      return DefWindowProc(hwnd, message, wParam, lParam);
    }

  if (pApplet == NULL)
    return DefWindowProc(hwnd, message, wParam, lParam);

  switch (message)
    {
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

    case WM_SETCURSOR:
      pApplet->DoSetCursor();
      return DefWindowProc(hwnd, message, wParam, lParam);

      // Handles the resizing of the window
    case WM_NCHITTEST:
      return pApplet->DoHitTest(lParam);

      // Repaint the icons as the window size is changing
    case WM_WINDOWPOSCHANGING:
      return pApplet->DoWindowPosChanging((WINDOWPOS *)lParam);

    case WM_ENTERSIZEMOVE:
      return pApplet->DoEnterSizeMove(hwnd);

    case WM_EXITSIZEMOVE:
      return pApplet->DoExitSizeMove(hwnd);

    case WM_SIZING:
      return pApplet->DoSizing(hwnd, (UINT)wParam, (LPRECT)lParam);

    case WM_MOVING:
      return pApplet->DoMoving(hwnd, (LPRECT)lParam);

    case WM_DISPLAYCHANGE:
      return pApplet->DoDisplayChange(hwnd);

    case WM_SYSCOMMAND:
      return pApplet->DoSysCommand(hwnd, message, wParam, lParam);

    case WM_TIMER:
      return pApplet->DoTimer((UINT_PTR)wParam);

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
  :BaseApplet(hInstance, myName, true, false)
{
  mainFont = NULL;
  displayLang = 0;
}

Applet::~Applet()
{
  // stop timer
  UINT langID = (ULONG_PTR)mainWnd + 100;
  KillTimer(mainWnd, langID);

  // Unregister the specified Emerge Desktop messages
  PostMessage(ELGetCoreWindow(), EMERGE_UNREGISTER, (WPARAM)mainWnd, (LPARAM)EMERGE_VWM);

  if (mainFont)
    DeleteObject(mainFont);
}

UINT Applet::Initialize()
{
  pSettings = std::tr1::shared_ptr<Settings>(new Settings());
  UINT ret = BaseApplet::Initialize(WindowProcedure, this, pSettings);
  if (ret == 0)
    return ret;

  // Set initial label
  UpdateLabel(GetLocaleId(0));

  // Set the window transparency
  UpdateGUI();

  // setup timer
  UINT langID = (ULONG_PTR)mainWnd + 100;
  SetTimer(mainWnd, langID, 1000, NULL);

  return ret;
}

LRESULT Applet::DoTimer(UINT_PTR timerID)
{
  if (timerID == MOUSE_TIMER)
    return BaseApplet::DoTimer(timerID);
  else
    UpdateLanguage();

  return 1;
}

LRESULT Applet::PaintContent(HDC hdc, RECT clientrt)
{
  CLIENTINFO clientInfo;
  FORMATINFO formatInfo;

  if (ELToLower(pSettings->GetHorizontalDirection()) == TEXT("center"))
    formatInfo.horizontalAlignment = EGDAT_HCENTER;
  else if (ELToLower(pSettings->GetHorizontalDirection()) == TEXT("right"))
    formatInfo.horizontalAlignment = EGDAT_RIGHT;
  else
    formatInfo.horizontalAlignment = EGDAT_LEFT;

  if (ELToLower(pSettings->GetVerticalDirection()) == TEXT("center"))
    formatInfo.verticalAlignment = EGDAT_VCENTER;
  else if (ELToLower(pSettings->GetHorizontalDirection()) == TEXT("bottom"))
    formatInfo.verticalAlignment = EGDAT_BOTTOM;
  else
    formatInfo.verticalAlignment = EGDAT_TOP;

  formatInfo.font = mainFont;
  formatInfo.color = guiInfo.colorFont;
  formatInfo.flags = 0;

  clientInfo.hdc = hdc;
  CopyRect(&clientInfo.rt, &clientrt);
  clientInfo.bgAlpha = guiInfo.alphaBackground;

  EGDrawAlphaText(guiInfo.alphaText, clientInfo, formatInfo, displayLabel);

  return 0;
}

void Applet::ShowConfig()
{
  Config config(mainInst, mainWnd, appletName, pSettings);
  if (config.Show() == IDOK)
    {
      displayLang = 0;
      UpdateLanguage();
      UpdateGUI();
    }
}

void Applet::AppletUpdate()
{
  if (mainFont != NULL)
    DeleteObject(mainFont);

  mainFont = CreateFontIndirect(pSettings->GetFont());
}

DWORD Applet::GetCurrentActiveThread()
{
  HWND hwnd = GetForegroundWindow();
  DWORD threadId = GetWindowThreadProcessId(hwnd, NULL);

  // Get GUI thread information
  GUITHREADINFO gui;
  ZeroMemory(&gui, sizeof(GUITHREADINFO));
  gui.cbSize = sizeof(GUITHREADINFO);
  GetGUIThreadInfo(threadId, &gui);

  // Get the most interesting hwnd of an application
  HWND active = (gui.hwndCaret != NULL) ? gui.hwndCaret :
                (gui.hwndFocus != NULL) ? gui.hwndFocus :
                (gui.hwndActive != NULL) ? gui.hwndActive : hwnd;

  return (active != hwnd) ? GetWindowThreadProcessId(active, NULL) : threadId;
}

LCID Applet::GetLocaleId(DWORD threadId)
{
  HKL hkl = GetKeyboardLayout(threadId);
  DWORD lgid = LOWORD(hkl);

  return MAKELCID(lgid, SORT_DEFAULT);
}

void Applet::UpdateLanguage()
{
  // 1. get main thread ID of currently active application
  // 2. get language of this thread
  // 3. compare to currently displayed language
  // 4. if not the same: update language, tooltip and initiate redraw
  DWORD threadId = GetCurrentActiveThread();
  LCID currentLang = GetLocaleId(threadId);
  if (currentLang != displayLang)
    {
      displayLang = currentLang;
      UpdateLabel(displayLang);
      UpdateTooltip(displayLang);
      // UpdateGUI();
      DrawAlphaBlend();
    }
}

void Applet::UpdateLabel(LCID lang)
{
  // update language label
  WCHAR label[MAX_LABEL];
  GetLocaleInfo(lang, pSettings->GetDisplayLCType(), (LPWSTR)&label, MAX_LABEL);
  if (pSettings->IsUpperCase())
    {
      CharUpper((LPWSTR)&label);
    }
  else
    {
      CharLower((LPWSTR)&label);
    }

  wcsncpy(displayLabel, label, MAX_LABEL);
}

void Applet::UpdateTooltip(LCID lang)
{
  // update tooltip
  WCHAR tooltip[MAX_LABEL];
  GetLocaleInfo(lang, LOCALE_SLANGUAGE, (LPWSTR)&tooltip, MAX_LABEL);

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
  ti.lpszText = tooltip;
  ti.rect = rect;

  // If it exists, modify the tooltip, if not add it
  if (exists)
    SendMessage(toolWnd, TTM_SETTOOLINFO, 0, (LPARAM)(LPTOOLINFO)&ti);
  else
    SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
}
