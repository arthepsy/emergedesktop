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

#include "Balloon.h"

WCHAR balloonName[] = TEXT("emergeTrayBalloon");

LRESULT CALLBACK Balloon::BalloonProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  CREATESTRUCT *cs;
  static Balloon *pBalloon = NULL;

  if (message == WM_CREATE)
    {
      cs = (CREATESTRUCT*)lParam;
      pBalloon = reinterpret_cast<Balloon*>(cs->lpCreateParams);
      return DefWindowProc(hwnd, message, wParam, lParam);
    }

  switch (message)
  {
    case WM_LBUTTONDOWN:
      DestroyWindow(hwnd);
      break;
  }

  return DefWindowProc(hwnd, message, wParam, lParam);
}

Balloon::Balloon(HINSTANCE hInstance)
{
  mainInst = hInstance;
}

Balloon::~Balloon()
{
}

bool Balloon::Initialize()
{
  WNDCLASSEX wincl;
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

  balloonWnd = CreateWindowEx(WS_EX_TOOLWINDOW, balloonName, NULL, WS_POPUP,
                              0, 0, 0, 0, NULL, NULL, mainInst, reinterpret_cast<LPVOID>(this));
  if (!balloonWnd)
    return false;

  return true;
}

bool Balloon::Show(POINT showPt)
{
  return (SetWindowPos(balloonWnd, HWND_TOP, showPt.x, showPt.y-100, 100, 100, SWP_SHOWWINDOW) == TRUE);
}
