// vim: tags+=../emergeLib/tags,../emergeGraphics/tags
//---
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
    case WM_PAINT:
      return pBalloon->DoPaint();
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
  ZeroMemory(&titleRect, sizeof(RECT));
  ZeroMemory(&infoRect, sizeof(RECT));
  icon = NULL;
}

Balloon::~Balloon()
{
  DestroyIcon(icon);
}

LRESULT Balloon::DoPaint()
{
  PAINTSTRUCT ps;
  RECT balloonRect;
  HBRUSH bgBrush = GetSysColorBrush(COLOR_WINDOW), frameBrush = CreateSolidBrush(RGB(0,0,0));
  HFONT infoFont = CreateFontIndirect(pSettings->GetInfoFont());
  HFONT infoTitleFont = CreateFontIndirect(pSettings->GetInfoFont());

  HDC hdc = BeginPaint(balloonWnd, &ps);

  if (hdc == NULL)
    return 1;

  GetClientRect(balloonWnd, &balloonRect);

  FillRect(hdc, &balloonRect, bgBrush);
  FrameRect(hdc, &balloonRect, frameBrush);

  if (icon)
    DrawIconEx(hdc, 5, 5, icon, 32, 32, 0, NULL, DI_NORMAL);

  DeleteObject(SelectObject(hdc, infoTitleFont));
  DrawText(hdc, infoTitle, -1, &titleRect, DT_SINGLELINE);
  DeleteObject(SelectObject(hdc, infoFont));
  DrawText(hdc, info, -1, &infoRect, DT_WORDBREAK);

  EndPaint(balloonWnd, &ps);

  DeleteObject(bgBrush);
  DeleteObject(frameBrush);
  DeleteObject(infoFont);
  DeleteObject(infoTitleFont);

  return 0;
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
  if (_wcsicmp(this->info, info) == 0)
    return false;

  HFONT infoFont = CreateFontIndirect(pSettings->GetInfoFont());

  wcscpy(this->info, info);

  infoRect.top = 30;
  infoRect.left = titleRect.left;
  infoRect.bottom = infoRect.top;
  infoRect.right = infoRect.left + 220;
  if (titleRect.right > infoRect.right)
    infoRect.right = titleRect.right;

  if (EGGetTextRect(info, infoFont, &infoRect, DT_WORDBREAK))
    {
      if (IsWindowVisible(balloonWnd))
        InvalidateRect(balloonWnd, &infoRect, TRUE);

      return true;
    }

  return false;
}

bool Balloon::SetInfoTitle(WCHAR *infoTitle)
{
  if (_wcsicmp(this->infoTitle, infoTitle) == 0)
    return false;

  HFONT infoTitleFont = CreateFontIndirect(pSettings->GetInfoFont());

  wcscpy(this->infoTitle, infoTitle);

  titleRect.top = 5;
  titleRect.left = 5;
  titleRect.bottom = titleRect.top;
  titleRect.right = titleRect.left;

  if (EGGetTextRect(infoTitle, infoTitleFont, &titleRect, DT_SINGLELINE))
    {
      if (IsWindowVisible(balloonWnd))
        InvalidateRect(balloonWnd, &titleRect, TRUE);

      return true;
    }

  return false;
}

bool Balloon::SetInfoFlags(DWORD infoFlags, HICON infoIcon)
{
  int offset = 0;

  if (this->infoFlags == infoFlags)
    return false;

  this->infoFlags = infoFlags;

  if ((infoFlags & NIIF_NONE) == NIIF_NONE)
    {
      if (icon)
        {
          DestroyIcon(icon);
          icon = NULL;
          offset = -37;
        }
    }

  if ((infoFlags & NIIF_INFO) == NIIF_INFO)
    {
      if (icon == NULL)
        offset = 37;
      else
        DestroyIcon(icon);
      icon = LoadIcon(NULL, IDI_INFORMATION);
    }

  if ((infoFlags & NIIF_WARNING) == NIIF_WARNING)
    {
      if (icon == NULL)
        offset = 37;
      else
        DestroyIcon(icon);
      icon = LoadIcon(NULL, IDI_WARNING);
    }

  if ((infoFlags & NIIF_ERROR) == NIIF_ERROR)
    {
      if (icon == NULL)
        offset = 37;
      else
        DestroyIcon(icon);
      icon = LoadIcon(NULL, IDI_ERROR);
    }

  if ((infoFlags & NIIF_USER) == NIIF_USER)
    {
      if (icon == NULL)
        offset = 37;
      else
        DestroyIcon(icon);
      icon = infoIcon;
    }

  if (offset != 0)
    {
      OffsetRect(&titleRect, offset, 0);
      OffsetRect(&infoRect, offset, 0);
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

  balloonWnd = CreateWindowEx(WS_EX_TOOLWINDOW, balloonName, NULL, WS_POPUP,
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

  width = infoRect.right + 5;
  height = infoRect.bottom + 5;
  if (height < 37)
    height = 37;

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

  return (SetWindowPos(balloonWnd, HWND_TOPMOST, x, y, width, height, SWP_SHOWWINDOW) == TRUE);
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

/*void Balloon::DrawAlphaBlend()
{
  HDC hdc, backgroundDC;
  RECT clientrt, contentrt;
  POINT srcPt;
  SIZE wndSz;
  BLENDFUNCTION bf;
  int dragBorder = guiInfo.dragBorder + guiInfo.bevelWidth + guiInfo.padding;

  CLIENTINFO clientInfo;
  FORMATINFO formatInfo;
  formatInfo.horizontalAlignment = EGDAT_LEFT;
  formatInfo.verticalAlignment = EGDAT_TOP;
  formatInfo.font = mainFont;
  formatInfo.color = pSettings->GetFontColour();

  clientInfo.hdc = hdc;
  CopyRect(&clientInfo.rt, &textRect);
  clientInfo.bgAlpha = guiInfo.alphaBackground;

  if (!GetClientRect(mainWnd, &clientrt))
    return;

  hdc = EGBeginPaint(mainWnd);
  CopyRect(&contentrt, &clientrt);
  EGFrameRect(backgroundDC, &contentrc, 100, pSettings->GetBorderColour(), 1);
  InflateRect(&contentrt, -1, -1);
  EGGradientFillRect(backgroundDC, &contentrt, pSettings->GetAlpha(), pSettings->GetGradientFrom(), pSettings->GetGradientTo(),
                     pSettings->GetBevel(), pSettings->GetGradientMethod());
  EGDrawAlphaText(100, clientInfo, formatInfo, infoTitle);
  EGDrawAlphaText(100, clientInfo, formatInfo, info);

  BitBlt(hdc, clientrt.left, clientrt.top, clientrt.right - clientrt.left, clientrt.bottom - clientrt.top,
         backgroundDC, 0, 0, SRCCOPY);

  bf.BlendOp = AC_SRC_OVER;
  bf.BlendFlags = 0;
  bf.AlphaFormat = AC_SRC_ALPHA;  // use source alpha
  bf.SourceConstantAlpha = guiInfo.alphaActive;

  wndSz.cx = clientrt.right;
  wndSz.cy = clientrt.bottom;
  srcPt.x = 0;
  srcPt.y = 0;

  UpdateLayeredWindow(balloonWnd, NULL, NULL, &wndSz, hdc, &srcPt, 0, &bf, ULW_ALPHA);

  // do cleanup
  EGEndPaint();
  DeleteDC(hdc);
  DeleteDC(backgroundDC);
}*/

