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

#include "StickyPage.h"

INT_PTR CALLBACK StickyPage::StickyPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static StickyPage *pStickyPage = NULL;
  PROPSHEETPAGE *psp;

  switch (message)
    {
    case WM_INITDIALOG:
      psp = (PROPSHEETPAGE*)lParam;
      pStickyPage = reinterpret_cast<StickyPage*>(psp->lParam);
      if (!pStickyPage)
        break;
      return pStickyPage->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      if (!pStickyPage)
        break;
      return pStickyPage->DoCommand(hwndDlg, wParam, lParam);

    case WM_NOTIFY:
      if (!pStickyPage)
        break;
      return pStickyPage->DoNotify(hwndDlg, lParam);
    }

  return FALSE;
}

StickyPage::StickyPage(HINSTANCE hInstance, std::tr1::shared_ptr<Settings> pSettings)
{
  this->hInstance = hInstance;
  this->pSettings = pSettings;

  InitCommonControls();

  toolWnd = CreateWindowEx(
              0,
              TOOLTIPS_CLASS,
              NULL,
              TTS_ALWAYSTIP|WS_POPUP|TTS_NOPREFIX,
              CW_USEDEFAULT, CW_USEDEFAULT,
              CW_USEDEFAULT, CW_USEDEFAULT,
              NULL,
              NULL,
              hInstance,
              NULL);

  if (toolWnd)
    {
      SendMessage(toolWnd, TTM_SETMAXTIPWIDTH, 0, 300);
      SetWindowPos(toolWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
                   SWP_NOACTIVATE);
    }

  ExtractIconEx(TEXT("emergeIcons.dll"), 2, NULL, &addIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 5, NULL, &editIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 3, NULL, &delIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 18, NULL, &fileIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 9, NULL, &saveIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 1, NULL, &abortIcon, 1);
}

StickyPage::~StickyPage()
{
  if (addIcon)
    DestroyIcon(addIcon);
  if (editIcon)
    DestroyIcon(editIcon);
  if (delIcon)
    DestroyIcon(delIcon);
  if (fileIcon)
    DestroyIcon(fileIcon);
  if (saveIcon)
    DestroyIcon(saveIcon);
  if (abortIcon)
    DestroyIcon(abortIcon);

  DestroyWindow(toolWnd);
}

