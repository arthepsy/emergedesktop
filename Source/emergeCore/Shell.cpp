// vim: tags+=../emergeLib/tags
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
//
// Notes:
//
// 1)	The FirstRunCheck function is based on code from StartUpRunner
// which is part of the LiteStep code base.
//
//----  --------------------------------------------------------------------------------------------------------

#include "Shell.h"

// Global Variables
std::vector<HANDLE> processList;

Shell::Shell()
{}

Shell::~Shell()
{}

//----  --------------------------------------------------------------------------------------------------------
// Function:	RunRegEntries
// Required:	HKEY key - registry whith entries to be executed
// 		bool clearEntry - delete the entry after execution
// Returns:	bool - true if entries exist, false otherwise
// Purpose:	Enumerates and executes the registry key contents
//----  --------------------------------------------------------------------------------------------------------
bool Shell::RunRegEntries(HKEY key, bool clearEntry, bool wait UNUSED, bool showStartupErrors)
{
  DWORD type, index = 0, valueSize = MAX_LINE_LENGTH, dataSize = MAX_LINE_LENGTH;
  BYTE data[MAX_LINE_LENGTH];
  WCHAR value[MAX_LINE_LENGTH];
  WCHAR error[MAX_LINE_LENGTH];
  bool found = false;

  // Loop while there are entries in the key
  while (RegEnumValue(key, index, value, &valueSize, NULL, &type, data, &dataSize) == ERROR_SUCCESS)
    {
      // If it's a string, execute it
      if ((type == REG_SZ) || (type == REG_EXPAND_SZ))
        {
          swprintf(error, TEXT("Failed to execute \"%s\""), (WCHAR*)data);
          if (!ELExecute((WCHAR*)data) && showStartupErrors)
            ELMessageBox(GetDesktopWindow(), error, (WCHAR*)TEXT("emergeCore"), ELMB_ICONWARNING|ELMB_OK);

          if (clearEntry)
            RegDeleteValue(key, value);

          found = true;
        }

      valueSize = MAX_LINE_LENGTH;
      dataSize = MAX_LINE_LENGTH;

      index++;
    }

  return found;
}

void Shell::RegisterShell(HWND hwnd, bool enable)
{
  if (enable)
    {
      if (ELRegisterShellHook(hwnd, RSH_REGISTER))
        if (ELRegisterShellHook(hwnd, RSH_TASKMGR))
          ShellMessage = RegisterWindowMessage(TEXT("SHELLHOOK"));
    }
  else
    ELRegisterShellHook(hwnd, RSH_UNREGISTER);
}

void Shell::BuildTaskList()
{
  EnumWindows(GetTaskCount, reinterpret_cast<LPARAM>(this));
}

BOOL CALLBACK Shell::GetTaskCount(HWND hwnd, LPARAM lParam)
{
  Shell *pShell = reinterpret_cast<Shell*>(lParam);
  if (pShell)
    pShell->CountTask(hwnd);

  return TRUE;
}

void Shell::CountTask(HWND hwnd)
{
  if ((IsWindowVisible(hwnd)) && ((GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) != WS_EX_TOOLWINDOW))
    UpdateSessionInformation(true, hwnd);
}

bool Shell::UpdateTaskCount(UINT message, UINT shellMessage, HWND task)
{
  if (message == ShellMessage)
    {
      switch (shellMessage)
        {
        case HSHELL_WINDOWDESTROYED:
          return UpdateSessionInformation(false, task);
        case HSHELL_WINDOWCREATED:
          return UpdateSessionInformation(true, task);
        }
    }

  return false;
}

bool Shell::UpdateSessionInformation(bool add, HWND task)
{
  HKEY key;
  DWORD ignore;
  size_t programCount = 0;
  bool result = false, found = false;
  size_t i = 0;
  std::vector<HWND>::iterator iter;

  for (i = 0; i < taskList.size(); i++)
    if (taskList[i] == task)
      {
        found = true;
        break;
      }

  if (add)
    {
      if (!found)
        taskList.push_back(task);
      else
        return false;
    }
  else
    {
      if (!found)
        return false;
      else
        taskList.erase(taskList.begin() + i);
    }

  if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("SessionInformation"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                     NULL, &key, &ignore) == ERROR_SUCCESS)
    {
      programCount = taskList.size();

      if (RegSetValueEx(key, TEXT("ProgramCount"), 0, REG_DWORD, (BYTE*)&programCount, sizeof(DWORD)) == ERROR_SUCCESS)
        result = true;

      ELCloseRegKey(key);
    }

  return result;
}

