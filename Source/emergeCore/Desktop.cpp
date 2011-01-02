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

#include "Desktop.h"

std::deque<HWND> hwndDeque;

WCHAR desktopClass[] = TEXT("progman");

BOOL CALLBACK Desktop::MinimizeWindowsEnum(HWND hwnd, LPARAM lParam)
{
  std::wstring debug;

  if (ELCheckWindow(hwnd))
    {
      if (!IsIconic(hwnd))
        ((std::deque<HWND>*)lParam)->push_front(hwnd);
    }

  return TRUE;
}

Desktop::Desktop(HINSTANCE hInstance, std::tr1::shared_ptr<MessageControl> pMessageControl)
{
  this->pMessageControl = pMessageControl;
  mainInst = hInstance;
  registered = false;
}

bool Desktop::Initialize()
{
  WNDCLASSEX wincl;
  ZeroMemory(&wincl, sizeof(WNDCLASSEX));

  // Register the window class
  wincl.hInstance = mainInst;
  wincl.lpszClassName = desktopClass;
  wincl.lpfnWndProc = DesktopProcedure;
  wincl.cbSize = sizeof (WNDCLASSEX);
  wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hCursor = LoadCursor (NULL, IDC_ARROW);

  // Register the window class, and if it fails quit the program
  if (!RegisterClassEx (&wincl))
    return false;

  // The class is registered, let's create the window
  mainWnd = CreateWindowEx(WS_EX_TOOLWINDOW, desktopClass, NULL, WS_POPUP,
                           0, 0, 0, 0, NULL, NULL, mainInst, reinterpret_cast<LPVOID>(this));

  // If the window failed to get created, unregister the class and quit the program
  if (!mainWnd)
    {
      ELMessageBox(GetDesktopWindow(),
                   (WCHAR*)TEXT("Failed to create desktop window"),
                   (WCHAR*)TEXT("emergeDesktop"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      return false;
    }

  SetWindowPos(mainWnd, HWND_BOTTOM, GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
               GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN),
               SWP_SHOWWINDOW);

  SetBackgroundImage();

  if (ELRegisterShellHook(mainWnd, RSH_PROGMAN))
    ShellMessage = RegisterWindowMessage(TEXT("SHELLHOOK"));

  return true;
}

Desktop::~Desktop()
{
  if (registered)
    {
      // Unregister the window class
      UnregisterClass(desktopClass, mainInst);
    }
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
LRESULT CALLBACK Desktop::DesktopProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  CREATESTRUCT *cs;
  static Desktop *pDesktop = NULL;

  if (message == WM_CREATE)
    {
      cs = (CREATESTRUCT*)lParam;
      pDesktop = reinterpret_cast<Desktop*>(cs->lpCreateParams);
      return DefWindowProc(hwnd, message, wParam, lParam);
    }

  if (pDesktop == NULL)
    return DefWindowProc(hwnd, message, wParam, lParam);

  switch (message)
    {
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

    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      PaintDesktop(hdc);
      EndPaint(hwnd, &ps);
    }
    break;

    case WM_RBUTTONDOWN:
      pDesktop->ShowMenu(CORE_RIGHTMENU);
      break;

    case WM_MBUTTONDOWN:
      pDesktop->ShowMenu(CORE_MIDMENU);
      break;

    case WM_LBUTTONDOWN:
      pDesktop->ShowMenu(CORE_LEFTMENU);
      break;

    case WM_DISPLAYCHANGE:
      pDesktop->DoDisplayChange(hwnd);
      break;

    case WM_WINDOWPOSCHANGING:
      pDesktop->DoWindowPosChanging((LPWINDOWPOS)lParam);
      break;

    case WM_TIMER:
      return pDesktop->DoTimer((UINT_PTR)wParam);

    case WM_DESTROY:
    case WM_NCDESTROY:
      PostQuitMessage(0);
      break;

      // If not handled just forward the message on
    default:
      return pDesktop->DoDefault(hwnd, message, wParam, lParam);
    }

  return 0;
}

/** \brief Handle the WM_DISPLAYCHANGE message
 *
 * \param HWND hwnd
 * \return LRESULT 1
 *
 */
LRESULT Desktop::DoDisplayChange(HWND hwnd)
{
  /**< Adjust the window position to cover the desktop */
  SetWindowPos(hwnd, HWND_BOTTOM,
               GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
               GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN),
               SWP_SHOWWINDOW);

  /**< Set the background image again so that the user doesn't end up with a blank background */
  SetBackgroundImage();

  return 1;
}

LRESULT Desktop::DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT Desktop::DoTimer(UINT_PTR timerID)
{
  struct __stat64 bgstat;
  RECT bgRect;

  if (timerID == BACKGROUND_TIMER)
    {
      _wstat64(bgImage, &bgstat);

      if (modifyTime != bgstat.st_mtime)
        {
          modifyTime = bgstat.st_mtime;
          GetClientRect(mainWnd, &bgRect);
          InvalidateRect(mainWnd, &bgRect, TRUE);
        }

      return 0;
    }

  return 1;
}

void Desktop::DoWindowPosChanging(LPWINDOWPOS winPos)
{
  if (minimizedWindowDeque.empty())
    {
      winPos->hwndInsertAfter = HWND_BOTTOM;
      winPos->flags |= SWP_NOACTIVATE;
      winPos->flags &= ~SWP_NOZORDER;
    }
  else
    winPos->hwndInsertAfter = NULL;
}

void Desktop::ToggleDesktop()
{
  std::deque<HWND>::iterator iter;
  if (!minimizedWindowDeque.empty())
    {
      for (iter = minimizedWindowDeque.begin(); iter != minimizedWindowDeque.end(); ++iter)
        ShowWindow(*iter, SW_SHOWNOACTIVATE);
      minimizedWindowDeque.clear();
      SetWindowPos(mainWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOACTIVATE);
    }
  else
    {
      EnumWindows(MinimizeWindowsEnum, (LPARAM)&minimizedWindowDeque);
      SetWindowPos(mainWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOSENDCHANGING);
      for (iter = minimizedWindowDeque.begin(); iter != minimizedWindowDeque.end(); ++iter)
        ShowWindow(*iter, SW_SHOWMINNOACTIVE);
    }
}

void Desktop::ShowMenu(UINT menu)
{
  pMessageControl->DispatchMessage(EMERGE_CORE, menu);
}

bool Desktop::SetBackgroundImage()
{
  HKEY key;
  DWORD type, bgImageSize = MAX_PATH;
  bool ret = false;
  struct __stat64 bgstat;

  KillTimer(mainWnd, BACKGROUND_TIMER);

  if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop\\"), 0, KEY_ALL_ACCESS,
                   &key) == ERROR_SUCCESS)
    {
      if (RegQueryValueEx(key, TEXT("Wallpaper"), NULL, &type, (BYTE*)bgImage, &bgImageSize) ==
          ERROR_SUCCESS)
        {
          _wstat64(bgImage, &bgstat);
          modifyTime = bgstat.st_mtime;

          SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (void*)bgImage,
                               SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
          SetTimer(mainWnd, BACKGROUND_TIMER, BACKGROUND_POLL_INTERVAL, NULL);

          ret = true;
        }

      RegCloseKey(key);
    }

  return ret;
}

