//----  --------------------------------------------------------------------------------------------------------
//
// This file is part of the Emerge Desktop source code
// Copyright (C) 2004-2011 The Emerge Desktop Development Team
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

#define _NO_W32_PSEUDO_MODIFIERS

#include "../emergeLib/emergeLib.h"
#include <stdio.h>

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
  WCHAR commandLine[MAX_LINE_LENGTH], error[MAX_LINE_LENGTH];

  MultiByteToWideChar(CP_ACP, 0, _strlwr(lpCmdLine), (int)strlen(lpCmdLine) + 1, commandLine,
                      (int)sizeof(commandLine)/(int)sizeof(commandLine[0]));

  if (wcslen(commandLine) == 0)
    {
      swprintf(error, TEXT("Usage: emerge <Internal Command>"));
      ELMessageBox(GetDesktopWindow(), error, (WCHAR*)TEXT("emerge"), ELMB_ICONERROR|ELMB_OK|ELMB_MODAL);
      return 1;
    }

  if (!ELExecuteInternal(commandLine))
    {
      swprintf(error, TEXT("Failed to execute \"%ls\""), commandLine);
      ELMessageBox(GetDesktopWindow(), error, (WCHAR*)TEXT("emerge"), ELMB_ICONWARNING|ELMB_OK|ELMB_MODAL);
      return 2;
    }

  // The program return-value is 0 - The value that PostQuitMessage() gave
  return 0;
}

