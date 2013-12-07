//----  --------------------------------------------------------------------------------------------------------
//
// This file is part of the Emerge Desktop source code
// Copyright (C) 2004-2013 The Emerge Desktop Development Team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//----  --------------------------------------------------------------------------------------------------------

#include "main.h"

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
int WINAPI WinMain (HINSTANCE hInstance UNUSED,
                    HINSTANCE hPrevInstance UNUSED,
                    LPSTR lpCmdLine,
                    int nCmdShow UNUSED)

{
  std::wstring commandLine, error;

  commandLine = ELstringTowstring(lpCmdLine, CP_ACP);

  if (commandLine.empty())
  {
    error = TEXT("Usage: emerge <Internal Command>");
    ELMessageBox(GetDesktopWindow(), error, TEXT("emerge"), ELMB_ICONERROR | ELMB_OK | ELMB_MODAL);
    return 1;
  }

  if (!ELExecuteFileOrCommand(commandLine))
  {
    error = TEXT("Failed to execute ");
    error = error + commandLine;
    ELMessageBox(GetDesktopWindow(), error, TEXT("emerge"), ELMB_ICONWARNING | ELMB_OK | ELMB_MODAL);
    return 2;
  }

  // The program return-value is 0 - The value that PostQuitMessage() gave
  return 0;
}

