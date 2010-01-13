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

#include "Applet.h"

#define VK_WIN 0x5B

Applet::Applet(HINSTANCE hInstance)
{
  mainInst = hInstance;
  lwinID = 0;
  rwinID = 0;
}

UINT Applet::Initialize()
{
  // Retrieve the window dimension attributes out of the registry
  hotkeyCount = 0;

  mainWnd = EAEInitializeAppletWindow(mainInst, WindowProcedure, this);

  // If the window failed to get created, unregister the class and quit the program
  if (!mainWnd)
    return 0;

  // Disable menu animation, as it overrides the alpha blending
  SystemParametersInfo(SPI_SETMENUANIMATION, 0, (PVOID)false, SPIF_SENDCHANGE);

  SetWindowPos(mainWnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE);
  ShowWindow(mainWnd, SW_SHOW);

  if (!RegisterHotKey(mainWnd, 0, MOD_SHIFT, VK_ESCAPE))
    {
      ELMessageBox(GetDesktopWindow(),
                   (WCHAR*)TEXT("Failed to register Hotkey combination Shift+Escape\n required for Actions Dialog."),
                   (WCHAR*)TEXT("emergeHotkeys"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      return 0;
    }

  pSettings = std::tr1::shared_ptr<Settings>(new Settings(mainWnd));
  pSettings->BuildList(false);
  pActions = std::tr1::shared_ptr<Actions>(new Actions(mainInst, mainWnd, pSettings));
  pActions->RegisterHotkeyList(true);

  PostMessage(ELGetCoreWindow(), EMERGE_REGISTER, (WPARAM)mainWnd, (LPARAM)EMERGE_CORE);

  return 1;
}

Applet::~Applet()
{
  PostMessage(ELGetCoreWindow(), EMERGE_UNREGISTER, (WPARAM)mainWnd, (LPARAM)EMERGE_CORE);

  UnregisterHotKey(mainWnd, 0);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	WindowProcedure
// Required:	HWND hwnd - window handle that message was sent to
// 		UINT message - action to handle
// 		WPARAM wParam - dependant on message
// 		LPARAM lParam - dependant on message
// Returns:	LRESULT
// Purpose:	Handles messages sent from DispatchMessage
//----  --------------------------------------------------------------------------------------------------------
LRESULT CALLBACK Applet::WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  COPYDATASTRUCT *cpData;
  CREATESTRUCT *cs;
  static Applet *pApplet = NULL;

  if (message == WM_CREATE)
    {
      cs = (CREATESTRUCT*)lParam;
      pApplet = reinterpret_cast<Applet*>(cs->lpCreateParams);
      return DefWindowProc(hwnd, message, wParam, lParam);
    }

  if (pApplet == NULL)
    return DefWindowProc(hwnd, message, wParam, lParam);

  switch (message)
    {
    case WM_COPYDATA:
      cpData = (COPYDATASTRUCT *)lParam;
      if (cpData->dwData == EMERGE_MESSAGE)
        return pApplet->DoCopyData(cpData);
      break;

      // Send a quit message when the window is destroyed
    case WM_DESTROY:
    case WM_NCDESTROY:
      PostQuitMessage (0);
      break;

    case WM_SYSCOMMAND:
      switch (wParam)
        {
        case SC_CLOSE:
        case SC_MAXIMIZE:
        case SC_MINIMIZE:
          break;
        default:
          return DefWindowProc(hwnd, message, wParam, lParam);
        }
      break;

    case WM_TIMER:
      return pApplet->DoTimer((UINT)wParam);

    case WM_HOTKEY:
      SetTimer(hwnd, (UINT)wParam, 50, NULL);
      break;

      // If not handled just forward the message on to MessageControl
    default:
      return DefWindowProc (hwnd, message, wParam, lParam);
    }

  return 0;
}

LRESULT Applet::DoCopyData(COPYDATASTRUCT *cds)
{
  std::wstring theme = reinterpret_cast<WCHAR*>(cds->lpData);

  if (cds->dwData == EMERGE_MESSAGE)
    {
      SetEnvironmentVariable(TEXT("ThemeDir"), theme.c_str());
      return 1;
    }

  return 0;
}

LRESULT Applet::DoTimer(UINT index)
{
  HotkeyCombo *hc = NULL;
  int key;

  if (index == 0)
    key = VK_ESCAPE;
  else
    {
      UINT item = pSettings->FindHotkeyListItem(index);
      if (item == pSettings->GetHotkeyListSize())
        return 0;
      hc = pSettings->GetHotkeyListItem(item);
      key = hc->GetHotkeyKey();
      if (key == VK_LWIN)
        {
          if ((lwinID != 0) && (lwinID != index))
            {
              KillTimer(mainWnd, lwinID);
              lwinID = index;
            }
          else if (lwinID != index)
            lwinID = index;
        }
      else if (key == VK_RWIN)
        {
          if ((rwinID != 0) && (rwinID != index))
            {
              KillTimer(mainWnd, rwinID);
              rwinID = index;
            }
          else if (rwinID != index)
            rwinID = index;
        }
    }
  if (!ELIsKeyDown(key))
    {
      if (hc)
        {
          if (hc->GetHotkeyModifiers() & MOD_WIN)
            {
              if ((key != VK_LWIN) && (key != VK_RWIN))
                {
                  if (lwinID)
                    {
                      KillTimer(mainWnd, lwinID);
                      lwinID = 0;
                    }
                  if (rwinID)
                    {
                      KillTimer(mainWnd, rwinID);
                      rwinID = 0;
                    }
                }
            }
        }
      KillTimer(mainWnd, index);
      ExecuteAction(index);
    }

  return 0;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ExecuteAction
// Required:	int index - index of action to be taken
// Returns:		Nothing
// Purpose:		Execute specified hotkey action
//----  --------------------------------------------------------------------------------------------------------
void Applet::ExecuteAction(UINT index)
{
  if (index == 0)
    pActions->Show();
  else
    {
      UINT item = pSettings->FindHotkeyListItem(index);
      if (item == pSettings->GetHotkeyListSize())
        return;

      ELExecuteAll(pSettings->GetHotkeyListItem(item)->GetHotkeyAction(), (WCHAR*)TEXT("\0"));
    }
}
