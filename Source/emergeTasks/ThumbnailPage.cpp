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
#include "ThumbnailPage.h"
#include "Applet.h"

INT_PTR CALLBACK ThumbnailPage::ThumbnailPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static ThumbnailPage *pThumbnailPage = NULL;
  PROPSHEETPAGE *psp;

  switch (message)
    {
    case WM_INITDIALOG:
      psp = (PROPSHEETPAGE*)lParam;
      pThumbnailPage = reinterpret_cast<ThumbnailPage*>(psp->lParam);
      if (!pThumbnailPage)
        break;
      return pThumbnailPage->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pThumbnailPage->DoCommand(hwndDlg, wParam, lParam);

    case WM_NOTIFY:
      if (!pThumbnailPage)
        break;
      return pThumbnailPage->DoNotify(hwndDlg, lParam);
    }

  return FALSE;
}

ThumbnailPage::ThumbnailPage(std::tr1::shared_ptr<Settings> pSettings)
{
  this->pSettings = pSettings;
  colourRect.left = colourRect.top = 0;
  colourRect.right = colourRect.bottom = 16;
  hbmColourFrom = NULL;
  hbmColourTo = NULL;
  buttonTitleFont = NULL;
  ColourFrom = RGB(0,0,0);
  ColourTo = RGB(0,0,0);
  ColourText = RGB(0,0,0);
  ColourBorder = RGB(0,0,0);
  hbmColourText = NULL;
  hbmColourBorder = NULL;
}

ThumbnailPage::~ThumbnailPage()
{
  if (buttonTitleFont)
    DeleteObject(buttonTitleFont);
  if (buttonInfoFont)
    DeleteObject(buttonInfoFont);
  if (hbmColourFrom)
    DeleteObject(hbmColourFrom);
  if (hbmColourTo)
    DeleteObject(hbmColourTo);
}

BOOL ThumbnailPage::DoInitDialog(HWND hwndDlg)
{
  if (pSettings->GetEnableThumbnails())
    SendDlgItemMessage(hwndDlg, IDC_ENABLETHUMBNAIL, BM_SETCHECK, BST_CHECKED, 0);

  /*HWND sliderWnd = GetDlgItem(hwndDlg, IDC_SLIDER);
  SendMessage(sliderWnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 100));
  SendMessage(sliderWnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pSettings->GetAlpha());
  SetDlgItemInt(hwndDlg, IDC_ALPHA, pSettings->GetAlpha(), false);

  CopyMemory(&newTitleFont, pSettings->GetInfoTitleFont(), sizeof(LOGFONT));
  if (buttonTitleFont)
    DeleteObject(buttonTitleFont);
  buttonTitleFont = CreateFontIndirect(&newTitleFont);
  SendDlgItemMessage(hwndDlg, IDC_TITLEFONTBUTTON, WM_SETFONT, (WPARAM)buttonTitleFont, (LPARAM)TRUE);
  SetDlgItemText(hwndDlg, IDC_TITLEFONTBUTTON, newTitleFont.lfFaceName);

  CopyMemory(&newInfoFont, pSettings->GetInfoFont(), sizeof(LOGFONT));
  if (buttonInfoFont)
    DeleteObject(buttonInfoFont);
  buttonInfoFont = CreateFontIndirect(&newInfoFont);
  SendDlgItemMessage(hwndDlg, IDC_INFOFONTBUTTON, WM_SETFONT, (WPARAM)buttonInfoFont, (LPARAM)TRUE);
  SetDlgItemText(hwndDlg, IDC_INFOFONTBUTTON, newInfoFont.lfFaceName);

  // colors
  ColourFrom = pSettings->GetGradientFrom();
  ColourTo = pSettings->GetGradientTo();
  ColourText = pSettings->GetTextColor();
  ColourBorder = pSettings->GetBorderColor();

  if (SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_GETCOUNT, 0, 0) == 0)
    {
      SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Solid"));
      SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Vertical"));
      SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Horizontal"));
      SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("CrossDiagonal"));
      SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Diagonal"));
      SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Pipecross"));
      SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Elliptic"));
      SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Rectangle"));
      SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Pyramid"));
    }

  std::wstring lower = pSettings->GetGradientMethod();
  lower = ELToLower(lower);
  if (lower.find(TEXT("vertical")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_SETCURSEL, (WPARAM)1, 0);
  else if (lower.find(TEXT("horizontal")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_SETCURSEL, (WPARAM)2, 0);
  else if (lower.find(TEXT("crossdiagonal")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_SETCURSEL, (WPARAM)3, 0);
  else if (lower.find(TEXT("diagonal")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_SETCURSEL, (WPARAM)4, 0);
  else if (lower.find(TEXT("pipecross")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_SETCURSEL, (WPARAM)5, 0);
  else if (lower.find(TEXT("elliptic")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_SETCURSEL, (WPARAM)6, 0);
  else if (lower.find(TEXT("rectangle")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_SETCURSEL, (WPARAM)7, 0);
  else if (lower.find(TEXT("pyramid")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_SETCURSEL, (WPARAM)8, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_GRADIENTMETHOD, CB_SETCURSEL, (WPARAM)0, 0);

  hbmColourFrom = EGCreateBitmap(0xff, ColourFrom, colourRect);
  if (hbmColourFrom)
    SendDlgItemMessage(hwndDlg, IDC_GRADIENTFROMCOLOUR, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmColourFrom);

  hbmColourTo = EGCreateBitmap(0xff, ColourTo, colourRect);
  if (hbmColourTo)
    SendDlgItemMessage(hwndDlg, IDC_GRADIENTTOCOLOUR, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmColourTo);

  hbmColourText = EGCreateBitmap(0xff, ColourText, colourRect);
  if (hbmColourText)
    SendDlgItemMessage(hwndDlg, IDC_INFOTEXTCOLOUR, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmColourText);

  hbmColourBorder = EGCreateBitmap(0xff, ColourBorder, colourRect);
  if (hbmColourBorder)
    SendDlgItemMessage(hwndDlg, IDC_INFOBORDERCOLOUR, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmColourBorder);*/

  return TRUE;
}

