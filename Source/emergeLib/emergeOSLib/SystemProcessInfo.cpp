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

#include "SystemProcessInfo.h"

/*!
  @fn ELOSVersionInfo()
  @brief Determines the version of the Operating System
  @return Version of Operating System

  @note Windows 8.1       6.3
  Windows Server 2012 R2  6.3
  Windows 8               6.2
  Windows Server 2012     6.2
  Windows 7	              6.1
  Windows Server 2008 R2  6.1
  Windows Server 2008	    6.0
  Windows Vista	          6.0
  Windows Server 2003 R2	5.2
  Windows Server 2003	    5.2
  Windows XP	            5.1
  Windows 2000	          5.0
  */

double ELOSVersionInfo()
{
  OSVERSIONINFO osv;
  ZeroMemory(&osv, sizeof(osv));
  osv.dwOSVersionInfoSize = sizeof(osv);
  GetVersionEx(&osv);

  double fMajor = (double)osv.dwMajorVersion;
  double fMinor = (double)osv.dwMinorVersion / 10.0;

  return (fMajor + fMinor);
}

std::wstring ELGetProcessIDApp(DWORD processID, bool fullName)
{
  DWORD needed;
  HANDLE hProcess;
  HMODULE hMod;
  WCHAR tmp[MAX_PATH];
  std::wstring applet = TEXT("");

  ZeroMemory(&tmp, MAX_PATH);

  // Get a handle to the process.
  hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |  PROCESS_VM_READ,  FALSE, processID);

  // Get the process name.
  if (NULL != hProcess )
  {
    if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &needed))
    {
      if (fullName)
      {
        GetModuleFileNameEx(hProcess, hMod, tmp, sizeof(tmp));
      }
      else
      {
        GetModuleBaseName(hProcess, hMod, tmp, sizeof(tmp));
      }
    }

    // Print the process name and identifier.
    applet = tmp;

    CloseHandle(hProcess);
  }

  return applet;
}

/*!
  @fn ELGetWindowApp(HWND hWnd, bool fullName)
  @brief Determines the process name based on the supplied window handle
  @param hWnd Window handle
  @param processName Populated with process owning the window handle
  @param fullName if true return fully qualified name, if false return basename
  @return std::wstring
  */

std::wstring ELGetWindowApp(HWND hWnd, bool fullName)
{
  DWORD processID;

  GetWindowThreadProcessId(hWnd, &processID);
  return ELGetProcessIDApp(processID, fullName);
}
