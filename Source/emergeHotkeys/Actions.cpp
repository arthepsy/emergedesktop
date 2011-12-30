// vim: tag+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
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
//----  --------------------------------------------------------------------------------------------------------

#include "Actions.h"
#include <windowsx.h>

BOOL CALLBACK Actions::ActionsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static Actions *pActions = NULL;

  switch (message)
    {
    case WM_INITDIALOG:
      pActions = reinterpret_cast<Actions*>(lParam);
      if (!pActions)
        break;
      return pActions->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pActions->DoCommand(hwndDlg, wParam, lParam);

    case WM_NOTIFY:
        {
          long lResult = pActions->DoNotify(hwndDlg, lParam);
          SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, lResult);
          return TRUE;
        }
    }

  return FALSE;
}

Actions::Actions(HINSTANCE hInstance, HWND mainWnd, std::tr1::shared_ptr<Settings> pSettings)
{
  this->hInstance = hInstance;
  this->mainWnd = mainWnd;
  this->pSettings = pSettings;
  toggleSort[0] = false;
  toggleSort[1] = false;
  wcscpy(myName, TEXT("Actions"));
  edit = false;
  dialogVisible = false;

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
  ExtractIconEx(TEXT("emergeIcons.dll"), 6, NULL, &folderIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 18, NULL, &fileIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 9, NULL, &saveIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 1, NULL, &abortIcon, 1);

  pSettings->GetSortInfo(myName, &lvSortInfo.sortInfo);
  toggleSort[lvSortInfo.sortInfo.subItem] = lvSortInfo.sortInfo.ascending;
}

Actions::~Actions()
{
  UnregisterHotkeyList();

  if (addIcon)
    DestroyIcon(addIcon);
  if (delIcon)
    DestroyIcon(delIcon);
  if (editIcon)
    DestroyIcon(editIcon);
  if (fileIcon)
    DestroyIcon(fileIcon);
  if (folderIcon)
    DestroyIcon(folderIcon);

  DestroyWindow(toolWnd);
}

void Actions::UnregisterHotkeyList()
{
  for (UINT i = 0; i < pSettings->GetHotkeyListSize(); i++)
    UnregisterHotKey(mainWnd, pSettings->GetHotkeyListItem(i)->GetHotkeyID());
}

void Actions::RegisterHotkeyList(bool showError)
{
  bool failedHotkeys = false;
  HotkeyCombo *pHotkeyCombo;

  for (UINT i = 0; i < pSettings->GetHotkeyListSize(); i++)
    {
      pHotkeyCombo = pSettings->GetHotkeyListItem(i);
      if (!RegisterHotKey(mainWnd, pHotkeyCombo->GetHotkeyID(),
                          pHotkeyCombo->GetHotkeyModifiers(),
                          pHotkeyCombo->GetHotkeyKey()))
        {
          failedHotkeys = true;
          pHotkeyCombo->SetValid(false);
        }
    }

  if (failedHotkeys && showError)
    {
      if (ELMessageBox(GetDesktopWindow(), TEXT("Some Hotkeys failed to register"),
                       (WCHAR*)TEXT("emergeHotkeys"),
                       ELMB_OK|ELMB_ICONERROR|ELMB_MODAL) == IDOK)
        Show();
    }
}

int Actions::Show()
{
  if (!dialogVisible)
    return (int)DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ACTIONS), mainWnd, (DLGPROC)ActionsDlgProc, (LPARAM)this);

  return 0;
}

