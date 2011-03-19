// vim: tags+=../emergeLib/tags
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

#include "AliasEditor.h"

INT_PTR CALLBACK AliasEditor::AliasDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static AliasEditor *pAliasEditor = NULL;
  PROPSHEETPAGE *psp;

  switch (message)
    {
    case WM_INITDIALOG:
      psp = (PROPSHEETPAGE*)lParam;
      pAliasEditor = reinterpret_cast<AliasEditor*>(psp->lParam);
      if (!pAliasEditor)
        break;
      return pAliasEditor->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pAliasEditor->DoCommand(hwndDlg, wParam, lParam);

    case WM_NOTIFY:
      return pAliasEditor->DoNotify(hwndDlg, lParam);

    case WM_HOTKEY:
      return pAliasEditor->DoHotkey(hwndDlg, wParam);
    }

  return FALSE;
}

AliasEditor::AliasEditor(HINSTANCE hInstance, HWND mainWnd, std::tr1::shared_ptr<Settings> pSettings)
{
  this->hInstance = hInstance;
  this->mainWnd = mainWnd;
  this->pSettings = pSettings;
  edit = false;
  toggleSort[0] = false;
  toggleSort[1] = false;
  wcscpy(myName, TEXT("AliasEditor"));

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
  ExtractIconEx(TEXT("emergeIcons.dll"), 3, NULL, &delIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 18, NULL, &browseIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 9, NULL, &saveIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 1, NULL, &abortIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 5, NULL, &editIcon, 1);

  pSettings->GetSortInfo(myName, &lvSortInfo.sortInfo);
  toggleSort[lvSortInfo.sortInfo.subItem] = lvSortInfo.sortInfo.ascending;
}

AliasEditor::~AliasEditor()
{
  if (addIcon)
    DestroyIcon(addIcon);
  if (delIcon)
    DestroyIcon(delIcon);
  if (browseIcon)
    DestroyIcon(browseIcon);
  if (saveIcon)
    DestroyIcon(saveIcon);
  if (abortIcon)
    DestroyIcon(abortIcon);
  if (editIcon)
    DestroyIcon(editIcon);

  DestroyWindow(toolWnd);
}

