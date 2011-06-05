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

// Function pointers
void (WINAPI *lpShellDDEInit)(bool) = NULL;
int (WINAPI *lpWinList_Init)() = NULL;
int (WINAPI *lpWinList_Terminate)() = NULL;
void (WINAPI *lpRunInstallUninstallStubs)(int) = NULL;
bool (WINAPI *lpFileIconInit)(BOOL) = NULL;

// Global Variables
HMODULE shdocvmDLL = NULL;
HMODULE explorerFrameDLL = NULL;
HMODULE shell32DLL = NULL;

WCHAR explorerClass[ ] = TEXT("EmergeDesktopExplorer");

Applet::Applet(HINSTANCE hInstance)
{
  mainInst = hInstance;
  mainWnd = NULL;
  m_hThread = NULL;
}

UINT Applet::Initialize(bool showDesktop)
{
  WNDCLASSEX wincl;
  ZeroMemory(&wincl, sizeof(WNDCLASSEX));

  if (FAILED(OleInitialize(NULL)))
    {
      ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("COM initialization failed"), (WCHAR*)TEXT("Explorer"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      return 0;
    }

  // Register the window class
  wincl.hInstance = mainInst;
  wincl.lpszClassName = explorerClass;
  wincl.lpfnWndProc = WindowProcedure;
  wincl.style = CS_DBLCLKS;
  wincl.cbSize = sizeof (WNDCLASSEX);
  wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hCursor = LoadCursor (NULL, IDC_ARROW);

  // Register the window class, and if it fails quit the program
  if (!RegisterClassEx (&wincl))
    return 0;

  // The class is registered, let's create the window
  mainWnd = CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE, explorerClass, NULL, WS_POPUP,
                           0, 0, 0, 0, NULL, NULL, mainInst, reinterpret_cast<LPVOID>(this));

  // If the window failed to get created, unregister the class and quit the program
  if (!mainWnd)
    return 0;

  ShowWindow(mainWnd, SW_SHOW);

  PostMessage(ELGetCoreWindow(), EMERGE_REGISTER, (WPARAM)mainWnd, (LPARAM)EMERGE_CORE);

  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
  SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

  // Start the shell functions
  ShellServicesInit();

  // Create Desktop Thread if showDesktop specified
  if (showDesktop)
    m_hThread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, &m_dwThreadID);

  return 1;
}

Applet::~Applet()
{
  // Send quit message to SHDesktopMessageLoop
  if(m_dwThreadID)
    PostThreadMessage(m_dwThreadID, WM_QUIT, 0, 0);

  // Terminate thread
  if (m_hThread)
    {
      if (WaitForSingleObject(m_hThread, 1000) != WAIT_OBJECT_0)
        TerminateThread(m_hThread, 0);

      CloseHandle(m_hThread);
      m_hThread = 0;
    }

  ShellServicesTerminate();

  OleUninitialize();

  PostMessage(ELGetCoreWindow(), EMERGE_UNREGISTER, (WPARAM)mainWnd, (LPARAM)EMERGE_CORE);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	WindowProcedure
// Required:	HWND hwnd - window handle that message was sent to
// 		UINT message - action to handle
// 		WPARAM wParam - dependant on message
// 		LPARAM lParam - dependant on message
// Returns:	LRESULT
// Purpose:	Handles messages sent from DispatchMessage
//----  --------------------------------------------------------------------------------------------------------
LRESULT CALLBACK Applet::WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  CREATESTRUCT *cs;
  static Applet *pApplet = NULL;

  if (message == WM_CREATE)
    {
      cs = (CREATESTRUCT*)lParam;
      pApplet = reinterpret_cast<Applet*>(cs->lpCreateParams);
      return DefWindowProc(hwnd, message, wParam, lParam);
    }

  if (pApplet == NULL)
    return DefWindowProc(hwnd, message, wParam, lParam);

  switch (message)
    {
      // Send a quit message when the window is destroyed
    case WM_DESTROY:
    case WM_NCDESTROY:
      PostQuitMessage (0);
      break;

    case WM_SYSCOMMAND:
      switch (wParam)
        {
        case SC_CLOSE:
        case SC_MAXIMIZE:
        case SC_MINIMIZE:
          break;
        default:
          return DefWindowProc(hwnd, message, wParam, lParam);
        }
      break;

      // If not handled just forward the message on to MessageControl
    default:
      return DefWindowProc (hwnd, message, wParam, lParam);
    }

  return 0;
}

