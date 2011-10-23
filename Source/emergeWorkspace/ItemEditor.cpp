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

#include "ItemEditor.h"

BOOL CALLBACK ItemEditor::MenuDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static ItemEditor *pItemEditor = NULL;

  switch (message)
    {
    case WM_INITDIALOG:
      pItemEditor = reinterpret_cast<ItemEditor*>(lParam);
      if (!pItemEditor)
        break;
      return pItemEditor->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pItemEditor->DoMenuCommand(hwndDlg, wParam, lParam);
    }

  return FALSE;
}

ItemEditor::ItemEditor(HINSTANCE hInstance, HWND mainWnd)
{
  (*this).hInstance = hInstance;
  (*this).mainWnd = mainWnd;
  section = NULL;

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

  ExtractIconEx(TEXT("emergeIcons.dll"), 6, NULL, &browseIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 18, NULL, &fileIcon, 1);

  dialogVisible = false;
}

ItemEditor::~ItemEditor()
{
  if (browseIcon)
    DestroyIcon(browseIcon);
  if (fileIcon)
    DestroyIcon(fileIcon);

  DestroyWindow(toolWnd);
}

int ItemEditor::Show(TiXmlElement *section, WCHAR *name, WCHAR *value, UINT type, WCHAR *workingDir)
{
  (*this).section = section;
  wcscpy((*this).name, name);
  wcscpy((*this).value, value);
  (*this).type = type;
  wcscpy((*this).workingDir, workingDir);
  dialogVisible = true;
  return (int)DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ITEMEDIT), mainWnd, (DLGPROC)MenuDlgProc, (LPARAM)this);
}

BOOL ItemEditor::DoInitDialog(HWND hwndDlg)
{
  RECT rect;
  int x, y;
  TOOLINFO ti;

  ZeroMemory(&ti, sizeof(TOOLINFO));
  ELGetWindowRect(hwndDlg, &rect);

  HWND nameWnd = GetDlgItem(hwndDlg, IDC_ITEMNAME);
  HWND nameTextWnd = GetDlgItem(hwndDlg, IDC_NAMETEXT);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);
  HWND typeTextWnd = GetDlgItem(hwndDlg, IDC_TYPETEXT);
  HWND valueWnd = GetDlgItem(hwndDlg, IDC_ITEMVALUE);
  HWND valueTextWnd = GetDlgItem(hwndDlg, IDC_VALUETEXT);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_ITEMCOMMAND);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEITEM);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTITEM);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_ITEMBROWSE);
  HWND dirWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIR);
  HWND workingDirTextWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIRTEXT);
  HWND dirBrowseWnd = GetDlgItem(hwndDlg, IDC_DIRBROWSE);
  HWND specialFoldersWnd = GetDlgItem(hwndDlg, IDC_ITEMSPECIALFOLDERS);
  HWND commandArgWnd = GetDlgItem(hwndDlg, IDC_COMMANDARG);

  x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
  y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);
  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
  ELStealFocus(hwndDlg);

  ELPopulateInternalCommandList(commandWnd);
  ShowWindow(commandWnd, false);

  PopulateSpecialFolders(specialFoldersWnd);
  ShowWindow(specialFoldersWnd, false);

  if (fileIcon)
    SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
  if (browseIcon)
    SendMessage(dirBrowseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)browseIcon);

  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
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

  ti.hwnd = browseWnd;
  ti.uId = (ULONG_PTR)browseWnd;
  ti.lpszText = (WCHAR*)TEXT("");
  GetClientRect(browseWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  SetDlgItemText(hwndDlg, IDC_ITEMNAME, name);
  SetDlgItemText(hwndDlg, IDC_WORKINGDIR, workingDir);

  EnableWindow(nameWnd, true);
  EnableWindow(nameTextWnd, true);
  EnableWindow(typeWnd, true);
  EnableWindow(typeTextWnd, true);
  EnableWindow(valueWnd, false);
  EnableWindow(valueTextWnd, false);
  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);
  EnableWindow(commandWnd, false);
  EnableWindow(browseWnd, false);
  EnableWindow(dirWnd, false);
  EnableWindow(workingDirTextWnd, false);
  EnableWindow(dirBrowseWnd, false);
  EnableWindow(specialFoldersWnd, false);
  EnableWindow(commandArgWnd, false);

  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Separator"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Executable"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Internal Command"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("DateTime"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Special Folder"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Submenu"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Folder Menu"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Tasks Menu"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Settings Menu"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Help Menu"));

  SendMessage(typeWnd, CB_SETCURSEL,
              (WPARAM)GetTypeValue(type), 0);
  switch (type)
    {
    case IT_INTERNAL_COMMAND:
    {
      int commandIndex = (int)SendMessage(commandWnd, CB_FINDSTRINGEXACT, (WPARAM)-1,
                                          (LPARAM)ELStripInternalCommandArg(value).c_str());
      if (commandIndex != CB_ERR)
        {
          SendMessage(commandWnd, CB_SETCURSEL, commandIndex, 0);
          SetDlgItemText(hwndDlg, IDC_COMMANDARG, ELGetInternalCommandArg(value).c_str());
        }
    }
    break;
    case IT_SPECIAL_FOLDER:
    {
      int folder = ELIsSpecialFolder(value);
      if (ELGetSpecialFolder(folder, value))
        SendMessage(specialFoldersWnd, CB_SETCURSEL,
                    SendMessage(specialFoldersWnd,
                                CB_FINDSTRINGEXACT,
                                (WPARAM)-1,
                                (LPARAM)value),
                    0);
    }
    break;
    default:
      SetDlgItemText(hwndDlg, IDC_ITEMVALUE, value);
      break;
    }
  EnableFields(hwndDlg);

  return TRUE;
}