BOOL StickyPage::DoInitDialog(HWND hwndDlg)
{
  int iRet;
  LVCOLUMN lvCol;
  HWND listWnd = GetDlgItem(hwndDlg, IDC_STICKYLIST);
  HWND addWnd = GetDlgItem(hwndDlg, IDC_ADDAPP);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITAPP);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELAPP);
  HWND fileWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);
  HWND appWnd = GetDlgItem(hwndDlg, IDC_APPLICATION);
  TOOLINFO ti;
  DWORD dwRet;

  edit = false;
  saveCount = 0;
  deleteCount = 0;

  ZeroMemory(&ti, sizeof(TOOLINFO));

  if (addIcon)
    SendMessage(addWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)addIcon);
  if (editIcon)
    SendMessage(editWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)editIcon);
  if (delIcon)
    SendMessage(delWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)delIcon);
  if (fileIcon)
    SendMessage(fileWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
  if (saveIcon)
    SendMessage(saveWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)saveIcon);
  if (abortIcon)
    SendMessage(abortWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)abortIcon);

  EnableWindow(editWnd, false);
  EnableWindow(delWnd, false);
  EnableWindow(fileWnd, false);
  EnableWindow(appWnd, false);
  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);

  lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
  lvCol.pszText = (WCHAR*)TEXT("Application");
  lvCol.cx = 160;

  iRet = ListView_InsertColumn(listWnd, 0, &lvCol);
  dwRet = ListView_SetExtendedListViewStyle(listWnd,  LVS_EX_FULLROWSELECT);

  if (pSettings->GetHideSticky())
    SendDlgItemMessage(hwndDlg, IDC_HIDESTICKY, BM_SETCHECK, BST_CHECKED, 0);

  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = addWnd;
  ti.uId = (ULONG_PTR)addWnd;
  ti.hinst = hInstance;
  ti.lpszText = (WCHAR*)TEXT("Add sticky application");
  GetClientRect(addWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = editWnd;
  ti.uId = (ULONG_PTR)editWnd;
  ti.lpszText = (WCHAR*)TEXT("Edit sticky application");
  GetClientRect(editWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = delWnd;
  ti.uId = (ULONG_PTR)delWnd;
  ti.lpszText = (WCHAR*)TEXT("Remove sticky application");
  GetClientRect(delWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = fileWnd;
  ti.uId = (ULONG_PTR)fileWnd;
  ti.lpszText = (WCHAR*)TEXT("Browse for an application");
  GetClientRect(fileWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = saveWnd;
  ti.uId = (ULONG_PTR)saveWnd;
  ti.lpszText = (WCHAR*)TEXT("Save Changes");
  GetClientRect(saveWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = abortWnd;
  ti.uId = (ULONG_PTR)abortWnd;
  ti.lpszText = (WCHAR*)TEXT("Discard Changes");
  GetClientRect(abortWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  PopulateList(listWnd);

  return TRUE;
}

bool StickyPage::CheckSaveCount(HWND hwndDlg)
{
  if ((saveCount != 0) || (deleteCount != 0))
    {
      if (ELMessageBox(hwndDlg,
                       (WCHAR*)TEXT("All current modifications will be lost.  To save and exit press OK.\n\nDo you wish to continue?"),
                       (WCHAR*)TEXT("emergeVWM"),
                       ELMB_YESNO|ELMB_ICONQUESTION|ELMB_MODAL) == IDYES)
        return true;
      else
        return false;
    }

  return true;
}

bool StickyPage::CheckFields(HWND hwndDlg)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  HWND appText = GetDlgItem(hwndDlg, IDC_APPLICATION);

  if (!IsWindowEnabled(appText))
    return true;

  if (GetDlgItemText(hwndDlg, IDC_APPLICATION, tmp, MAX_LINE_LENGTH) != 0)
    {
      if (MessageBox(hwndDlg, TEXT("The current application will be lost.\n\nDo you wish to continue?"),
                     TEXT("emergeVWM"), MB_TOPMOST | MB_YESNO | MB_ICONQUESTION) == IDYES)
        return true;
      else
        return false;
    }

  return true;
}

BOOL StickyPage::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  switch (LOWORD(wParam))
    {
    case IDC_DELAPP:
      return DoDelete(hwndDlg);
    case IDC_EDITAPP:
      return DoEdit(hwndDlg);
    case IDC_ADDAPP:
      return DoAdd(hwndDlg);
    case IDC_SAVEAPP:
      return DoSave(hwndDlg);
    case IDC_ABORTAPP:
      return DoAbort(hwndDlg);
    case IDC_BROWSE:
      return DoBrowse(hwndDlg);
    }

  return FALSE;
}

bool StickyPage::DoEdit(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_STICKYLIST);
  HWND fileWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);
  HWND appWnd = GetDlgItem(hwndDlg, IDC_APPLICATION);

  if (ListView_GetSelectedCount(listWnd) > 1)
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("You can only edit one item at a time."),
                   (WCHAR*)TEXT("emergeLauncher"), ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);

      return false;
    }

  edit = true;
  EnableWindow(saveWnd, true);
  EnableWindow(abortWnd, true);
  EnableWindow(appWnd, true);
  EnableWindow(fileWnd, true);

  return true;
}

