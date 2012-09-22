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

#include "Desktop.h"

std::deque<HWND> hwndDeque;

WCHAR desktopClass[] = TEXT("EmergeDesktopProgman");

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
  mainWnd = NULL;
  registered = false;
  explorerDesktop = false;
  ZeroMemory(currentBG, MAX_PATH);
  SetRectEmpty(&currentDesktopRect);
  dirWatch = INVALID_HANDLE_VALUE;
  ShellMessage = 0;
  ZeroMemory(bgImage, MAX_PATH);
  wallpaperThread = NULL;
}

bool Desktop::Initialize(bool explorerDesktop)
{
  WNDCLASSEX wincl;
  ZeroMemory(&wincl, sizeof(WNDCLASSEX));
  UINT SWPFlags = 0;
  this->explorerDesktop = explorerDesktop;

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

  if (explorerDesktop)
    SWPFlags = SWP_HIDEWINDOW;
  else
    SWPFlags = SWP_SHOWWINDOW;

  SetWindowPos(mainWnd, HWND_BOTTOM, GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
               GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN),
               SWPFlags);

  SetBackgroundImage();

  if (ELRegisterShellHook(mainWnd, RSH_PROGMAN))
    ShellMessage = RegisterWindowMessage(TEXT("SHELLHOOK"));

  return true;
}

Desktop::~Desktop()
{
  // Kill the wallpaperThread
  TerminateThread(wallpaperThread, 0);

  if (registered)
    {
      // Unregister the window class
      UnregisterClass(desktopClass, mainInst);
    }
}

void Desktop::ShowDesktop(bool show)
{
  if (show)
    ShowWindow(mainWnd, SW_SHOW);
  else
    ShowWindow(mainWnd, SW_HIDE);
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

    case WM_SETTINGCHANGE:
      pDesktop->SetBackgroundImage();
      break;

    case WM_ERASEBKGND:
      return PaintDesktop((HDC)wParam);

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
  // If explorerDesktop is active, hide the Core's desktop window
  UINT SWPFlags = SWP_SHOWWINDOW;
  if (explorerDesktop)
    SWPFlags = SWP_HIDEWINDOW;

  // Determine newDesktopRect based on virtual desktop size
  RECT newDesktopRect;
  newDesktopRect.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
  newDesktopRect.right = newDesktopRect.left +
                         GetSystemMetrics(SM_CXVIRTUALSCREEN);
  newDesktopRect.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
  newDesktopRect.bottom = newDesktopRect.top +
                          GetSystemMetrics(SM_CYVIRTUALSCREEN);

  // Check to see if newDesktopRect is the same as currentDesktopRect...
  if (!EqualRect(&newDesktopRect, &currentDesktopRect))
    {
      // If so, copy newDesktopRect to currentDesktopRect for future comparisons
      CopyRect(&currentDesktopRect, &newDesktopRect);

      // Adjust the window position to cover the desktop
      SetWindowPos(hwnd, HWND_BOTTOM,
                   newDesktopRect.left, newDesktopRect.top,
                   GetSystemMetrics(SM_CXVIRTUALSCREEN),
                   GetSystemMetrics(SM_CYVIRTUALSCREEN),
                   SWPFlags);

      // Force a repaint of the background
      InvalidateRect(hwnd, NULL, TRUE);

      return 1;
    }

  return 0;
}

LRESULT Desktop::DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  return DefWindowProc(hwnd, message, wParam, lParam);
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
      while (!minimizedWindowDeque.empty())
        {
          ShowWindow(minimizedWindowDeque.front(), SW_RESTORE);
          minimizedWindowDeque.pop_front();
        }
      if (!explorerDesktop)
        SetWindowPos(mainWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOACTIVATE);
    }
  else
    {
      EnumWindows(MinimizeWindowsEnum, (LPARAM)&minimizedWindowDeque);
      if (!explorerDesktop)
        SetWindowPos(mainWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOSENDCHANGING);
      for (iter = minimizedWindowDeque.begin(); iter != minimizedWindowDeque.end(); ++iter)
        ShowWindow(*iter, SW_SHOWMINNOACTIVE);
    }
}

void Desktop::ShowMenu(UINT menu)
{
  pMessageControl->DispatchMessage(EMERGE_CORE, menu, NULL);
}

bool Desktop::SetBackgroundImage()
{
  DWORD threadState, threadID;
  WCHAR newBG[MAX_PATH];

  // Retrieve the new wallpaper
  SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, newBG, 0);
  // Compare the background to the current background
  if (wcsicmp(newBG, currentBG) != 0)
    {
      // if different, store
      wcscpy(currentBG, newBG);

      // If dirWatch is valid close it
      if (dirWatch != INVALID_HANDLE_VALUE)
        FindCloseChangeNotification(dirWatch);

      // Set dirWatch to monitor the background path
      PathRemoveFileSpec(newBG);
      dirWatch = FindFirstChangeNotification(newBG, FALSE,
                                             FILE_NOTIFY_CHANGE_LAST_WRITE);

      // Force Desktop to repaint
      InvalidateDesktop();

      // Start a new thread to watch the wallpaper directory
      GetExitCodeThread(wallpaperThread, &threadState);
      if (threadState != STILL_ACTIVE)
        wallpaperThread = CreateThread(NULL, 0, WallpaperThreadProc, this, 0, &threadID);
    }

  return true;
}

BOOL Desktop::InvalidateDesktop()
{
  SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, SETWALLPAPER_DEFAULT, 0);
  return InvalidateRect(mainWnd, NULL, TRUE);
}

DWORD WINAPI Desktop::WallpaperThreadProc(LPVOID lpParameter)
{
  // reinterpret lpParameter as a Desktop*
  Desktop *pDesktop = reinterpret_cast< Desktop* >(lpParameter);

  // If dirWatch is invalid exit the thread
  if (pDesktop->dirWatch == INVALID_HANDLE_VALUE)
    ExitThread(1);

  // Start an infinite loop to watch the directory
  while (true)
    {
      // Wait for a file write to occur
      WaitForSingleObject(pDesktop->dirWatch, INFINITE);
      // Reset dirWatch, if it fails, kill the thread
      if (FindNextChangeNotification(pDesktop->dirWatch) == FALSE)
        ExitThread(0);
      // If the reset is successful, invalidate the desktop
      pDesktop->InvalidateDesktop();
    }

  return 0;
}

