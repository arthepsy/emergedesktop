// vim: tags+=../emergeLib/tags
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

#include "TrayIcon.h"
#include "Balloon.h"

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

Balloon::Balloon(HINSTANCE hInstance, TrayIcon *pTrayIcon)
{
  mainInst = hInstance;
  this->pTrayIcon = pTrayIcon;
}

Balloon::~Balloon()
{
}

LRESULT Balloon::DoPaint()
{
  PAINTSTRUCT ps;
  RECT balloonRect, titleRect;
  HBRUSH bgBrush = GetSysColorBrush(COLOR_WINDOW), frameBrush = CreateSolidBrush(RGB(0,0,0));

  HDC hdc = BeginPaint(balloonWnd, &ps);

  if (hdc == NULL)
    return 1;

  GetClientRect(balloonWnd, &balloonRect);

  FillRect(hdc, &balloonRect, bgBrush);
  FrameRect(hdc, &balloonRect, frameBrush);

  InflateRect(&balloonRect, -4, -4);
  CopyRect(&titleRect, &balloonRect);
  titleRect.bottom = 20;
  DrawText(hdc, infoTitle, -1, &titleRect, DT_SINGLELINE);

  balloonRect.top = titleRect.bottom + 10;
  DrawText(hdc, info, -1, &balloonRect, DT_WORDBREAK);

  EndPaint(balloonWnd, &ps);

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

void Balloon::SetInfo(WCHAR *info)
{
  wcscpy(this->info, info);
}

void Balloon::SetInfoTitle(WCHAR *infoTitle)
{
  wcscpy(this->infoTitle, infoTitle);
}

void Balloon::SetInfoFlags(DWORD infoFlags)
{
  this->infoFlags = infoFlags;
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
  pTrayIcon->SendMessage(NIN_BALLOONSHOW);
  SetTimer(balloonWnd, BALLOON_TIMER_ID, 10000, NULL);
  return (SetWindowPos(balloonWnd, HWND_TOPMOST, showPt.x - (250/2), showPt.y-100, 250, 100, SWP_SHOWWINDOW) == TRUE);
}
