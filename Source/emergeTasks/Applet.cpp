// vim:tags+=../emergeLib/tags,../emergeBaseClasses/tags
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

WCHAR myName[] = TEXT("emergeTasks");

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
  COPYDATASTRUCT *cpData;
  static Applet *pApplet = NULL;

  if (message == WM_CREATE)
    {
      pApplet = reinterpret_cast<Applet*>(((CREATESTRUCT*)lParam)->lpCreateParams);
      SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)pApplet);
      return DefWindowProc(hwnd, message, wParam, lParam);
    }
  else
    pApplet = reinterpret_cast<Applet*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

  if (pApplet == NULL)
    return DefWindowProc(hwnd, message, wParam, lParam);

  switch (message)
    {
    case WM_COPYDATA:
      cpData = (COPYDATASTRUCT *)lParam;
      if (cpData->dwData == EMERGE_MESSAGE)
        return pApplet->DoCopyData(cpData);
      break;

      // Needed to handle changing the system colors.  It forces
      // a repaint of the window as well as the frame.
    case WM_SYSCOLORCHANGE:
      return pApplet->DoSysColorChange();

      // Allow for window dragging via Ctrl - Left - Click dragging
    case WM_NCLBUTTONDOWN:
      pApplet->DoNCLButtonDown();
      return DefWindowProc(hwnd, message, wParam, lParam);

      // Display the mainMenu via Ctrl - Right - Click
    case WM_NCRBUTTONUP:
      return pApplet->DoNCRButtonUp();

      // Forward the appropriate clicks to the icons
    case WM_RBUTTONUP:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
      return pApplet->TaskMouseEvent(message, lParam);

      // Reset the cursor back to the standard arrow after dragging
    case WM_NCLBUTTONUP:
      pApplet->DoNCLButtonUp();
      return DefWindowProc(hwnd, message, wParam, lParam);

    case WM_SETCURSOR:
      pApplet->DoSetCursor();
      return DefWindowProc(hwnd, message, wParam, lParam);

      // Handles the resizing of the window
    case WM_NCHITTEST:
      return pApplet->DoHitTest(lParam);

      // Repaint the icons as the window size is changing
    case WM_WINDOWPOSCHANGING:
      return pApplet->DoWindowPosChanging((WINDOWPOS *)lParam);

    case WM_ENTERSIZEMOVE:
      return pApplet->DoEnterSizeMove(hwnd);

    case WM_EXITSIZEMOVE:
      return pApplet->DoExitSizeMove(hwnd);

    case WM_SIZING:
      return pApplet->DoSizing(hwnd, (UINT)wParam, (LPRECT)lParam);

    case WM_MOVING:
      return pApplet->DoMoving(hwnd, (LPRECT)lParam);

    case WM_DISPLAYCHANGE:
      return pApplet->DoDisplayChange(hwnd);

    case WM_SYSCOMMAND:
      return pApplet->DoSysCommand(hwnd, message, wParam, lParam);

    case WM_TIMER:
      return pApplet->DoTimer((UINT_PTR)wParam);

    case WM_DESTROY:
    case WM_NCDESTROY:
      PostQuitMessage(0);
      break;

      // If not handled just forward the message on
    default:
      return pApplet->DoDefault(hwnd, message, wParam, lParam);
    }

  return 0;
}

Applet::Applet(HINSTANCE hInstance)
  :BaseApplet(hInstance, myName, true)
{
  activeWnd = NULL;
}

Applet::~Applet()
{
  // Unregister the specified Emerge Desktop messages
  PostMessage(ELGetCoreWindow(), EMERGE_UNREGISTER, (WPARAM)mainWnd, (LPARAM)EMERGE_VWM);

  // Cleanup the icon vectors
  EnterCriticalSection(&vectorLock);
  while (!taskList.empty())
    {
      taskList.front()->DeleteTip(mainWnd, toolWnd);
      taskList.erase(taskList.begin());
    }
  LeaveCriticalSection(&vectorLock);

  DeleteCriticalSection(&vectorLock);
}

UINT Applet::Initialize()
{
  pSettings = std::tr1::shared_ptr<Settings>(new Settings());

  UINT ret = BaseApplet::Initialize(WindowProcedure, this, pSettings);
  if (ret == 0)
    return ret;

  // Initialize Settings, Config and AppletMenu classes
  //pSettings = reinterpret_cast<Settings*>(pBaseSettings.get());
  //pSettings = std::tr1::shared_ptr<Settings>(new Settings());

  // Set the window transparency
  UpdateGUI();

  // Create a critical section to control access to the taskList vector
  InitializeCriticalSection(&vectorLock);

  // Register the exiting tasks
  BuildTasksList();

  PostMessage(ELGetCoreWindow(), EMERGE_REGISTER, (WPARAM)mainWnd, (LPARAM)EMERGE_VWM);

  return ret;
}