bool StickyPage::UpdateSettings(HWND hwndDlg)
{
  WCHAR stickyApp[MAX_LINE_LENGTH];
  HWND listWnd = GetDlgItem(hwndDlg, IDC_STICKYLIST);

  if (SendDlgItemMessage(hwndDlg, IDC_HIDESTICKY, BM_GETCHECK, 0, 0) == BST_CHECKED)
    pSettings->SetHideSticky(true);
  else if (SendDlgItemMessage(hwndDlg, IDC_HIDESTICKY, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    pSettings->SetHideSticky(false);

  if ((saveCount != 0) || (deleteCount != 0))
    {
      int i = 0;
      while (pSettings->GetStickyListSize() != 0)
        pSettings->DeleteStickyListItem(0);

      while (i < ListView_GetItemCount(listWnd))
        {
          ZeroMemory(stickyApp, MAX_LINE_LENGTH);
          ListView_GetItemText(listWnd, i, 0, stickyApp, MAX_LINE_LENGTH);
          pSettings->AddStickyListItem(stickyApp);

          i++;
        }

      pSettings->WriteStickyList();
    }

  pSettings->WriteSettings();

  return true;
}

bool StickyPage::PopulateList(HWND listWnd)
{
  bool ret = false;
  LVITEM lvItem;
  int iRet;

  lvItem.mask = LVIF_TEXT;

  for (UINT i = 0; i < pSettings->GetStickyListSize(); i++)
    {
      ret = true;

      lvItem.iItem = i;
      lvItem.iSubItem = 0;
      lvItem.pszText = pSettings->GetStickyListItem(i);
      lvItem.cchTextMax = (int)wcslen(pSettings->GetStickyListItem(i));

      iRet = ListView_InsertItem(listWnd, &lvItem);
    }

  return ret;
}

bool StickyPage::DoDelete(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_STICKYLIST);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELAPP);
  bool ret = false;
  int prevItem = 0, i = 0;
  BOOL bRet;

  if (ListView_GetSelectedCount(listWnd) > 1)
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("You can only delete one item at a time."),
                   (WCHAR*)TEXT("emergeVWM"), ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);

      return ret;
    }

  while (i < ListView_GetItemCount(listWnd))
    {
      if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
        {
          ret = true;
          prevItem = ListView_GetNextItem(listWnd, i, LVNI_ABOVE);
          deleteCount++;
          bRet = ListView_DeleteItem(listWnd, i);

          ListView_SetItemState(listWnd, i, LVIS_SELECTED,
                                LVIS_SELECTED);
          if (!ListView_EnsureVisible(listWnd, i, FALSE))
            {
              if (prevItem != -1)
                {
                  ListView_SetItemState(listWnd, prevItem, LVIS_SELECTED,
                                        LVIS_SELECTED);
                  bRet = ListView_EnsureVisible(listWnd, prevItem, FALSE);
                }
            }

          break;
        }
      else
        i++;
    }

  if (ListView_GetItemCount(listWnd) == 0)
    EnableWindow(delWnd, false);

  return ret;
}

bool StickyPage::DoAdd(HWND hwndDlg)
{
  HWND fileWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);
  HWND appWnd = GetDlgItem(hwndDlg, IDC_APPLICATION);

  EnableWindow(saveWnd, true);
  EnableWindow(abortWnd, true);
  EnableWindow(appWnd, true);
  EnableWindow(fileWnd, true);

  return true;
}

bool StickyPage::DoAbort(HWND hwndDlg)
{
  HWND fileWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);
  HWND appWnd = GetDlgItem(hwndDlg, IDC_APPLICATION);

  SetDlgItemText(hwndDlg, IDC_APPLICATION, TEXT(""));

  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);
  EnableWindow(appWnd, false);
  EnableWindow(fileWnd, false);
  edit = false;

  return true;
}