BOOL Actions::DoInitDialog(HWND hwndDlg)
{
  RECT rect;
  LVCOLUMN lvCol;
  int x, y;
  TOOLINFO ti;

  dialogVisible = true;

  HWND listWnd = GetDlgItem(hwndDlg, IDC_ACTIONSLIST);
  HWND keyWnd = GetDlgItem(hwndDlg, IDC_KEY);
  HWND keyTextWnd = GetDlgItem(hwndDlg, IDC_KEYTEXT);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_COMMAND);
  HWND commandArgWnd = GetDlgItem(hwndDlg, IDC_COMMANDARG);
  HWND addWnd = GetDlgItem(hwndDlg, IDC_ADDAPP);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELAPP);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_MODAPP);
  HWND fileWnd = GetDlgItem(hwndDlg, IDC_FILE);
  HWND folderWnd = GetDlgItem(hwndDlg, IDC_FOLDER);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);
  HWND shiftWnd = GetDlgItem(hwndDlg, IDC_SHIFT);
  HWND ctrlWnd = GetDlgItem(hwndDlg, IDC_CTRL);
  HWND winWnd = GetDlgItem(hwndDlg, IDC_WIN);
  HWND altWnd = GetDlgItem(hwndDlg, IDC_ALT);
  HWND appWnd = GetDlgItem(hwndDlg, IDC_APPLICATION);
  HWND inWnd = GetDlgItem(hwndDlg, IDC_INTERNAL);
  HWND exWnd = GetDlgItem(hwndDlg, IDC_EXTERNAL);

  ZeroMemory(&ti, sizeof(TOOLINFO));
  ELGetWindowRect(hwndDlg, &rect);

  saveCount = 0;
  deleteCount = 0;

  x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
  y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);

  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);

  ELStealFocus(hwndDlg);
  SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0,  SWP_NOSIZE|SWP_NOMOVE);

  lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
  lvCol.pszText = (WCHAR*)TEXT("Key Combination");
  lvCol.cx = 160;
  (void)ListView_InsertColumn(listWnd, 0, &lvCol);

  lvCol.pszText = (WCHAR*)TEXT("Action");
  lvCol.cx = 160;
  (void)ListView_InsertColumn(listWnd, 1, &lvCol);

  (void)ListView_SetExtendedListViewStyle(listWnd,  LVS_EX_FULLROWSELECT);

  PopulateList(listWnd);
  PopulateKeys(keyWnd);
  ELPopulateInternalCommandList(commandWnd);

  SendDlgItemMessage(hwndDlg, IDC_EXTERNAL, BM_CLICK, 0, 0);

  pSettings->BuildList(mainWnd, true);

  if (addIcon)
    SendMessage(addWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)addIcon);
  if (delIcon)
    SendMessage(delWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)delIcon);
  if (editIcon)
    SendMessage(editWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)editIcon);
  if (fileIcon)
    SendMessage(fileWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
  if (folderIcon)
    SendMessage(folderWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)folderIcon);
  if (saveIcon)
    SendMessage(saveWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)saveIcon);
  if (abortIcon)
    SendMessage(abortWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)abortIcon);

  EnableWindow(delWnd, false);
  EnableWindow(editWnd, false);
  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);
  EnableWindow(keyWnd, false);
  EnableWindow(keyTextWnd, false);
  EnableWindow(commandWnd, false);
  EnableWindow(fileWnd, false);
  EnableWindow(folderWnd, false);
  EnableWindow(shiftWnd, false);
  EnableWindow(ctrlWnd, false);
  EnableWindow(winWnd, false);
  EnableWindow(altWnd, false);
  EnableWindow(appWnd, false);
  EnableWindow(commandWnd, false);
  EnableWindow(commandArgWnd, false);
  EnableWindow(inWnd, false);
  EnableWindow(exWnd, false);

  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = addWnd;
  ti.uId = (ULONG_PTR)addWnd;
  ti.hinst = hInstance;
  ti.lpszText = (WCHAR*)TEXT("Add Hotkey");
  GetClientRect(addWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = delWnd;
  ti.uId = (ULONG_PTR)delWnd;
  ti.lpszText = (WCHAR*)TEXT("Delete Hotkey");
  GetClientRect(delWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = editWnd;
  ti.uId = (ULONG_PTR)editWnd;
  ti.lpszText = (WCHAR*)TEXT("Edit Hotkey");
  GetClientRect(editWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = fileWnd;
  ti.uId = (ULONG_PTR)fileWnd;
  ti.lpszText = (WCHAR*)TEXT("Browse for a file");
  GetClientRect(fileWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = folderWnd;
  ti.uId = (ULONG_PTR)folderWnd;
  ti.lpszText = (WCHAR*)TEXT("Browse for a folder");
  GetClientRect(folderWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = saveWnd;
  ti.uId = (ULONG_PTR)saveWnd;
  ti.lpszText = (WCHAR*)TEXT("Save hotkey definition");
  GetClientRect(saveWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = abortWnd;
  ti.uId = (ULONG_PTR)abortWnd;
  ti.lpszText = (WCHAR*)TEXT("Discard hotkey definition");
  GetClientRect(abortWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  lvSortInfo.listWnd = listWnd;
  (void)ListView_SortItemsEx(listWnd, ListViewCompareProc, (LPARAM)&lvSortInfo);

  return TRUE;
}

int CALLBACK Actions::ListViewCompareProc (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  WCHAR szBuf1[MAX_LINE_LENGTH], szBuf2[MAX_LINE_LENGTH];
  PLISTVIEWSORTINFO si = (PLISTVIEWSORTINFO)lParamSort;

  ListView_GetItemText(si->listWnd, lParam1, si->sortInfo.subItem, szBuf1, MAX_LINE_LENGTH);
  ListView_GetItemText(si->listWnd, lParam2, si->sortInfo.subItem, szBuf2, MAX_LINE_LENGTH);

  if (si->sortInfo.ascending)
    return(wcscmp(szBuf1, szBuf2));
  else
    return(wcscmp(szBuf1, szBuf2) * -1);
}


bool Actions::CheckSaveCount(HWND hwndDlg)
{
  if ((saveCount != 0) || (deleteCount != 0))
    {
      if (ELMessageBox(hwndDlg,
                       (WCHAR*)TEXT("All current modifications will be lost.  To save and exit press OK.\n\nDo you wish to continue?"),
                       (WCHAR*)TEXT("emergeHotkeys"),
                       ELMB_YESNO|ELMB_ICONQUESTION|ELMB_MODAL) == IDYES)
        return true;
      else
        return false;
    }

  return true;
}

bool Actions::CheckFields(HWND hwndDlg)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  bool displayCheck = false;
  HWND applicationWnd = GetDlgItem(hwndDlg, IDC_APPLICATION);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_COMMAND);
  HWND externalWnd = GetDlgItem(hwndDlg, IDC_EXTERNAL);
  HWND internalWnd = GetDlgItem(hwndDlg, IDC_INTERNAL);

  if (!IsWindowEnabled(applicationWnd) && !IsWindowEnabled(commandWnd))
    return true;

  if ((SendMessage(externalWnd, BM_GETCHECK, 0, 0) == BST_CHECKED) &&
      IsWindowEnabled(externalWnd))
    {
      if (GetDlgItemText(hwndDlg, IDC_APPLICATION, tmp, MAX_LINE_LENGTH) != 0)
        displayCheck = true;
    }
  else if (IsWindowEnabled(internalWnd))
    {
      if (GetDlgItemText(hwndDlg, IDC_COMMAND, tmp, MAX_LINE_LENGTH) != 0)
        displayCheck = true;
    }

  if (displayCheck)
    {
      if (ELMessageBox(hwndDlg,
                       (WCHAR*)TEXT("The current hotkey definition will be lost.\n\nDo you wish to continue?"),
                       (WCHAR*)TEXT("emergeHotkeys"),
                       ELMB_YESNO|ELMB_ICONQUESTION|ELMB_MODAL) == IDYES)
        return true;
      else
        return false;
    }

  return true;
}

BOOL Actions::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  switch (LOWORD(wParam))
    {
    case IDOK:
      if (!CheckFields(hwndDlg))
        break;
      dialogVisible = false;
      return EndDialog(hwndDlg, wParam);

    case IDCANCEL:
      if (!CheckFields(hwndDlg))
        break;
      if (!CheckSaveCount(hwndDlg))
        break;
      return DoCancel(hwndDlg, wParam);

    case IDC_DELAPP:
      return DoDelete(hwndDlg);

    case IDC_ADDAPP:
      return DoAdd(hwndDlg);

    case IDC_SAVEAPP:
      return DoSave(hwndDlg);

    case IDC_ABORTAPP:
      return DoAbort(hwndDlg);

    case IDC_MODAPP:
      return DoModify(hwndDlg);

    case IDC_EXIT:
      if (CheckFields(hwndDlg))
        PostQuitMessage (0);
      return TRUE;

    case IDC_INTERNAL:
      return DoInternal(hwndDlg);

    case IDC_EXTERNAL:
      return DoExternal(hwndDlg);

    case IDC_FOLDER:
      return DoBrowse(hwndDlg, true);

    case IDC_FILE:
      return DoBrowse(hwndDlg, false);

    case IDC_ABOUT:
      return DoAbout();

      //    case IDC_KEY:
      //      return KeyCheck(hwndDlg, wParam, lParam);
    }

  return FALSE;
}

bool Actions::PopulateList(HWND listWnd)
{
  bool ret = false;
  LVITEM lvItem;

  lvItem.mask = LVIF_TEXT;

  for (UINT i = 0; i < pSettings->GetHotkeyListSize(); i++)
    {
      ret = true;

      lvItem.iItem = i;
      lvItem.iSubItem = 0;
      lvItem.pszText = pSettings->GetHotkeyListItem(i)->GetHotkeyString();
      lvItem.cchTextMax = (int)wcslen(pSettings->GetHotkeyListItem(i)->GetHotkeyString());
      (void)ListView_InsertItem(listWnd, &lvItem);
      ListView_SetItemText(listWnd, i, 1, pSettings->GetHotkeyListItem(i)->GetHotkeyAction());
    }

  return ret;
}

bool Actions::DoDelete(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_ACTIONSLIST);
  bool ret = false;
  int i = 0, prevItem = 0;
  UINT index;
  WCHAR tmpKey[MAX_LINE_LENGTH], tmpAction[MAX_LINE_LENGTH];

  if (ListView_GetSelectedCount(listWnd) > 1)
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("You can only delete one item at a time."),
                   (WCHAR*)TEXT("emergeHotkeys"), ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);

      return ret;
    }

  while (i < ListView_GetItemCount(listWnd))
    {
      if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
        {
          ret = true;
          prevItem = ListView_GetNextItem(listWnd, i, LVNI_ABOVE);
          deleteCount++;
          ListView_GetItemText(listWnd, i, 0, tmpKey, MAX_LINE_LENGTH);
          ListView_GetItemText(listWnd, i, 1, tmpAction, MAX_LINE_LENGTH);
          index = pSettings->FindHotkeyListItem(tmpKey, tmpAction);

          if (index < pSettings->GetHotkeyListSize())
            {
              UnregisterHotKey(mainWnd,
                               pSettings->GetHotkeyListItem(index)->GetHotkeyID());
              pSettings->DeleteHotkeyListItem(index);
            }

          (void)ListView_DeleteItem(listWnd, i);

          ListView_SetItemState(listWnd, i, LVIS_SELECTED,
                                LVIS_SELECTED);
          if (!ListView_EnsureVisible(listWnd, i, FALSE))
            {
              if (prevItem != -1)
                {
                  ListView_SetItemState(listWnd, prevItem, LVIS_SELECTED,
                                        LVIS_SELECTED);
                  (void)ListView_EnsureVisible(listWnd, prevItem, FALSE);
                }
            }

          break;
        }
      else
        i++;
    }

  if (ListView_GetItemCount(listWnd) == 0)
    EnableFields(hwndDlg, false);

  return ret;
}