std::tr1::shared_ptr<BaseSettings> Applet::createSettings()
{
  return std::tr1::shared_ptr<BaseSettings>(new Settings());
}

void Applet::UpdateIcons()
{
  TaskVector::iterator iter;

  EnterCriticalSection(&vectorLock);
  iter = taskList.begin();
  while (iter != taskList.end())
    {
      (*iter)->UpdateIcon();
      iter++;
    }
  LeaveCriticalSection(&vectorLock);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	PaintIcons
// Required:	HDC hdc - Device contect of calling window
// 		RECT r - client area of the calling window
// Returns:	LRESULT
// Purpose:	Paints the icons on the calling window
//----  --------------------------------------------------------------------------------------------------------
bool Applet::PaintItem(HDC hdc, UINT index, int x, int y, RECT rect)
{
  WCHAR windowTitle[MAX_LINE_LENGTH];
  ULONG_PTR response = 0;

  TaskVector::iterator iter = taskList.begin() + index;

  (*iter)->SetRect(rect);

  // Update the tooltip
  SendMessageTimeout((*iter)->GetWnd(), WM_GETTEXT, MAX_LINE_LENGTH, reinterpret_cast<LPARAM>(windowTitle), SMTO_ABORTIFHUNG, 100, &response);
  if (response != 0)
    (*iter)->UpdateTip(mainWnd, toolWnd, windowTitle);
  (*iter)->CreateNewIcon(guiInfo.alphaForeground, guiInfo.alphaBackground);

  if ((*iter)->GetVisible())
    {
      if (((*iter)->GetWnd() == activeWnd) && pSettings->GetHiliteActive())
        {
          InflateRect(&rect, 1, 1);
          EGFillRect(hdc, &rect, guiInfo.alphaSelected, guiInfo.colorSelected);
        }

      // Draw the icon
      DrawIconEx(hdc, x, y, (*iter)->GetIcon(), pSettings->GetIconSize(),
                 pSettings->GetIconSize(), 0, NULL, DI_NORMAL);
    }

  return true;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	AddTask
// Required:	HWND task - handle to the new task's window
// Returns:	LRESULT
// Purpose:	Adds a task to the tasks vector
//----  --------------------------------------------------------------------------------------------------------
LRESULT Applet::AddTask(HWND task)
{
  TaskVector::iterator iter = FindTask(task);
  HICON icon;
  RECT wndRect;
  UINT SWPFlags = SWP_NOZORDER | SWP_NOACTIVATE;

  if (iter != taskList.end())
    return 1;

  if (pSettings->GetIconSize() == 32)
    icon = EGGetWindowIcon(task, false, true);
  else
    icon = EGGetWindowIcon(task, true, true);

  TaskPtr taskPtr(new Task(task, icon, mainInst));
  EnterCriticalSection(&vectorLock);
  taskList.push_back(taskPtr);
  LeaveCriticalSection(&vectorLock);

  if (pSettings->GetAutoSize())
    {
      if (GetWindowRect(mainWnd, &wndRect))
        {
          AdjustRect(&wndRect);
          UpdateIcons();
          if ((GetVisibleIconCount() > 0) && !appletHidden)
            SWPFlags |= SWP_SHOWWINDOW;
          SetWindowPos(mainWnd, NULL, wndRect.left, wndRect.top,
                       wndRect.right - wndRect.left,
                       wndRect.bottom - wndRect.top,
                       SWPFlags);
        }
    }

  DrawAlphaBlend();

  return 0;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ModifyTask
// Required:	HWND task - handle to the new task's window
// Returns:	LRESULT
// Purpose:	Updates the specified task in the tasks vector
//----  --------------------------------------------------------------------------------------------------------
LRESULT Applet::ModifyTask(HWND task)
{
  TaskVector::iterator iter = FindTask(task);
  HICON icon;

  if (iter == taskList.end())
    return 1;

  if (pSettings->GetIconSize() == 32)
    icon = EGGetWindowIcon(task, false, false);
  else
    icon = EGGetWindowIcon(task, true, false);

  (*iter)->SetIcon(icon);
  DrawAlphaBlend();

  return 0;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	RemoveTask
// Required:	HWND task - handle to the task's window
// Returns:	LRESULT
// Purpose:	Removes a task from the tasks vector
//----  --------------------------------------------------------------------------------------------------------
LRESULT Applet::RemoveTask(HWND task)
{
  TaskVector::iterator iter = FindTask(task);
  RECT wndRect;
  UINT SWPFlags = SWP_NOZORDER | SWP_NOACTIVATE;

  if (iter == taskList.end())
    return 1;

  (*iter)->DeleteTip(mainWnd, toolWnd);
  EnterCriticalSection(&vectorLock);
  taskList.erase(iter);
  LeaveCriticalSection(&vectorLock);

  if (pSettings->GetAutoSize())
    {
      if (GetWindowRect(mainWnd, &wndRect))
        {
          AdjustRect(&wndRect);
          UpdateIcons();
          if (GetVisibleIconCount() == 0)
            SWPFlags |= SWP_HIDEWINDOW;
          SetWindowPos(mainWnd, NULL, wndRect.left, wndRect.top,
                       wndRect.right - wndRect.left,
                       wndRect.bottom - wndRect.top,
                       SWPFlags);
        }
    }

  DrawAlphaBlend();

  return 0;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	CleanTasks
// Required:	Nothing
// Returns:	bool
// Purpose:	Removes invalid icons from the valid icon vector
//----  --------------------------------------------------------------------------------------------------------
bool Applet::CleanTasks()
{
  RECT wndRect;
  bool refresh = false;
  TaskVector::iterator iter = taskList.begin();
  UINT SWPFlags = SWP_NOZORDER | SWP_NOACTIVATE;

  // Go through each of the elements in the trayIcons array
  while (iter != taskList.end())
    {
      // If the icon does not have a valid window handle, remove it
      if (!IsWindow((*iter)->GetWnd()))
        {
          (*iter)->DeleteTip(mainWnd, toolWnd);
          refresh = true;
          EnterCriticalSection(&vectorLock);
          taskList.erase(iter);
          // Restart from the begining of the taskList so that iter is in a
          // known state.
          iter = taskList.begin();
          LeaveCriticalSection(&vectorLock);
        }
      else
        iter++;
    }

  if (refresh && pSettings->GetAutoSize())
    {
      if (GetWindowRect(mainWnd, &wndRect))
        {
          AdjustRect(&wndRect);
          UpdateIcons();
          if (GetVisibleIconCount() == 0)
            SWPFlags |= SWP_HIDEWINDOW;
          SetWindowPos(mainWnd, NULL, wndRect.left, wndRect.top,
                       wndRect.right - wndRect.left,
                       wndRect.bottom - wndRect.top,
                       SWPFlags);
        }
    }

  return refresh;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	TaskMouseEvent
// Required:	UINT message - action to handle
// 		LPARAM lParam - location of the mouse
// Returns:	LRESULT
// Purpose:	Forwards the mouse message to the icon window
//----  --------------------------------------------------------------------------------------------------------
LRESULT Applet::TaskMouseEvent(UINT message, LPARAM lParam)
{
  TaskVector::iterator iter;
  HWND windowHandle;
  POINT pt;

  pt.x = LOWORD(lParam);
  pt.y = HIWORD(lParam);

  // Traverse the valid icon vector to see if the mouse is in the bounding rectangle
  // of the current icon
  iter = taskList.begin();
  while (iter != taskList.end())
    {
      if (PtInRect((*iter)->GetRect(), pt))
        {
          if ((message == WM_LBUTTONDOWN) || (message == WM_LBUTTONDBLCLK))
            {
              windowHandle = (*iter)->GetWnd();

              if (ELIsModal(windowHandle))
                ELSwitchToThisWindow(GetLastActivePopup(windowHandle));
              else
                {
                  if (windowHandle == activeWnd)
                    ShowWindow(windowHandle, SW_MINIMIZE);
                  else
                    {
                      if (IsIconic(windowHandle))
                        ShowWindow(windowHandle, SW_RESTORE);
                      else
                        SetForegroundWindow(windowHandle);
                    }
                }
            }
          else if (message == WM_RBUTTONUP)
            (*iter)->DisplayMenu(mainWnd);

          return 0;
        }
      iter++;
    }

  if (message == WM_LBUTTONDBLCLK)
    {
      ELExecute((WCHAR*)TEXT("taskmgr.exe"));
      return 0;
    }

  return 1;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	BuildTasksList
// Requires:	Nothing
// Returns:	Nothing
// Purpose:	Adds all existing windows to the task list
//----  --------------------------------------------------------------------------------------------------------
void Applet::BuildTasksList()
{
  // Enumerate existing tasks
  EnumWindows(EnumTasksList, (LPARAM)this);
}

void Applet::SetActive(HWND hwnd)
{
  activeWnd = hwnd;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	EnumTasksList
// Requires:	HWND hwnd - window handle
// 		LPARAM lParam - not used
// Returns:	BOOL
// Purpose:	Enumerates all the existing tasks
//----  --------------------------------------------------------------------------------------------------------
BOOL CALLBACK Applet::EnumTasksList(HWND hwnd, LPARAM lParam)
{
  static Applet *pApplet = reinterpret_cast<Applet*>(lParam);

  if (ELCheckWindow(hwnd))
    pApplet->AddTask(hwnd);

  return true;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	SetFlash
// Required:	HWND task - handle to the task's window
//		bool flash - tasks is flashing?
// Returns:	LRESULT
// Purpose:	Update the task flash state
//----  --------------------------------------------------------------------------------------------------------
LRESULT Applet::SetFlash(HWND task, bool flash)
{
  bool refresh = false;

  TaskVector::iterator iter = FindTask(task);
  std::map<HWND, UINT>::iterator mapIter;

  if ((iter == taskList.end()) || !pSettings->GetEnableFlash())
    return 1;

  if (flash && !(*iter)->GetFlash())
    {
      (*iter)->ToggleVisible();
      (*iter)->SetFlash(true);
      UINT timerID = flashMap.size() + 1001;
      flashMap.insert(std::pair<HWND, UINT>(task, timerID));
      SetTimer(mainWnd, timerID, pSettings->GetFlashInterval(), (TIMERPROC)FlashTimerProc);
      refresh = true;
    }
  else if (!flash && (*iter)->GetFlash())
    {
      mapIter = flashMap.find(task);
      if (mapIter != flashMap.end())
        {
          KillTimer(mainWnd, mapIter->second);
          (*iter)->SetFlash(false);
          (*iter)->SetFlashCount(0);

          if (!(*iter)->GetVisible())
            {
              (*iter)->ToggleVisible();
              refresh = true;
            }

          flashMap.erase(mapIter);
        }
    }

  if (refresh)
    DrawAlphaBlend();

  return 0;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	FindTask
// Required:	HWND task - handle to the task's window
// Returns:	taskItem
// Purpose:	Find the task in the taskList vector
//----  --------------------------------------------------------------------------------------------------------
TaskVector::iterator Applet::FindTask(HWND hwnd)
{
  TaskVector::iterator iter;

  // Go through each of the elements in the trayIcons array
  EnterCriticalSection(&vectorLock);
  iter = taskList.begin();
  while (iter != taskList.end())
    {
      if ((*iter)->GetWnd() == hwnd)
        break;
      iter++;
    }
  LeaveCriticalSection(&vectorLock);

  return iter;
}

LRESULT Applet::DoTimer(UINT_PTR timerID)
{
  if (timerID == MOUSE_TIMER)
    {
      CleanTasks();

      return BaseApplet::DoTimer(timerID);
    }

  return 1;
}

LRESULT Applet::DoSizing(HWND hwnd, UINT edge, LPRECT rect)
{
  UpdateIcons();
  return BaseApplet::DoSizing(hwnd, edge, rect);
}

LRESULT Applet::DoEmergeNotify(UINT messageClass, UINT message)
{
  if ((messageClass & EMERGE_VWM) == EMERGE_VWM)
    {
      activeWnd = NULL;
      DrawAlphaBlend();

      return 0;
    }

  return BaseApplet::DoEmergeNotify(messageClass, message);
}

void Applet::DoTaskModify(UINT id)
{
  std::map<HWND, UINT>::iterator mapIter;

  mapIter = modifyMap.begin();
  while (mapIter != modifyMap.end())
    {
      if (mapIter->second == id)
        {
          ModifyTask(mapIter->first);
          KillTimer(mainWnd, mapIter->second);
          modifyMap.erase(mapIter);

          return;
        }
      mapIter++;
    }
}

void Applet::DoTaskFlash(UINT id)
{
  std::map<HWND, UINT>::iterator mapIter;
  TaskVector::iterator iter;
  UINT flashCount = (UINT)pSettings->GetFlashCount();
  bool refresh = true;
  HWND foregroundWnd = GetForegroundWindow();

  mapIter = flashMap.begin();
  while (mapIter != flashMap.end())
    {
      if (mapIter->second == id)
        {
          iter = FindTask(mapIter->first);
          if (iter != taskList.end())
            {
              // If the task is the foreground window, clear flashing
              if ((*iter)->GetWnd() == foregroundWnd)
                {
                  SetFlash((*iter)->GetWnd(), false);
                  return;
                }

              if (flashCount != 0)
                {
                  if ((*iter)->GetFlashCount() < (flashCount * 2 - 1))
                    (*iter)->SetFlashCount((*iter)->GetFlashCount() + 1);
                  else
                    refresh = false;
                }

              if (refresh)
                {
                  (*iter)->ToggleVisible();
                  DrawAlphaBlend();
                }

              return;
            }
        }
      mapIter++;
    }
}

VOID CALLBACK Applet::ModifyTimerProc(HWND hwnd, UINT uMsg UNUSED, UINT_PTR idEvent, DWORD dwTime UNUSED)
{
  Applet *pApplet = reinterpret_cast<Applet*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

  pApplet->DoTaskModify(idEvent);
}

VOID CALLBACK Applet::FlashTimerProc(HWND hwnd, UINT uMsg UNUSED, UINT_PTR idEvent, DWORD dwTime UNUSED)
{
  Applet *pApplet = reinterpret_cast<Applet*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

  pApplet->DoTaskFlash(idEvent);
}

LRESULT Applet::DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  HWND task = (HWND)lParam;
  UINT shellMessage = (UINT)wParam;
  std::map<HWND, UINT>::iterator mapIter;
  UINT modifyIndex = modifyMap.size() + 1;
  std::pair<std::map<HWND,UINT>::iterator, bool> pr;

  if (message == ShellMessage)
    {
      switch (shellMessage)
        {
          //A new "task" was created
        case HSHELL_WINDOWCREATED:
          AddTask(task);
          break;

          // A "task" was modified
        case HSHELL_REDRAW:
          pr = modifyMap.insert(std::pair<HWND, UINT>(task, modifyIndex));
          if (pr.second)
            SetTimer(hwnd, modifyIndex, MODIFY_POLL_TIME, (TIMERPROC)ModifyTimerProc);
          break;

          // A "task" was ended
        case HSHELL_WINDOWDESTROYED:
          SetFlash(task, false);
          return RemoveTask(task);

          // A "task" was activated
        case HSHELL_RUDEAPPACTIVATED:
        case HSHELL_WINDOWACTIVATED:
          if ((task != mainWnd) && (task != activeWnd))
            {
              activeWnd = task;
              DrawAlphaBlend();
            }
          SetFlash(task, false);
          break;

          // A "task" was flashed
        case HSHELL_FLASH:
          SetFlash(task, true);
          break;
        }
    }

  if (message == EMERGE_NOTIFY)
    return DoEmergeNotify(shellMessage, (UINT)lParam);

  return BaseApplet::DoDefault(hwnd, message, wParam, lParam);
}

void Applet::ResetTaskIcons()
{
  TaskVector::iterator iter = taskList.begin();
  HICON icon;

  while (iter != taskList.end())
    {
      if (pSettings->GetIconSize() == 32)
        icon = EGGetWindowIcon((*iter)->GetWnd(), false, true);
      else
        icon = EGGetWindowIcon((*iter)->GetWnd(), true, true);

      (*iter)->SetIcon(icon);

      iter++;
    }
}

void Applet::AppletUpdate()
{
  UINT_PTR timerID;
  TaskVector::iterator iter;

  pSettings->ReadSettings();

  ResetTaskIcons();

  while (!flashMap.empty())
    {
      KillTimer(mainWnd, flashMap.begin()->second);
      flashMap.erase(flashMap.begin());
    }

  iter = taskList.begin();
  while (iter != taskList.end())
    {
      if ((*iter)->GetFlash())
        {
          timerID = flashMap.size() + 1001;

          flashMap.insert(std::pair<HWND, UINT>((*iter)->GetWnd(), timerID));
          SetTimer(mainWnd, timerID, pSettings->GetFlashInterval(), (TIMERPROC)FlashTimerProc);
        }

      (*iter)->UpdateIcon();
      iter++;
    }
}

void Applet::ShowConfig()
{
  Config config(mainInst, mainWnd, pSettings);
  config.Show();
  UpdateGUI();
}

size_t Applet::GetIconCount()
{
  return taskList.size();
}

