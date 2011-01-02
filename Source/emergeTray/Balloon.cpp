// vim: tags+=../emergeLib/tags,../emergeGraphics/tags
//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2011  The Emerge Desktop Development Team
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
  Balloon *pBalloon = NULL;

  if (message == WM_CREATE)
    {
      pBalloon = reinterpret_cast<Balloon*>(((CREATESTRUCT*)lParam)->lpCreateParams);
      SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)pBalloon);
      return DefWindowProc(hwnd, message, wParam, lParam);
    }
  else
    pBalloon = reinterpret_cast<Balloon*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

  if (pBalloon == NULL)
    return DefWindowProc(hwnd, message, wParam, lParam);

  switch (message)
    {
    case WM_LBUTTONDOWN:
      return pBalloon->DoLButtonDown();
    case WM_RBUTTONDOWN:
    case WM_TIMER:
      return pBalloon->DoTimer();
    }

  return DefWindowProc(hwnd, message, wParam, lParam);
}

Balloon::Balloon(HINSTANCE hInstance, TrayIcon *pTrayIcon, Settings *pSettings)
{
  mainInst = hInstance;
  this->pTrayIcon = pTrayIcon;
  this->pSettings = pSettings;

  wcscpy(info, TEXT("\0"));
  wcscpy(infoTitle, TEXT("\0"));
  infoFlags = 0;
  iconSize = 16;
  ZeroMemory(&titleRect, sizeof(RECT));
  ZeroMemory(&infoRect, sizeof(RECT));
  icon = NULL;
}

Balloon::~Balloon()
{
  DestroyIcon(icon);
}

LRESULT Balloon::DoLButtonDown()
{
  ShowWindow(balloonWnd, SW_HIDE);
  pTrayIcon->SendMessage(NIN_BALLOONUSERCLICK);
  KillTimer(balloonWnd, BALLOON_TIMER_ID);
  return 0;
}

LRESULT Balloon::DoTimer()
{
  ShowWindow(balloonWnd, SW_HIDE);
  pTrayIcon->SendMessage(NIN_BALLOONTIMEOUT);
  KillTimer(balloonWnd, BALLOON_TIMER_ID);
  return 0;
}

bool Balloon::SetInfo(WCHAR *info)
{
  bool ret = false;
  HFONT infoFont = CreateFontIndirect(pSettings->GetInfoFont());

  wcscpy(this->info, info);

  infoRect.top = iconSize;
  if (titleRect.bottom > iconSize)
    infoRect.top = titleRect.bottom;
  infoRect.top += 5;
  infoRect.left = 5;
  infoRect.bottom = infoRect.top;
  infoRect.right = infoRect.left + 220;
  if (titleRect.right > infoRect.right)
    infoRect.right = titleRect.right;

  ret = EGGetTextRect(info, infoFont, &infoRect, DT_WORDBREAK);

  DeleteObject(infoFont);

  return ret;
}

bool Balloon::SetInfoTitle(WCHAR *infoTitle)
{
  bool ret = false;
  HFONT infoTitleFont = CreateFontIndirect(pSettings->GetInfoTitleFont());

  wcscpy(this->infoTitle, infoTitle);

  titleRect.top = 5;
  titleRect.left = 5;
  if (icon)
    titleRect.left += iconSize + 5;
  titleRect.bottom = titleRect.top;
  titleRect.right = titleRect.left;

  ret = EGGetTextRect(infoTitle, infoTitleFont, &titleRect, DT_SINGLELINE);

  DeleteObject(infoTitleFont);

  return ret;
}

