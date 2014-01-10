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
  HANDLE hProcess;
  WCHAR tmp[MAX_PATH];

  ZeroMemory(&tmp, MAX_PATH);

  // Get a handle to the process.
  hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |  PROCESS_VM_READ,  FALSE, processID);

  // Get the process name.
  if (hProcess != NULL)
  {
    SetLastError(ERROR_SUCCESS); //clear the last error flag, since GetModuleFileNameEx might trigger an error under certain
    //circumstances (being run on a 64-bit process from a 32-bit app) and we need to reliably see whether that happens
    //http://winprogger.com/getmodulefilenameex-enumprocessmodulesex-failures-in-wow64/ explains why this happens
    GetModuleFileNameEx(hProcess, NULL, tmp, sizeof(tmp)); //get the file path for the given process handle
    if (GetLastError() == ERROR_PARTIAL_COPY)
    {
      GetProcessImageFileName(hProcess, tmp, sizeof(tmp));
      MapDevicePathToDrivePath(tmp);
    }

    if (wcslen(tmp) > 0)
    {
      GetLongPathName(tmp, tmp, sizeof(tmp)); //according to MSDN community content, under very specific circumstances
      //(a process is launched by short name on a x64 system), GetModuleFileNameEx will return a short name. Expand it
      //just in case.
    }

    if (!fullName)
    {
      wcscpy(tmp, PathFindFileName(tmp)); //pull the filename out of the path
    }

    CloseHandle(hProcess);
  }

  return tmp;
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

bool MapDevicePathToDrivePath(LPWSTR path)
{
  WCHAR logicalDriveStrings[MAX_PATH];
  WCHAR driveName[3] = TEXT(" :");
  WCHAR dosDeviceName[MAX_PATH];
  size_t dosDeviceNameLength;
  WCHAR workingPath[MAX_PATH];
  bool success = false;

  ZeroMemory(logicalDriveStrings, MAX_PATH);
  ZeroMemory(dosDeviceName, MAX_PATH);
  ZeroMemory(workingPath, MAX_PATH);

  if (GetLogicalDriveStrings(MAX_PATH, logicalDriveStrings)) //get a list of the drive letters available
  {
    WCHAR* logicalDrivePtr = logicalDriveStrings;

    while ((*logicalDrivePtr) && (success == false)) //keep iterating until we successfully complete or run out of characters in the drives buffer
    {
      *driveName = *logicalDrivePtr; //get the current drive letter; note that the colon from the driveName initialization will stay, so we'll end up
      //with "(DriveLetter):". Example: "C:"

      if (QueryDosDevice(driveName, dosDeviceName, MAX_PATH)) //see if the drive letter has a matching DOS device name
      {
        dosDeviceNameLength = wcslen(dosDeviceName);
        if (_wcsnicmp(dosDeviceName, path, dosDeviceNameLength) == 0) //see if the DOS device name matches the one in the path we were given
        {
          swprintf(workingPath, TEXT("%s%s"), driveName, path+dosDeviceNameLength); //copy the drive letter and the remainder of the path
          //we were given into a working buffer. Note that the DOS device name will not come along, since path is technically a pointer to the start
          //of the path string; path + dosDeviceNameLength is a pointer to the part of the path directly following the DOS device name
          wcscpy(path, workingPath); //copy our working buffer back into the path buffer we were given
          success = true; //and set the success flag so we can break the loop
        }
      }

      *logicalDrivePtr++; //move to the next character in the drive letter buffer
    }
  }

  return success;
}
