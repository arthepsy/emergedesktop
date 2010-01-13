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

#include "Desktop.h"

WCHAR desktopClass[] = TEXT("progman");

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

  if (ELRegisterShellHook(mainWnd, RSH_TASKMGR))
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
    {
      SetWindowPos(hwnd, HWND_BOTTOM,
                   GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
                   GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN),
                   SWP_SHOWWINDOW);
    }
    break;

    case WM_WINDOWPOSCHANGING:
    {
      LPWINDOWPOS lpWinPos = (LPWINDOWPOS)lParam;

      lpWinPos->hwndInsertAfter = HWND_BOTTOM;
      lpWinPos->flags |= SWP_NOACTIVATE;
      lpWinPos->flags &= ~SWP_NOZORDER;
    }
    break;

    case WM_DESTROY:
    case WM_NCDESTROY:
      PostQuitMessage(0);
      break;

      // If not handled just forward the message on
    default:
      //if (!pDesktop->DoDefault(message, (UINT)wParam, (HWND)lParam))
      return DefWindowProc(hwnd, message, wParam, lParam);
    }

  return 0;
}

void Desktop::ShowMenu(UINT menu)
{
  pMessageControl->DispatchMessage(EMERGE_CORE, menu);
}

LRESULT Desktop::DoDefault(UINT message, UINT shellMessage, HWND task)
{
  WCHAR taskClass[MAX_LINE_LENGTH];
  WINDOWINFO wndInfo;
  wndInfo.cbSize = sizeof(WINDOWINFO);

  if (message == ShellMessage)
    {
      switch (shellMessage)
        {
          //A new "task" was created
        case HSHELL_WINDOWCREATED:
          RealGetWindowClass(task, taskClass, MAX_LINE_LENGTH);
          if (_wcsicmp(taskClass, TEXT("$$$Secure UAP Dummy Window Class For Interim Dialog")) == 0)
            ELSwitchToThisWindow(task);
          return 1;
        }

    }

  return 0;
}

bool Desktop::SetBackgroundImage()
{
  HKEY key;
  WCHAR bgImage[MAX_PATH];
  DWORD type, bgImageSize = MAX_PATH;
  bool ret = false;

  if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop\\"), 0, KEY_ALL_ACCESS,
                   &key) == ERROR_SUCCESS)
    {
      if (RegQueryValueEx(key, TEXT("Wallpaper"), NULL, &type, (BYTE*)bgImage, &bgImageSize) ==
          ERROR_SUCCESS)
        {
          SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (void*)bgImage,
                               SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
          ret = true;
        }

      RegCloseKey(key);
    }

  return ret;
}

