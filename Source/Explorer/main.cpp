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

//----  --------------------------------------------------------------------------------------------------------
// Function:	WinMain
// Required:	HINSTANCE hThisInstance - the instance of this application
// 		HINSTANCE hPrevInstance - not used
// 		LPSTR lpCmdLine - command line arguments
// 		int nCmdShow - Show / Hide window indicator
// Returns:	int
// Purpose:	Creates the window class as well as the main and tooltip
// 		windows
//----  --------------------------------------------------------------------------------------------------------
int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance UNUSED,
                    LPSTR lpCmdLine UNUSED,
                    int nCmdShow UNUSED)

{
  MSG messages;
  bool showDesktop = false;
  WCHAR app[MAX_PATH], args[MAX_LINE_LENGTH];

  // If Windows Explorer is running as the shell, then don't start
  if (ELIsExplorerShell())
    return 1;

  /**< Check to see if the explorer desktop should be created */
  ELParseCommand(GetCommandLine(), app, args);
  if (_wcsicmp(args, TEXT("/showdesktop")) == 0)
    showDesktop = true;

  // Check to see if Explorer is already running, if so exit
  HANDLE hMutex = CreateMutex(NULL, false, TEXT("Explorer"));
  if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
      if (!showDesktop)
        {
          WCHAR passCmd[MAX_LINE_LENGTH];
          std::wstring explorer = TEXT("%WINDIR%\\explorer.exe");
          swprintf(passCmd, TEXT("%ls %ls"), ELExpandVars(explorer).c_str(), args);
          ELExecute(passCmd);
        }
      CloseHandle(hMutex);
      return 2;
    }

  Applet applet(hInstance);

  if (!applet.Initialize(showDesktop))
    return 3;

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
