// vim: tags+=../emergeLib/tags,../emergeGraphics/tags
//---
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
//---

#include "Balloon.h"
#include "TrayIcon.h"

WCHAR balloonName[] = TEXT("emergeTrayBalloon");

LRESULT CALLBACK Balloon::BalloonProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  Balloon* pBalloon = NULL;

  if (message == WM_CREATE)
  {
    pBalloon = reinterpret_cast<Balloon*>(((CREATESTRUCT*)lParam)->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pBalloon);
    return DefWindowProc(hwnd, message, wParam, lParam);
  }
  else
  {
    pBalloon = reinterpret_cast<Balloon*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  if (pBalloon == NULL)
  {
    return DefWindowProc(hwnd, message, wParam, lParam);
  }

  switch (message)
  {
  case WM_LBUTTONDOWN:
    return pBalloon->DoLButtonDown();
  case WM_RBUTTONDOWN:
    return pBalloon->Hide();
  }

  return DefWindowProc(hwnd, message, wParam, lParam);
}

Balloon::Balloon(HINSTANCE hInstance, TrayIcon* pTrayIcon, Settings* pSettings)
{
  mainInst = hInstance;
  this->pSettings = pSettings;

  // Retrieve info from pTrayIcon to accomodate SendMessage
  trayIconWnd = pTrayIcon->GetWnd();
  trayIconID = pTrayIcon->GetID();
  trayIconVersion = pTrayIcon->GetIconVersion();
  trayIconCallbackMessage = pTrayIcon->GetCallback();
  CopyRect(&trayIconRect, pTrayIcon->GetRect());

  wcscpy(info, TEXT("\0"));
  wcscpy(infoTitle, TEXT("\0"));
  infoFlags = 0;
  iconWidth = 0;
  iconHeight = 0;
  ZeroMemory(&titleRect, sizeof(RECT));
  ZeroMemory(&infoRect, sizeof(RECT));
  icon = NULL;
  balloonWnd = NULL;
  showThread = NULL;
  showID = 0;
  showPt.x = 0;
  showPt.y = 0;
}

Balloon::~Balloon()
{
  DestroyIcon(icon);
}

LRESULT Balloon::DoLButtonDown()
{
  ShowWindow(balloonWnd, SW_HIDE);
  SendMessage(NIN_BALLOONUSERCLICK);
  TerminateThread(showThread, 0);
  return 0;
}

void Balloon::SetIconVersion(UINT iconVersion)
{
  trayIconVersion = iconVersion;
}

void Balloon::SetCallbackMessage(UINT callbackMessage)
{
  trayIconCallbackMessage = callbackMessage;
}

void Balloon::SetIconRect(RECT rect)
{
  CopyRect(&trayIconRect, &rect);
}

// This is a duplication of TrayIcon::SendMessage().  There is a race condition
// that can occur with DoTimer if TrayIcon::SendMessage() is called at the
// time that TrayIcon is being destroyed.
BOOL Balloon::SendMessage(LPARAM lParam)
{
  if (trayIconVersion == NOTIFYICON_VERSION_4)
  {
    POINT messagePt;

    messagePt.x = trayIconRect.left;
    messagePt.y = trayIconRect.top;
    ClientToScreen(balloonWnd, &messagePt);

    return SendNotifyMessage(trayIconWnd, trayIconCallbackMessage,
                             MAKEWPARAM(messagePt.x, messagePt.y),
                             MAKELPARAM(lParam, trayIconID));
  }

  return SendNotifyMessage(trayIconWnd, trayIconCallbackMessage,
                           WPARAM(trayIconID), lParam);
}

bool Balloon::SetInfo(WCHAR* info)
{
  bool ret = false;
  HFONT infoFont = CreateFontIndirect(pSettings->GetInfoFont());

  wcscpy(this->info, info);

  if (IsRectEmpty(&titleRect))
  {
    infoRect.top = 0;
  }
  else
  {
    infoRect.top = iconHeight;
    if (titleRect.bottom > iconHeight)
    {
      infoRect.top = titleRect.bottom;
    }
  }
  infoRect.top += 5;
  infoRect.left = 5;
  infoRect.bottom = infoRect.top;
  infoRect.right = infoRect.left + 220;
  if (titleRect.right > infoRect.right)
  {
    infoRect.right = titleRect.right;
  }

  ret = EGGetTextRect(info, infoFont, &infoRect, DT_WORDBREAK);

  DeleteObject(infoFont);

  return ret;
}