void ItemEditor::PopulateSpecialFolders(HWND specialFoldersWnd)
{
  WCHAR tmp[MAX_PATH];

  if (ELGetSpecialFolder(CSIDL_PERSONAL, tmp))
    SendMessage(specialFoldersWnd, CB_ADDSTRING, 0, (LPARAM)tmp);
  if (ELGetSpecialFolder(CSIDL_DRIVES, tmp))
    SendMessage(specialFoldersWnd, CB_ADDSTRING, 0, (LPARAM)tmp);
  if (ELGetSpecialFolder(CSIDL_NETWORK, tmp))
    SendMessage(specialFoldersWnd, CB_ADDSTRING, 0, (LPARAM)tmp);
  if (ELGetSpecialFolder(CSIDL_CONTROLS, tmp))
    SendMessage(specialFoldersWnd, CB_ADDSTRING, 0, (LPARAM)tmp);
  if (ELGetSpecialFolder(CSIDL_BITBUCKET, tmp))
    SendMessage(specialFoldersWnd, CB_ADDSTRING, 0, (LPARAM)tmp);
}

bool ItemEditor::GetVisible()
{
  return dialogVisible;
}

BOOL ItemEditor::DoMenuCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  switch (LOWORD(wParam))
    {
    case IDOK:
      if (!DoSaveItem(hwndDlg))
        return FALSE;
    case IDCANCEL:
      dialogVisible = false;
      EndDialog(hwndDlg, wParam);
      return TRUE;
    case IDC_ITEMTYPE:
      if (HIWORD(wParam) == CBN_SELCHANGE)
        return DoSelChange(hwndDlg, (HWND)lParam);
      break;
    case IDC_ITEMBROWSE:
      return DoBrowseItem(hwndDlg, false);
    case IDC_DIRBROWSE:
      return DoBrowseItem(hwndDlg, true);
    }

  return FALSE;
}