bool Balloon::SetInfoFlags(DWORD infoFlags, HICON infoIcon)
{
  HICON tmpIcon = NULL;

  if (this->infoFlags == infoFlags)
    return false;

  this->infoFlags = infoFlags;

  if ((infoFlags & NIIF_LARGE_ICON) == NIIF_LARGE_ICON)
    iconSize = 32;
  else
    iconSize = 16;

  if (infoFlags == NIIF_NONE)
    {
      if (icon)
        {
          DestroyIcon(icon);
          icon = NULL;
        }
    }
  else if ((infoFlags & NIIF_INFO) == NIIF_INFO)
    tmpIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(OIC_INFORMATION), IMAGE_ICON, iconSize, iconSize, LR_SHARED);
  else if ((infoFlags & NIIF_WARNING) == NIIF_WARNING)
    tmpIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(OIC_WARNING), IMAGE_ICON, iconSize, iconSize, LR_SHARED);
  else if ((infoFlags & NIIF_ERROR) == NIIF_ERROR)
    tmpIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(OIC_ERROR), IMAGE_ICON, iconSize, iconSize, LR_SHARED);
  else if ((infoFlags & NIIF_USER) == NIIF_USER)
    tmpIcon = infoIcon;

  if (tmpIcon)
    {
      if (icon)
        DestroyIcon(icon);
      icon = EGConvertIcon(tmpIcon, 255);
      DestroyIcon(tmpIcon);
    }

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
        return false;
    }

  balloonWnd = CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_LAYERED, balloonName, NULL, WS_POPUP,
                              0, 0, 0, 0, NULL, NULL, mainInst, reinterpret_cast<LPVOID>(this));
  if (!balloonWnd)
    return false;

  return true;
}

bool Balloon::Show(POINT showPt)
{
  int x, y, xoffset, width, height;
  HMONITOR balloonMonitor = MonitorFromWindow(balloonWnd, MONITOR_DEFAULTTONULL);
  MONITORINFO balloonMonitorInfo;
  balloonMonitorInfo.cbSize = sizeof(MONITORINFO);

  if (!GetMonitorInfo(balloonMonitor, &balloonMonitorInfo))
    return false;

  // If the string length of info or infoTitle is 0, do not display a balloon.
  if ((wcslen(info) == 0) || (wcslen(infoTitle) == 0))
    return false;

  width = infoRect.right;
  if (titleRect.right > infoRect.right)
    width = titleRect.right;
  width += 5;
  height = infoRect.bottom + 5;

  y = showPt.y - height;
  if (y < balloonMonitorInfo.rcMonitor.top)
    y = showPt.y + ICON_SIZE;

  x = showPt.x - (width / 2);
  xoffset = balloonMonitorInfo.rcMonitor.right - (x + width);
  if (xoffset < 0)
    x += xoffset;
  if (x < balloonMonitorInfo.rcMonitor.left)
    x = balloonMonitorInfo.rcMonitor.left;

  pTrayIcon->SendMessage(NIN_BALLOONSHOW);
  SetTimer(balloonWnd, BALLOON_TIMER_ID, 10000, NULL);

  if (SetWindowPos(balloonWnd, HWND_TOPMOST, x, y, width, height, SWP_SHOWWINDOW))
    return DrawAlphaBlend();

  return false;
}

bool Balloon::Hide()
{
  if (IsWindowVisible(balloonWnd))
    {
      pTrayIcon->SendMessage(NIN_BALLOONHIDE);
      ShowWindow(balloonWnd, SW_HIDE);

      return true;
    }

  return false;
}

bool Balloon::DrawAlphaBlend()
{
  RECT clientrt, contentrt;
  HDC hdc;
  POINT srcPt;
  SIZE wndSz;
  BLENDFUNCTION bf;
  CLIENTINFO clientInfo;
  FORMATINFO formatInfo;
  int alpha = (pSettings->GetAlpha() * 255) / 100;

  if (!GetClientRect(balloonWnd, &clientrt))
    return false;

  hdc = EGBeginPaint(balloonWnd);
  CopyRect(&contentrt, &clientrt);
  EGFrameRect(hdc, &contentrt, 255, pSettings->GetBorderColor(), 1);
  InflateRect(&contentrt, -1, -1);
  if (wcsicmp(pSettings->GetGradientMethod(), TEXT("Solid")) == 0)
    EGFillRect(hdc, &clientrt, alpha, pSettings->GetGradientFrom());
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
    DrawIconEx(hdc, 5, 5, icon, iconSize, iconSize, 0, NULL, DI_NORMAL);

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
  EGEndPaint();
  DeleteDC(hdc);

  return true;
}

