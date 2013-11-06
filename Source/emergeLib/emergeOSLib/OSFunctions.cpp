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

#include "OSFunctions.h"

HMODULE ELGetSystemLibrary(std::wstring library)
{
  WCHAR libraryPath[MAX_PATH];

  if (GetSystemDirectory(libraryPath, MAX_PATH) == 0)
  {
    return NULL;
  }

  wcscat(libraryPath, TEXT("\\"));
  wcscat(libraryPath, library.c_str());

  return GetModuleHandle(libraryPath);
}

HMODULE ELGetEmergeLibrary(std::wstring library)
{
  std::wstring libraryPath;

  libraryPath = ELGetCurrentPath();

  if (libraryPath.empty())
  {
    return NULL;
  }

  libraryPath = libraryPath + TEXT("\\");
  libraryPath = libraryPath + library;

  return GetModuleHandle(libraryPath.c_str());
}

HMODULE ELLoadSystemLibrary(std::wstring library)
{
  WCHAR libraryPath[MAX_PATH];

  if (GetSystemDirectory(libraryPath, MAX_PATH) == 0)
  {
    return NULL;
  }

  wcscat(libraryPath, TEXT("\\"));
  wcscat(libraryPath, library.c_str());

  return LoadLibrary(libraryPath);
}

HMODULE ELLoadEmergeLibrary(std::wstring library)
{
  std::wstring libraryPath;

  libraryPath = ELGetCurrentPath();
  if (libraryPath.empty())
  {
    return NULL;
  }

  libraryPath = libraryPath + TEXT("\\");
  libraryPath = libraryPath + library;

  return LoadLibrary(libraryPath.c_str());
}

void* ELLockShared(HANDLE sharedMem, DWORD processID)
{
  if (emergeLibGlobals::getShlwapiDLL())
  {
    if (MSSHLockShared == NULL)
    {
      MSSHLockShared = (fnSHLockShared)GetProcAddress(emergeLibGlobals::getShlwapiDLL(), (LPCSTR)8);
    }
  }

  if (MSSHLockShared && sharedMem)
  {
    return MSSHLockShared(sharedMem, processID);
  }

  return NULL;
}