bool ItemEditor::DoBrowseItem(HWND hwndDlg, bool workingDir)
{
  OPENFILENAME ofn;
  BROWSEINFO bi;
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);
  WCHAR tmp[MAX_PATH], program[MAX_PATH], arguments[MAX_LINE_LENGTH];
  WCHAR initPath[MAX_PATH];
  UINT type = GetValueType((int)SendMessage(typeWnd, CB_GETCURSEL, 0, 0));
  std::wstring workingPath;

  ZeroMemory(tmp, MAX_PATH);

  if ((type == IT_FILE_MENU) || workingDir)
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
              if (workingDir)
                SetDlgItemText(hwndDlg, IDC_WORKINGDIR, tmp);
              else
                SetDlgItemText(hwndDlg, IDC_ITEMVALUE, tmp);

              return true;
            }
        }
    }
  else
    {
      ZeroMemory(&ofn, sizeof(ofn));

      ELGetCurrentPath(initPath);

      ofn.lpstrInitialDir = initPath;
      ofn.lStructSize = sizeof(ofn);
      ofn.hwndOwner = hwndDlg;
      ofn.lpstrFilter = TEXT("All Files (*.*)\0*.*\0");
      GetDlgItemText(hwndDlg, IDC_ITEMVALUE, tmp, MAX_PATH);
      workingPath = tmp;
      workingPath = ELExpandVars(workingPath);
      if (ELParseCommand(workingPath.c_str(), program, arguments))
        wcscpy(tmp, program);
      ofn.lpstrFile = tmp;
      ofn.nMaxFile = MAX_PATH;
      ofn.lpstrTitle = TEXT("Browse For File");
      ofn.lpstrDefExt = NULL;
      ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS;

      if (GetOpenFileName(&ofn))
        {
          ELUnExpandVars(tmp);
          if (type == IT_SPECIAL_FOLDER)
            swprintf(tmp, TEXT("*%s"), PathFindFileName(tmp));

          SetDlgItemText(hwndDlg, IDC_ITEMVALUE, tmp);

          return true;
        }
    }

  return false;
}

int ItemEditor::GetTypeValue(UINT type)
{
  switch (type)
    {
    case IT_SEPARATOR:
      return 0;
    case IT_EXECUTABLE:
      return 1;
    case IT_INTERNAL_COMMAND:
      return 2;
    case IT_DATE_TIME:
      return 3;
    case IT_SPECIAL_FOLDER:
      return 4;
    case IT_XML_MENU:
      return 5;
    case IT_FILE_MENU:
      return 6;
    case IT_TASKS_MENU:
      return 7;
    case IT_SETTINGS_MENU:
      return 8;
    case IT_HELP_MENU:
      return 9;
    }

  return -1;
}