bool Balloon::SetInfoTitle(WCHAR* infoTitle)
{
  bool ret = false;
  HFONT infoTitleFont = CreateFontIndirect(pSettings->GetInfoTitleFont());

  wcscpy(this->infoTitle, infoTitle);

  if (!wcslen(infoTitle))
  {
    SetRectEmpty(&titleRect);
  }
  else
  {
    titleRect.top = 5;
    titleRect.left = 5;
    if (icon)
    {
      titleRect.left += iconWidth + 5;
    }
    titleRect.bottom = titleRect.top;
    titleRect.right = titleRect.left;
  }

  ret = EGGetTextRect(infoTitle, infoTitleFont, &titleRect, DT_SINGLELINE);

  DeleteObject(infoTitleFont);

  return ret;
}

bool Balloon::SetInfoFlags(DWORD infoFlags, HICON infoIcon)
{
  HICON tmpIcon = NULL;

  if (this->infoFlags == infoFlags)
  {
    return false;
  }

  this->infoFlags = infoFlags;

  if ((infoFlags & NIIF_LARGE_ICON) == NIIF_LARGE_ICON)
  {
    iconWidth = GetSystemMetrics(SM_CXICON);
    iconHeight = GetSystemMetrics(SM_CYICON);
  }
  else
  {
    iconWidth = GetSystemMetrics(SM_CXSMICON);
    iconHeight = GetSystemMetrics(SM_CYSMICON);
  }

  if ((infoFlags & NIIF_NONE) == NIIF_NONE)
  {
    if (icon)
    {
      DestroyIcon(icon);
      icon = NULL;
    }
  }
  else if ((infoFlags & NIIF_INFO) == NIIF_INFO)
  {
    tmpIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(OIC_INFORMATION), IMAGE_ICON, iconWidth, iconHeight, LR_SHARED);
  }
  else if ((infoFlags & NIIF_WARNING) == NIIF_WARNING)
  {
    tmpIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(OIC_WARNING), IMAGE_ICON, iconWidth, iconHeight, LR_SHARED);
  }
  else if ((infoFlags & NIIF_ERROR) == NIIF_ERROR)
  {
    tmpIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(OIC_ERROR), IMAGE_ICON, iconWidth, iconHeight, LR_SHARED);
  }
  else if ((infoFlags & NIIF_USER) == NIIF_USER)
  {
    tmpIcon = CopyIcon(infoIcon);
  }

  if (tmpIcon)
  {
    if (icon)
    {
      DestroyIcon(icon);
    }
    icon = EGConvertIcon(tmpIcon, 255);
    DestroyIcon(tmpIcon);
  }

  // Disable this for now as it is not a correct implementation
  /*if ((infoFlags & NIIF_RESPECT_QUIET_TIME) == NIIF_RESPECT_QUIET_TIME)
    {
      if (GetTickCount() < 3600000)
        return false;
    }*/

  return true;
}

bool Balloon::Initialize()
{
  WNDCLASSEX wincl;

  if (!GetClassInfoEx(mainInst, balloonName, &wincl))
  {
    ZeroMemory(&wincl, sizeof(WNDCLASSEX));

    // Register the window class
    wincl.hInstance = mainInst;
    wincl.lpszClassName = balloonName;
    wincl.lpfnWndProc = BalloonProcedure;
    wincl.cbSize = sizeof (WNDCLASSEX);
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);

    if (!RegisterClassEx (&wincl))
    {
      return false;
    }
  }

  balloonWnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_LAYERED, balloonName, NULL, WS_POPUP,
                              0, 0, 0, 0, NULL, NULL, mainInst, reinterpret_cast<LPVOID>(this));
  if (!balloonWnd)
  {
    return false;
  }

  return true;
}

bool Balloon::Show(POINT showPt)
{
  SendMessage(NIN_BALLOONSHOW);

  this->showPt = showPt;

  DWORD threadState;
  GetExitCodeThread(showThread, &threadState);
  if (threadState != STILL_ACTIVE)
  {
    showThread = CreateThread(NULL, 0, ShowThreadProc, this, 0, &showID);
  }

  return true;
}

DWORD WINAPI Balloon::ShowThreadProc(LPVOID lpParameter UNUSED)
{
  Balloon* pBalloon = reinterpret_cast< Balloon* >(lpParameter);

  // Wait for 100 ms before showing the window to handle flicking
  WaitForSingleObject(GetCurrentThread(), 100);
  pBalloon->ShowBalloon();

  // Wait as per the timeout values before timing out the balloon
  WaitForSingleObject(GetCurrentThread(), 10000);
  pBalloon->TimeoutBalloon();

  return 0;
}

void Balloon::TimeoutBalloon()
{
  ShowWindow(balloonWnd, SW_HIDE);
  SendMessage(NIN_BALLOONTIMEOUT);
}