void Shell::ClearSessionInformation()
{
  RegDeleteKey(HKEY_CURRENT_USER, TEXT("SessionInformation"));
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	RunRegStartup
// Required:	Nothing
// Returns:	Nothing
// Purpose:	Executes the contents of the startup registry keys
//----  --------------------------------------------------------------------------------------------------------
void Shell::RunRegStartup(bool showStartupErrors)
{
  HKEY key;

  // Execute the HKLM Run key
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                   0,
                   KEY_READ,
                   &key) == ERROR_SUCCESS)
    {
      RunRegEntries(key, false, false, showStartupErrors);
      ELCloseRegKey(key);
    }

  // Execute the HKCU Run key
  if (RegOpenKeyEx(HKEY_CURRENT_USER,
                   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                   0,
                   KEY_READ,
                   &key) == ERROR_SUCCESS)
    {
      RunRegEntries(key, false, false, showStartupErrors);
      ELCloseRegKey(key);
    }

  // Execute the HKLM 32-bit Run key if it exists
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   TEXT("Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run"),
                   0,
                   KEY_READ,
                   &key) == ERROR_SUCCESS)
    {
      RunRegEntries(key, false, false, showStartupErrors);
      ELCloseRegKey(key);
    }

  // Execute the HKLM RunOnce key, and remove the entry when
  // executed
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce"),
                   0,
                   KEY_ALL_ACCESS,
                   &key) == ERROR_SUCCESS)
    {
      RunRegEntries(key, true, false, showStartupErrors);
      ELCloseRegKey(key);
    }

  // Execute the HKCU RunOnce key, and remove the entry when
  // executed
  if (RegOpenKeyEx(HKEY_CURRENT_USER,
                   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce"),
                   0,
                   KEY_ALL_ACCESS,
                   &key) == ERROR_SUCCESS)
    {
      RunRegEntries(key, true, false, showStartupErrors);
      ELCloseRegKey(key);
    }

  // Execute the HKLM 32-bit RunOnce key if it exists
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   TEXT("Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnce"),
                   0,
                   KEY_READ,
                   &key) == ERROR_SUCCESS)
    {
      RunRegEntries(key, true, false, showStartupErrors);
      ELCloseRegKey(key);
    }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	RunFolderEntries
// Required:	LPTSTR path - path for which to execute files
// Returns:	Nothing
// Purpose:	Enumerates and executes the files in the path
//----  --------------------------------------------------------------------------------------------------------
void Shell::RunFolderEntries(LPTSTR path, bool showStartupErrors)
{
  WCHAR appPath[MAX_PATH], app[MAX_PATH];
  WIN32_FIND_DATA findData;
  HANDLE find;
  bool found = false;
  WCHAR error[MAX_LINE_LENGTH];

  // Check the format of the specified path
  if (path[wcslen(path) - 1] != '\\')
    wcscat(path, TEXT("\\"));
  wcscpy(appPath, path);
  wcscat(path, TEXT("*"));

  // find the first file
  find = FindFirstFile(path, &findData);
  if (find != INVALID_HANDLE_VALUE)
    found = true;

  // loop through all the files in the path
  while (found)
    {
      // Make sure it's a file
      if (wcscmp(findData.cFileName, TEXT(".")) &&
          wcscmp(findData.cFileName, TEXT("..")) &&
          !(findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
          !(findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
        {
          wcscpy(app, appPath);
          wcscat(app, findData.cFileName);
          swprintf(error, TEXT("Failed to execute \"%s\""), app);
          if (!ELExecute((WCHAR*)app) && showStartupErrors)
            ELMessageBox(GetDesktopWindow(), error, (WCHAR*)TEXT("emergeCore"), ELMB_ICONWARNING|ELMB_OK);
        }

      // Get the next file
      if (FindNextFile(find, &findData) == 1)
        found = true;
      else
        found = false;
    }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	RunFolderStartup
// Required:	Nothing
// Returns:	Nothing
// Purpose:	Executes the contents of the startup folders
//----  --------------------------------------------------------------------------------------------------------
void Shell::RunFolderStartup(bool showStartupErrors)
{
  WCHAR szPath[MAX_PATH];
  LPITEMIDLIST item;
  LPMALLOC pMalloc;

  // Get the contents of the common startup folder
  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_STARTUP, &item)))
    {
      if (SHGetPathFromIDList(item, szPath))
        {
          // Execute the contents
          RunFolderEntries(szPath, showStartupErrors);

          // Cleanup
          if (SUCCEEDED(SHGetMalloc(&pMalloc)))
            {
              pMalloc->Free(item);
              pMalloc->Release();
            }
        }
    }

  // Get the contents of the current user's startup folder
  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_STARTUP, &item)))
    {
      if (SHGetPathFromIDList(item, szPath))
        {
          // Execute the contents
          RunFolderEntries(szPath, showStartupErrors);

          // Cleanup
          if (SUCCEEDED(SHGetMalloc(&pMalloc)))
            {
              pMalloc->Free(item);
              pMalloc->Release();
            }
        }
    }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	FirstRunCheck