bool ItemEditor::ShowFields(HWND hwndDlg, int index)
{
  HWND valueWnd = GetDlgItem(hwndDlg, IDC_ITEMVALUE);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_ITEMCOMMAND);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_ITEMBROWSE);
  HWND specialFoldersWnd = GetDlgItem(hwndDlg, IDC_ITEMSPECIALFOLDERS);
  HWND commandArgWnd = GetDlgItem(hwndDlg, IDC_COMMANDARG);
  HWND workingDirWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIR);
  HWND dirBrowseWnd = GetDlgItem(hwndDlg, IDC_DIRBROWSE);

  switch (index)
    {
    case 0:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Execute:"));
      SetDlgItemText(hwndDlg, IDC_WORKINGDIRTEXT, TEXT("Working Directory:"));
      if (fileIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
      ShowWindow(valueWnd, true);
      ShowWindow(browseWnd, true);
      ShowWindow(commandWnd, false);
      ShowWindow(specialFoldersWnd, false);
      ShowWindow(commandArgWnd, false);
      ShowWindow(workingDirWnd, true);
      ShowWindow(dirBrowseWnd, true);
      break;
    case 2:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Command:"));
      SetDlgItemText(hwndDlg, IDC_WORKINGDIRTEXT, TEXT("Argument:"));
      if (fileIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
      ShowWindow(valueWnd, false);
      ShowWindow(browseWnd, false);
      ShowWindow(specialFoldersWnd, false);
      ShowWindow(commandWnd, true);
      ShowWindow(commandArgWnd, true);
      ShowWindow(workingDirWnd, false);
      ShowWindow(dirBrowseWnd, false);
      break;
    case 3:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Format:"));
      SetDlgItemText(hwndDlg, IDC_WORKINGDIRTEXT, TEXT("Working Directory:"));
      if (fileIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
      ShowWindow(valueWnd, true);
      ShowWindow(browseWnd, true);
      ShowWindow(commandWnd, false);
      ShowWindow(specialFoldersWnd, false);
      ShowWindow(commandArgWnd, false);
      ShowWindow(workingDirWnd, true);
      ShowWindow(dirBrowseWnd, true);
      break;
    case 4:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Special Folder:"));
      SetDlgItemText(hwndDlg, IDC_WORKINGDIRTEXT, TEXT("Working Directory:"));
      if (fileIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
      ShowWindow(valueWnd, false);
      ShowWindow(browseWnd, false);
      ShowWindow(commandWnd, false);
      ShowWindow(specialFoldersWnd, true);
      ShowWindow(commandArgWnd, false);
      ShowWindow(workingDirWnd, true);
      ShowWindow(dirBrowseWnd, true);
      break;
    case 5:
    case 7:
    case 8:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Execute:"));
      SetDlgItemText(hwndDlg, IDC_WORKINGDIRTEXT, TEXT("Working Directory:"));
      if (fileIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
      ShowWindow(valueWnd, true);
      ShowWindow(browseWnd, true);
      ShowWindow(commandWnd, false);
      ShowWindow(specialFoldersWnd, false);
      ShowWindow(commandArgWnd, false);
      ShowWindow(workingDirWnd, true);
      ShowWindow(dirBrowseWnd, true);
      break;
    case 1:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Execute:"));
      SetDlgItemText(hwndDlg, IDC_WORKINGDIRTEXT, TEXT("Working Directory:"));
      if (fileIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
      ShowWindow(valueWnd, true);
      ShowWindow(browseWnd, true);
      ShowWindow(commandWnd, false);
      ShowWindow(specialFoldersWnd, false);
      ShowWindow(commandArgWnd, false);
      ShowWindow(workingDirWnd, true);
      ShowWindow(dirBrowseWnd, true);
      break;
    case 6:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Path:"));
      SetDlgItemText(hwndDlg, IDC_WORKINGDIRTEXT, TEXT("Working Directory:"));
      if (browseIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)browseIcon);
      ShowWindow(valueWnd, true);
      ShowWindow(browseWnd, true);
      ShowWindow(commandWnd, false);
      ShowWindow(specialFoldersWnd, false);
      ShowWindow(commandArgWnd, false);
      ShowWindow(workingDirWnd, true);
      ShowWindow(dirBrowseWnd, true);
      break;
    default:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Execute:"));
      SetDlgItemText(hwndDlg, IDC_WORKINGDIRTEXT, TEXT("Working Directory:"));
      if (fileIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
      ShowWindow(valueWnd, true);
      ShowWindow(browseWnd, true);
      ShowWindow(commandWnd, false);
      ShowWindow(specialFoldersWnd, false);
      ShowWindow(commandArgWnd, false);
      ShowWindow(workingDirWnd, true);
      ShowWindow(dirBrowseWnd, true);
      break;
    }

  return true;
}

bool ItemEditor::EnableFields(HWND hwndDlg)
{
  HWND nameWnd = GetDlgItem(hwndDlg, IDC_ITEMNAME);
  HWND nameTextWnd = GetDlgItem(hwndDlg, IDC_NAMETEXT);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);
  HWND typeTextWnd = GetDlgItem(hwndDlg, IDC_TYPETEXT);
  HWND valueWnd = GetDlgItem(hwndDlg, IDC_ITEMVALUE);
  HWND valueTextWnd = GetDlgItem(hwndDlg, IDC_VALUETEXT);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_ITEMCOMMAND);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_ITEMBROWSE);
  HWND dirWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIR);
  HWND workingDirTextWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIRTEXT);
  HWND dirBrowseWnd = GetDlgItem(hwndDlg, IDC_DIRBROWSE);
  HWND specialFoldersWnd = GetDlgItem(hwndDlg, IDC_ITEMSPECIALFOLDERS);
  HWND commandArgWnd = GetDlgItem(hwndDlg, IDC_COMMANDARG);
  int index = GetValueType((int)SendMessage(typeWnd, CB_GETCURSEL, 0, 0));

  EnableWindow(typeWnd, true);
  EnableWindow(typeTextWnd, true);

  switch (index)
    {
    case IT_SEPARATOR:
      EnableWindow(dirWnd, false);
      EnableWindow(workingDirTextWnd, false);
      EnableWindow(dirBrowseWnd, false);
      EnableWindow(nameWnd, false);
      EnableWindow(nameTextWnd, false);
      EnableWindow(valueWnd, false);
      EnableWindow(valueTextWnd, false);
      EnableWindow(browseWnd, false);
      EnableWindow(commandArgWnd, false);
      break;
    case IT_INTERNAL_COMMAND:
      EnableWindow(dirWnd, false);
      EnableWindow(workingDirTextWnd, true);
      EnableWindow(dirBrowseWnd, false);
      EnableWindow(commandWnd, true);
      EnableWindow(commandArgWnd, true);
      EnableWindow(nameWnd, true);
      EnableWindow(nameTextWnd, true);
      EnableWindow(valueTextWnd, true);
      break;
    case IT_DATE_TIME:
      EnableWindow(dirWnd, false);
      EnableWindow(workingDirTextWnd, false);
      EnableWindow(dirBrowseWnd, false);
      EnableWindow(valueWnd, true);
      EnableWindow(valueTextWnd, true);
      EnableWindow(browseWnd, false);
      EnableWindow(nameWnd, false);
      EnableWindow(nameTextWnd, true);
      EnableWindow(commandArgWnd, false);
      break;
    case IT_SPECIAL_FOLDER:
      EnableWindow(dirWnd, false);
      EnableWindow(workingDirTextWnd, false);
      EnableWindow(dirBrowseWnd, false);
      EnableWindow(specialFoldersWnd, true);
      EnableWindow(nameWnd, true);
      EnableWindow(nameTextWnd, true);
      EnableWindow(valueTextWnd, true);
      EnableWindow(commandArgWnd, false);
      break;
    case IT_XML_MENU:
    case IT_SETTINGS_MENU:
    case IT_HELP_MENU:
    case IT_TASKS_MENU:
      EnableWindow(dirWnd, false);
      EnableWindow(workingDirTextWnd, false);
      EnableWindow(dirBrowseWnd, false);
      EnableWindow(valueWnd, false);
      EnableWindow(valueTextWnd, false);
      EnableWindow(browseWnd, false);
      EnableWindow(nameWnd, true);
      EnableWindow(nameTextWnd, true);
      EnableWindow(commandArgWnd, false);
      break;
    case IT_EXECUTABLE:
      EnableWindow(dirWnd, true);
      EnableWindow(workingDirTextWnd, true);
      EnableWindow(dirBrowseWnd, true);
      EnableWindow(nameWnd, true);
      EnableWindow(nameTextWnd, true);
      EnableWindow(valueWnd, true);
      EnableWindow(valueTextWnd, true);
      EnableWindow(browseWnd, true);
      EnableWindow(commandArgWnd, false);
      break;
    case IT_FILE_MENU:
      EnableWindow(dirWnd, false);
      EnableWindow(workingDirTextWnd, false);
      EnableWindow(dirBrowseWnd, false);
      EnableWindow(nameWnd, true);
      EnableWindow(nameTextWnd, true);
      EnableWindow(valueWnd, true);
      EnableWindow(valueTextWnd, true);
      EnableWindow(browseWnd, true);
      EnableWindow(commandArgWnd, false);
      break;
    default:
      EnableWindow(dirWnd, false);
      EnableWindow(workingDirTextWnd, false);
      EnableWindow(dirBrowseWnd, false);
      EnableWindow(nameWnd, true);
      EnableWindow(nameTextWnd, true);
      EnableWindow(valueWnd, false);
      EnableWindow(valueTextWnd, false);
      EnableWindow(browseWnd, false);
      EnableWindow(commandArgWnd, false);
    }

  return ShowFields(hwndDlg, index);
}