bool Actions::DoModify(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_ACTIONSLIST);
  UINT i = 0;
  WCHAR tmpKey[MAX_LINE_LENGTH], tmpAction[MAX_LINE_LENGTH];

  if (ListView_GetSelectedCount(listWnd) > 1)
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("You can only delete one item at a time."),
                   (WCHAR*)TEXT("emergeHotkeys"), ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);

      return false;
    }

  while (i < pSettings->GetHotkeyListSize())
    {
      if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
        break;

      i++;
    }

  if (i < pSettings->GetHotkeyListSize())
    {
      ListView_GetItemText(listWnd, i, 0, tmpKey, MAX_LINE_LENGTH);
      ListView_GetItemText(listWnd, i, 1, tmpAction, MAX_LINE_LENGTH);

      editIndex = pSettings->FindHotkeyListItem(tmpKey, tmpAction);
      edit = true;

      EnableFields(hwndDlg, true);
    }

  return true;
}

bool Actions::PopulateFields(HWND hwndDlg, int modIndex)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_ACTIONSLIST);
  HWND keyWnd = GetDlgItem(hwndDlg, IDC_KEY);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_COMMAND);
  HWND internalWnd = GetDlgItem(hwndDlg, IDC_INTERNAL);
  HWND externalWnd = GetDlgItem(hwndDlg, IDC_EXTERNAL);
  int keyIndex = 0, commandIndex = 0;
  WCHAR tmpKey[MAX_LINE_LENGTH], tmpAction[MAX_LINE_LENGTH], tmp[MAX_LINE_LENGTH], *token;

  ListView_GetItemText(listWnd, modIndex, 0, tmpKey, MAX_LINE_LENGTH);
  wcscpy(tmp, tmpKey);

  SendDlgItemMessage(hwndDlg, IDC_WIN, BM_SETCHECK, BST_UNCHECKED, 0);
  SendDlgItemMessage(hwndDlg, IDC_ALT, BM_SETCHECK, BST_UNCHECKED, 0);
  SendDlgItemMessage(hwndDlg, IDC_CTRL, BM_SETCHECK, BST_UNCHECKED, 0);
  SendDlgItemMessage(hwndDlg, IDC_SHIFT, BM_SETCHECK, BST_UNCHECKED, 0);

  token = wcstok(tmp, TEXT("+"));

  while (token != NULL)
    {
      if (_wcsicmp(token, TEXT("win")) == 0)
        SendDlgItemMessage(hwndDlg, IDC_WIN, BM_SETCHECK, BST_CHECKED, 0);
      else if (_wcsicmp(token, TEXT("alt")) == 0)
        SendDlgItemMessage(hwndDlg, IDC_ALT, BM_SETCHECK, BST_CHECKED, 0);
      else if (_wcsicmp(token, TEXT("ctrl")) == 0)
        SendDlgItemMessage(hwndDlg, IDC_CTRL, BM_SETCHECK, BST_CHECKED, 0);
      else if (_wcsicmp(token, TEXT("shift")) == 0)
        SendDlgItemMessage(hwndDlg, IDC_SHIFT, BM_SETCHECK, BST_CHECKED, 0);
      else
        {
          keyIndex = (int)SendMessage(keyWnd, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)token);
          if (keyIndex != CB_ERR)
            SendMessage(keyWnd, CB_SETCURSEL, keyIndex, 0);

          break;
        }

      token = wcstok(NULL, TEXT("+"));
    }

  ListView_GetItemText(listWnd, modIndex, 1, tmpAction, MAX_LINE_LENGTH);

  commandIndex = (int)SendMessage(commandWnd, CB_FINDSTRINGEXACT, (WPARAM)-1,
                                  (LPARAM)ELStripInternalCommandArg(tmpAction).c_str());

  EnableWindow(internalWnd, true);
  EnableWindow(externalWnd, true);
  if (commandIndex == CB_ERR)
    {
      SendMessage(externalWnd, BM_CLICK, 0, 0);
      SetDlgItemText(hwndDlg, IDC_APPLICATION, tmpAction);
      SendMessage(commandWnd, CB_SETCURSEL, (WPARAM)-1, 0);
      SetDlgItemText(hwndDlg, IDC_COMMANDARG, TEXT(""));
    }
  else
    {
      SendMessage(internalWnd, BM_CLICK, 0, 0);
      SendMessage(commandWnd, CB_SETCURSEL, commandIndex, 0);
      SetDlgItemText(hwndDlg, IDC_APPLICATION, TEXT(""));
      SetDlgItemText(hwndDlg, IDC_COMMANDARG, ELGetInternalCommandArg(tmpAction).c_str());
    }
  EnableWindow(internalWnd, false);
  EnableWindow(externalWnd, false);

  SetFocus(listWnd);

  return true;
}