// Required:	Nothing
// Returns:	bool
// Purpose:	Checks to see if the startup programs have run
//----  --------------------------------------------------------------------------------------------------------
bool Shell::FirstRunCheck()
{
  bool result = false;
  HKEY hkExplorer;
  WCHAR sessionInfo[MAX_LINE_LENGTH];

  HANDLE hToken;
  if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) != 0)
    {
      TOKEN_STATISTICS tsStats;
      DWORD dwOutSize;

      if (GetTokenInformation(hToken, TokenStatistics, &tsStats, sizeof(tsStats), &dwOutSize))
        {
          swprintf(sessionInfo, TEXT("SessionInfo\\%08x%08x"),
                   (UINT)tsStats.AuthenticationId.HighPart,
                   (UINT)tsStats.AuthenticationId.LowPart);

          // Create the SessionInfo and StartUpHasBeenRun keys
          LONG lResult = RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER, 0, NULL,
                                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hkExplorer, NULL);

          if (lResult == ERROR_SUCCESS)
            {
              HKEY hkSessionInfo;

              lResult = RegCreateKeyEx(hkExplorer, sessionInfo, 0, NULL, REG_OPTION_VOLATILE,
                                       KEY_WRITE, NULL, &hkSessionInfo, NULL);

              if (lResult == ERROR_SUCCESS)
                {
                  DWORD dwDisposition;
                  HKEY hkStartup;
                  lResult = RegCreateKeyEx(hkSessionInfo,  TEXT("StartupHasBeenRun"),
                                           0, NULL, REG_OPTION_VOLATILE, KEY_WRITE, NULL,
                                           &hkStartup, &dwDisposition);

                  RegCloseKey(hkStartup);

                  if (dwDisposition == REG_CREATED_NEW_KEY)
                    result = true;

                  RegCloseKey(hkSessionInfo);
                }

              RegCloseKey(hkExplorer);
            }
        }

      CloseHandle(hToken);
    }

  return result;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	LoadSSO
// Required:	Nothing
// Returns:	Nothing
// Purpose:	Loads the 2K/XP system icons
//----  --------------------------------------------------------------------------------------------------------
void Shell::LoadSSO()
{
  HKEY key, subkey;
  int i = 0;
  WCHAR valueName[32];
  WCHAR data[40];
  DWORD valueSize;
  DWORD dataSize;
  CLSID clsid, trayclsid;
  IOleCommandTarget *target = NULL;

  valueSize = 32 * sizeof(WCHAR);
  dataSize = 40 * sizeof(WCHAR);
  i = 0;

  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellServiceObjects"),
                   0, KEY_READ, &key) == ERROR_SUCCESS)
    {
      while (RegEnumKeyEx(key, i, data, &dataSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
          if (RegOpenKeyEx(key, data, 0, KEY_READ, &subkey) == ERROR_SUCCESS)
            {
              if (RegQueryValueEx(subkey, TEXT("AutoStart"), NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
                {
                  CLSIDFromString(data, &clsid);
                  if (clsid != trayclsid)
                    {
                      target = ELStartSSO(clsid);
                      if (target)
                        ssoIconList.push_back(target);
                    }
                }

              RegCloseKey(subkey);
            }

          valueSize = 32 * sizeof(valueName[0]);
          dataSize = 40 * sizeof(data[0]);
          i++;
        }

      RegCloseKey(key);
    }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	UnloadSSO
// Required:	Nothing
// Returns:	Nothing
// Purpose:	Unload the 2K/XP system icons
//----  --------------------------------------------------------------------------------------------------------
void Shell::UnloadSSO()
{
  // Go through each element of the array and stop it...
  while (!ssoIconList.empty())
    {
      if (ssoIconList.back()->Exec(&CGID_ShellServiceObject, OLECMDID_SAVE,
                                   OLECMDEXECOPT_DODEFAULT, NULL, NULL) == S_OK)
        ssoIconList.back()->Release();
      ssoIconList.pop_back();
    }
}