BOOL AliasEditor::DoInitDialog(HWND hwndDlg)
{
  RECT rect;
  int x, y;
  LVCOLUMN lvCol;
  TOOLINFO ti;
  dlgWnd = hwndDlg;

  ZeroMemory(&ti, sizeof(TOOLINFO));

  HWND addWnd = GetDlgItem(hwndDlg, IDC_ADDAPP);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELAPP);
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);
  HWND aliasWnd = GetDlgItem(hwndDlg, IDC_ALIAS);
  HWND aliasTextWnd = GetDlgItem(hwndDlg, IDC_ALIASTEXT);
  HWND actionTextWnd = GetDlgItem(hwndDlg, IDC_ACTIONTEXT);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITAPP);

  saveCount = 0;
  deleteCount = 0;

  GetWindowRect(hwndDlg, &rect);

  x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
  y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);
  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
  ELStealFocus(hwndDlg);

  lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
  lvCol.pszText = (WCHAR*)TEXT("Alias");
  lvCol.cx = 70;
  if (ListView_InsertColumn(listWnd, 0, &lvCol) == -1)
    return FALSE;

  lvCol.pszText = (WCHAR*)TEXT("Action");
  lvCol.cx = MAX_PATH;
  if (ListView_InsertColumn(listWnd, 1, &lvCol) == -1)
    return FALSE;

  if (ListView_SetExtendedListViewStyle(listWnd,  LVS_EX_FULLROWSELECT) != 0)
    return FALSE;

  if (addIcon)
    SendMessage(addWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)addIcon);
  if (delIcon)
    SendMessage(delWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)delIcon);
  if (browseIcon)
    SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)browseIcon);
  if (saveIcon)
    SendMessage(saveWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)saveIcon);
  if (abortIcon)
    SendMessage(abortWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)abortIcon);
  if (editIcon)
    SendMessage(editWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)editIcon);

  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = addWnd;
  ti.uId = (ULONG_PTR)addWnd;
  ti.hinst = hInstance;
  ti.lpszText = (WCHAR*)TEXT("New Alias (Ctrl+N)");
  GetClientRect(addWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = delWnd;
  ti.uId = (ULONG_PTR)delWnd;
  ti.lpszText = (WCHAR*)TEXT("Delete Alias (Del)");
  GetClientRect(delWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = editWnd;
  ti.uId = (ULONG_PTR)editWnd;
  ti.lpszText = (WCHAR*)TEXT("Edit Alias (Ctrl+E)");
  GetClientRect(editWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = browseWnd;
  ti.uId = (ULONG_PTR)browseWnd;
  ti.lpszText = (WCHAR*)TEXT("Browse for Launch Applet");
  GetClientRect(browseWnd, &ti.rect);
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

  EnableWindow(delWnd, false);
  EnableWindow(appletWnd, false);
  EnableWindow(browseWnd, false);
  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);
  EnableWindow(aliasWnd, false);
  EnableWindow(aliasTextWnd, false);
  EnableWindow(actionTextWnd, false);
  EnableWindow(editWnd, false);

  bool ret;
  lvSortInfo.listWnd = listWnd;
  ret = ListView_SortItemsEx(listWnd, ListViewCompareProc, (LPARAM)&lvSortInfo);

  WNDPROC oldAliasProc;
  oldAliasProc = (WNDPROC)SetWindowLongPtr(aliasWnd,GWLP_WNDPROC,(LONG_PTR)AliasProc);
  SendMessage(aliasWnd, WM_APP+1, (WPARAM)hwndDlg, (LPARAM)oldAliasProc);
  SendMessage(aliasWnd, WM_APP+2, 0, (LPARAM)this);

  WNDPROC oldAppletProc;
  oldAppletProc = (WNDPROC)SetWindowLongPtr(appletWnd,GWLP_WNDPROC,(LONG_PTR)AppletProc);
  SendMessage(appletWnd, WM_APP+1, (WPARAM)hwndDlg, (LPARAM)oldAppletProc);

  if (!RegisterHotKey(hwndDlg, HOTKEY_N, MOD_CONTROL, 'N'))
    {
      ELMessageBox(hwndDlg,
                   (WCHAR*)TEXT("Failed to register Hotkey combination Ctrl+N"),
                   (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      return FALSE;
    }

  if (!RegisterHotKey(hwndDlg, HOTKEY_E, MOD_CONTROL, 'E'))
    {
      ELMessageBox(hwndDlg,
                   (WCHAR*)TEXT("Failed to register Hotkey combination Ctrl+E"),
                   (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      return FALSE;
    }

  return TRUE;
}

LRESULT CALLBACK AliasEditor::AliasProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
  static WNDPROC oldProc = NULL;
  static HWND hwndDlg = NULL;
  static AliasEditor *pThis = NULL;
  WCHAR alias[MAX_LINE_LENGTH];

  if (message == WM_APP+1)
    {
      hwndDlg = (HWND)wParam;
      oldProc = (WNDPROC)lParam;
    }

  if (message == WM_APP+2)
    pThis = (AliasEditor*)lParam;

  if ((oldProc == NULL) || (hwndDlg == NULL) || (pThis == NULL))
    return DefWindowProc(hwnd, message, wParam, lParam);

  switch (message)
    {
      /**< This is needed to capture the VK_RETURN */
    case WM_GETDLGCODE:
      return (DLGC_WANTALLKEYS|CallWindowProc(oldProc, hwnd, message, wParam, lParam));

    case WM_CHAR:
      /**< This is needed to avoid message beeps */
      if ((wParam == VK_RETURN) || (wParam == VK_TAB) || (wParam == VK_ESCAPE))
        return 0;
      else
        return (CallWindowProc(oldProc, hwnd, message, wParam, lParam));

    case WM_KEYDOWN:
      if ((wParam == VK_RETURN) || (wParam == VK_TAB))
        {
          GetWindowText(hwnd, alias, MAX_LINE_LENGTH);

          if (pThis->AliasCheck(hwndDlg, alias))
            PostMessage(hwndDlg, WM_NEXTDLGCTL, 0, 0);

          return FALSE;
        }

      if (wParam == VK_ESCAPE)
        {
          PostMessage(hwndDlg, WM_COMMAND, IDC_ABORTAPP, 0);
          return FALSE;
        }
      break;
    }

  return CallWindowProc(oldProc, hwnd, message, wParam, lParam);
}

LRESULT CALLBACK AliasEditor::AppletProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
  static WNDPROC oldProc = NULL;
  static HWND hwndDlg = NULL;
  WCHAR error[MAX_LINE_LENGTH], action[MAX_LINE_LENGTH];

  if (message == WM_APP+1)
    {
      hwndDlg = (HWND)wParam;
      oldProc = (WNDPROC)lParam;
    }

  if ((oldProc == NULL) || (hwndDlg == NULL))
    return DefWindowProc(hwnd, message, wParam, lParam);

  switch (message)
    {
      /**< This is needed to capture the VK_RETURN */
    case WM_GETDLGCODE:
      return (DLGC_WANTALLKEYS|CallWindowProc(oldProc, hwnd, message, wParam, lParam));

    case WM_CHAR:
      /**< This is needed to avoid message beeps */
      if ((wParam == VK_RETURN) || (wParam == VK_TAB) || (wParam == VK_ESCAPE))
        return 0;
      else
        return (CallWindowProc(oldProc, hwnd, message, wParam, lParam));

    case WM_KEYDOWN:
      if (wParam == VK_RETURN)
        {
          GetWindowText(hwnd, action, MAX_LINE_LENGTH);
          if (wcslen(action) == 0)
            {
              swprintf(error, TEXT("Action is empty."));
              ELMessageBox(hwndDlg, error, (WCHAR*)TEXT("emergeCore"),
                           ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
            }
          else
            {
              PostMessage(hwndDlg, WM_COMMAND, IDC_SAVEAPP, 0);
            }
          return FALSE;
        }

      if (wParam == VK_TAB)
        {
          PostMessage(hwndDlg, WM_NEXTDLGCTL, 0, 0);
          return FALSE;
        }

      if (wParam == VK_ESCAPE)
        {
          PostMessage(hwndDlg, WM_COMMAND, IDC_ABORTAPP, 0);
          return FALSE;
        }
      break;
    }

  return CallWindowProc(oldProc, hwnd, message, wParam, lParam);
}

bool AliasEditor::CheckSaveCount(HWND hwndDlg)
{
  if ((saveCount != 0) || (deleteCount != 0))
    {
      if (ELMessageBox(hwndDlg,
                       (WCHAR*)TEXT("All current modifications will be lost.  To save and exit press OK.\n\nDo you wish to continue?"),
                       (WCHAR*)TEXT("emergeCore"),
                       ELMB_YESNO|ELMB_ICONQUESTION|ELMB_MODAL) == IDYES)
        return true;
      else
        return false;
    }

  return true;
}

bool AliasEditor::CheckFields(HWND hwndDlg)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);

  if (!IsWindowEnabled(appletWnd))
    return true;

  if (GetDlgItemText(hwndDlg, IDC_APPLET, tmp, MAX_LINE_LENGTH) != 0)
    {
      if (ELMessageBox(hwndDlg,
                       (WCHAR*)TEXT("The current applet will be lost.\n\nDo you wish to continue?"),
                       (WCHAR*)TEXT("emergeCore"),
                       ELMB_YESNO|ELMB_ICONQUESTION|ELMB_MODAL) == IDYES)
        return true;
      else
        return false;
    }

  return true;
}

BOOL AliasEditor::DoHotkey(HWND hwndDlg, int hotkeyID)
{
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);
  HWND aliasWnd = GetDlgItem(hwndDlg, IDC_ALIAS);

  if (!IsWindowEnabled(appletWnd) && !IsWindowEnabled(aliasWnd))
    {
      switch (hotkeyID)
        {
        case HOTKEY_N:
          DoAliasAdd(hwndDlg);
          break;
        case HOTKEY_E:
          edit = true;
          DoAliasAdd(hwndDlg);
          break;
        }
    }

  return TRUE;
}

BOOL AliasEditor::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  switch (LOWORD(wParam))
    {
    case IDC_DELAPP:
      return DoAliasDelete(hwndDlg);
    case IDC_ADDAPP:
      return DoAliasAdd(hwndDlg);
    case IDC_EDITAPP:
      edit = true;
      return DoAliasAdd(hwndDlg);
    case IDC_SAVEAPP:
      return DoAliasSave(hwndDlg);
    case IDC_ABORTAPP:
      return DoAliasAbort(hwndDlg);
    case IDC_BROWSE:
      return DoAliasBrowse(hwndDlg);
    }

  return FALSE;
}