bool ELUnlockShared(void* sharedPtr)
{
  if (emergeLibGlobals::getShlwapiDLL())
  {
    if (MSSHUnlockShared == NULL)
    {
      MSSHUnlockShared = (fnSHUnlockShared)GetProcAddress(emergeLibGlobals::getShlwapiDLL(), (LPCSTR)9);
    }
  }

  if (MSSHUnlockShared && sharedPtr)
  {
    return (MSSHUnlockShared(sharedPtr) == TRUE);
  }

  return false;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELDisplayRunDialog
// Requires:	Nothing
// Returns:	bool
// Purpose:	Displays the MS Run dialog
//----  --------------------------------------------------------------------------------------------------------
bool ELDisplayRunDialog()
{
  if ((emergeLibGlobals::getShell32DLL()) && (MSRun == NULL))
  {
    MSRun = (lpfnMSRun)GetProcAddress(emergeLibGlobals::getShell32DLL(), (LPCSTR) 61);
  }
  if (MSRun)
  {
    MSRun(NULL, NULL, NULL, NULL, NULL, 0);
    return true;
  }

  return false;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELQuit
// Requires:	Nothing
// Returns:	bool
// Purpose:	Displays the Emerge Desktop Quit dialog
//----  --------------------------------------------------------------------------------------------------------
bool ELQuit(bool prompt)
{
  UINT response = IDYES;

  if (prompt)
    response = ELMessageBox(GetDesktopWindow(),
                            (WCHAR*)TEXT("Do you want to quit Emerge Desktop?"),
                            (WCHAR*)TEXT("Emerge Desktop"),
                            ELMB_YESNO | ELMB_ICONQUESTION | ELMB_MODAL);

  if (response == IDYES)
  {
    ELDispatchCoreMessage(EMERGE_CORE, CORE_QUIT);
    return true;
  }

  return false;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELShutdown
// Requires:	HWND wnd - calling applications window
// Returns:		bool
// Purpose:		Displays the MS Shutdown dialog
//----  --------------------------------------------------------------------------------------------------------
bool ELDisplayShutdownDialog(HWND wnd)
{
  Shutdown shutdown(emergeLibGlobals::getEmergeLibInstance(), wnd);
  shutdown.Show();

  return false;
}

/*!
  @fn ELExit(UINT uFlag, bool prompt)
  @brief Alternate shutdown method
  @param uFlag shutdown method.  Valid flags include: EWX_FORCE, EWX_FORCEIFHUNG, EMERGE_LOGOFF, EMERGE_HALT,
  EMERGE_REBOOT, EMERGE_SUSPEND, EMERGE_HIBERNATE
  @param prompt if true, display dialogue, if false, no dialogue
  @return true if successful
  */

bool ELExit(EXITFLAGS uFlag, bool prompt)
{
  HANDLE hToken;
  TOKEN_PRIVILEGES tkp;
  WCHAR messageText[MAX_LINE_LENGTH], method[20];
  bool exitStatus = true;
  UINT response = IDYES;
  UINT mode = EWX_FORCE;
  bool elevate = false;
  UINT reason = 0;
#ifndef _W64
  LPTSTR pData = NULL;
  DWORD cbReturned = 0;
#endif

  switch (uFlag)
  {
  case EMERGE_LOGOFF:
    mode |= EWX_LOGOFF;
    wcscpy(method, TEXT("Logoff"));
    break;
  case EMERGE_REBOOT:
    mode |= EWX_REBOOT;
    elevate = true;
    wcscpy(method, TEXT("Reboot"));
    break;
  case EMERGE_HALT:
    mode |= EWX_POWEROFF;
    elevate = true;
    wcscpy(method, TEXT("Halt"));
    break;
  case EMERGE_SUSPEND:
    elevate = true;
    wcscpy(method, TEXT("Suspend"));
    break;
  case EMERGE_HIBERNATE:
    elevate = true;
    wcscpy(method, TEXT("Hibernate"));
    break;
  case EMERGE_DISCONNECT:
    wcscpy(method, TEXT("Disconnect"));
    break;
  default:
    return false;
  }

  if (elevate)
  {
    reason = SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED;

    /* Obtain the privileges necessary to shutdown the computer */
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
      return false;
    }

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);

    if (GetLastError() != ERROR_SUCCESS)
    {
      swprintf(messageText,
               TEXT("You do not have persission to perform a %ls\n on this system."),
               method);
      ELMessageBox(GetDesktopWindow(), messageText, (WCHAR*)TEXT("Emerge Desktop"),
                   ELMB_OK | ELMB_ICONERROR | ELMB_MODAL);
      return false;
    }
  }

  if (prompt)
  {
    swprintf(messageText, TEXT("Are you sure you want to %ls?"), method);
    response = ELMessageBox(GetDesktopWindow(),
                            messageText,
                            (WCHAR*)TEXT("Emerge Desktop"),
                            ELMB_ICONQUESTION | ELMB_YESNO | ELMB_MODAL);
  }

  if (response == IDYES)
  {
    switch (uFlag)
    {
    case EMERGE_SUSPEND:
      if (!SetSuspendState(FALSE, FALSE, FALSE))
      {
        exitStatus = false;
      }
      break;
    case EMERGE_HIBERNATE:
      if (!SetSuspendState(TRUE, FALSE, FALSE))
      {
        exitStatus = false;
      }
      break;
#ifndef _W64
    case EMERGE_DISCONNECT:
      if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                                     WTS_CURRENT_SESSION, WTSConnectState,
                                     &pData, &cbReturned)
          && cbReturned == sizeof(int))
      {
        if (*((int*)pData) == WTSActive)

          if (!WTSDisconnectSession(WTS_CURRENT_SERVER_HANDLE,
                                    WTS_CURRENT_SESSION,
                                    FALSE))
          {
            exitStatus = false;
          }
      }
      break;
#endif
    default:
      if (!ExitWindowsEx(mode, reason))
      {
        exitStatus = false;
      }
    }
  }

  if (elevate)
  {
    /* Release the privileges necessary to shutdown the computer */
    tkp.Privileges[0].Attributes = 0;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);
    CloseHandle(hToken);
  }

  if (!exitStatus)
  {
    swprintf(messageText, TEXT("Failed to %ls."), method);
    ELMessageBox(GetDesktopWindow(), messageText, (WCHAR*)TEXT("Emerge Desktop"),
                 ELMB_ICONERROR | ELMB_OK | ELMB_MODAL);
    return false;
  }

  return true;
}

HANDLE ELActivateActCtxForDll(LPCTSTR pszDll, PULONG_PTR pulCookie)
{
  HANDLE hContext = INVALID_HANDLE_VALUE;

  typedef HANDLE (WINAPI * CreateActCtx_t)(PACTCTX pCtx);
  typedef BOOL (WINAPI * ActivateActCtx_t)(HANDLE hCtx, ULONG_PTR * pCookie);

  CreateActCtx_t fnCreateActCtx = (CreateActCtx_t)
                                  GetProcAddress(emergeLibGlobals::getKernel32DLL(), "CreateActCtxW");

  ActivateActCtx_t fnActivateActCtx = (ActivateActCtx_t)
                                      GetProcAddress(emergeLibGlobals::getKernel32DLL(), "ActivateActCtx");

  if (fnCreateActCtx != NULL && fnActivateActCtx != NULL)
  {
    ACTCTX act;
    ZeroMemory(&act, sizeof(act));
    act.cbSize = sizeof(act);
    act.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
    act.lpSource = pszDll;
    act.lpResourceName = MAKEINTRESOURCE(123);

    hContext = fnCreateActCtx(&act);

    if (hContext != INVALID_HANDLE_VALUE)
    {
      if (!fnActivateActCtx(hContext, pulCookie))
      {
        ELDeactivateActCtx(hContext, NULL);
        hContext = INVALID_HANDLE_VALUE;
      }
    }
  }

  return hContext;
}

