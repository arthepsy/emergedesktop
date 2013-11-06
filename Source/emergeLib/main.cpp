/*!
  @file main.cpp
  @brief Utility functions for Emerge Desktop
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2013  The Emerge Desktop Development Team

  @attention Emerge Desktop is free software; you can redistribute it and/or
  modify  it under the terms of the GNU General Public License as published
  by  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  @attention Emerge Desktop is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  @attention You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  @note 1)	For MSVC users, put the AggressiveOptimize.h header file (available from
http://www.nopcode.com) in your 'include' directory.  It cuts down on executable
filesize.
@note 2)	The ELExit code is based on a function from the LiteShell code base.

@note 3)  Using vectors for strings (taken from
http://www.codeguru.com/forum/archive/index.php/t-193852.html):
Whenever a function wants a constant pointer to a string (LPCSTR, LPWCSTR), you can
supply the string using the c_str() function. No problem since the buffer is not
modified by the function.

However, if the function wants to modify the string contents (an LPSTR or LPWSTR),
you can't use c_str() since c_str() returns a pointer to an unmodifiable buffer.
Even so, a std::(w)string may not store its contents in a contiguous buffer, which
is what the function requires. Instead you use a vector<WCHAR> or a vector<char> to
store your characters. You must make sure that the vector is sized (via reserve or
resize)appropriately when it is constructed.

A vector is guaranteed to have its contents stored in contiguous memory, and its
contents are modifiable, providing you have a sufficient number of elements in the
vector. You then pass the address of the first element of the vector -- this is no
different than passing the address of the first character. The advantage of this is
that you don't have to hard code your array sizes, since the constructor for a vector
takes any integer argument for the size.

This is a trick that you won't learn from Microsoft or MSDN -- how to use std::string,
vector<> and std::wstring with their API (or any function that requires pointers to
buffers). In the Scott Meyers book "Effective STL", he has a whole chapter on just this
topic alone.
*/

#include "main.h"

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL UNUSED, DWORD fdwReason UNUSED, LPVOID lpvReserved UNUSED)
{
  /*switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
      emergeLibInstance = hinstDLL;
      if (mprDLL == NULL)
        mprDLL = ELLoadSystemLibrary(TEXT("mpr.dll"));
      if (shell32DLL == NULL)
        shell32DLL = ELLoadSystemLibrary(TEXT("shell32.dll"));
      if (user32DLL == NULL)
        user32DLL = ELLoadSystemLibrary(TEXT("user32.dll"));
      if (kernel32DLL == NULL)
        kernel32DLL = ELLoadSystemLibrary(TEXT("kernel32.dll"));
      if (dwmapiDLL == NULL)
        dwmapiDLL = ELLoadSystemLibrary(TEXT("dwmapi.dll"));
      if (shlwapiDLL == NULL)
        shlwapiDLL = ELLoadSystemLibrary(TEXT("shlwapi.dll"));
      break;
    case DLL_PROCESS_DETACH:
      if (mprDLL != NULL)
        {
          FreeLibrary(mprDLL);
          mprDLL = NULL;
        }
      if (shlwapiDLL != NULL)
        {
          FreeLibrary(shlwapiDLL);
          shlwapiDLL = NULL;
        }
      if (shell32DLL != NULL)
        {
          FreeLibrary(shell32DLL);
          shell32DLL = NULL;
        }
      if (user32DLL != NULL)
        {
          FreeLibrary(user32DLL);
          user32DLL = NULL;
        }
      if (kernel32DLL != NULL)
        {
          FreeLibrary(kernel32DLL);
          kernel32DLL = NULL;
        }
      break;
    }*/

  return TRUE;
}