bool Actions::DoAdd(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_ACTIONSLIST);

  /**< Clear any existing selected items */
  for (int i = 0; i < ListView_GetItemCount(listWnd); i++)
    ListView_SetItemState(listWnd, i, 0, LVIS_SELECTED);

  // Set Dialogue items after clearing the selected items, if not they won't
  // be set correctly
  SendDlgItemMessage(hwndDlg, IDC_WIN, BM_SETCHECK, BST_UNCHECKED, 0);
  SendDlgItemMessage(hwndDlg, IDC_ALT, BM_SETCHECK, BST_UNCHECKED, 0);
  SendDlgItemMessage(hwndDlg, IDC_CTRL, BM_SETCHECK, BST_UNCHECKED, 0);
  SendDlgItemMessage(hwndDlg, IDC_SHIFT, BM_SETCHECK, BST_UNCHECKED, 0);
  SendDlgItemMessage(hwndDlg, IDC_COMMAND, CB_SETCURSEL, (WPARAM)-1, 0);
  SendDlgItemMessage(hwndDlg, IDC_KEY, CB_SETCURSEL, (WPARAM)-1, 0);
  SetDlgItemText(hwndDlg, IDC_APPLICATION, TEXT(""));
  SendDlgItemMessage(hwndDlg, IDC_EXTERNAL, BM_SETCHECK, BST_CHECKED, 0);
  SendDlgItemMessage(hwndDlg, IDC_INTERNAL, BM_SETCHECK, BST_UNCHECKED, 0);

  return EnableFields(hwndDlg, true);
}