bool AliasEditor::UpdateAliases(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  WCHAR alias[MAX_LINE_LENGTH], action[MAX_LINE_LENGTH];
  int i = 0;
  std::wstring aliasFile = TEXT("%EmergeDir%\\files\\cmd.txt");
  aliasFile = ELExpandVars(aliasFile);

  if ((saveCount == 0) && (deleteCount == 0))
    return true;

  FILE *fp = _wfopen(aliasFile.c_str(), TEXT("w"));

  if (!fp)
    return false;

  // Loop while there are entries in the key
  while (i < ListView_GetItemCount(listWnd))
    {
      ListView_GetItemText(listWnd, i, 0, alias, MAX_LINE_LENGTH);
      ListView_GetItemText(listWnd, i, 1, action, MAX_LINE_LENGTH);

      fwprintf(fp, TEXT("%s\t%s\n"), alias, action);

      i++;
    }

  fclose(fp);

  return true;
}

bool AliasEditor::PopulateList(HWND listWnd)
{
  bool ret = false;
  std::wstring aliasFile = TEXT("%EmergeDir%\\files\\cmd.txt");
  aliasFile = ELExpandVars(aliasFile);
  int index = 0;

  WCHAR line[MAX_LINE_LENGTH];

  FILE *fp = _wfopen(aliasFile.c_str(), TEXT("r"));

  if (!fp)
    return false;

  while (fgetws(line, MAX_LINE_LENGTH, fp))
    InsertListViewItem(listWnd, index++, line);

  fclose(fp);

  return ret;
}

