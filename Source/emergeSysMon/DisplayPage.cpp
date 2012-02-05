//---
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
//---

#include <string>
#include <algorithm>
#include <cctype>
#include "DisplayPage.h"
#include "Applet.h"

static COLORREF custColours[16];

INT_PTR CALLBACK DisplayPage::DisplayPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static DisplayPage *pDisplayPage = NULL;
  PROPSHEETPAGE *psp;

  switch (message)
    {
    case WM_INITDIALOG:
      psp = (PROPSHEETPAGE*)lParam;
      pDisplayPage = reinterpret_cast<DisplayPage*>(psp->lParam);
      if (!pDisplayPage)
        break;
      return pDisplayPage->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pDisplayPage->DoCommand(hwndDlg, wParam, lParam);

    case WM_NOTIFY:
      if (!pDisplayPage)
        break;
      return pDisplayPage->DoNotify(hwndDlg, lParam);
    }

  return FALSE;
}

DisplayPage::DisplayPage(std::tr1::shared_ptr<Settings> pSettings)
{
  this->pSettings = pSettings;
  colourRect.left = colourRect.top = 0;
  colourRect.right = colourRect.bottom = 16;
  hbmCPUColourFrom = NULL;
  hbmCPUColourTo = NULL;
  hbmMemColourFrom = NULL;
  hbmMemColourTo = NULL;
}

DisplayPage::~DisplayPage()
{
  if (buttonFont)
    DeleteObject(buttonFont);
  if (hbmCPUColourFrom)
    DeleteObject(hbmCPUColourFrom);
  if (hbmCPUColourTo)
    DeleteObject(hbmCPUColourTo);
  if (hbmMemColourFrom)
    DeleteObject(hbmMemColourFrom);
  if (hbmMemColourTo)
    DeleteObject(hbmMemColourTo);
}