bool ItemEditor::DoSelChange(HWND hwndDlg, HWND typeWnd)
{
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_ITEMBROWSE);

  SetDlgItemText(hwndDlg, IDC_ITEMVALUE, TEXT("\0"));

  SetTooltip(browseWnd, GetValueType((int)SendMessage(typeWnd, CB_GETCURSEL, 0, 0)));

  return EnableFields(hwndDlg);
}

UINT ItemEditor::GetValueType(int value)
{
  switch (value)
    {
    case 0:
      return IT_SEPARATOR;
    case 1:
      return IT_EXECUTABLE;
    case 2:
      return IT_INTERNAL_COMMAND;
    case 3:
      return IT_DATE_TIME;
    case 4:
      return IT_SPECIAL_FOLDER;
    case 5:
      return IT_XML_MENU;
    case 6:
      return IT_FILE_MENU;
    case 7:
      return IT_TASKS_MENU;
    case 8:
      return IT_SETTINGS_MENU;
    case 9:
      return IT_HELP_MENU;
    }

  return 0;
}

void ItemEditor::SetTooltip(HWND browseWnd, UINT type)
{
  TOOLINFO ti;
  ZeroMemory(&ti, sizeof(TOOLINFO));

  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.hwnd = browseWnd;
  ti.uId = (ULONG_PTR)browseWnd;

  SendMessage(toolWnd, TTM_GETTOOLINFO, 0,(LPARAM) (LPTOOLINFO) &ti);

  switch (type)
    {
    case IT_EXECUTABLE:
      ti.lpszText = (WCHAR*)TEXT("Browse for a file");
      break;
    case IT_SPECIAL_FOLDER:
      ti.lpszText = (WCHAR*)TEXT("Browse for an special folder");
      break;
    case IT_FILE_MENU:
      ti.lpszText = (WCHAR*)TEXT("Browse for a directory");
    }

  SendMessage(toolWnd, TTM_SETTOOLINFO, 0, (LPARAM)(LPTOOLINFO)&ti);
}