void AliasEditor::InsertListViewItem(HWND listWnd, int index, const WCHAR *item)
{
  LVITEM lvItem;
  WCHAR *value, *command = NULL, *tmp = NULL;
  std::wstring workingItem;

  tmp = _wcsdup(item);

  value = wcstok(tmp, TEXT(" \t"));
  if (value != NULL)
    command = wcstok(NULL, TEXT("\n"));

  if (command != NULL)
    ELStripLeadingSpaces(command);

  lvItem.mask = LVIF_TEXT;
  lvItem.iItem = index;
  lvItem.iSubItem = 0;
  lvItem.pszText = value;
  lvItem.cchTextMax = (int)wcslen(lvItem.pszText);
  if (ListView_InsertItem(listWnd, &lvItem) != -1)
    ListView_SetItemText(listWnd, lvItem.iItem, 1, command);

  free(tmp);
}

bool AliasEditor::DoAliasDelete(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELAPP);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITAPP);
  bool ret = false;
  int i = 0, prevItem = 0;

  if (ListView_GetSelectedCount(listWnd) > 1)
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("You can only delete one item at a time."),
                   (WCHAR*)TEXT("emergeCore"), ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);

      return ret;
    }

  while (i < ListView_GetItemCount(listWnd))
    {
      if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
        {
          prevItem = ListView_GetNextItem(listWnd, i, LVNI_ABOVE);
          if (!ListView_DeleteItem(listWnd, i))
            return ret;
          deleteCount++;

          ListView_SetItemState(listWnd, i, LVIS_SELECTED,
                                LVIS_SELECTED);
          if (!ListView_EnsureVisible(listWnd, i, FALSE))
            {
              if (prevItem != -1)
                {
                  ListView_SetItemState(listWnd, prevItem, LVIS_SELECTED,
                                        LVIS_SELECTED);
                  ret = (ListView_EnsureVisible(listWnd, prevItem, FALSE) == TRUE);
                }
            }

          break;
        }
      else
        i++;
    }

  if (ListView_GetItemCount(listWnd) == 0)
    {
      EnableWindow(delWnd, false);
      EnableWindow(editWnd, false);
    }

  return ret;
}

bool AliasEditor::DoAliasAdd(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);
  HWND aliasWnd = GetDlgItem(hwndDlg, IDC_ALIAS);
  HWND aliasTextWnd = GetDlgItem(hwndDlg, IDC_ALIASTEXT);
  HWND actionTextWnd = GetDlgItem(hwndDlg, IDC_ACTIONTEXT);

  if (edit)
    {
      if (ListView_GetSelectedCount(listWnd) > 1)
        {
          ELMessageBox(hwndDlg, (WCHAR*)TEXT("You can only edit one item at a time."),
                       (WCHAR*)TEXT("emergeCore"), ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);

          return false;
        }
    }
  else
    {
      /**< Clear any existing selected items */
      for (int i = 0; i < ListView_GetItemCount(listWnd); i++)
        ListView_SetItemState(listWnd, i, 0, LVIS_SELECTED);
      SetDlgItemText(hwndDlg, IDC_APPLET, TEXT(""));
      SetDlgItemText(hwndDlg, IDC_ALIAS, TEXT(""));
    }

  EnableWindow(appletWnd, true);
  EnableWindow(browseWnd, true);
  EnableWindow(saveWnd, true);
  EnableWindow(abortWnd, true);
  EnableWindow(aliasWnd, true);
  EnableWindow(actionTextWnd, true);
  EnableWindow(aliasTextWnd, true);
  EnableWindow(listWnd, false);

  SetFocus(aliasWnd);
  return true;
}