BOOL DisplayPage::DoInitDialog(HWND hwndDlg)
{
  if (pSettings->GetShowNumbers())
    SendDlgItemMessage(hwndDlg, IDC_SHOWNUMBERS, BM_SETCHECK, BST_CHECKED, 0);

  if (_wcsicmp(pSettings->GetNumberPosition(), TEXT("left")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_TEXTLEFT, BM_SETCHECK, BST_CHECKED, 0);
  if (_wcsicmp(pSettings->GetNumberPosition(), TEXT("down")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_TEXTDOWN, BM_SETCHECK, BST_CHECKED, 0);
  if (_wcsicmp(pSettings->GetNumberPosition(), TEXT("right")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_TEXTRIGHT, BM_SETCHECK, BST_CHECKED, 0);
  if (_wcsicmp(pSettings->GetNumberPosition(), TEXT("up")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_TEXTUP, BM_SETCHECK, BST_CHECKED, 0);

  CopyMemory(&newFont, pSettings->GetFont(), sizeof(LOGFONT));
  if (buttonFont)
    DeleteObject(buttonFont);
  buttonFont = CreateFontIndirect(&newFont);
  SendDlgItemMessage(hwndDlg, IDC_FONTBUTTON, WM_SETFONT, (WPARAM)buttonFont, (LPARAM)TRUE);
  SetDlgItemText(hwndDlg, IDC_FONTBUTTON, newFont.lfFaceName);

  // colors
  CPUColourFrom = pSettings->GetCPUGradientFrom();
  CPUColourTo = pSettings->GetCPUGradientTo();
  MemColourFrom = pSettings->GetMemGradientFrom();
  MemColourTo = pSettings->GetMemGradientTo();

  if (SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_GETCOUNT, 0, 0) == 0)
    {
      SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_ADDSTRING, 0, (LPARAM)TEXT("Solid"));
      SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_ADDSTRING, 0, (LPARAM)TEXT("Vertical"));
      SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_ADDSTRING, 0, (LPARAM)TEXT("Horizontal"));
      SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_ADDSTRING, 0, (LPARAM)TEXT("CrossDiagonal"));
      SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_ADDSTRING, 0, (LPARAM)TEXT("Diagonal"));
      SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_ADDSTRING, 0, (LPARAM)TEXT("Pipecross"));
      SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_ADDSTRING, 0, (LPARAM)TEXT("Elliptic"));
      SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_ADDSTRING, 0, (LPARAM)TEXT("Rectangle"));
      SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_ADDSTRING, 0, (LPARAM)TEXT("Pyramid"));
    }

  std::wstring lower = pSettings->GetCPUGradientMethod();
  std::transform(lower.begin(), lower.end(), lower.begin(), (int(*)(int)) std::tolower);
  if (lower.find(TEXT("vertical")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_SETCURSEL, (WPARAM)1, 0);
  else if (lower.find(TEXT("horizontal")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_SETCURSEL, (WPARAM)2, 0);
  else if (lower.find(TEXT("crossdiagonal")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_SETCURSEL, (WPARAM)3, 0);
  else if (lower.find(TEXT("diagonal")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_SETCURSEL, (WPARAM)4, 0);
  else if (lower.find(TEXT("pipecross")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_SETCURSEL, (WPARAM)5, 0);
  else if (lower.find(TEXT("elliptic")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_SETCURSEL, (WPARAM)6, 0);
  else if (lower.find(TEXT("rectangle")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_SETCURSEL, (WPARAM)7, 0);
  else if (lower.find(TEXT("pyramid")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_SETCURSEL, (WPARAM)8, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_METHODCPU, CB_SETCURSEL, (WPARAM)0, 0);

  hbmCPUColourFrom = EGCreateBitmap(0xff, CPUColourFrom, colourRect);
  if (hbmCPUColourFrom)
    SendDlgItemMessage(hwndDlg, IDC_FROMCOLOURCPU, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCPUColourFrom);

  hbmCPUColourTo = EGCreateBitmap(0xff, CPUColourTo, colourRect);
  if (hbmCPUColourTo)
    SendDlgItemMessage(hwndDlg, IDC_TOCOLOURCPU, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCPUColourTo);

  if (SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_GETCOUNT, 0, 0) == 0)
    {
      SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_ADDSTRING, 0, (LPARAM)TEXT("Solid"));
      SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_ADDSTRING, 0, (LPARAM)TEXT("Vertical"));
      SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_ADDSTRING, 0, (LPARAM)TEXT("Horizontal"));
      SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_ADDSTRING, 0, (LPARAM)TEXT("CrossDiagonal"));
      SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_ADDSTRING, 0, (LPARAM)TEXT("Diagonal"));
      SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_ADDSTRING, 0, (LPARAM)TEXT("Pipecross"));
      SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_ADDSTRING, 0, (LPARAM)TEXT("Elliptic"));
      SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_ADDSTRING, 0, (LPARAM)TEXT("Rectangle"));
      SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_ADDSTRING, 0, (LPARAM)TEXT("Pyramid"));
    }

  lower = pSettings->GetMemGradientMethod();
  std::transform(lower.begin(), lower.end(), lower.begin(), (int(*)(int)) std::tolower);
  if (lower.find(TEXT("vertical")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_SETCURSEL, (WPARAM)1, 0);
  else if (lower.find(TEXT("horizontal")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_SETCURSEL, (WPARAM)2, 0);
  else if (lower.find(TEXT("crossdiagonal")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_SETCURSEL, (WPARAM)3, 0);
  else if (lower.find(TEXT("diagonal")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_SETCURSEL, (WPARAM)4, 0);
  else if (lower.find(TEXT("pipecross")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_SETCURSEL, (WPARAM)5, 0);
  else if (lower.find(TEXT("elliptic")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_SETCURSEL, (WPARAM)6, 0);
  else if (lower.find(TEXT("rectangle")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_SETCURSEL, (WPARAM)7, 0);
  else if (lower.find(TEXT("pyramid")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_SETCURSEL, (WPARAM)8, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_METHODMEM, CB_SETCURSEL, (WPARAM)0, 0);

  hbmMemColourFrom = EGCreateBitmap(0xff, MemColourFrom, colourRect);
  if (hbmMemColourFrom)
    SendDlgItemMessage(hwndDlg, IDC_FROMCOLOURMEM, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmMemColourFrom);

  hbmMemColourTo = EGCreateBitmap(0xff, MemColourTo, colourRect);
  if (hbmMemColourTo)
    SendDlgItemMessage(hwndDlg, IDC_TOCOLOURMEM, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmMemColourTo);

  UpdateEnabledStates(hwndDlg);

  return TRUE;
}

BOOL DisplayPage::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  switch (LOWORD(wParam))
    {
    case IDC_FONTBUTTON:
      if (DoFontChooser(hwndDlg))
        {
          if (buttonFont)
            DeleteObject(buttonFont);
          buttonFont = CreateFontIndirect(&newFont);
          SendDlgItemMessage(hwndDlg, IDC_FONTBUTTON, WM_SETFONT, (WPARAM)buttonFont, (LPARAM)TRUE);
          SetDlgItemText(hwndDlg, IDC_FONTBUTTON, newFont.lfFaceName);

          return TRUE;
        }

      return FALSE;
    case IDC_SHOWNUMBERS:
      UpdateEnabledStates(hwndDlg);
      return TRUE;
    case IDC_FROMCOLOURCPU:
      if (DoColourChooser(&CPUColourFrom, hwndDlg))
        {
          UpdateColorButton(hwndDlg, hbmCPUColourFrom, CPUColourFrom, LOWORD(wParam));
          return TRUE;
        }
      return FALSE;
    case IDC_TOCOLOURCPU:
      if (DoColourChooser(&CPUColourTo, hwndDlg))
        {
          UpdateColorButton(hwndDlg, hbmCPUColourTo, CPUColourTo, LOWORD(wParam));
          return TRUE;
        }
      return FALSE;
    case IDC_FROMCOLOURMEM:
      if (DoColourChooser(&MemColourFrom, hwndDlg))
        {
          UpdateColorButton(hwndDlg, hbmMemColourFrom, MemColourFrom, LOWORD(wParam));
          return TRUE;
        }
      return FALSE;
    case IDC_TOCOLOURMEM:
      if (DoColourChooser(&MemColourTo, hwndDlg))
        {
          UpdateColorButton(hwndDlg, hbmMemColourTo, MemColourTo, LOWORD(wParam));
          return TRUE;
        }
      return FALSE;
    }

  return FALSE;
}

INT_PTR DisplayPage::DoNotify(HWND hwndDlg, LPARAM lParam)
{
  NMHDR *phdr = (NMHDR*)lParam;

  switch (phdr->code)
    {
    case PSN_APPLY:
      if (UpdateSettings(hwndDlg))
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
      else
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID);
      return 1;

    case PSN_SETACTIVE:
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
      return 1;

    case PSN_KILLACTIVE:
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
      return 1;
    }

  return 0;
}

void DisplayPage::UpdateColorButton(HWND hwndDlg, HBITMAP bitmap, COLORREF colour, int controlId)
{
  if (bitmap)
    DeleteObject(bitmap);
  bitmap = EGCreateBitmap(0xff, colour, colourRect);
  if (bitmap)
    DeleteObject((HBITMAP)SendDlgItemMessage(hwndDlg, controlId, BM_SETIMAGE,
                 IMAGE_BITMAP, (LPARAM)bitmap));
}

bool DisplayPage::DoFontChooser(HWND hwndDlg)
{
  CHOOSEFONT chooseFont;

  ZeroMemory(&chooseFont, sizeof(CHOOSEFONT));
  chooseFont.lStructSize = sizeof(CHOOSEFONT);
  chooseFont.hwndOwner = hwndDlg;
  chooseFont.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_NOSCRIPTSEL;
  chooseFont.lpLogFont = &newFont;

  return (ChooseFont(&chooseFont) == TRUE);
}

BOOL DisplayPage::DoColourChooser(COLORREF *colour, HWND hwndDlg)
{
  CHOOSECOLOR chooseColour;
  COLORREF tmpColour = *colour;
  BOOL res;

  ZeroMemory(&chooseColour, sizeof(CHOOSECOLOR));
  chooseColour.lStructSize = sizeof(CHOOSECOLOR);
  chooseColour.hwndOwner = hwndDlg;
  chooseColour.rgbResult = tmpColour;
  chooseColour.lpCustColors = (LPDWORD)custColours;
  chooseColour.Flags = CC_RGBINIT | CC_FULLOPEN;

  res = ChooseColor(&chooseColour);

  if (res)
    *colour = chooseColour.rgbResult;

  return res;
}

bool DisplayPage::UpdateSettings(HWND hwndDlg)
{
  BOOL success = true;
  const WCHAR *tmpValue = NULL;

  if (SendDlgItemMessage(hwndDlg, IDC_SHOWNUMBERS, BM_GETCHECK, 0, 0) == BST_CHECKED)
    success = true;
  else if (SendDlgItemMessage(hwndDlg, IDC_SHOWNUMBERS, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    success = false;
  pSettings->SetShowNumbers(success);

  if (SendDlgItemMessage(hwndDlg, IDC_TEXTUP, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("up");
  if (SendDlgItemMessage(hwndDlg, IDC_TEXTRIGHT, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("right");
  if (SendDlgItemMessage(hwndDlg, IDC_TEXTDOWN, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("down");
  if (SendDlgItemMessage(hwndDlg, IDC_TEXTLEFT, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("left");
  pSettings->SetNumberPosition((WCHAR*)tmpValue);

  WCHAR methodString[MAX_LINE_LENGTH];
  GetDlgItemText(hwndDlg, IDC_METHODCPU, methodString, MAX_LINE_LENGTH);
  pSettings->SetCPUGradientMethod(methodString);
  pSettings->SetCPUGradientFrom(CPUColourFrom);
  pSettings->SetCPUGradientTo(CPUColourTo);
  GetDlgItemText(hwndDlg, IDC_METHODMEM, methodString, MAX_LINE_LENGTH);
  pSettings->SetMemGradientMethod(methodString);
  pSettings->SetMemGradientFrom(MemColourFrom);
  pSettings->SetMemGradientTo(MemColourTo);

  pSettings->SetFont(&newFont);

  // commit the changes, if any
  pSettings->WriteSettings();

  return true;
}

void DisplayPage::UpdateEnabledStates(HWND hwndDlg)
{
  bool showNumbers = SendDlgItemMessage(hwndDlg, IDC_SHOWNUMBERS, BM_GETCHECK, 0, 0) == BST_CHECKED;
  EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTUP), showNumbers);
  EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTDOWN), showNumbers);
  EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTLEFT), showNumbers);
  EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTRIGHT), showNumbers);
}
