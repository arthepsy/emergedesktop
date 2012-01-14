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

#include "Core.h"

//----  --------------------------------------------------------------------------------------------------------
// Function:	WinMain
// Required:	HINSTANCE hThisInstance - the instance of this application
// 		HINSTANCE hPrevInstance - not used
// 		LPTSTR lpCmdLine - command line arguments
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
  WCHAR app[MAX_PATH], args[MAX_LINE_LENGTH], argsTmp[MAX_LINE_LENGTH], *token;
  HANDLE hMutex = NULL;
  bool abort = true, block = false;

  if (!ELParseCommand(GetCommandLine(), app, argsTmp))
    {
      ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("Failed to parse command line"), (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      return 1;
    }

  token = wcstok(argsTmp, TEXT(" \t"));

  if (token == NULL)
    wcscpy(args, TEXT("\0"));
  else
    wcscpy(args, token);

  if (wcslen(args) == 0)
    {
      abort = false;
      block = true;
    }
  else
    {
      if (wcsstr(args, TEXT("/shellchanger")) != NULL)
        abort = false;

      if (wcsstr(args, TEXT("/nostartup")) != NULL)
        {
          abort = false;
          block = true;
        }
    }

  if (abort)
    {
      ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("Invalid command line switch"), (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      return 1;
    }

  if (block)
    {
      // Check to see if emergeCore is already running, if so exit
      hMutex = CreateMutex(NULL, false, TEXT("emergeCore"));
      if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
          CloseHandle(hMutex);
          return 1;
        }

      // Switching shell event
      HANDLE hEv = OpenEvent(EVENT_MODIFY_STATE, false, L"Global\\msgina: ShellReadyEvent");
      if(hEv)
        {
          SetEvent(hEv);
          CloseHandle(hEv);
        }
      hEv = OpenEvent(EVENT_MODIFY_STATE, false, L"msgina: ShellReadyEvent");
      if(hEv)
        {
          SetEvent(hEv);
          CloseHandle(hEv);
        }
      hEv = OpenEvent(EVENT_MODIFY_STATE, false, L"ShellDesktopSwitchEvent");
      if(hEv)
        {
          SetEvent(hEv);
          CloseHandle(hEv);
        }
    }

  Core core(hInstance);

  if (!core.Initialize(args))
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
  if (hMutex)
    CloseHandle(hMutex);

  // The program return-value is 0 - The value that PostQuitMessage() gave
  return (int)messages.wParam;
}