bool AliasEditor::DoAliasAbort(HWND hwndDlg)
{
  int i = 0;
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);
  HWND aliasWnd = GetDlgItem(hwndDlg, IDC_ALIAS);
  HWND aliasTextWnd = GetDlgItem(hwndDlg, IDC_ALIASTEXT);
  HWND actionTextWnd = GetDlgItem(hwndDlg, IDC_ACTIONTEXT);
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);

  if (edit)
    {
      while (i < ListView_GetItemCount(listWnd))
        {
          if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
            {
              PopulateFields(hwndDlg, i);
              break;
            }

          i++;
        }
    }
  else
    {
      SetDlgItemText(hwndDlg, IDC_APPLET, TEXT(""));
      SetDlgItemText(hwndDlg, IDC_ALIAS, TEXT(""));
    }

  EnableWindow(appletWnd, false);
  EnableWindow(browseWnd, false);
  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);
  EnableWindow(aliasWnd, false);
  EnableWindow(aliasTextWnd, false);
  EnableWindow(actionTextWnd, false);
  EnableWindow(listWnd, true);
  edit = false;

  SetFocus(aliasWnd);
  return true;
}

bool AliasEditor::FindListSubItem(HWND listWnd, int subItem, WCHAR *searchString)
{
  int listSize = ListView_GetItemCount(listWnd), i = 0;
  WCHAR item[MAX_LINE_LENGTH];
  bool ret = false;

  for (i = 0; i < listSize; i++)
    {
      ListView_GetItemText(listWnd, i, subItem, item, MAX_LINE_LENGTH);
      if (_wcsicmp(item, searchString) == 0)
        {
          ret = true;
          break;
        }
    }

  return ret;
}

