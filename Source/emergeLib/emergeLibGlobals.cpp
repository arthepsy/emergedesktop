//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2013  The Emerge Desktop Development Team
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

#include "emergeLibGlobals.h"

emergeLibGlobals::emergeLibGlobals()
{
  /* This is dark sorcery that gets a DLL's HINSTANCE. GetModuleFilename gets the filename of the EXE the DLL is running in,
  not the DLL itself. So a little hacking is necessary...
  Taken from http://stackoverflow.com/questions/2126657/how-can-i-get-hinstance-from-a-dll, which links to
  http://www.codeguru.com/cpp/w-p/dll/tips/article.php/c3635/Tip-Detecting-a-HMODULEHINSTANCE-Handle-Within-the-Module-Youre-Running-In.htm */
  MEMORY_BASIC_INFORMATION mbi;
  static int dummy;
  VirtualQuery( &dummy, &mbi, sizeof(mbi) );
  emergeLibInstance = reinterpret_cast<HINSTANCE>(mbi.AllocationBase);

  mprDLL = ELLoadSystemLibrary(TEXT("mpr.dll"));
  shell32DLL = ELLoadSystemLibrary(TEXT("shell32.dll"));
  user32DLL = ELLoadSystemLibrary(TEXT("user32.dll"));
  kernel32DLL = ELLoadSystemLibrary(TEXT("kernel32.dll"));
  dwmapiDLL = ELLoadSystemLibrary(TEXT("dwmapi.dll"));
  shlwapiDLL = ELLoadSystemLibrary(TEXT("shlwapi.dll"));
}

emergeLibGlobals::~emergeLibGlobals()
{
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
}

HINSTANCE emergeLibGlobals::getEmergeLibInstance()
{
  static emergeLibGlobals* globals;

  if (globals == NULL)
    globals = new emergeLibGlobals;

  return globals->emergeLibInstance;
}

HMODULE emergeLibGlobals::getMprDLL()
{
  static emergeLibGlobals* globals;

  if (globals == NULL)
    globals = new emergeLibGlobals;

  return globals->mprDLL;
}

HMODULE emergeLibGlobals::getShell32DLL()
{
  static emergeLibGlobals* globals;

  if (globals == NULL)
    globals = new emergeLibGlobals;

  return globals->shell32DLL;
}

HMODULE emergeLibGlobals::getUser32DLL()
{
  static emergeLibGlobals* globals;

  if (globals == NULL)
    globals = new emergeLibGlobals;

  return globals->user32DLL;
}

HMODULE emergeLibGlobals::getKernel32DLL()
{
  static emergeLibGlobals* globals;

  if (globals == NULL)
    globals = new emergeLibGlobals;

  return globals->kernel32DLL;
}

HMODULE emergeLibGlobals::getDwmapiDLL()
{
  static emergeLibGlobals* globals;

  if (globals == NULL)
    globals = new emergeLibGlobals;

  return globals->dwmapiDLL;
}

HMODULE emergeLibGlobals::getShlwapiDLL()
{
  static emergeLibGlobals* globals;

  if (globals == NULL)
    globals = new emergeLibGlobals;

  return globals->shlwapiDLL;
}