HANDLE ELActivateActCtxForClsid(REFCLSID rclsid, PULONG_PTR pulCookie)
{
  HANDLE hContext = INVALID_HANDLE_VALUE;
  TCHAR szCLSID[39] = { 0 };

  //
  // Get the DLL that implements the COM object in question
  //
  if (SUCCEEDED(CLSIDToString(rclsid, szCLSID, 39)))
  {
    TCHAR szSubkey[MAX_PATH] = { 0 };

    HRESULT hr = wsprintf(szSubkey, TEXT("CLSID\\%ls\\InProcServer32"), szCLSID);

    if (SUCCEEDED(hr))
    {
      TCHAR szDll[MAX_PATH] = { 0 };
      DWORD cbDll = sizeof(szDll);

      LONG lres = SHGetValue(
                    HKEY_CLASSES_ROOT, szSubkey, NULL, NULL, szDll, &cbDll);

      if (lres == ERROR_SUCCESS)
      {
        //
        // Activate the custom manifest (if any) of that DLL
        //
        hContext = ELActivateActCtxForDll(szDll, pulCookie);
      }
    }
  }

  return hContext;
}

void ELDeactivateActCtx(HANDLE hActCtx, ULONG_PTR* pulCookie)
{
  typedef BOOL (WINAPI * DeactivateActCtx_t)(DWORD dwFlags, ULONG_PTR ulc);
  typedef void (WINAPI * ReleaseActCtx_t)(HANDLE hActCtx);

  DeactivateActCtx_t fnDeactivateActCtx = (DeactivateActCtx_t)
                                          GetProcAddress(emergeLibGlobals::getKernel32DLL(), "DeactivateActCtx");

  ReleaseActCtx_t fnReleaseActCtx = (ReleaseActCtx_t)
                                    GetProcAddress(emergeLibGlobals::getKernel32DLL(), "ReleaseActCtx");

  if (fnDeactivateActCtx != NULL && fnReleaseActCtx != NULL)
  {
    if (hActCtx != INVALID_HANDLE_VALUE)
    {
      if (pulCookie != NULL)
      {
        fnDeactivateActCtx(0, *pulCookie);
      }

      fnReleaseActCtx(hActCtx);
    }
  }
}

IOleCommandTarget* ELStartSSO(CLSID clsid)
{
  LPVOID lpVoid;
  IOleCommandTarget* target = NULL;

  // The SSO might have a custom manifest.
  // Activate it before loading the object.
  //ULONG_PTR ulCookie;
  //HANDLE hContext = ELActivateActCtxForClsid(clsid, &ulCookie);

  if (SUCCEEDED(CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER, IID_IOleCommandTarget,
                                 &lpVoid)))
  {
    // Start ShellServiceObject
    reinterpret_cast <IOleCommandTarget*> (lpVoid)->Exec(&CGID_ShellServiceObject,
        OLECMDID_NEW,
        OLECMDEXECOPT_DODEFAULT,
        NULL, NULL);
    target = reinterpret_cast <IOleCommandTarget*>(lpVoid);
  }

  //if (hContext != INVALID_HANDLE_VALUE)
  //  ELDeactivateActCtx(hContext, &ulCookie);

  return target;
}

bool ELRegisterShellHook(HWND hwnd, RSHFLAGS method)
{
  HWND shellWnd = NULL;
  MINIMIZEDMETRICS minMetrics;
  bool result = false;

  if ((emergeLibGlobals::getShell32DLL()) && (MSRegisterShellHookWindow == NULL))
  {
    MSRegisterShellHookWindow = (lpfnMSRegisterShellHookWindow)GetProcAddress(emergeLibGlobals::getShell32DLL(), (LPSTR)((long)0xB5));
  }
  if (MSRegisterShellHookWindow)
  {
    if (method == RSH_TASKMGR)
    {
      shellWnd = hwnd;

      // Hide minimized windows
      ZeroMemory(&minMetrics, sizeof(MINIMIZEDMETRICS));
      minMetrics.cbSize = sizeof(MINIMIZEDMETRICS);
      SystemParametersInfo(SPI_GETMINIMIZEDMETRICS, sizeof(MINIMIZEDMETRICS), &minMetrics,
                           0);
      minMetrics.iArrange |= ARW_HIDE;
      SystemParametersInfo(SPI_SETMINIMIZEDMETRICS, sizeof(MINIMIZEDMETRICS), &minMetrics,
                           SPIF_SENDCHANGE);
    }

    result = (MSRegisterShellHookWindow(shellWnd, method) == TRUE);
  }

  return result;
}

bool ELIsKeyDown(UINT virtualKey)
{
  return ((GetAsyncKeyState(virtualKey) & 0x8000) == 0x8000);
}

HRESULT CLSIDToString(REFCLSID rclsid, LPTSTR ptzBuffer, size_t cchBuffer)
{
  LPOLESTR pOleString = NULL;

  HRESULT hr = ProgIDFromCLSID(rclsid, &pOleString);

  if (FAILED(hr))
  {
    hr = StringFromCLSID(rclsid, &pOleString);
  }

  if (SUCCEEDED(hr) && pOleString)
  {
    wcsncpy(ptzBuffer, pOleString, cchBuffer);
  }

  CoTaskMemFree(pOleString);

  return hr;
}