DWORD WINAPI Applet::ThreadFunc(LPVOID pvParam UNUSED)
{
  LPVOID lpVoid;
  DWORD registerCookie;
  TShellDesktopTrayFactory *explorerFactory = new TShellDesktopTrayFactory();
  TShellDesktopTray *explorerTray = NULL;
  HMODULE shell32DLL = ELLoadSystemLibrary(TEXT("shell32.dll"));
  if (!shell32DLL)
	  return 1;

  // Initialize COM for this thread
  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

  // Register the IShellDesktopTray COM Object
  if (FAILED(CoRegisterClassObject(IID_IShellDesktopTray,
                                   LPUNKNOWN(explorerFactory),
                                   CLSCTX_LOCAL_SERVER,
                                   REGCLS_MULTIPLEUSE,
                                   &registerCookie)))
    return 1;

  // Create the ShellDesktopTray interface
  explorerTray = new TShellDesktopTray();
  explorerTray->QueryInterface(IID_IShellDesktopTray, &lpVoid);
  IShellDesktopTray *iTray = reinterpret_cast <IShellDesktopTray*> (lpVoid);

  SHCREATEDESKTOP SHCreateDesktop = (SHCREATEDESKTOP)GetProcAddress(shell32DLL, (LPCSTR)200);
  SHDESKTOPMESSAGELOOP SHDesktopMessageLoop = (SHDESKTOPMESSAGELOOP)GetProcAddress(shell32DLL, (LPCSTR)201);

  if (SHCreateDesktop && SHDesktopMessageLoop)
    {
      // Create the desktop
      HANDLE hDesktop = SHCreateDesktop(iTray);

      SendMessage(GetDesktopWindow(), 0x400, 0, 0);

      // Run the desktop message loop
      if (hDesktop)
        SHDesktopMessageLoop(hDesktop);
    }

  iTray->Release();
  explorerTray->Release();
  explorerFactory->Release();

  // Revoke the COM object
  CoRevokeClassObject(registerCookie);

  CoUninitialize();

  FreeLibrary(shell32DLL);

  return 0;
}

void Applet::ShellServicesInit()
{
  shdocvmDLL = ELLoadSystemLibrary(TEXT("shdocvw.dll"));
  explorerFrameDLL = ELLoadSystemLibrary(TEXT("ExplorerFrame.dll"));
  shell32DLL = ELLoadSystemLibrary(TEXT("shell32.dll"));

  if (shell32DLL)
    {
      lpShellDDEInit = (void (__stdcall *) (bool))GetProcAddress(shell32DLL, (LPCSTR)188);
      lpFileIconInit = (bool (__stdcall *) (BOOL))GetProcAddress(shell32DLL, (LPCSTR)660);
      lpRunInstallUninstallStubs = (void (WINAPI *)(int))GetProcAddress(shell32DLL, (LPCSTR)885);
    }

  if (explorerFrameDLL)
    lpWinList_Init = (int (WINAPI *) (void))GetProcAddress(explorerFrameDLL, (LPCSTR)110);
  else if (shdocvmDLL)
    lpWinList_Init = (int (WINAPI *) (void))GetProcAddress(shdocvmDLL, (LPCSTR)110);

  if (shdocvmDLL && !lpRunInstallUninstallStubs)
    lpRunInstallUninstallStubs = (void (WINAPI *)(int))GetProcAddress(shdocvmDLL, (LPCSTR)130);

  // Create a mutex telling that this is the Explorer shell
  HANDLE hIsShell = CreateMutex(NULL, false, L"Local\\ExplorerIsShellMutex");
  WaitForSingleObject(hIsShell, INFINITE);

  if (lpShellDDEInit)
    lpShellDDEInit(true);

  SetProcessShutdownParameters(3, 0);

  MSG msg;
  PeekMessageW(&msg, 0, WM_QUIT, WM_QUIT, false);

  // Wait for Scm to be created
  HANDLE hGScmEvent = OpenEvent(0x100002, false, L"Global\\ScmCreatedEvent");
  if (hGScmEvent == NULL)
    hGScmEvent = OpenEvent(0x100000, false, L"Global\\ScmCreatedEvent");
  if (hGScmEvent == NULL)
    hGScmEvent = CreateEvent(NULL, true, false, L"Global\\ScmCreatedEvent");

  if (hGScmEvent)
    {
      WaitForSingleObject(hGScmEvent, 6000);
      CloseHandle(hGScmEvent);
    }

  if (lpFileIconInit)
    lpFileIconInit(TRUE);

  if (lpWinList_Init)
    lpWinList_Init();

  // Event
  HANDLE CanRegisterEvent = CreateEvent(NULL, true, true, L"Local\\_fCanRegisterWithShellService");

  if (lpRunInstallUninstallStubs)
    lpRunInstallUninstallStubs(0);

  CloseHandle(CanRegisterEvent);
}

void Applet::ShellServicesTerminate()
{
  if (lpShellDDEInit)
    lpShellDDEInit(false);

  if (lpFileIconInit)
    lpFileIconInit(FALSE);

  if (explorerFrameDLL)
    lpWinList_Terminate = (int (WINAPI *) (void))GetProcAddress(explorerFrameDLL, (LPCSTR)111);
  else if (shdocvmDLL)
    lpWinList_Terminate = (int (WINAPI *) (void))GetProcAddress(shdocvmDLL, (LPCSTR)111);

  if (lpWinList_Terminate)
    lpWinList_Terminate();

  if (shdocvmDLL)
    FreeLibrary(shdocvmDLL);

  if (shell32DLL)
    FreeLibrary(shell32DLL);

  if (explorerFrameDLL)
    FreeLibrary(explorerFrameDLL);
}