bool StickyPage::DoSave(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_STICKYLIST);
  HWND fileWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);
  HWND appWnd = GetDlgItem(hwndDlg, IDC_APPLICATION);
  bool ret = false;
  LVFINDINFO lvFI;
  LVITEM lvItem;
  WCHAR tmp[MAX_LINE_LENGTH], error[MAX_LINE_LENGTH];

  ZeroMemory(tmp, MAX_LINE_LENGTH);

  lvFI.flags = LVFI_STRING;
  lvItem.mask = LVIF_TEXT;

  GetDlgItemText(hwndDlg, IDC_APPLICATION, tmp, MAX_LINE_LENGTH);
  if (wcslen(tmp) > 0)
    {
      lvFI.psz = tmp;

      if (ListView_FindItem(listWnd, 0, &lvFI) == -1)
        {
          lvItem.iSubItem = 0;
          lvItem.pszText = tmp;
          lvItem.cchTextMax = (int)wcslen(tmp);

          if (edit)
            {
              int i = 0;
              while (i < ListView_GetItemCount(listWnd))
                {
                  if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
                    break;

                  i++;
                }
              if (ListView_DeleteItem(listWnd, i))
                lvItem.iItem = i;
            }
          else
            lvItem.iItem = ListView_GetItemCount(listWnd);

          if (ListView_InsertItem(listWnd, &lvItem) != -1)
            {
              saveCount++;

              ret = true;
            }
        }
      else
        {
          swprintf(error, TEXT("%s is already in the sticky application list"), tmp);
          ELMessageBox(GetDesktopWindow(), error, (WCHAR*)TEXT("emergeVWM"),
                       ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
        }

      SetDlgItemText(hwndDlg, IDC_APPLICATION, TEXT(""));
    }

  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);
  EnableWindow(appWnd, false);
  EnableWindow(fileWnd, false);
  edit = false;

  return ret;
}

bool StickyPage::DoBrowse(HWND hwndDlg)
{
  bool ret = false;
  OPENFILENAME ofn;
  WCHAR tmp[MAX_PATH];

  ZeroMemory(tmp, MAX_PATH);
  ZeroMemory(&ofn, sizeof(ofn));

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hwndDlg;
  ofn.lpstrFilter = TEXT("All Files (*.*)\0*.*\0");
  ofn.lpstrFile = tmp;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrTitle = TEXT("Browse For File");
  ofn.lpstrDefExt = NULL;
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS;

  if (GetOpenFileName(&ofn))
    {
      SetDlgItemText(hwndDlg, IDC_APPLICATION, PathFindFileName(tmp));

      ret = true;
    }

  return ret;
}

BOOL StickyPage::DoNotify(HWND hwndDlg, LPARAM lParam)
{
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELAPP);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITAPP);
  WCHAR appName[MAX_LINE_LENGTH];
  HWND listWnd = GetDlgItem(hwndDlg, IDC_STICKYLIST);
  int itemIndex;

  switch (((LPNMITEMACTIVATE)lParam)->hdr.code)
    {
    case LVN_ITEMCHANGED:
      EnableWindow(delWnd, true);
      EnableWindow(editWnd, true);
      itemIndex = ((LPNMLISTVIEW)lParam)->iItem;
      if (ListView_GetItemState(listWnd, itemIndex, LVIS_SELECTED))
        {
          ListView_GetItemText(listWnd, itemIndex, 0, appName, MAX_LINE_LENGTH);
          SetDlgItemText(hwndDlg, IDC_APPLICATION, appName);
        }
      return TRUE;

    case PSN_APPLY:
      if (CheckFields(hwndDlg) && UpdateSettings(hwndDlg))
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
      else
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID);
      return TRUE;

    case PSN_SETACTIVE:
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
      return TRUE;

    case PSN_KILLACTIVE:
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
      return TRUE;

    case PSN_QUERYCANCEL:
      if (CheckFields(hwndDlg) && CheckSaveCount(hwndDlg))
        SetWindowLong(hwndDlg,DWLP_MSGRESULT,FALSE);
      else
        SetWindowLong(hwndDlg,DWLP_MSGRESULT,TRUE);
      return TRUE;
    }

  return FALSE;
}
