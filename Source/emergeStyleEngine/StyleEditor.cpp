// vim: tags+=../emergeLib/tags,../emergeGraphics/tags
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

#include "StyleEditor.h"

static COLORREF custColours[16];

INT_PTR CALLBACK StyleEditor::StyleEditorDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static StyleEditor *pStyleEditor = NULL;
  PROPSHEETPAGE *psp;

  switch (message)
    {
    case WM_INITDIALOG:
      psp = (PROPSHEETPAGE*)lParam;
      pStyleEditor = reinterpret_cast<StyleEditor*>(psp->lParam);
      if (!pStyleEditor)
        break;
      return pStyleEditor->DoInitDialog(hwndDlg, true);

    case WM_COMMAND:
      if (!pStyleEditor)
        break;
      return pStyleEditor->DoCommand(hwndDlg, wParam, lParam);

    case WM_NOTIFY:
      if (!pStyleEditor)
        break;
      return pStyleEditor->DoNotify(hwndDlg, lParam);
    }

  return FALSE;
}

StyleEditor::StyleEditor(HWND mainWnd, std::wstring instanceName)
{
  this->mainWnd = mainWnd;
  this->instanceName = instanceName;

  tmpFile = ELGetTempFileName();

  hbmColourBackground = NULL;
  hbmColourForeground = NULL;
  hbmColourSelected = NULL;
  hbmColourFrame = NULL;
  hbmColourFont = NULL;

  colourRect.left = colourRect.top = 0;
  colourRect.right = colourRect.bottom = 16;
}

StyleEditor::~StyleEditor()
{
  if (hbmColourBackground)
    DeleteObject(hbmColourBackground);
  if (hbmColourForeground)
    DeleteObject(hbmColourForeground);
  if (hbmColourSelected)
    DeleteObject(hbmColourSelected);
  if (hbmColourFrame)
    DeleteObject(hbmColourFrame);
  if (hbmColourFont)
    DeleteObject(hbmColourFont);
  if (hbmColourFrom)
    DeleteObject(hbmColourFrom);
  if (hbmColourTo)
    DeleteObject(hbmColourTo);
  if (hbmColourBorder)
    DeleteObject(hbmColourBorder);

  ClearPanelMap();

  DeleteFile(tmpFile.c_str());
}

int StyleEditor::Edit(std::wstring styleName)
{
  style = styleName;
  origStyle = style;

  ESEReadStyle(style, &guiInfo);
  ESEReadStyle((WCHAR*)TEXT(""), &defaultGuiInfo);

  origGuiInfo = guiInfo;

  colourBackground = guiInfo.colorBackground;
  colourForeground = guiInfo.colorForeground;
  colourSelected = guiInfo.colorSelected;
  colourFrame = guiInfo.colorFrame;
  colourFont = guiInfo.colorFont;
  colourTo = guiInfo.gradientTo;
  colourFrom = guiInfo.gradientFrom;
  colourBorder = guiInfo.colorBorder;

  return 1;
}

LPCTSTR StyleEditor::GetTemplate()
{
  return MAKEINTRESOURCE(IDD_STYLEEDITOR);
}

void StyleEditor::BuildPanelMap(HWND hwndDlg)
{
  std::map< HTREEITEM, std::tr1::shared_ptr<PanelSet> >::iterator iter;

  iter = panelMap.find(hitemOpacity);
  if (iter != panelMap.end())
    {
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC2));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC3));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC4));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC5));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC6));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC7));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC8));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC9));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC10));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC11));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_ACTIVESLIDER));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_ACTIVEALPHA));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_INACTIVESLIDER));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_INACTIVEALPHA));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_BACKGROUNDSLIDER));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_BACKGROUNDALPHA));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_MENUSLIDER));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_MENUALPHA));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_FOREGROUNDSLIDER));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_FOREGROUNDALPHA));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_FRAMESLIDER));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_FRAMEALPHA));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_SELECTEDSLIDER));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_SELECTEDALPHA));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_BORDERSLIDER));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_BORDERALPHA));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_TEXTSLIDER));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_TEXTALPHA));
    }

  iter = panelMap.find(hitemColor);
  if (iter != panelMap.end())
    {
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC12));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC13));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC14));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC15));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC16));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC17));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC18));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_BACKGROUNDCOLOUR));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_FOREGROUNDCOLOUR));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_SELECTEDCOLOUR));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_FRAMECOLOUR));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_FONTCOLOUR));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_BORDERCOLOUR));
    }

  iter = panelMap.find(hitemGradient);
  if (iter != panelMap.end())
    {
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC22));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC23));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC24));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC25));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_METHOD));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_INTERLACED));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_FROMCOLOUR));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_TOCOLOUR));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_SWITCH));
    }

  iter = panelMap.find(hitemMisc);
  if (iter != panelMap.end())
    {
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC19));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC20));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC21));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC26));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_STATIC27));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_SHADOW));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_BLUR));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_PADDING));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_PADDINGUPDOWN));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_BEVEL));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_BEVELUPDOWN));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_BORDER));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_BORDERUPDOWN));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_FLAT));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_RAISED));
      iter->second->insert(iter->second->begin(), GetDlgItem(hwndDlg, IDC_SUNKEN));
    }
}

