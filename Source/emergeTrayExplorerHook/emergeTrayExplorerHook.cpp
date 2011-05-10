#include "emergeTrayExplorerHook.h"

WCHAR szTrayName[ ] = TEXT("Shell_TrayWnd");
WCHAR szNotifyName[ ] = TEXT("TrayNotifyWnd");
WCHAR szReBarName[ ] = TEXT("ReBarWindow32");
WCHAR szClockName[ ] = TEXT("TrayClockWClass");
WCHAR szTaskSwName[ ] = TEXT("MSTaskSwWClass");

LRESULT CALLBACK messageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == TRAYHOOK_MSGPROC_ATTACH)
    {
      trayMsgHandler = (HWND)lParam;
      return 0;
    }

  if (trayMsgHandler)
    {
      if (uMsg == WM_COPYDATA)
        {
          //WM_COPYDATA is a special message. Windows moves the data referenced by lParam from the sending process to the receiving process, but the data cannot be accessed outside those processes.
          //This means we can't just directly pass any WM_COPYDATA messages we receive to our message handler; we have to recreate the data within our process space.
          //I did try directly passing WM_COPYDATA messages, but emergeTray crashed, probably from memory access errors. This works much better.
          PCOPYDATASTRUCT receivedData = (PCOPYDATASTRUCT)lParam;
          DWORD dataSize = receivedData->cbData;
          void * dataPtr = malloc(dataSize);
          memcpy(dataPtr, receivedData->lpData, dataSize);

          COPYDATASTRUCT sendingData;
          sendingData.dwData = receivedData->dwData;
          sendingData.cbData = dataSize;
          sendingData.lpData = dataPtr;
          SendMessage(trayMsgHandler, uMsg, wParam, (LPARAM)(LPVOID)&sendingData);
            {
              if ((GetLastError() == ERROR_INVALID_HANDLE) || (GetLastError() == ERROR_INVALID_WINDOW_HANDLE))
                trayMsgHandler = 0; //trayMsgHandler is an invalid handle (the window probably closed), so clear it; we don't want to keep sending messages to nothing
              return 0; //eat all messages headed for the Taskbar
            }
        }

      SendMessage(trayMsgHandler, uMsg, wParam, lParam);
        {
          if ((GetLastError() == ERROR_INVALID_HANDLE) || (GetLastError() == ERROR_INVALID_WINDOW_HANDLE))
            trayMsgHandler = 0; //trayMsgHandler is an invalid handle (the window probably closed), so clear it; we don't want to keep sending messages to nothing
          return 0; //eat all messages headed for the Taskbar
        }
    }

  return DefWindowProc(hwnd, uMsg, wParam, lParam); //if we don't have a message handler getting the messages, we might as well send them to Explorer
}

bool Install()
{
  HWND taskBarhWnd = FindWindow(szTrayName, NULL);

  if (taskBarhWnd)
    {
      oldTaskbarWndProc = SetWindowLongPtr(taskBarhWnd, GWLP_WNDPROC, (LONG_PTR)messageProc);

      HWND notifyhWnd = FindWindowEx(taskBarhWnd, NULL, szNotifyName, NULL);
      if (notifyhWnd)
        {
          oldNotifyWndProc = SetWindowLongPtr(notifyhWnd, GWLP_WNDPROC, (LONG_PTR)messageProc);

          HWND sysPagerhWnd = FindWindowEx(notifyhWnd, NULL, TEXT("SysPager"), NULL);
          if (sysPagerhWnd)
            {
              HWND trayhWnd = FindWindowEx(sysPagerhWnd, NULL, TEXT("ToolbarWindow32"), NULL);
              if (trayhWnd)
                oldTrayWndProc = SetWindowLongPtr(trayhWnd, GWLP_WNDPROC, (LONG_PTR)messageProc);
            }

          HWND clockhWnd = FindWindowEx(notifyhWnd, NULL, szClockName, NULL);
          if (clockhWnd)
            oldClockWndProc = SetWindowLongPtr(clockhWnd, GWLP_WNDPROC, (LONG_PTR)messageProc);
        }

      HWND rebarhWnd = FindWindowEx(taskBarhWnd, NULL, szReBarName, NULL);
      if (rebarhWnd)
        {
          oldRebarWndProc = SetWindowLongPtr(rebarhWnd, GWLP_WNDPROC, (LONG_PTR)messageProc);

          HWND taskhWnd = FindWindowEx(rebarhWnd, NULL, szTaskSwName, NULL);
          if (taskhWnd)
            oldTaskWndProc = SetWindowLongPtr(taskhWnd, GWLP_WNDPROC, (LONG_PTR)messageProc);
        }
    }

  return (oldTaskbarWndProc);
}

void Uninstall()
{
  HWND taskBarhWnd = FindWindow(szTrayName, NULL);

  if (taskBarhWnd)
    {
      SetWindowLongPtr(taskBarhWnd, GWLP_WNDPROC, oldTaskbarWndProc);

      HWND notifyhWnd = FindWindowEx(taskBarhWnd, NULL, szNotifyName, NULL);
      if (notifyhWnd)
        {
          SetWindowLongPtr(notifyhWnd, GWLP_WNDPROC, oldNotifyWndProc);

          HWND sysPagerhWnd = FindWindowEx(notifyhWnd, NULL, TEXT("SysPager"), NULL);
          if (sysPagerhWnd)
            {
              HWND trayhWnd = FindWindowEx(sysPagerhWnd, NULL, TEXT("ToolbarWindow32"), NULL);
              if (trayhWnd)
                SetWindowLongPtr(trayhWnd, GWLP_WNDPROC, oldTrayWndProc);
            }

          HWND clockhWnd = FindWindowEx(notifyhWnd, NULL, szClockName, NULL);
          if (clockhWnd)
            SetWindowLongPtr(clockhWnd, GWLP_WNDPROC, oldClockWndProc);
        }

      HWND rebarhWnd = FindWindowEx(taskBarhWnd, NULL, szReBarName, NULL);
      if (rebarhWnd)
        {
          SetWindowLongPtr(rebarhWnd, GWLP_WNDPROC, oldRebarWndProc);

          HWND taskhWnd = FindWindowEx(rebarhWnd, NULL, szTaskSwName, NULL);
          if (taskhWnd)
            SetWindowLongPtr(taskhWnd, GWLP_WNDPROC, oldTaskWndProc);
        }
    }
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL UNUSED, DWORD fdwReason, LPVOID lpvReserved UNUSED)
{
  switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        {
          hInst = hinstDLL;
          if (!Install())
            {
              MessageBox(NULL, TEXT("Hooking the system tray failed!"), TEXT("Error!"), MB_OK|MB_ICONERROR);
              //return false;
            }
          break;
        }
    case DLL_PROCESS_DETACH:
        {
          Uninstall();
          break;
        }
    }

  return true;
}