bool Actions::DoAbort(HWND hwndDlg)
{
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_COMMAND);
  HWND keyWnd = GetDlgItem(hwndDlg, IDC_KEY);

  SetDlgItemText(hwndDlg, IDC_APPLICATION, TEXT(""));
  SendMessage(commandWnd, CB_SETCURSEL, (WPARAM)-1, 0);
  SendMessage(keyWnd, CB_SETCURSEL, (WPARAM)-1, 0);

  SendDlgItemMessage(hwndDlg, IDC_WIN, BM_SETCHECK, BST_UNCHECKED, 0);
  SendDlgItemMessage(hwndDlg, IDC_ALT, BM_SETCHECK, BST_UNCHECKED, 0);
  SendDlgItemMessage(hwndDlg, IDC_CTRL, BM_SETCHECK, BST_UNCHECKED, 0);
  SendDlgItemMessage(hwndDlg, IDC_SHIFT, BM_SETCHECK, BST_UNCHECKED, 0);

  edit = false;

  return EnableFields(hwndDlg, false);
}

bool Actions::EnableFields(HWND hwndDlg, bool enable)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_ACTIONSLIST);
  HWND keyWnd = GetDlgItem(hwndDlg, IDC_KEY);
  HWND keyTextWnd = GetDlgItem(hwndDlg, IDC_KEYTEXT);
  HWND fileWnd = GetDlgItem(hwndDlg, IDC_FILE);
  HWND folderWnd = GetDlgItem(hwndDlg, IDC_FOLDER);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);
  HWND addWnd = GetDlgItem(hwndDlg, IDC_ADDAPP);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_MODAPP);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELAPP);
  HWND shiftWnd = GetDlgItem(hwndDlg, IDC_SHIFT);
  HWND ctrlWnd = GetDlgItem(hwndDlg, IDC_CTRL);
  HWND winWnd = GetDlgItem(hwndDlg, IDC_WIN);
  HWND altWnd = GetDlgItem(hwndDlg, IDC_ALT);
  HWND appWnd = GetDlgItem(hwndDlg, IDC_APPLICATION);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_COMMAND);
  HWND commandArgWnd = GetDlgItem(hwndDlg, IDC_COMMANDARG);
  HWND inWnd = GetDlgItem(hwndDlg, IDC_INTERNAL);
  HWND exWnd = GetDlgItem(hwndDlg, IDC_EXTERNAL);

  if (enable)
    {
      if (SendDlgItemMessage(hwndDlg, IDC_EXTERNAL, BM_GETCHECK, 0, 0) == BST_CHECKED)
        {
          EnableWindow(appWnd, true);
          EnableWindow(commandWnd, false);
          EnableWindow(commandArgWnd, false);
          EnableWindow(fileWnd, true);
          EnableWindow(folderWnd, true);
        }
      else
        {
          EnableWindow(appWnd, false);
          EnableWindow(commandWnd, true);
          EnableWindow(commandArgWnd, true);
          EnableWindow(fileWnd, false);
          EnableWindow(folderWnd, false);
        }
      EnableWindow(saveWnd, true);
      EnableWindow(abortWnd, true);
      EnableWindow(keyWnd, true);
      EnableWindow(keyTextWnd, true);
      EnableWindow(shiftWnd, true);
      EnableWindow(ctrlWnd, true);
      EnableWindow(winWnd, true);
      EnableWindow(altWnd, true);
      EnableWindow(inWnd, true);
      EnableWindow(exWnd, true);
      EnableWindow(editWnd, false);
      EnableWindow(delWnd, false);
      EnableWindow(addWnd, false);
      EnableWindow(listWnd, false);
    }
  else
    {
      EnableWindow(fileWnd, false);
      EnableWindow(folderWnd, false);
      EnableWindow(appWnd, false);
      EnableWindow(commandWnd, false);
      EnableWindow(commandArgWnd, false);
      EnableWindow(saveWnd, false);
      EnableWindow(abortWnd, false);
      EnableWindow(keyWnd, false);
      EnableWindow(keyTextWnd, false);
      EnableWindow(shiftWnd, false);
      EnableWindow(ctrlWnd, false);
      EnableWindow(winWnd, false);
      EnableWindow(altWnd, false);
      EnableWindow(inWnd, false);
      EnableWindow(exWnd, false);
      if (ListView_GetSelectedCount(listWnd) > 0)
        {
          EnableWindow(editWnd, true);
          EnableWindow(delWnd, true);
        }
      else
        {
          EnableWindow(editWnd, false);
          EnableWindow(delWnd, false);
        }
      EnableWindow(addWnd, true);
      EnableWindow(listWnd, true);
    }

  return true;
}

