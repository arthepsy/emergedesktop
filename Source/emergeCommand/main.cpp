//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2007  The Emerge Desktop Development Team
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

#include "Applet.h"

BOOL CALLBACK FocusWindowEnum(HWND hwnd, LPARAM lParam UNUSED)
{
  WCHAR window[MAX_PATH];

  ZeroMemory(window, MAX_PATH);
  ELGetWindowApp(hwnd, window, false);
  _wcslwr(window);

  if (wcscmp(window, TEXT("emergecommand.exe")) == 0)
    {
      if (!IsWindowVisible(hwnd))
        return true;

      SetForegroundWindow(hwnd);
      SendMessage(hwnd, WM_LBUTTONDOWN, 0, 0);

      return false;
    }

  return true;
}

//-----
// Function:	WinMain
// Required:	HINSTANCE hThisInstance - the instance of this application
// 		HINSTANCE hPrevInstance - not used
// 		LPSTR lpCmdLine - command line arguments
// 		int nCmdShow - Show / Hide window indicator
// Returns:	int
// Purpose:	Creates the window class as well as the main and tooltip
// 		windows
//-----
int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance UNUSED,
                    LPSTR lpCmdLine UNUSED,
                    int nCmdShow UNUSED)

{
  MSG messages;

  // Check to see if iTray is already running, if so exit
  HANDLE hMutex = CreateMutex(NULL, false, TEXT("emergeCommand"));
  if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
      EnumWindows(FocusWindowEnum, 0);
      CloseHandle(hMutex);
      return 0;
    }

  Applet applet(hInstance);

  if (!applet.Initialize())
    return 0;

  // Run the message loop. It will run until GetMessage() returns 0
  while (GetMessage (&messages, NULL, 0, 0))
    {
      // Translate virtual-key messages into character messages
      TranslateMessage(&messages);
      // Send message to WindowProcedure
      DispatchMessage(&messages);
    }

  // Clean-up the Mutex
  CloseHandle(hMutex);

  // The program return-value is 0 - The value that PostQuitMessage() gave
  return (int)messages.wParam;
}