void StyleEditor::ClearPanelMap()
{
  while (!panelMap.empty())
    panelMap.erase(panelMap.begin());

  panelMap.clear();
}

void StyleEditor::ShowPanel(HTREEITEM panel)
{
  std::map< HTREEITEM, std::tr1::shared_ptr<PanelSet> >::iterator iter;
  PanelSet::iterator iter2;
  UINT method= SW_HIDE;

  iter = panelMap.begin();

  while (iter != panelMap.end())
    {
      if (iter->first == panel)
        method = SW_SHOW;
      else
        method = SW_HIDE;

      iter2 = iter->second->begin();
      while (iter2 != iter->second->end())
        {
          ShowWindow((HWND)*iter2, method);
          iter2++;
        }

      iter++;
    }
}

BOOL StyleEditor::DoInitDialog(HWND hwndDlg, bool updatePos)
{
  RECT rect;
  std::wstring lower;

  HWND okWnd = GetDlgItem(hwndDlg, IDOK);
  HWND treeWnd = GetDlgItem(hwndDlg, IDC_PANELTREE);
  HWND blurWnd = GetDlgItem(hwndDlg, IDC_BLUR);
  EnableWindow(okWnd, (!style.empty()));

  BuildPanelMap(hwndDlg);

  if (updatePos)
    {
      rect = ELGetWindowRect(hwndDlg);

      int x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
      int y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);
      SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
      ELStealFocus(hwndDlg);
    }

  SetDlgItemText(hwndDlg, IDC_STYLENAME, PathFindFileName(style.c_str()));

  SendDlgItemMessage(hwndDlg, IDC_PADDINGUPDOWN, UDM_SETRANGE, (WPARAM)0, (LPARAM)100);
  SendDlgItemMessage(hwndDlg, IDC_BEVELUPDOWN, UDM_SETRANGE, (WPARAM)0, (LPARAM)100);
  SendDlgItemMessage(hwndDlg, IDC_BORDERUPDOWN, UDM_SETRANGE, (WPARAM)0, (LPARAM)100);

  SendDlgItemMessage(hwndDlg, IDC_ACTIVESLIDER, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(20, 100));
  SendDlgItemMessage(hwndDlg, IDC_ACTIVESLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)guiInfo.alphaActive);
  SetDlgItemInt(hwndDlg, IDC_ACTIVEALPHA, guiInfo.alphaActive, false);

  SendDlgItemMessage(hwndDlg, IDC_INACTIVESLIDER, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 100));
  SendDlgItemMessage(hwndDlg, IDC_INACTIVESLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)guiInfo.alphaInactive);
  SetDlgItemInt(hwndDlg, IDC_INACTIVEALPHA, guiInfo.alphaInactive, false);

  SendDlgItemMessage(hwndDlg, IDC_BACKGROUNDSLIDER, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 100));
  SendDlgItemMessage(hwndDlg, IDC_BACKGROUNDSLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)guiInfo.alphaBackground);
  SetDlgItemInt(hwndDlg, IDC_BACKGROUNDALPHA, guiInfo.alphaBackground, false);

  SendDlgItemMessage(hwndDlg, IDC_MENUSLIDER, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(20, 100));
  SendDlgItemMessage(hwndDlg, IDC_MENUSLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)guiInfo.alphaMenu);
  SetDlgItemInt(hwndDlg, IDC_MENUALPHA, guiInfo.alphaMenu, false);

  SendDlgItemMessage(hwndDlg, IDC_FOREGROUNDSLIDER, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 100));
  SendDlgItemMessage(hwndDlg, IDC_FOREGROUNDSLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)guiInfo.alphaForeground);
  SetDlgItemInt(hwndDlg, IDC_FOREGROUNDALPHA, guiInfo.alphaForeground, false);

  SendDlgItemMessage(hwndDlg, IDC_FRAMESLIDER, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 100));
  SendDlgItemMessage(hwndDlg, IDC_FRAMESLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)guiInfo.alphaFrame);
  SetDlgItemInt(hwndDlg, IDC_FRAMEALPHA, guiInfo.alphaFrame, false);

  SendDlgItemMessage(hwndDlg, IDC_SELECTEDSLIDER, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 100));
  SendDlgItemMessage(hwndDlg, IDC_SELECTEDSLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)guiInfo.alphaSelected);
  SetDlgItemInt(hwndDlg, IDC_SELECTEDALPHA, guiInfo.alphaSelected, false);

  SendDlgItemMessage(hwndDlg, IDC_BORDERSLIDER, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 100));
  SendDlgItemMessage(hwndDlg, IDC_BORDERSLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)guiInfo.alphaBorder);
  SetDlgItemInt(hwndDlg, IDC_BORDERALPHA, guiInfo.alphaBorder, false);

  SendDlgItemMessage(hwndDlg, IDC_TEXTSLIDER, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(20, 100));
  SendDlgItemMessage(hwndDlg, IDC_TEXTSLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)guiInfo.alphaText);
  SetDlgItemInt(hwndDlg, IDC_TEXTALPHA, guiInfo.alphaText, false);

  hbmColourBackground = EGCreateBitmap(0xff, colourBackground, colourRect);
  if (hbmColourBackground)
    SendDlgItemMessage(hwndDlg, IDC_BACKGROUNDCOLOUR, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmColourBackground);

  hbmColourForeground = EGCreateBitmap(0xff, colourForeground, colourRect);
  if (hbmColourBackground)
    SendDlgItemMessage(hwndDlg, IDC_FOREGROUNDCOLOUR, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmColourForeground);

  hbmColourSelected = EGCreateBitmap(0xff, colourSelected, colourRect);
  if (hbmColourSelected)
    SendDlgItemMessage(hwndDlg, IDC_SELECTEDCOLOUR, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmColourSelected);

  hbmColourFrame = EGCreateBitmap(0xff, colourFrame, colourRect);
  if (hbmColourFrame)
    SendDlgItemMessage(hwndDlg, IDC_FRAMECOLOUR, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmColourFrame);

  hbmColourFont = EGCreateBitmap(0xff, colourFont, colourRect);
  if (hbmColourFont)
    SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmColourFont);

  hbmColourBorder = EGCreateBitmap(0xff, colourBorder, colourRect);
  if (hbmColourBorder)
    SendDlgItemMessage(hwndDlg, IDC_BORDERCOLOUR, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmColourBorder);

  if (guiInfo.windowShadow)
    SendDlgItemMessage(hwndDlg, IDC_SHADOW, BM_SETCHECK, BST_CHECKED, 0);

  if (guiInfo.windowBlur)
    SendDlgItemMessage(hwndDlg, IDC_BLUR, BM_SETCHECK, BST_CHECKED, 0);

  SetDlgItemInt(hwndDlg, IDC_BEVEL, guiInfo.bevelWidth, false);
  SetDlgItemInt(hwndDlg, IDC_BORDER, guiInfo.dragBorder, false);
  SetDlgItemInt(hwndDlg, IDC_PADDING, guiInfo.padding, false);

  if (SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_GETCOUNT, 0, 0) == 0)
    {
      SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Solid"));
      SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Vertical"));
      SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Horizontal"));
      SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("CrossDiagonal"));
      SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Diagonal"));
      SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Pipecross"));
      SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Elliptic"));
      SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Rectangle"));
      SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Pyramid"));
    }

  lower = ELToLower(guiInfo.gradientMethod);
  if (lower.find(TEXT("vertical")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_SETCURSEL, (WPARAM)1, 0);
  else if (lower.find(TEXT("horizontal")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_SETCURSEL, (WPARAM)2, 0);
  else if (lower.find(TEXT("crossdiagonal")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_SETCURSEL, (WPARAM)3, 0);
  else if (lower.find(TEXT("diagonal")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_SETCURSEL, (WPARAM)4, 0);
  else if (lower.find(TEXT("pipecross")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_SETCURSEL, (WPARAM)5, 0);
  else if (lower.find(TEXT("elliptic")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_SETCURSEL, (WPARAM)6, 0);
  else if (lower.find(TEXT("rectangle")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_SETCURSEL, (WPARAM)7, 0);
  else if (lower.find(TEXT("pyramid")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_SETCURSEL, (WPARAM)8, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_METHOD, CB_SETCURSEL, (WPARAM)0, 0);

  if (lower.find(TEXT("interlaced")) != std::wstring::npos)
    SendDlgItemMessage(hwndDlg, IDC_INTERLACED, BM_SETCHECK, BST_CHECKED, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_INTERLACED, BM_SETCHECK, BST_UNCHECKED, 0);

  if (lower.find(TEXT("raised")) != std::wstring::npos)
    {
      SendDlgItemMessage(hwndDlg, IDC_RAISED, BM_SETCHECK, BST_CHECKED, 0);
      SendDlgItemMessage(hwndDlg, IDC_SUNKEN, BM_SETCHECK, BST_UNCHECKED, 0);
      SendDlgItemMessage(hwndDlg, IDC_FLAT, BM_SETCHECK, BST_UNCHECKED, 0);
    }
  else if (lower.find(TEXT("sunken")) != std::wstring::npos)
    {
      SendDlgItemMessage(hwndDlg, IDC_SUNKEN, BM_SETCHECK, BST_CHECKED, 0);
      SendDlgItemMessage(hwndDlg, IDC_RAISED, BM_SETCHECK, BST_UNCHECKED, 0);
      SendDlgItemMessage(hwndDlg, IDC_FLAT, BM_SETCHECK, BST_UNCHECKED, 0);
    }
  else
    {
      SendDlgItemMessage(hwndDlg, IDC_FLAT, BM_SETCHECK, BST_CHECKED, 0);
      SendDlgItemMessage(hwndDlg, IDC_SUNKEN, BM_SETCHECK, BST_UNCHECKED, 0);
      SendDlgItemMessage(hwndDlg, IDC_RAISED, BM_SETCHECK, BST_UNCHECKED, 0);
    }

  hbmColourFrom = EGCreateBitmap(0xff, colourFrom, colourRect);
  if (hbmColourFrom)
    SendDlgItemMessage(hwndDlg, IDC_FROMCOLOUR, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmColourFrom);

  hbmColourTo = EGCreateBitmap(0xff, colourTo, colourRect);
  if (hbmColourTo)
    SendDlgItemMessage(hwndDlg, IDC_TOCOLOUR, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmColourTo);

  TVINSERTSTRUCT tvInsert;

  if (TreeView_DeleteAllItems(treeWnd))
    {
      ClearPanelMap();

      tvInsert.hParent = NULL;
      tvInsert.hInsertAfter=TVI_ROOT;
      tvInsert.item.mask = TVIF_TEXT;
      tvInsert.item.pszText = (WCHAR*)TEXT("Opacity");
      hitemOpacity = TreeView_InsertItem(treeWnd, &tvInsert);
      panelMap.insert(std::pair<HTREEITEM, PanelSet*>(hitemOpacity, new PanelSet));

      tvInsert.item.pszText = (WCHAR*)TEXT("Color");
      hitemColor = TreeView_InsertItem(treeWnd, &tvInsert);
      panelMap.insert(std::pair<HTREEITEM, PanelSet*>(hitemColor, new PanelSet));

      tvInsert.item.pszText = (WCHAR*)TEXT("Gradient");
      hitemGradient = TreeView_InsertItem(treeWnd, &tvInsert);
      panelMap.insert(std::pair<HTREEITEM, PanelSet*>(hitemGradient, new PanelSet));

      tvInsert.item.pszText = (WCHAR*)TEXT("Miscellaneous");
      hitemMisc = TreeView_InsertItem(treeWnd, &tvInsert);
      panelMap.insert(std::pair<HTREEITEM, PanelSet*>(hitemMisc, new PanelSet));

      BuildPanelMap(hwndDlg);
      (void)TreeView_SelectItem(treeWnd, hitemOpacity);
    }

  if (!EGIsCompositionEnabled())
    EnableWindow(blurWnd, FALSE);

  return TRUE;
}

BOOL StyleEditor::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  bool update = false;

  switch (LOWORD(wParam))
    {
    case IDC_SAVEAS:
      DoSaveAs(hwndDlg);
      style = DoSaveStyle(hwndDlg, style);
      if (!style.empty())
        DoInitDialog(hwndDlg, false);
      return TRUE;
    case IDC_LOAD:
      return DoLoad(hwndDlg);
    case IDC_SWITCH:
      return DoSwitch(hwndDlg);
    case IDC_DEFAULTS:
      return DoDefaults(hwndDlg);
    case IDC_BACKGROUNDCOLOUR:
      if (ELIsKeyDown(VK_SHIFT))
        {
          colourBackground = defaultGuiInfo.colorBackground;
          update = true;
        }
      else
        update = (DoColourChooser(&colourBackground, hwndDlg) == TRUE);

      if (update)
        {
          if (hbmColourBackground)
            DeleteObject(hbmColourBackground);
          hbmColourBackground = EGCreateBitmap(0xff, colourBackground, colourRect);
          if (hbmColourBackground)
            DeleteObject((HBITMAP)SendDlgItemMessage(hwndDlg, IDC_BACKGROUNDCOLOUR, BM_SETIMAGE, IMAGE_BITMAP,
                                                     (LPARAM)hbmColourBackground));
          return TRUE;
        }
      return FALSE;
    case IDC_FOREGROUNDCOLOUR:
      if (ELIsKeyDown(VK_SHIFT))
        {
          colourForeground = defaultGuiInfo.colorForeground;
          update = true;
        }
      else
        update = (DoColourChooser(&colourForeground, hwndDlg) == TRUE);

      if (update)
        {
          if (hbmColourForeground)
            DeleteObject(hbmColourForeground);
          hbmColourForeground = EGCreateBitmap(0xff, colourForeground, colourRect);
          if (hbmColourForeground)
            DeleteObject((HBITMAP)SendDlgItemMessage(hwndDlg, IDC_FOREGROUNDCOLOUR, BM_SETIMAGE, IMAGE_BITMAP,
                                                     (LPARAM)hbmColourForeground));
          return TRUE;
        }
      return FALSE;
    case IDC_SELECTEDCOLOUR:
      if (ELIsKeyDown(VK_SHIFT))
        {
          colourSelected = defaultGuiInfo.colorSelected;
          update = true;
        }
      else
        update = (DoColourChooser(&colourSelected, hwndDlg) == TRUE);

      if (update)
        {
          if (hbmColourSelected)
            DeleteObject(hbmColourSelected);
          hbmColourSelected = EGCreateBitmap(0xff, colourSelected, colourRect);
          if (hbmColourSelected)
            DeleteObject((HBITMAP)SendDlgItemMessage(hwndDlg, IDC_SELECTEDCOLOUR, BM_SETIMAGE, IMAGE_BITMAP,
                                                     (LPARAM)hbmColourSelected));
          return TRUE;
        }
      return FALSE;
    case IDC_FRAMECOLOUR:
      if (ELIsKeyDown(VK_SHIFT))
        {
          colourFrame = defaultGuiInfo.colorFrame;
          update = true;
        }
      else
        update = (DoColourChooser(&colourFrame, hwndDlg) == TRUE);

      if (update)
        {
          if (hbmColourFrame)
            DeleteObject(hbmColourFrame);
          hbmColourFrame = EGCreateBitmap(0xff, colourFrame, colourRect);
          if (hbmColourFrame)
            DeleteObject((HBITMAP)SendDlgItemMessage(hwndDlg, IDC_FRAMECOLOUR, BM_SETIMAGE, IMAGE_BITMAP,
                                                     (LPARAM)hbmColourFrame));
          return TRUE;
        }
      return FALSE;
    case IDC_FONTCOLOUR:
      if (ELIsKeyDown(VK_SHIFT))
        {
          colourFont = defaultGuiInfo.colorFont;
          update = true;
        }
      else
        update = (DoColourChooser(&colourFont, hwndDlg) == TRUE);

      if (update)
        {
          if (hbmColourFont)
            DeleteObject(hbmColourFont);
          hbmColourFont = EGCreateBitmap(0xff, colourFont, colourRect);
          if (hbmColourFont)
            DeleteObject((HBITMAP)SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, BM_SETIMAGE, IMAGE_BITMAP,
                                                     (LPARAM)hbmColourFont));
          return TRUE;
        }
      return FALSE;
    case IDC_BORDERCOLOUR:
      if (ELIsKeyDown(VK_SHIFT))
        {
          colourBorder = defaultGuiInfo.colorBorder;
          update = true;
        }
      else
        update = (DoColourChooser(&colourBorder, hwndDlg) == TRUE);

      if (update)
        {
          if (hbmColourBorder)
            DeleteObject(hbmColourBorder);
          hbmColourBorder = EGCreateBitmap(0xff, colourBorder, colourRect);
          if (hbmColourBorder)
            DeleteObject((HBITMAP)SendDlgItemMessage(hwndDlg, IDC_BORDERCOLOUR, BM_SETIMAGE,
                                                     IMAGE_BITMAP, (LPARAM)hbmColourBorder));
          return TRUE;
        }
      return FALSE;
    case IDC_FROMCOLOUR:
      if (ELIsKeyDown(VK_SHIFT))
        {
          colourFrom = defaultGuiInfo.gradientFrom;
          update = true;
        }
      else
        update = (DoColourChooser(&colourFrom, hwndDlg) == TRUE);

      if (update)
        {
          if (hbmColourFrom)
            DeleteObject(hbmColourFrom);
          hbmColourFrom = EGCreateBitmap(0xff, colourFrom, colourRect);
          if (hbmColourFont)
            DeleteObject((HBITMAP)SendDlgItemMessage(hwndDlg, IDC_FROMCOLOUR, BM_SETIMAGE,
                                                     IMAGE_BITMAP, (LPARAM)hbmColourFrom));
          return TRUE;
        }
      return FALSE;
    case IDC_TOCOLOUR:
      if (ELIsKeyDown(VK_SHIFT))
        {
          colourTo = defaultGuiInfo.gradientTo;
          update = true;
        }
      else
        update = (DoColourChooser(&colourTo, hwndDlg) == TRUE);

      if (update)
        {
          if (hbmColourTo)
            DeleteObject(hbmColourTo);
          hbmColourTo = EGCreateBitmap(0xff, colourTo, colourRect);
          if (hbmColourTo)
            DeleteObject((HBITMAP)SendDlgItemMessage(hwndDlg, IDC_TOCOLOUR, BM_SETIMAGE, IMAGE_BITMAP,
                                                     (LPARAM)hbmColourTo));
          return TRUE;
        }
      return FALSE;
    case IDC_PREVIEW:
      ESESetStyle(tmpFile);
      tmpFile = DoSaveStyle(hwndDlg, tmpFile);
      if (!tmpFile.empty())
        ELDispatchCoreMessage(EMERGE_CORE, CORE_REFRESH, instanceName);
      return TRUE;
    }

  return FALSE;
}

BOOL StyleEditor::DoSwitch(HWND hwndDlg)
{
  COLORREF colourTmp = colourFrom;
  colourFrom = colourTo;
  colourTo = colourTmp;

  if (hbmColourFrom)
    DeleteObject(hbmColourFrom);
  hbmColourFrom = EGCreateBitmap(0xff, colourFrom, colourRect);
  if (hbmColourFont)
    DeleteObject((HBITMAP)SendDlgItemMessage(hwndDlg, IDC_FROMCOLOUR, BM_SETIMAGE,
                                             IMAGE_BITMAP, (LPARAM)hbmColourFrom));

  if (hbmColourTo)
    DeleteObject(hbmColourTo);
  hbmColourTo = EGCreateBitmap(0xff, colourTo, colourRect);
  if (hbmColourTo)
    DeleteObject((HBITMAP)SendDlgItemMessage(hwndDlg, IDC_TOCOLOUR, BM_SETIMAGE,
                                             IMAGE_BITMAP, (LPARAM)hbmColourTo));
  return TRUE;
}

BOOL StyleEditor::DoLoad(HWND hwndDlg)
{
  OPENFILENAME ofn;
  WCHAR file[MAX_PATH], extension[MAX_PATH];
  std::wstring workingPath = TEXT("%ThemeDir%");
  workingPath = ELExpandVars(workingPath);

  wcscpy(extension, TEXT("eds"));

  ZeroMemory(file, MAX_PATH);
  ZeroMemory(&ofn, sizeof(ofn));

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hwndDlg;
  ofn.lpstrFilter = TEXT("Style (*.eds)\0*.eds\0");
  ofn.lpstrFile = file;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrInitialDir = workingPath.c_str();
  ofn.lpstrTitle = TEXT("Load Style");
  ofn.lpstrDefExt = extension;
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS;

  if (GetOpenFileName(&ofn))
    {
      ELUnExpandVars(file);
      style = file;

      ESEReadStyle(style, &guiInfo);

      colourBackground = guiInfo.colorBackground;
      colourForeground = guiInfo.colorForeground;
      colourSelected = guiInfo.colorSelected;
      colourFrame = guiInfo.colorFrame;
      colourFont = guiInfo.colorFont;
      colourTo = guiInfo.gradientTo;
      colourFrom = guiInfo.gradientFrom;
      colourBorder = guiInfo.colorBorder;

      DoInitDialog(hwndDlg, false);

      ESESetStyle(style);

      ELDispatchCoreMessage(EMERGE_CORE, CORE_REFRESH, instanceName);

      return TRUE;
    }

  return FALSE;
}

BOOL StyleEditor::DoDefaults(HWND hwndDlg)
{
  guiInfo = defaultGuiInfo;

  colourBackground = guiInfo.colorBackground;
  colourForeground = guiInfo.colorForeground;
  colourSelected = guiInfo.colorSelected;
  colourFrame = guiInfo.colorFrame;
  colourFont = guiInfo.colorFont;
  colourTo = guiInfo.gradientTo;
  colourFrom = guiInfo.gradientFrom;
  colourBorder = guiInfo.colorBorder;

  DoInitDialog(hwndDlg, false);

  return TRUE;
}

void StyleEditor::DoSaveAs(HWND hwndDlg)
{
  OPENFILENAME ofn;
  HWND okWnd = GetDlgItem(hwndDlg, IDOK);
  WCHAR file[MAX_PATH], extension[MAX_PATH];
  std::wstring theme = ELGetThemeName(), oldTheme, newThemePath, oldThemePath;

  ZeroMemory(file, MAX_PATH);
  ZeroMemory(&ofn, sizeof(ofn));
  newThemePath = TEXT("%ThemeDir%");

  if (!ELIsModifiedTheme(theme))
    oldTheme = theme;

  if (ELSetModifiedTheme(theme))
    {
      oldThemePath = TEXT("%EmergeDir%\\themes\\") + oldTheme;
      oldThemePath += TEXT("\\*");

      if ((ELGetFileSpecialFlags(newThemePath) & SF_DIRECTORY) != SF_DIRECTORY)
        ELCreateDirectory(newThemePath);

      ELFileOp(mainWnd, false, FO_COPY, oldThemePath, newThemePath);
    }

  wcscpy(extension, TEXT("eds"));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hwndDlg;
  ofn.lpstrFilter = TEXT("Style (*.eds)\0*.eds\0");
  ofn.lpstrFile = file;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrTitle = TEXT("Save Style As");
  ofn.lpstrDefExt = extension;
  newThemePath += TEXT("\\Styles");
  if ((ELGetFileSpecialFlags(newThemePath) & SF_DIRECTORY) != SF_DIRECTORY)
    ELCreateDirectory(newThemePath);
  newThemePath = ELExpandVars(newThemePath);
  ofn.lpstrInitialDir = newThemePath.c_str();
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS;

  if (GetSaveFileName(&ofn))
    {
      EnableWindow(okWnd, (wcslen(file) != 0));
      ELUnExpandVars(file);
      style = file;
      ESESetStyle(style);
    }
  else
    {
      if (!oldThemePath.empty())
        {
          size_t styledir = newThemePath.find_last_of(TEXT("\\"));
          ELFileOp(mainWnd, false, FO_DELETE, newThemePath.substr(0, styledir));
          size_t copystar = oldThemePath.find_last_of(TEXT("\\"));
          oldThemePath = ELExpandVars(oldThemePath.substr(0, copystar));
          ELSetTheme(oldThemePath);
        }
    }
}

BOOL StyleEditor::DoColourChooser(COLORREF *colour, HWND hwndDlg)
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

std::wstring StyleEditor::DoSaveStyle(HWND hwndDlg, std::wstring fileName)
{
  std::wstring saveFile;
  WCHAR methodBuffer[MAX_LINE_LENGTH];
  std::wstring methodString;

  saveFile = fileName;
  if (saveFile.empty())
    {
      DoSaveAs(hwndDlg);
      saveFile = ESEGetStyle();
    }

  guiInfo.alphaActive = GetDlgItemInt(hwndDlg, IDC_ACTIVEALPHA, NULL, FALSE);
  guiInfo.alphaInactive = GetDlgItemInt(hwndDlg, IDC_INACTIVEALPHA, NULL, FALSE);
  guiInfo.alphaMenu = GetDlgItemInt(hwndDlg, IDC_MENUALPHA, NULL, FALSE);
  guiInfo.alphaBackground = GetDlgItemInt(hwndDlg, IDC_BACKGROUNDALPHA, NULL, FALSE);
  guiInfo.alphaForeground = GetDlgItemInt(hwndDlg, IDC_FOREGROUNDALPHA, NULL, FALSE);
  guiInfo.alphaFrame = GetDlgItemInt(hwndDlg, IDC_FRAMEALPHA, NULL, FALSE);
  guiInfo.alphaSelected = GetDlgItemInt(hwndDlg, IDC_SELECTEDALPHA, NULL, FALSE);
  guiInfo.alphaBorder = GetDlgItemInt(hwndDlg, IDC_BORDERALPHA, NULL, FALSE);
  guiInfo.alphaText = GetDlgItemInt(hwndDlg, IDC_TEXTALPHA, NULL, FALSE);

  guiInfo.colorBackground = colourBackground;
  guiInfo.colorForeground = colourForeground;
  guiInfo.colorSelected = colourSelected;
  guiInfo.colorFrame = colourFrame;
  guiInfo.colorFont = colourFont;
  guiInfo.colorBorder = colourBorder;

  if (SendDlgItemMessage(hwndDlg, IDC_SHADOW, BM_GETCHECK, 0, 0) == BST_CHECKED)
    guiInfo.windowShadow = true;
  else if (SendDlgItemMessage(hwndDlg, IDC_SHADOW, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    guiInfo.windowShadow = false;

  if (SendDlgItemMessage(hwndDlg, IDC_BLUR, BM_GETCHECK, 0, 0) == BST_CHECKED)
    guiInfo.windowBlur = true;
  else if (SendDlgItemMessage(hwndDlg, IDC_BLUR, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    guiInfo.windowBlur = false;

  guiInfo.bevelWidth = GetDlgItemInt(hwndDlg, IDC_BEVEL, NULL, FALSE);
  guiInfo.dragBorder = GetDlgItemInt(hwndDlg, IDC_BORDER, NULL, FALSE);
  guiInfo.padding = GetDlgItemInt(hwndDlg, IDC_PADDING, NULL, FALSE);

  guiInfo.gradientTo = colourTo;
  guiInfo.gradientFrom = colourFrom;

  GetDlgItemText(hwndDlg, IDC_METHOD, methodBuffer, MAX_LINE_LENGTH);
  methodString = methodBuffer;

  if (SendDlgItemMessage(hwndDlg, IDC_INTERLACED, BM_GETCHECK, 0, 0) == BST_CHECKED)
    methodString = methodString + TEXT("Interlaced");

  if (SendDlgItemMessage(hwndDlg, IDC_RAISED, BM_GETCHECK, 0, 0) == BST_CHECKED)
    methodString = methodString + TEXT("Raised");
  else if (SendDlgItemMessage(hwndDlg, IDC_SUNKEN, BM_GETCHECK, 0, 0) == BST_CHECKED)
    methodString = methodString + TEXT("Sunken");
  else
    methodString = methodString + TEXT("Flat");

  guiInfo.gradientMethod = methodString;

  if (ESEWriteStyle(fileName, &guiInfo, hwndDlg))
    return saveFile;
  else
    return TEXT("");
}

BOOL StyleEditor::DoNotify(HWND hwndDlg, LPARAM lParam)
{
  HWND activeSliderWnd = GetDlgItem(hwndDlg, IDC_ACTIVESLIDER);
  HWND inactiveSliderWnd = GetDlgItem(hwndDlg, IDC_INACTIVESLIDER);
  HWND menuSliderWnd = GetDlgItem(hwndDlg, IDC_MENUSLIDER);
  HWND backgroundSliderWnd = GetDlgItem(hwndDlg, IDC_BACKGROUNDSLIDER);
  HWND foregroundSliderWnd = GetDlgItem(hwndDlg, IDC_FOREGROUNDSLIDER);
  HWND frameSliderWnd = GetDlgItem(hwndDlg, IDC_FRAMESLIDER);
  HWND selectedSliderWnd = GetDlgItem(hwndDlg, IDC_SELECTEDSLIDER);
  HWND borderSliderWnd = GetDlgItem(hwndDlg, IDC_BORDERSLIDER);
  HWND textSliderWnd = GetDlgItem(hwndDlg, IDC_TEXTSLIDER);

  UINT sliderValue;

  NMHDR *nmhdr = (NMHDR*)lParam;

  switch (nmhdr->code)
    {
    case PSN_APPLY:
      style = DoSaveStyle(hwndDlg, style);
      if (!style.empty())
        {
          ESESetStyle(style);
          SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
        }
      else
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID);
      return 1;

    case PSN_SETACTIVE:
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
      return 1;

    case PSN_KILLACTIVE:
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
      return 1;

    case PSN_RESET:
      ESESetStyle(origStyle);
      return 1;

    case TVN_SELCHANGING:
        {
          HTREEITEM selected = ((LPNMTREEVIEW)lParam)->itemNew.hItem;
          ShowPanel(selected);
        }
      return 0;
    }

  if (nmhdr->hwndFrom == activeSliderWnd)
    {
      sliderValue = (UINT)SendMessage(activeSliderWnd, TBM_GETPOS, 0, 0);
      SetDlgItemInt(hwndDlg, IDC_ACTIVEALPHA, sliderValue, false);
      return TRUE;
    }

  if (nmhdr->hwndFrom == inactiveSliderWnd)
    {
      sliderValue = (UINT)SendMessage(inactiveSliderWnd, TBM_GETPOS, 0, 0);
      SetDlgItemInt(hwndDlg, IDC_INACTIVEALPHA, sliderValue, false);
      return TRUE;
    }

  if (nmhdr->hwndFrom == menuSliderWnd)
    {
      sliderValue = (UINT)SendMessage(menuSliderWnd, TBM_GETPOS, 0, 0);
      SetDlgItemInt(hwndDlg, IDC_MENUALPHA, sliderValue, false);
      return TRUE;
    }

  if (nmhdr->hwndFrom == backgroundSliderWnd)
    {
      sliderValue = (UINT)SendMessage(backgroundSliderWnd, TBM_GETPOS, 0, 0);
      SetDlgItemInt(hwndDlg, IDC_BACKGROUNDALPHA, sliderValue, false);
      return TRUE;
    }

  if (nmhdr->hwndFrom == foregroundSliderWnd)
    {
      sliderValue = (UINT)SendMessage(foregroundSliderWnd, TBM_GETPOS, 0, 0);
      SetDlgItemInt(hwndDlg, IDC_FOREGROUNDALPHA, sliderValue, false);
      return TRUE;
    }

  if (nmhdr->hwndFrom == frameSliderWnd)
    {
      sliderValue = (UINT)SendMessage(frameSliderWnd, TBM_GETPOS, 0, 0);
      SetDlgItemInt(hwndDlg, IDC_FRAMEALPHA, sliderValue, false);
      return TRUE;
    }

  if (nmhdr->hwndFrom == selectedSliderWnd)
    {
      sliderValue = (UINT)SendMessage(selectedSliderWnd, TBM_GETPOS, 0, 0);
      SetDlgItemInt(hwndDlg, IDC_SELECTEDALPHA, sliderValue, false);
      return TRUE;
    }

  if (nmhdr->hwndFrom == borderSliderWnd)
    {
      sliderValue = (UINT)SendMessage(borderSliderWnd, TBM_GETPOS, 0, 0);
      SetDlgItemInt(hwndDlg, IDC_BORDERALPHA, sliderValue, false);
      return TRUE;
    }

  if (nmhdr->hwndFrom == textSliderWnd)
    {
      sliderValue = (UINT)SendMessage(textSliderWnd, TBM_GETPOS, 0, 0);
      SetDlgItemInt(hwndDlg, IDC_TEXTALPHA, sliderValue, false);
      return TRUE;
    }

  return FALSE;
}