bool ItemEditor::DoSaveItem(HWND hwndDlg)
{
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);
  HWND nameWnd = GetDlgItem(hwndDlg, IDC_ITEMNAME);
  WCHAR name[MAX_LINE_LENGTH], value[MAX_LINE_LENGTH], workingDir[MAX_LINE_LENGTH], tmp[MAX_LINE_LENGTH];
  ZeroMemory(name, MAX_LINE_LENGTH);
  ZeroMemory(workingDir, MAX_LINE_LENGTH);
  ZeroMemory(tmp, MAX_LINE_LENGTH);
  int type;

  if (IsWindowEnabled(nameWnd))
    {
      GetDlgItemText(hwndDlg, IDC_ITEMNAME, name, MAX_LINE_LENGTH);
      if (wcslen(name) == 0)
        {
          ELMessageBox(hwndDlg, (WCHAR*)TEXT("Name cannot be empty"), (WCHAR*)TEXT("emergeWorkspace"),
                       ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
          return false;
        }
    }

  type = GetValueType((int)SendMessage(typeWnd, CB_GETCURSEL, 0, 0));
  switch (type)
    {
    case 2:
      GetDlgItemText(hwndDlg, IDC_ITEMCOMMAND, value, MAX_LINE_LENGTH);
      GetDlgItemText(hwndDlg, IDC_COMMANDARG, tmp, MAX_LINE_LENGTH);
      if (wcslen(tmp))
        {
          wcscat(value, L" ");
          wcscat(value, tmp);
        }
      break;
    case 4:
      GetDlgItemText(hwndDlg, IDC_ITEMSPECIALFOLDERS, tmp, MAX_LINE_LENGTH);
      if (!ELSpecialFolderValue(tmp, value))
        wcscpy(value, TEXT(""));
      break;
    default:
      GetDlgItemText(hwndDlg, IDC_ITEMVALUE, value, MAX_LINE_LENGTH);
      break;
    }
  GetDlgItemText(hwndDlg, IDC_WORKINGDIR, workingDir, MAX_LINE_LENGTH);

  ELWriteXMLStringValue(section, (WCHAR*)TEXT("Name"), name);
  ELWriteXMLIntValue(section, (WCHAR*)TEXT("Type"), type);
  if (wcslen(value) > 0)
    {
      ELRelativePathFromAbsPath(value, MAX_LINE_LENGTH);
      ELWriteXMLStringValue(section, (WCHAR*)TEXT("Value"), value);
    }
  if (wcslen(workingDir) > 0)
    {
      ELRelativePathFromAbsPath(workingDir, MAX_LINE_LENGTH);
      ELWriteXMLStringValue(section, (WCHAR*)TEXT("WorkingDir"), workingDir);
    }

  TiXmlDocument *configXML = ELGetXMLConfig(section);
  if (configXML)
    return ELWriteXMLConfig(configXML);

  return false;
}
