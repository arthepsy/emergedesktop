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

#include "Config.h"
#include "Applet.h"

INT_PTR CALLBACK Config::ConfigDlgProc(HWND hwndDlg, UINT message, WPARAM wParam UNUSED, LPARAM lParam UNUSED)
{
  static Config *pConfig = NULL;

  switch (message)
    {
    case WM_INITDIALOG:
      pConfig = reinterpret_cast<Config*>(lParam);
      if (!pConfig)
        break;
      return pConfig->DoInitDialog(hwndDlg);
    }

  return 0;
}

Config::Config(HINSTANCE hInstance, HWND mainWnd, std::tr1::shared_ptr<Settings> pSettings)
{
  this->hInstance = hInstance;
  this->mainWnd = mainWnd;
  this->pSettings = pSettings;

  pConfigPage = std::tr1::shared_ptr<ConfigPage>(new ConfigPage(pSettings));
  pTextPage = std::tr1::shared_ptr<TextPage>(new TextPage(pSettings));
  pStyleEditor = std::tr1::shared_ptr<StyleEditor>(new StyleEditor(mainWnd));
}

Config::~Config()
{
}

int Config::Show()
{
  return (int)DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CONFIG), mainWnd, (DLGPROC)ConfigDlgProc, (LPARAM)this);
}

INT_PTR Config::DoInitDialog(HWND hwndDlg)
{
  int ret;
  PROPSHEETPAGE psp[3];
  PROPSHEETHEADER psh;

  ELStealFocus(hwndDlg);
  SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);

  psp[0].dwSize = sizeof(PROPSHEETPAGE);
  psp[0].dwFlags = PSP_USETITLE;
  psp[0].hInstance = hInstance;
  psp[0].pszTemplate = MAKEINTRESOURCE(IDD_CONFIG_PAGE);
  psp[0].pfnDlgProc = pConfigPage->ConfigPageDlgProc;
  psp[0].pszTitle = TEXT("General");
  psp[0].lParam = reinterpret_cast<LPARAM>(pConfigPage.get());
  psp[0].pfnCallback = NULL;

  psp[1].dwSize = sizeof(PROPSHEETPAGE);
  psp[1].dwFlags = PSP_USETITLE;
  psp[1].hInstance = hInstance;
  psp[1].pszTemplate = MAKEINTRESOURCE(IDD_TEXT_PAGE);
  psp[1].pfnDlgProc = pTextPage->TextPageDlgProc;
  psp[1].pszTitle = TEXT("Position");
  psp[1].lParam = reinterpret_cast<LPARAM>(pTextPage.get());
  psp[1].pfnCallback = NULL;

  pStyleEditor->Edit(pSettings->GetStyleFile());
  psp[2].dwSize = sizeof(PROPSHEETPAGE);
  psp[2].dwFlags = PSP_USETITLE;
  psp[2].hInstance = GetModuleHandle(TEXT("emergeStyleEngine.dll"));
  psp[2].pszTemplate = pStyleEditor->GetTemplate();
  psp[2].pfnDlgProc = pStyleEditor->StyleEditorDlgProc;
  psp[2].pszTitle = TEXT("Style Editor");
  psp[2].lParam = reinterpret_cast<LPARAM>(pStyleEditor.get());
  psp[2].pfnCallback = NULL;

  psh.dwSize = sizeof(PROPSHEETHEADER);
  psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
  psh.hwndParent = hwndDlg;
  psh.hInstance = hInstance;
  psh.pszCaption = TEXT("emergeCommand Configuration");
  psh.nPages = sizeof(psp) /
               sizeof(PROPSHEETPAGE);
  psh.nStartPage = 0;
  psh.ppsp = (LPCPROPSHEETPAGE) &psp;
  psh.pfnCallback = NULL;

  ret = PropertySheet(&psh);

  if (ret >= 1)
    {
      pSettings->SetStyleFile(ESEGetStyle());
      pSettings->WriteSettings();
      EndDialog(hwndDlg, IDOK);
    }
  if (ret <= 0)
    EndDialog(hwndDlg, IDCANCEL);

  return 1;
}

