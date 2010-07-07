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
  SetWindowText(balloonWnd, infoTitle);
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
      wincl.hbrBackground = GetSysColorBrush(COLOR_WINDOW);

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
  SetTimer(balloonWnd, BALLOON_TIMER_ID, 5000, NULL);
  return (SetWindowPos(balloonWnd, HWND_TOP, showPt.x, showPt.y-100, 100, 100, SWP_SHOWWINDOW) == TRUE);
}
