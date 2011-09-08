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

HANDLE executeThread = NULL, keyUpEvent = NULL;
UINT hotkeyCode = 0;

Applet::Applet(HINSTANCE hInstance)
  :BaseApplet(hInstance, TEXT("emergeHotkeys"), false, false)
{
  hotkeyCount = 0;
  executeThread = NULL;

  keyHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);

  keyUpEvent = CreateEvent(NULL, TRUE, FALSE, L"emergeHotkeys_KeyUpEvent");
}

UINT Applet::Initialize()
{
  UINT ret = 1;

  pSettings = std::tr1::shared_ptr<Settings>(new Settings());
  ret = BaseApplet::Initialize(WindowProcedure, this, pSettings);
  if (ret == 0)
    return ret;

  SetWindowPos(mainWnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE);
  ShowWindow(mainWnd, SW_SHOW);

  // Populate the hotkeyList vector
  pSettings->BuildList(mainWnd, false);

  pActions = std::tr1::shared_ptr<Actions>(new Actions(mainInst, mainWnd, pSettings));
  pActions->RegisterHotkeyList(true);

  return ret;
}

void Applet::ShowConfig()
{
  pActions->Show();
}

Applet::~Applet()
{
  UnhookWindowsHookEx(keyHook);
  CloseHandle(keyUpEvent);
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
  static Applet *pApplet = NULL;

  if (message == WM_CREATE)
    {
      pApplet = reinterpret_cast<Applet*>(((CREATESTRUCT*)lParam)->lpCreateParams);
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

    case WM_HOTKEY:
      pApplet->ExecuteAction((UINT)wParam);
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
  if ( nCode == HC_ACTION )
    {
      PKBDLLHOOKSTRUCT pkbHookStruct = (PKBDLLHOOKSTRUCT)lParam;

      switch (wParam)
        {
        case WM_KEYUP:
        case WM_SYSKEYUP:
          // if the WM_(SYS)KEYUP is the hotkey, trigger the event so the thread will
          // end.
          if (pkbHookStruct->vkCode == hotkeyCode)
            SetEvent(keyUpEvent);
          break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
          // If the WM_(SYS)KEYDOWN is not the hotkey, terminate the thread
          // without letting it execute.
          if (pkbHookStruct->vkCode != hotkeyCode)
            {
              TerminateThread(executeThread, 0);
              hotkeyCode = 0;
            }
          break;
        }
    }

  return CallNextHookEx((HHOOK)WH_KEYBOARD_LL, nCode, wParam, lParam);
}

DWORD WINAPI Applet::ExecuteThreadProc(LPVOID lpParameter)
{
  // reinterpret lpParameter as a HotKeyCombo*
  HotkeyCombo *hc = reinterpret_cast< HotkeyCombo* >(lpParameter);

  // Wait for the WM_KEYUP event from the low level keyboard hook
  WaitForSingleObject(keyUpEvent, INFINITE);
  // Reset the event after receiving it
  ResetEvent(keyUpEvent);

  // execute VK_LWIN on WM_KEYUP as it can also by the MOD_WIN key
  if (hotkeyCode == VK_LWIN)
    ELExecuteAll(hc->GetHotkeyAction(), (WCHAR*)TEXT("\0"));

  // Clear the hotkeyCode and exit the thread
  hotkeyCode = 0;
  ExitThread(0);

  return 0;
}

void Applet::ExecuteAction(UINT index)
{
  DWORD threadID, threadState;
  HotkeyCombo *hc;

  UINT item = pSettings->FindHotkeyListItem(index);
  if (item == pSettings->GetHotkeyListSize())
    return;

  // Check the executeThread state
  GetExitCodeThread(executeThread, &threadState);
  // Use it to control the multiple WM_HOTKEY messages being sent while the
  // hotkey is down.
  if (threadState != STILL_ACTIVE)
    {
      // If the thread is no longer active, it means it finished executing (i.e.
      // there was WM_KEYUP or a new WM_HOTKEY was activated via a WM_KEYDOWN)
      hc = pSettings->GetHotkeyListItem(item);
      hotkeyCode = hc->GetHotkeyKey();
      // If the hotkey is not VK_LWIN, execute it right away
      if (hotkeyCode != VK_LWIN)
        ELExecuteAll(hc->GetHotkeyAction(), (WCHAR*)TEXT("\0"));
      // Create a thread to check for the WM_KEYUP event
      executeThread = CreateThread(NULL, 0, ExecuteThreadProc, hc, 0, &threadID);
    }
}