bool Balloon::ShowBalloon()
{
  int x, y, xoffset, width, height;
  HMONITOR balloonMonitor = MonitorFromWindow(balloonWnd, MONITOR_DEFAULTTONULL);
  MONITORINFO balloonMonitorInfo;
  balloonMonitorInfo.cbSize = sizeof(MONITORINFO);

  if (!GetMonitorInfo(balloonMonitor, &balloonMonitorInfo))
  {
    return false;
  }

  // If the string length of info.
  if (wcslen(info) == 0)
  {
    return false;
  }

  width = infoRect.right;
  if (wcslen(infoTitle) != 0)
  {
    if (titleRect.right > infoRect.right)
    {
      width = titleRect.right;
    }
  }
  width += 5;
  height = infoRect.bottom + 5;

  y = showPt.y - height;
  if (y < balloonMonitorInfo.rcMonitor.top)
  {
    y = showPt.y + ICON_SIZE;
  }

  x = showPt.x - (width / 2);
  xoffset = balloonMonitorInfo.rcMonitor.right - (x + width);
  if (xoffset < 0)
  {
    x += xoffset;
  }
  if (x < balloonMonitorInfo.rcMonitor.left)
  {
    x = balloonMonitorInfo.rcMonitor.left;
  }

  if (SetWindowPos(balloonWnd, HWND_TOPMOST, x, y, width, height, SWP_SHOWWINDOW))
  {
    DrawAlphaBlend();
  }

  return true;
}


LRESULT Balloon::Hide()
{
  SendMessage(NIN_BALLOONHIDE);
  ShowWindow(balloonWnd, SW_HIDE);
  TerminateThread(showThread, 0);

  return 0;
}

bool Balloon::DrawAlphaBlend()
{
  RECT clientrt, contentrt;
  POINT srcPt;
  SIZE wndSz;
  BLENDFUNCTION bf;
  CLIENTINFO clientInfo;
  FORMATINFO formatInfo;
  int alpha = (pSettings->GetAlpha() * 255) / 100;

  if (!GetClientRect(balloonWnd, &clientrt))
  {
    return false;
  }

  HDC hdc = CreateCompatibleDC(NULL);
  HBITMAP hbitmap = EGCreateBitmap(0x00, RGB(0, 0, 0), clientrt);
  HGDIOBJ hobj = SelectObject(hdc, hbitmap);

  CopyRect(&contentrt, &clientrt);
  EGFrameRect(hdc, &contentrt, 255, pSettings->GetBorderColor(), 1);
  InflateRect(&contentrt, -1, -1);
  if (ELToLower(pSettings->GetGradientMethod()) == TEXT("solid"))
  {
    EGFillRect(hdc, &clientrt, alpha, pSettings->GetGradientFrom());
  }
  else
    EGGradientFillRect(hdc, &contentrt, alpha, pSettings->GetGradientFrom(),
                       pSettings->GetGradientTo(), 0, pSettings->GetGradientMethod());

  formatInfo.horizontalAlignment = EGDAT_LEFT;
  formatInfo.verticalAlignment = EGDAT_TOP;
  formatInfo.font = CreateFontIndirect(pSettings->GetInfoFont());
  formatInfo.color = pSettings->GetTextColor();
  formatInfo.flags = DT_WORDBREAK;
  clientInfo.hdc = hdc;
  CopyRect(&clientInfo.rt, &infoRect);
  clientInfo.bgAlpha = alpha;
  EGDrawAlphaText(255, clientInfo, formatInfo, info);
  DeleteObject(formatInfo.font);

  formatInfo.font = CreateFontIndirect(pSettings->GetInfoTitleFont());
  formatInfo.flags = DT_SINGLELINE;
  CopyRect(&clientInfo.rt, &titleRect);
  EGDrawAlphaText(255, clientInfo, formatInfo, infoTitle);
  DeleteObject(formatInfo.font);

  if (icon)
  {
    DrawIconEx(hdc, 5, 5, icon, iconWidth, iconHeight, 0, NULL, DI_NORMAL);
  }

  bf.BlendOp = AC_SRC_OVER;
  bf.BlendFlags = 0;
  bf.AlphaFormat = AC_SRC_ALPHA;  // use source alpha
  bf.SourceConstantAlpha = 255;

  wndSz.cx = clientrt.right;
  wndSz.cy = clientrt.bottom;
  srcPt.x = 0;
  srcPt.y = 0;

  UpdateLayeredWindow(balloonWnd, NULL, NULL, &wndSz, hdc, &srcPt, 0, &bf, ULW_ALPHA);

  // do cleanup
  SelectObject(hdc, hobj);
  DeleteDC(hdc);
  DeleteObject(hbitmap);

  return true;
}

