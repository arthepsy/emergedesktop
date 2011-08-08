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

#include "Applet.h"

#define VK_WIN 0x5B

HHOOK Applet::keyHook = NULL;
HWND Applet::mainWnd = NULL;
UINT Applet::keyID = 0;
UINT Applet::virtualKey = 0;

WCHAR myName[ ] = TEXT("emergeHotkeys");

Applet::Applet(HINSTANCE hInstance)
  :BaseApplet(hInstance, myName, false, false)
{
  mainWnd = NULL;
  mainInst = hInstance;
  hotkeyCount = 0;
}

UINT Applet::Initialize()
{
  mainWnd = EAEInitializeAppletWindow(mainInst, WindowProcedure, this);

  // If the window failed to get created, unregister the class and quit the program
  if (!mainWnd)
    return 0;

  // Disable menu animation, as it overrides the alpha blending
  SystemParametersInfo(SPI_SETMENUANIMATION, 0, (PVOID)false, SPIF_SENDCHANGE);

  SetWindowPos(mainWnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE);
  ShowWindow(mainWnd, SW_SHOW);

  pSettings = std::tr1::shared_ptr<Settings>(new Settings(mainWnd));
  pSettings->BuildList(false);
  pActions = std::tr1::shared_ptr<Actions>(new Actions(mainInst, mainWnd, pSettings));
  pActions->RegisterHotkeyList(true);

  PostMessage(ELGetCoreWindow(), EMERGE_REGISTER, (WPARAM)mainWnd, (LPARAM)EMERGE_CORE);

  return 1;
}

void Applet::ShowConfig()
{
  pActions->Show();
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
      return pApplet->DoCopyData((COPYDATASTRUCT *)lParam);

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
      return pApplet->DoDefault(hwnd, message, wParam, lParam);
    }

  return 0;
}

void Applet::Activate()
{
  pActions->Show();
}

LRESULT Applet::DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK Applet::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (nCode < 0)
    return CallNextHookEx(keyHook, nCode, wParam, lParam);

  PKBDLLHOOKSTRUCT pkbHookStruct = (PKBDLLHOOKSTRUCT)lParam;

  if (wParam == WM_KEYUP)
    {
      if (pkbHookStruct->vkCode != virtualKey)
        {
          if (keyID != 0)
            {
              KillTimer(mainWnd, keyID);
              keyID = 0;
            }
        }
    }

  return CallNextHookEx(keyHook, nCode, wParam, lParam);
}

LRESULT Applet::DoTimer(UINT index)
{
  HotkeyCombo *hc = NULL;
  int key;

  UINT item = pSettings->FindHotkeyListItem(index);
  if (item == pSettings->GetHotkeyListSize())
    return 0;
  hc = pSettings->GetHotkeyListItem(item);
  key = hc->GetHotkeyKey();
  if ((key == VK_LWIN) || (key = VK_RWIN))
    {
      virtualKey = key;
      if (keyID != index)
        {
          if (keyID != 0)
            KillTimer(mainWnd, keyID);
          keyID = index;
        }
      keyHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, mainInst, 0);
    }

  if (!ELIsKeyDown(key))
    {
      KillTimer(mainWnd, index);
      ExecuteAction(index);
      if (keyHook)
        UnhookWindowsHookEx(keyHook);
      keyID = 0;
      virtualKey = 0;
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
  UINT item = pSettings->FindHotkeyListItem(index);
  if (item == pSettings->GetHotkeyListSize())
    return;

  ELExecuteAll(pSettings->GetHotkeyListItem(item)->GetHotkeyAction(), (WCHAR*)TEXT("\0"));
}