bool AliasEditor::AliasCheck(HWND hwndDlg, WCHAR *alias)
{
  size_t aliasLength = wcslen(alias);
  WCHAR error[MAX_LINE_LENGTH];

  if (aliasLength == 0)
    return false;

  if (alias[0] != '.')
    {
      swprintf(error, TEXT("Alias must start with a period (.)"));
      ELMessageBox(hwndDlg, error, (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      return false;
    }

  for (size_t i = 0; i < aliasLength; i++)
    {
      if (alias[i] == ' ')
        {
          swprintf(error, TEXT("Alias cannot contain spaces"));
          ELMessageBox(hwndDlg, error, (WCHAR*)TEXT("emergeCore"),
                       ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
          return false;
        }
    }

  return true;
}

bool AliasEditor::DoAliasSave(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  HWND aliasWnd = GetDlgItem(hwndDlg, IDC_ALIAS);
  HWND aliasTextWnd = GetDlgItem(hwndDlg, IDC_ALIASTEXT);
  HWND actionTextWnd = GetDlgItem(hwndDlg, IDC_ACTIONTEXT);
  bool ret = false;
  LVITEM lvItem;
  WCHAR alias[MAX_LINE_LENGTH], error[MAX_LINE_LENGTH], action[MAX_LINE_LENGTH];
  int i = 0;

  ZeroMemory(alias, MAX_LINE_LENGTH);
  ZeroMemory(action, MAX_LINE_LENGTH);

  GetDlgItemText(hwndDlg, IDC_APPLET, action, MAX_LINE_LENGTH);
  GetDlgItemText(hwndDlg, IDC_ALIAS, alias, MAX_LINE_LENGTH);

  if (wcslen(alias) > 0)
    {
      if (!AliasCheck(hwndDlg, alias))
        return ret;

      if (edit)
        {
          while (i < ListView_GetItemCount(listWnd))
            {
              if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
                {
                  ListView_SetItemText(listWnd, i, 0, alias);
                  ListView_SetItemText(listWnd, i, 1, action);

                  saveCount++;
                  deleteCount++;

                  ret = true;
                  break;
                }

              i++;
            }
        }
      else
        {
          if (!FindListSubItem(listWnd, 0, alias))
            {
              lvItem.mask = LVIF_TEXT|LVIF_STATE;
              lvItem.iItem = ListView_GetItemCount(listWnd);
              lvItem.iSubItem = 0;
              lvItem.pszText = alias;
              lvItem.cchTextMax = (int)wcslen(lvItem.pszText);
              lvItem.state = LVIS_SELECTED;
              lvItem.stateMask = LVIS_SELECTED;
              if (ListView_InsertItem(listWnd, &lvItem) != -1)
                {
                  ListView_SetItemText(listWnd, lvItem.iItem, 1, action);

                  saveCount++;
                  deleteCount++;

                  lvSortInfo.listWnd = listWnd;
                  ret = ListView_SortItemsEx(listWnd, ListViewCompareProc, (LPARAM)&lvSortInfo);
                  SetDlgItemText(hwndDlg, IDC_APPLET, TEXT(""));
                  SetDlgItemText(hwndDlg, IDC_ALIAS, TEXT(""));
                }
            }
          else
            {
              swprintf(error, TEXT("%s is already in the applet launch list"), alias);
              ELMessageBox(hwndDlg, error, (WCHAR*)TEXT("emergeCore"),
                           ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
            }
        }
    }

  if (ret)
    {
      EnableWindow(saveWnd, false);
      EnableWindow(abortWnd, false);
      EnableWindow(appletWnd, false);
      EnableWindow(browseWnd, false);
      EnableWindow(aliasWnd, false);
      EnableWindow(aliasTextWnd, false);
      EnableWindow(actionTextWnd, false);
      EnableWindow(listWnd, true);
      edit = false;

      SetFocus(listWnd);
    }

  return ret;
}

bool AliasEditor::DoAliasBrowse(HWND hwndDlg)
{
  bool ret = false;
  OPENFILENAME ofn;
  WCHAR tmp[MAX_PATH], path[MAX_PATH], program[MAX_PATH], arguments[MAX_LINE_LENGTH];
  std::wstring workingPath;

  ZeroMemory(tmp, MAX_PATH);
  ZeroMemory(&ofn, sizeof(ofn));

  ELGetCurrentPath(path);

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hwndDlg;
  ofn.lpstrFilter = TEXT("All Files (*.*)\0*.*\0");
  GetDlgItemText(hwndDlg, IDC_APPLET, tmp, MAX_PATH);
  workingPath = tmp;
  workingPath = ELExpandVars(workingPath);
  if (ELParseCommand(workingPath.c_str(), program, arguments))
    wcscpy(tmp, program);
  ofn.lpstrFile = tmp;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrTitle = TEXT("Browse For Item");
  ofn.lpstrDefExt = NULL;
  ofn.lpstrInitialDir = path;
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS;

  if (GetOpenFileName(&ofn))
    {
      ELUnExpandVars(tmp);
      SetDlgItemText(hwndDlg, IDC_APPLET, tmp);

      ret = true;
    }

  return ret;
}

BOOL AliasEditor::PopulateFields(HWND hwndDlg, int index)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  WCHAR action[MAX_LINE_LENGTH], alias[MAX_LINE_LENGTH];

  ListView_GetItemText(listWnd, index, 0, alias, MAX_PATH);
  SetDlgItemText(hwndDlg, IDC_ALIAS, alias);

  ListView_GetItemText(listWnd, index, 1, action, MAX_PATH);
  SetDlgItemText(hwndDlg, IDC_APPLET, action);

  return TRUE;
}

int CALLBACK AliasEditor::ListViewCompareProc (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
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

BOOL AliasEditor::DoNotify(HWND hwndDlg, LPARAM lParam)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELAPP);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITAPP);
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);
  HWND aliasWnd = GetDlgItem(hwndDlg, IDC_ALIAS);
  int subItem;
  BOOL ret;

  switch (((LPNMITEMACTIVATE)lParam)->hdr.code)
    {
    case LVN_ITEMCHANGED:
      EnableWindow(delWnd, true);
      EnableWindow(editWnd, true);
      return PopulateFields(hwndDlg, ((LPNMLISTVIEW)lParam)->iItem);

    case LVN_KEYDOWN:
      if (!IsWindowEnabled(appletWnd) && !IsWindowEnabled(aliasWnd) && (((LPNMLVKEYDOWN) lParam)->wVKey == VK_DELETE))
        DoAliasDelete(hwndDlg);
      return TRUE;

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

    case PSN_APPLY:
      if (!CheckFields(hwndDlg))
        {
          SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID);
          return FALSE;
        }

      if (UpdateAliases(hwndDlg))
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
      else
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID);
      return TRUE;

    case PSN_SETACTIVE:
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
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