BOOL ThumbnailPage::DoCommand(HWND hwndDlg UNUSED, WPARAM wParam UNUSED, LPARAM lParam UNUSED)
{
  /*switch (LOWORD(wParam))
    {
    case IDC_TITLEFONTBUTTON:
      if (DoFontChooser(hwndDlg, &newTitleFont))
        {
          if (buttonTitleFont)
            DeleteObject(buttonTitleFont);
          buttonTitleFont = CreateFontIndirect(&newTitleFont);
          SendDlgItemMessage(hwndDlg, IDC_TITLEFONTBUTTON, WM_SETFONT, (WPARAM)buttonTitleFont, (LPARAM)TRUE);
          SetDlgItemText(hwndDlg, IDC_TITLEFONTBUTTON, newTitleFont.lfFaceName);

          return TRUE;
        }
      return FALSE;
    case IDC_INFOFONTBUTTON:
      if (DoFontChooser(hwndDlg, &newInfoFont))
        {
          if (buttonInfoFont)
            DeleteObject(buttonInfoFont);
          buttonInfoFont = CreateFontIndirect(&newInfoFont);
          SendDlgItemMessage(hwndDlg, IDC_INFOFONTBUTTON, WM_SETFONT, (WPARAM)buttonInfoFont, (LPARAM)TRUE);
          SetDlgItemText(hwndDlg, IDC_INFOFONTBUTTON, newInfoFont.lfFaceName);

          return TRUE;
        }
    case IDC_GRADIENTFROMCOLOUR:
      if (DoColourChooser(&ColourFrom, hwndDlg))
        {
          UpdateColorButton(hwndDlg, hbmColourFrom, ColourFrom, LOWORD(wParam));
          return TRUE;
        }
      return FALSE;
    case IDC_GRADIENTTOCOLOUR:
      if (DoColourChooser(&ColourTo, hwndDlg))
        {
          UpdateColorButton(hwndDlg, hbmColourTo, ColourTo, LOWORD(wParam));
          return TRUE;
        }
      return FALSE;
    case IDC_INFOTEXTCOLOUR:
      if (DoColourChooser(&ColourText, hwndDlg))
        {
          UpdateColorButton(hwndDlg, hbmColourText, ColourText, LOWORD(wParam));
          return TRUE;
        }
      return FALSE;
    case IDC_INFOBORDERCOLOUR:
      if (DoColourChooser(&ColourBorder, hwndDlg))
        {
          UpdateColorButton(hwndDlg, hbmColourBorder, ColourBorder, LOWORD(wParam));
          return TRUE;
        }
      return FALSE;
    }*/

  return FALSE;
}

INT_PTR ThumbnailPage::DoNotify(HWND hwndDlg, LPARAM lParam)
{
  //HWND sliderWnd = GetDlgItem(hwndDlg, IDC_SLIDER);
  NMHDR *phdr = (NMHDR*)lParam;

  /*if (phdr->hwndFrom == sliderWnd)
    {
      UINT sliderValue = (UINT)SendMessage(sliderWnd, TBM_GETPOS, 0, 0);
      SetDlgItemInt(hwndDlg, IDC_ALPHA, sliderValue, false);

      return 1;
    }*/

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

bool ThumbnailPage::UpdateSettings(HWND hwndDlg)
{
  if (SendDlgItemMessage(hwndDlg, IDC_ENABLETHUMBNAIL, BM_GETCHECK, 0, 0) == BST_CHECKED)
    pSettings->SetEnableThumbnails(true);
  else if (SendDlgItemMessage(hwndDlg, IDC_ENABLETHUMBNAIL, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    pSettings->SetEnableThumbnails(false);

  /*BOOL success;
  UINT result;

  WCHAR methodString[MAX_LINE_LENGTH];
  GetDlgItemText(hwndDlg, IDC_GRADIENTMETHOD, methodString, MAX_LINE_LENGTH);
  pSettings->SetGradientMethod(methodString);
  pSettings->SetGradientFrom(ColourFrom);
  pSettings->SetGradientTo(ColourTo);

  pSettings->SetInfoTitleFont(&newTitleFont);
  pSettings->SetInfoFont(&newInfoFont);

  result = GetDlgItemInt(hwndDlg, IDC_ALPHA, &success, false);
  if (success)
    pSettings->SetAlpha(result);
  else
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("Invalid value for alpha"), (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      SetDlgItemInt(hwndDlg, IDC_MENUALPHA, pSettings->GetAlpha(), false);
      return false;
    }

  pSettings->SetTextColor(ColourText);
  pSettings->SetBorderColor(ColourBorder);

  // commit the changes, if any
  pSettings->WriteSettings();*/

  return true;
}

