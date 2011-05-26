#include "emergeTrayExplorerHook.h"

LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (nCode < 0)
    return CallNextHookEx(NULL, nCode, wParam, lParam);

  CWPRETSTRUCT *cwpRetMsg = (CWPRETSTRUCT*)lParam;

  if ((!trayMsgHandler) && (cwpRetMsg->message != TRAYHOOK_MSGPROC_ATTACH))
    return CallNextHookEx(NULL, nCode, wParam, lParam);

  if (cwpRetMsg->message == TRAYHOOK_MSGPROC_ATTACH)
    {
      trayMsgHandler = (HWND)cwpRetMsg->lParam;
    }
  else if (cwpRetMsg->message == WM_COPYDATA)
    {
      //WM_COPYDATA is a special message. Windows moves the data referenced by
      //lParam from the sending process to the receiving process, but the data
      //cannot be accessed outside those processes.  This means we can't just
      //directly pass any WM_COPYDATA messages we receive to our message
      //handler; we have to recreate the data within our process space.  I did
      //try directly passing WM_COPYDATA messages, but emergeTray crashed,
      //probably from memory access errors. This works much better.
      PCOPYDATASTRUCT receivedData = (PCOPYDATASTRUCT)cwpRetMsg->lParam;
      DWORD dataSize = receivedData->cbData;
      void * dataPtr = malloc(dataSize);
      if (dataPtr)
        {
          memcpy(dataPtr, receivedData->lpData, dataSize);

          COPYDATASTRUCT sendingData;
          sendingData.dwData = receivedData->dwData;
          sendingData.cbData = dataSize;
          sendingData.lpData = dataPtr;
          SendMessage(trayMsgHandler, cwpRetMsg->message, cwpRetMsg->wParam,
                      (LPARAM)(LPVOID)&sendingData);
          if ((GetLastError() == ERROR_INVALID_HANDLE) ||
              (GetLastError() == ERROR_INVALID_WINDOW_HANDLE))
            trayMsgHandler = NULL; //trayMsgHandler is an invalid handle (the
                                   //window probably closed), so clear it; we
                                   //don't want to keep sending messages to
                                   //nothing
          free(dataPtr);
        }
    }

  return CallNextHookEx(NULL, nCode, wParam, lParam);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL UNUSED, DWORD fdwReason,
                    LPVOID lpvReserved UNUSED)
{
  switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
      hInst = hinstDLL;
      break;
    }
    }

  return true;
}
