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

#include "Shutdown.h"
#include <stdio.h>

extern Shutdown* pShutdown;

BOOL CALLBACK Shutdown::ShutdownDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static Shutdown* pShutdown = NULL;

  switch (message)
  {
  case WM_INITDIALOG:
    pShutdown = reinterpret_cast<Shutdown*>(lParam);
    if (!pShutdown)
    {
      break;
    }
    return pShutdown->DoInitDialog(hwndDlg);

  case WM_COMMAND:
    return pShutdown->DoCommand(hwndDlg, wParam, lParam);

  case WM_NOTIFY:
    return pShutdown->DoNotify(hwndDlg, lParam);
  }

  return FALSE;
}

Shutdown::Shutdown(HINSTANCE hInstance, HWND mainWnd)
{
  this->hInstance = hInstance;
  this->mainWnd = mainWnd;

  enableDisconnect = false;

  hIconsDLL = ELLoadEmergeLibrary(TEXT("emergeIcons.dll"));

  logoBMP = LoadBitmap((HINSTANCE)hIconsDLL, MAKEINTRESOURCE(IDB_LOGO));
}

Shutdown::~Shutdown()
{
  if (logoBMP)
  {
    DeleteObject(logoBMP);
  }

  FreeLibrary(hIconsDLL);
}

int Shutdown::Show()
{
  return (int)DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SHUTDOWN), mainWnd, (DLGPROC)ShutdownDlgProc, (LPARAM)this);
}

BOOL Shutdown::DoInitDialog(HWND hwndDlg)
{
  RECT rect;
  int x, y;
#ifndef _W64
  LPTSTR pData = NULL;
  DWORD cbReturned = 0;
#endif

  rect = ELGetWindowRect(hwndDlg);

  x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
  y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);
  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
  ELStealFocus(hwndDlg);

  if (logoBMP)
  {
    SendDlgItemMessage(hwndDlg, IDC_LOGO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)logoBMP);
  }

  SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Turn Off"));
  SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Restart"));
  SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Stand By"));
  SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Hibernate"));
  SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Logoff"));
#ifndef _W64
  if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                                 WTS_CURRENT_SESSION, WTSConnectState, &pData, &cbReturned) && cbReturned == sizeof(int))
  {
    if (*((int*)pData) == WTSActive)
    {
      enableDisconnect = true;
      SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Disconnect"));
    }
  }
  WTSFreeMemory(pData);
#endif

  SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_SETCURSEL, (WPARAM)0, 0);

  return TRUE;
}

BOOL Shutdown::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  switch (LOWORD(wParam))
  {
  case IDOK:
    if (DoShutdown(hwndDlg))
    {
      EndDialog(hwndDlg, wParam);
    }
    return TRUE;
  case IDCANCEL:
    EndDialog(hwndDlg, wParam);
    return TRUE;
  }

  return FALSE;
}

bool Shutdown::DoShutdown(HWND hwndDlg)
{
  int choice;
  EXITFLAGS method;

  choice = (int)SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_GETCURSEL, 0, 0);

  switch (choice)
  {
  case 0:
    method = EMERGE_HALT;
    break;
  case 1:
    method = EMERGE_REBOOT;
    break;
  case 2:
    method = EMERGE_SUSPEND;
    break;
  case 3:
    method = EMERGE_HIBERNATE;
    break;
  case 4:
    method = EMERGE_LOGOFF;
    break;
  case 5:
    if (enableDisconnect)
    {
      method = EMERGE_DISCONNECT;
      break;
    }
  default:
    return false;
  }

  ELExit(method, false);

  return true;
}

BOOL Shutdown::DoNotify(HWND hwndDlg UNUSED, LPARAM lParam UNUSED)
{
  return FALSE;
}

