// vim:tags+=../emergeLib/tags,../emergeBaseClasses/tags
//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2013  The Emerge Desktop Development Team
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

WCHAR myName[] = TEXT("emergePower");

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
  CREATESTRUCT* cs;
  static Applet* pApplet = NULL;

  if (message == WM_CREATE)
  {
    // Register to recieve the specified Emerge Desktop messages
    PostMessage(ELGetCoreWindow(), EMERGE_REGISTER, (WPARAM)hwnd, (LPARAM)EMERGE_CORE);

    cs = (CREATESTRUCT*)lParam;
    pApplet = reinterpret_cast<Applet*>(cs->lpCreateParams);
    return DefWindowProc(hwnd, message, wParam, lParam);
  }

  if (pApplet == NULL)
  {
    return DefWindowProc(hwnd, message, wParam, lParam);
  }

  switch (message)
  {
  case WM_COPYDATA:
    return pApplet->DoCopyData((COPYDATASTRUCT*)lParam);

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
    return pApplet->DoWindowPosChanging((WINDOWPOS*)lParam);

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
  : BaseApplet(hInstance, myName, true, false)
{
  mainFont = NULL;
}

Applet::~Applet()
{
  // stop timer
  UINT powerID = (ULONG_PTR)mainWnd + 100;
  KillTimer(mainWnd, powerID);

  // Unregister the specified Emerge Desktop messages
  PostMessage(ELGetCoreWindow(), EMERGE_UNREGISTER, (WPARAM)mainWnd, (LPARAM)EMERGE_VWM);

  if (mainFont)
  {
    DeleteObject(mainFont);
  }
}

UINT Applet::Initialize()
{
  pSettings = std::tr1::shared_ptr<Settings>(new Settings());
  UINT ret = BaseApplet::Initialize(WindowProcedure, this, pSettings);
  if (ret == 0)
  {
    return ret;
  }

  // Set the window transparency
  UpdateGUI();

  // setup timer
  UINT powerID = (ULONG_PTR)mainWnd + 100;
  SetTimer(mainWnd, powerID, pSettings->GetUpdateInterval() * 1000, NULL);
  UpdateStatus();

  return ret;
}

LRESULT Applet::PaintContent(HDC hdc, RECT clientrt)
{
  RECT r = clientrt;
  InflateRect(&r, -2, -2);
  bool vertical = (ELToLower(pSettings->GetDirectionOrientation()) == TEXT("vertical"));
  // percent
  if (status.BatteryLifePercent < 101)
  {
    if (vertical)
    {
      int height = r.bottom - r.top;
      height = height * status.BatteryLifePercent / 100;
      r.top = r.bottom - height;
    }
    else
    {
      int width = r.right - r.left;
      width = width * status.BatteryLifePercent / 100;
      r.right = r.left + width;
    }
    EGFillRect(hdc, &r, guiInfo.alphaForeground, guiInfo.colorForeground);
  }

  CopyRect(&r, &clientrt);
  // status
  WCHAR text[10];
  if (status.BatteryFlag & BATTERY_FLAG_CHARGING)
  {
    wcscpy(text, TEXT("+"));
  }
  else if (status.ACLineStatus & AC_LINE_ONLINE)
  {
    wcscpy(text, TEXT("="));
  }
  else
  {
    wcscpy(text, TEXT("-"));
  }
  //
  if (vertical)
  {
    wcscat(text, TEXT("\n"));
  }
  else
  {
    wcscat(text, TEXT(" "));
  }
  //
  WCHAR pct[5];
  if (status.BatteryLifePercent <= 100)
  {
    _itow(status.BatteryLifePercent, pct, 10);
    wcscat(pct, TEXT("%"));
    wcscat(text, pct);
  }
  else
  {
    wcscpy(text, TEXT("AC Power"));
  }
  DrawStatusChar(hdc, text, r);
  //
  EGFrameRect(hdc, &clientrt, guiInfo.alphaFrame, guiInfo.colorFrame, 1);
  return 0;
}

void Applet::DrawStatusChar(HDC& hdc, WCHAR* text, RECT& clientrt)
{
  CLIENTINFO clientInfo;
  FORMATINFO formatInfo;
  clientInfo.hdc = hdc;
  CopyRect(&clientInfo.rt, &clientrt);
  clientInfo.bgAlpha = guiInfo.alphaBackground;

  formatInfo.font = mainFont;
  formatInfo.color = guiInfo.colorFont;
  formatInfo.flags = 0;
  if (ELToLower(pSettings->GetHorizontalAlign()) == TEXT("center"))
  {
    formatInfo.horizontalAlignment = EGDAT_HCENTER;
  }
  else if (ELToLower(pSettings->GetHorizontalAlign()) == TEXT("right"))
  {
    formatInfo.horizontalAlignment = EGDAT_RIGHT;
  }
  else
  {
    formatInfo.horizontalAlignment = EGDAT_LEFT;
  }
  if (ELToLower(pSettings->GetVerticalAlign()) == TEXT("center"))
  {
    formatInfo.verticalAlignment = EGDAT_VCENTER;
  }
  else if (ELToLower(pSettings->GetVerticalAlign()) == TEXT("bottom"))
  {
    formatInfo.verticalAlignment = EGDAT_BOTTOM;
  }
  else
  {
    formatInfo.verticalAlignment = EGDAT_TOP;
  }

  EGDrawAlphaText(guiInfo.alphaText, clientInfo, formatInfo, text);
}

LRESULT Applet::DoTimer(UINT_PTR timerID)
{
  if (timerID == MOUSE_TIMER)
  {
    return BaseApplet::DoTimer(timerID);
  }
  else
  {
    UpdateStatus();
  }

  return 1;
}

void Applet::UpdateStatus()
{
  // get new power status
  GetSystemPowerStatus(&status);

  // create tooltip text
  WCHAR tip[MAX_PATH];
  WCHAR buf[MAX_PATH];
  tip[0] = 0;
  if (status.BatteryLifeTime != (DWORD) - 1)
  {
    int mins = status.BatteryLifeTime / 60;
    _itow(mins / 60, tip, 10);
    wcscat(tip, TEXT(":"));
    _itow(mins % 60, buf, 10);
    wcscat(tip, buf);
    wcscat(tip, TEXT(" remaining "));
  }

  if (status.BatteryLifePercent <= 100)
  {
    _itow(status.BatteryLifePercent, buf, 10);
    wcscat(tip, buf);
    wcscat(tip, TEXT("%"));
  }

  if (status.BatteryFlag & BATTERY_FLAG_CHARGING)
  {
    wcscat(tip, TEXT(" Charging"));
  }
  else if (status.ACLineStatus & AC_LINE_ONLINE)
  {
    wcscat(tip, TEXT(" On AC power"));
  }
  else
  {
    wcscat(tip, TEXT(" Discharging"));
  }


  // update tooltip
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
  exists = SendMessage(toolWnd, TTM_GETTOOLINFO, 0, (LPARAM) (LPTOOLINFO) &ti) ? true : false;

  //  complete the rest of the TOOLINFO structure
  ti.hinst = mainInst;
  ti.lpszText = tip;
  ti.rect = rect;

  // If it exists, modify the tooltip, if not add it
  if (exists)
  {
    SendMessage(toolWnd, TTM_SETTOOLINFO, 0, (LPARAM)(LPTOOLINFO)&ti);
  }
  else
  {
    SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
  }

  DrawAlphaBlend();
}

void Applet::AppletUpdate()
{
  if (mainFont != NULL)
  {
    DeleteObject(mainFont);
  }
  mainFont = CreateFontIndirect(pSettings->GetFont());
}

void Applet::ShowConfig()
{
  Config config(mainInst, mainWnd, appletName, pSettings);
  if (config.Show() == IDOK)
  {
    // restart timer
    UINT powerID = (ULONG_PTR)mainWnd + 100;
    KillTimer(mainWnd, powerID);
    SetTimer(mainWnd, powerID, pSettings->GetUpdateInterval() * 1000, NULL);

    UpdateGUI();
  }
}
