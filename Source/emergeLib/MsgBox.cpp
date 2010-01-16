//----  --------------------------------------------------------------------------------------------------------
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
//----  --------------------------------------------------------------------------------------------------------

#include "MsgBox.h"
#include <windowsx.h>
#include <stdio.h>

MsgBox *pMsgBox;

BOOL CALLBACK MsgBox::MsgBoxDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
    {
    case WM_INITDIALOG:
      return pMsgBox->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pMsgBox->DoCommand(hwndDlg, wParam, lParam);
    }

  return FALSE;
}

MsgBox::MsgBox(HINSTANCE hInstance, HWND mainWnd, const WCHAR *messageText, const WCHAR *messageTitle, DWORD iconType, DWORD buttonType)
{
  pMsgBox = this;
  (*this).hInstance = hInstance;
  (*this).mainWnd = mainWnd;
  msgIcon = NULL;
  modal = false;

  wcscpy(msgText, messageText);
  wcscpy(msgTitle, messageTitle);

  msgButtons = buttonType;

  switch (iconType)
    {
    case ELMB_ICONWARNING:
      msgIcon = LoadIcon(NULL, IDI_WARNING);
      sound = (WCHAR*)SND_ALIAS_SYSTEMASTERISK;
      break;
    case ELMB_ICONQUESTION:
      msgIcon = LoadIcon(NULL, IDI_QUESTION);
      sound = (WCHAR*)SND_ALIAS_SYSTEMQUESTION;
      break;
    case ELMB_ICONERROR:
      msgIcon = LoadIcon(NULL, IDI_ERROR);
      sound = (WCHAR*)SND_ALIAS_SYSTEMHAND;
      break;
    }
}

MsgBox::~MsgBox()
{
  if (msgIcon)
    DestroyIcon(msgIcon);
}

int MsgBox::Show(bool modal)
{
  int ret = IDOK;
  this->modal = modal;

  PlaySound(sound, NULL, SND_ALIAS_ID|SND_ASYNC);

  if (modal || (msgButtons == ELMB_YESNO))
    ret = DialogBox(hInstance, MAKEINTRESOURCE(IDD_MSGBOX), mainWnd, (DLGPROC)MsgBoxDlgProc);
  else
    {
      HWND msgWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MSGBOX), mainWnd, (DLGPROC)MsgBoxDlgProc);
      HHOOK msgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, NULL, GetCurrentThreadId());
      hookMap.insert(std::pair<HWND, HHOOK>(msgWnd, msgHook));
    }

  return ret;
}

LRESULT MsgBox::DoInitDialog(HWND hwndDlg)
{
  RECT rect, buttonRect, textRect, iconRect, msgTextRect = {0, 0, 0, 0};
  int x, y, xdelta, ydelta, icondelta, textWidth, textHeight;
  HWND textWnd = GetDlgItem(hwndDlg, IDC_MSGTEXT);
  HWND iconWnd = GetDlgItem(hwndDlg, IDC_MSGICON);
  HWND okWnd = GetDlgItem(hwndDlg, IDOK);
  HWND cancelWnd = GetDlgItem(hwndDlg, IDCANCEL);
  HDC hdc;
  HFONT oldFont;

  GetWindowRect(hwndDlg, &rect);
  GetClientRect(textWnd, &textRect);
  GetClientRect(iconWnd, &iconRect);

  hdc = GetDC(hwndDlg);
  oldFont = SelectFont(hdc, (HFONT)SendMessage(hwndDlg, WM_GETFONT, 0, 0));
  DrawTextEx(hdc, msgText, wcslen(msgText), &msgTextRect, DT_CALCRECT, NULL);
  textHeight = msgTextRect.bottom;
  textWidth = msgTextRect.right;
  SelectFont(hdc, oldFont);
  DeleteDC(hdc);

  SetWindowPos(textWnd, NULL, 0, 0, textWidth, textHeight, SWP_NOZORDER | SWP_NOMOVE);
  SetWindowText(textWnd, msgText);

  xdelta = textWidth - textRect.right;
  ydelta = textHeight - textRect.bottom;
  icondelta = iconRect.bottom - textRect.bottom;
  if (icondelta > ydelta)
    ydelta = icondelta;

  InflateRect(&rect, xdelta / 2 , ydelta / 2);

  x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
  y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);
  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, (rect.right - rect.left), (rect.bottom - rect.top), 0);

  GetClientRect(hwndDlg, &rect);
  if (msgButtons == ELMB_OK)
    {
      ShowWindow(cancelWnd, SW_HIDE);
      GetClientRect(okWnd, &buttonRect);
      x = ((rect.right - rect.left) / 2) - (buttonRect.right / 2);
      y = rect.bottom - buttonRect.bottom - 10;
      SetWindowPos(okWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
  else if (msgButtons == ELMB_YESNO)
    {
      SetWindowText(okWnd, TEXT("&Yes"));
      SetWindowText(cancelWnd, TEXT("&No"));
      GetClientRect(okWnd, &buttonRect);
      x = ((rect.right - rect.left) / 2) - buttonRect.right - 5;
      y = rect.bottom - (buttonRect.bottom - buttonRect.top) - 10;
      SetWindowPos(okWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
      x = ((rect.right - rect.left) / 2) + 5;
      SetWindowPos(cancelWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

  x = rect.right - iconRect.right - 5;
  y = 5;
  SetWindowPos(iconWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  ELStealFocus(hwndDlg);

  SetWindowText(hwndDlg, msgTitle);

  if (msgIcon)
    SendMessage(iconWnd, STM_SETIMAGE, IMAGE_ICON, (LPARAM)msgIcon);

  return TRUE;
}

LRESULT MsgBox::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  HookMap::iterator iter = hookMap.find(hwndDlg);
  LRESULT ret = FALSE;

  switch (LOWORD(wParam))
    {
    case IDCANCEL:
      if (msgButtons == ELMB_YESNO)
        wParam = IDNO;
      ret = TRUE;
      break;
    case IDOK:
      if (msgButtons == ELMB_YESNO)
        wParam = IDYES;
      ret = TRUE;
      break;
    }

  if (ret == TRUE)
    {
      if (modal || (msgButtons == ELMB_YESNO))
        EndDialog(hwndDlg, wParam);
      else
        {
          if (iter != hookMap.end())
            {
              UnhookWindowsHookEx(iter->second);
              hookMap.erase(iter);
            }
          DestroyWindow(hwndDlg);
        }
      if (hookMap.empty())
        delete this;
    }

  return ret;
}

bool MsgBox::IsHookMessage(LPMSG lpMsg)
{
  HookMap::iterator iter = hookMap.begin();

  while (iter != hookMap.end())
    {
      if (IsDialogMessage(iter->first, lpMsg))
        return true;
      iter++;
    }

  return false;
}

LRESULT CALLBACK MsgBox::GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  LPMSG lpMsg = (LPMSG) lParam;

  if ( nCode >= 0 && PM_REMOVE == wParam )
    {
      // Don't translate non-input events.
      if ( (lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST) )
        {
          if (pMsgBox->IsHookMessage(lpMsg))
            {
              // The value returned from this hookproc is ignored,
              // and it cannot be used to tell Windows the message has been handled.
              // To avoid further processing, convert the message to WM_NULL
              // before returning.
              lpMsg->message = WM_NULL;
              lpMsg->lParam  = 0;
              lpMsg->wParam  = 0;
            }
        }
    }

  return CallNextHookEx((HHOOK)WH_GETMESSAGE, nCode, wParam, lParam);
}