bool Actions::DoSave(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_ACTIONSLIST);
  bool ret = false;
  LVITEM lvItem;
  WCHAR tmpKey[MAX_LINE_LENGTH], tmpAction[MAX_LINE_LENGTH], tmp[MAX_LINE_LENGTH],
        error[MAX_LINE_LENGTH];
  int index = ListView_GetItemCount(listWnd);
  HotkeyCombo *hc;

  ZeroMemory(tmpAction, MAX_LINE_LENGTH);
  ZeroMemory(tmpKey, MAX_LINE_LENGTH);

  if (SendDlgItemMessage(hwndDlg, IDC_EXTERNAL, BM_GETCHECK, 0, 0) == BST_CHECKED)
      GetDlgItemText(hwndDlg, IDC_APPLICATION, tmpAction, MAX_LINE_LENGTH);
  if (SendDlgItemMessage(hwndDlg, IDC_INTERNAL, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
      GetDlgItemText(hwndDlg, IDC_COMMAND, tmp, MAX_LINE_LENGTH);
      wcscpy(tmpAction, tmp);
      if (GetDlgItemText(hwndDlg, IDC_COMMANDARG, tmp, MAX_LINE_LENGTH) != 0)
        {
          wcscat(tmpAction, L" ");
          wcscat(tmpAction, tmp);
        }
    }
  if (SendDlgItemMessage(hwndDlg, IDC_SHIFT, BM_GETCHECK, 0, 0) == BST_CHECKED)
    wcscat(tmpKey, TEXT("Shift+"));
  if (SendDlgItemMessage(hwndDlg, IDC_CTRL, BM_GETCHECK, 0, 0) == BST_CHECKED)
    wcscat(tmpKey, TEXT("Ctrl+"));
  if (SendDlgItemMessage(hwndDlg, IDC_WIN, BM_GETCHECK, 0, 0) == BST_CHECKED)
    wcscat(tmpKey, TEXT("Win+"));
  if (SendDlgItemMessage(hwndDlg, IDC_ALT, BM_GETCHECK, 0, 0) == BST_CHECKED)
    wcscat(tmpKey, TEXT("Alt+"));
  GetDlgItemText(hwndDlg, IDC_KEY, tmp, MAX_LINE_LENGTH);
  wcscat(tmpKey, tmp);

  if (edit)
    {
      index = 0;
      while (index < ListView_GetItemCount(listWnd))
        {
          if (ListView_GetItemState(listWnd, index, LVIS_SELECTED))
            break;

          index++;
        }

      // No selected item was found, abort the save
      if (index == ListView_GetItemCount(listWnd))
        return false;

      UnregisterHotKey(mainWnd, pSettings->GetHotkeyListItem(editIndex)->GetHotkeyID());
      pSettings->DeleteHotkeyListItem(editIndex);

      // Deletion of the currently selected item failed, abort the save
      if (!ListView_DeleteItem(listWnd, index))
        return false;
    }

  if ((wcslen(tmpKey) > 0) && (wcslen(tmpAction) > 0))
    {
      hc = new HotkeyCombo(tmpKey, tmpAction, false);
      if (RegisterHotKey(mainWnd, hc->GetHotkeyID(), hc->GetHotkeyModifiers(),
                         hc->GetHotkeyKey()))
        {
          lvItem.mask = LVIF_TEXT|LVIF_STATE;
          lvItem.iItem = index;
          lvItem.iSubItem = 0;
          lvItem.pszText = tmpKey;
          lvItem.cchTextMax = (int)wcslen(tmpKey);
          lvItem.state = LVIS_SELECTED;
          lvItem.stateMask = LVIS_SELECTED;

          pSettings->AddHotkeyListItem(hc);
          if (ListView_InsertItem(listWnd, &lvItem) != -1)
            {
              ListView_SetItemText(listWnd, index, 1, tmpAction);

              ClearFields(hwndDlg);

              saveCount++;
              deleteCount++;

              EnableFields(hwndDlg, false);

              edit = false;
              ret = true;
            }
        }
      else
        {
          swprintf(error, TEXT("Failed to register Hotkey combination %ls."), tmpKey);
          ELMessageBox(GetDesktopWindow(), error, (WCHAR*)TEXT("emergeHotkeys"),
                       ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
          return false;
        }
    }

  lvSortInfo.listWnd = listWnd;
  ret = ListView_SortItemsEx(listWnd, ListViewCompareProc, (LPARAM)&lvSortInfo);

  return ret;
}

void Actions::PopulateKeys(HWND keyWnd)
{
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("LeftWinKey"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("RightWinKey"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F1"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F2"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F3"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F4"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F5"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F6"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F7"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F8"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F9"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F10"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F11"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F12"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F13"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F14"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F15"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F16"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F17"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F18"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F19"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F20"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F21"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F22"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F23"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F24"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("PrtScr"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Pause"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Insert"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Delete"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Home"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("End"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("PageUp"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("PageDown"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Left"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Right"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Up"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Down"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Tab"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Backspace"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Space"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Enter"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("["));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("]"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("\\"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT(";"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("'"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT(","));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("."));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("`"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("0"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("1"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("2"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("3"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("4"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("5"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("6"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("7"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("8"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("9"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Num0"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Num1"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Num2"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Num3"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Num4"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Num5"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Num6"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Num7"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Num8"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Num9"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Multiply"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Divide"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Add"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Subtract"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Decimal"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Escape"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("BrowserBack"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("BrowserForward"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("BrowserRefresh"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("BrowserStop"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("BrowserSearch"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("BrowserFavorites"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("BrowserHome"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VolumeMute"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VolumeUp"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VolumeDown"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("PlayerNext"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("PlayerPrevious"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("PlayerStop"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("PlayerPause"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("LaunchMail"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("LaunchPlayer"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("LaunchApp1"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("LaunchApp2"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Sleep"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("A"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("B"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("C"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("D"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("E"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("F"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("G"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("H"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("I"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("J"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("K"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("L"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("M"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("N"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("O"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("P"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Q"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("R"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("S"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("T"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("U"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("V"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("W"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("X"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Y"));
  SendMessage(keyWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Z"));
}

bool Actions::DoInternal(HWND hwndDlg)
{
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_COMMAND);
  HWND commandArgWnd = GetDlgItem(hwndDlg, IDC_COMMANDARG);
  HWND applicationWnd = GetDlgItem(hwndDlg, IDC_APPLICATION);
  HWND folderWnd = GetDlgItem(hwndDlg, IDC_FOLDER);
  HWND fileWnd = GetDlgItem(hwndDlg, IDC_FILE);
  HWND keyWnd = GetDlgItem(hwndDlg, IDC_KEY);

  if (!IsWindowEnabled(keyWnd))
    return true;

  EnableWindow(applicationWnd, false);
  EnableWindow(folderWnd, false);
  EnableWindow(fileWnd, false);
  EnableWindow(commandWnd, true);
  EnableWindow(commandArgWnd, true);

  return true;
}

bool Actions::DoExternal(HWND hwndDlg)
{
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_COMMAND);
  HWND commandArgWnd = GetDlgItem(hwndDlg, IDC_COMMANDARG);
  HWND applicationWnd = GetDlgItem(hwndDlg, IDC_APPLICATION);
  HWND folderWnd = GetDlgItem(hwndDlg, IDC_FOLDER);
  HWND fileWnd = GetDlgItem(hwndDlg, IDC_FILE);
  HWND keyWnd = GetDlgItem(hwndDlg, IDC_KEY);

  if (!IsWindowEnabled(keyWnd))
    return true;

  EnableWindow(applicationWnd, true);
  EnableWindow(folderWnd, true);
  EnableWindow(fileWnd, true);
  EnableWindow(commandWnd, false);
  EnableWindow(commandArgWnd, false);

  return true;
}

bool Actions::DoBrowse(HWND hwndDlg, bool folder)
{
  bool ret = false;
  BROWSEINFO bi;
  OPENFILENAME ofn;
  WCHAR tmp[MAX_PATH];

  ZeroMemory(tmp, MAX_PATH);

  if (folder)
    {
      ZeroMemory(&bi, sizeof(BROWSEINFO));
      bi.hwndOwner = hwndDlg;
      bi.ulFlags = BIF_NEWDIALOGSTYLE;
      bi.lpszTitle = TEXT("Select a folder:");

      LPITEMIDLIST pItemIDList = SHBrowseForFolder(&bi);

      if (pItemIDList != NULL)
        {
          if (SHGetPathFromIDList(pItemIDList, tmp))
            {
              IMalloc* pMalloc = NULL;
              if (SUCCEEDED(SHGetMalloc(&pMalloc)))
                {
                  pMalloc->Free(pItemIDList);
                  pMalloc->Release();
                }

              ELUnExpandVars(tmp);
              std::wstring workingTmp = ELRelativePathFromAbsPath(tmp);
              SetDlgItemText(hwndDlg, IDC_APPLICATION, workingTmp.c_str());

              ret = true;
            }
        }
    }
  else
    {
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
          ELUnExpandVars(tmp);
          std::wstring workingTmp = ELRelativePathFromAbsPath(tmp);
          SetDlgItemText(hwndDlg, IDC_APPLICATION, workingTmp.c_str());

          ret = true;
        }
    }

  return ret;
}

bool Actions::ClearFields(HWND hwndDlg)
{
  HWND keyWnd = GetDlgItem(hwndDlg, IDC_KEY);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_COMMAND);
  HWND modifyWnd = GetDlgItem(hwndDlg, IDC_MODAPP);

  SetDlgItemText(hwndDlg, IDC_APPLICATION, TEXT(""));
  SendMessage(keyWnd, CB_SETCURSEL, (WPARAM)-1, 0);
  SendMessage(commandWnd, CB_SETCURSEL, (WPARAM)-1, 0);
  SendDlgItemMessage(hwndDlg, IDC_WIN, BM_SETCHECK, BST_UNCHECKED, 0);
  SendDlgItemMessage(hwndDlg, IDC_ALT, BM_SETCHECK, BST_UNCHECKED, 0);
  SendDlgItemMessage(hwndDlg, IDC_CTRL, BM_SETCHECK, BST_UNCHECKED, 0);
  SendDlgItemMessage(hwndDlg, IDC_SHIFT, BM_SETCHECK, BST_UNCHECKED, 0);

  EnableWindow(modifyWnd, true);

  return true;
}

bool Actions::DoCancel(HWND hwndDlg, WPARAM wParam)
{
  UnregisterHotkeyList();
  pSettings->WriteList(true);
  pSettings->BuildList(mainWnd, false);
  RegisterHotkeyList(false);

  dialogVisible = false;
  EndDialog(hwndDlg, wParam);

  return true;
}

bool Actions::DoAbout()
{
  WCHAR tmp[MAX_LINE_LENGTH];
  VERSIONINFO versionInfo;
  bool ret = false;

  if (ELAppletVersionInfo(mainWnd, &versionInfo))
    {
      swprintf(tmp, TEXT("%ls\n\nVersion: %ls\n\nAuthor: %ls"),
               versionInfo.Description,
               versionInfo.Version,
               versionInfo.Author);

      ELMessageBox(GetDesktopWindow(), tmp, (WCHAR*)TEXT("emergeHotkeys"),
                   ELMB_OK|ELMB_ICONQUESTION|ELMB_MODAL);

      ret = true;
    }

  return ret;
}

bool Actions::KeyCheck(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
  WCHAR tmp[MAX_LINE_LENGTH];

  if (HIWORD(wParam) == CBN_SELCHANGE)
    {
      GetWindowText((HWND)lParam, tmp, MAX_LINE_LENGTH);
      if ((_wcsicmp(tmp, TEXT("LeftWinKey")) == 0) ||
          (_wcsicmp(tmp, TEXT("RightWinKey")) == 0))
        {
          if ((SendDlgItemMessage(hwndDlg, IDC_SHIFT, BM_GETCHECK, 0, 0) == BST_UNCHECKED) &&
              (SendDlgItemMessage(hwndDlg, IDC_CTRL, BM_GETCHECK, 0, 0) == BST_UNCHECKED) &&
              (SendDlgItemMessage(hwndDlg, IDC_WIN, BM_GETCHECK, 0, 0) == BST_UNCHECKED) &&
              (SendDlgItemMessage(hwndDlg, IDC_ALT, BM_GETCHECK, 0, 0) == BST_UNCHECKED))
            {
              swprintf(tmp,TEXT("Using the Win key with no modifier makes it\nhard to use the Win key for other hotkeys."));
              ELMessageBox(hwndDlg, tmp, (WCHAR*)TEXT("emergeHotkeys"),
                           ELMB_OK|ELMB_ICONWARNING|ELMB_MODAL);

              return true;
            }
        }
    }

  return false;
}

BOOL Actions::DoNotify(HWND hwndDlg UNUSED, LPARAM lParam)
{
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELAPP);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_MODAPP);
  HWND listWnd = GetDlgItem(hwndDlg, IDC_ACTIONSLIST);
  int subItem;
  BOOL ret;
  WCHAR tmpKey[MAX_LINE_LENGTH], tmpAction[MAX_LINE_LENGTH];

  switch (((LPNMHDR)lParam)->code)
    {
    case LVN_ITEMCHANGED:
      EnableWindow(delWnd, true);
      EnableWindow(editWnd, true);
      return PopulateFields(hwndDlg, ((LPNMLISTVIEW)lParam)->iItem);

    case LVN_COLUMNCLICK:
      subItem = ((LPNMLISTVIEW)lParam)->iSubItem;
      if (toggleSort[subItem])
        toggleSort[subItem] = false;
      else
        toggleSort[subItem] = true;
      lvSortInfo.sortInfo.ascending = toggleSort[subItem];
      lvSortInfo.sortInfo.subItem = subItem;
      pSettings->SetSortInfo(myName, &lvSortInfo.sortInfo);
      ret = ListView_SortItemsEx(listWnd, ListViewCompareProc, (LPARAM)&lvSortInfo);
      return ret;

      // Handle the NM_CUSTOMDRAW to change the text colour of invalid hotkeys to
      // red
    case NM_CUSTOMDRAW:
      LPNMLVCUSTOMDRAW lpLvCustomDraw = (LPNMLVCUSTOMDRAW)lParam;
      switch (lpLvCustomDraw->nmcd.dwDrawStage)
        {
          // In the CDDS_PREPAINT stage tell the OS that we want to custom paint
          // the entry
        case CDDS_PREPAINT:
          // Instruct the OS to send the CDDS_ITEMPREPAINT message
          return CDRF_NOTIFYITEMDRAW;
        case CDDS_ITEMPREPAINT:
          // Instruct the OS that we want to handle subitems
          return CDRF_NOTIFYSUBITEMDRAW;
        case CDDS_ITEMPREPAINT|CDDS_SUBITEM:
          ListView_GetItemText(listWnd, lpLvCustomDraw->nmcd.dwItemSpec, 0,
                               tmpKey, MAX_LINE_LENGTH);
          ListView_GetItemText(listWnd, lpLvCustomDraw->nmcd.dwItemSpec, 1,
                               tmpAction, MAX_LINE_LENGTH);
          if (!pSettings->IsValidHotkey(pSettings->FindHotkeyListItem(tmpKey,
                                                                      tmpAction)))
            lpLvCustomDraw->clrText = RGB(255,0,0);
          return CDRF_NEWFONT;
        default:
          return CDRF_DODEFAULT;
        }
    }

  return FALSE;
}
