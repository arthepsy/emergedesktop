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
#include "Config.h"

WCHAR myName[ ] = TEXT("emergeVWM");

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
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDBLCLK:
      return pApplet->DesktopMouseEvent(hwnd, message, lParam);

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

      // Write the width height to the registry if the size changed
    case WM_SIZE:
      return pApplet->MySize();

    case WM_SYSCOMMAND:
      return pApplet->DoSysCommand(hwnd, message, wParam, lParam);

    case WM_TIMER:
      return pApplet->DoTimer((UINT)wParam);

    case WM_DISPLAYCHANGE:
      return pApplet->DoDisplayChange(hwnd);

    case WM_DESTROY:
    case WM_NCDESTROY:
      PostQuitMessage(0);
      break;

    case WM_NOTIFY:
      return pApplet->DoNotify(hwnd, lParam);

      // If not handled just forward the message on
    default:
      return pApplet->DoDefault(hwnd, message, wParam, lParam);
    }

  return 0;
}

LRESULT Applet::DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if (message == EMERGE_NOTIFY)
    {
      if (DoEmergeNotify((UINT)wParam, (UINT)lParam) == 0)
        return 0;
    }

  return BaseApplet::DoDefault(hwnd, message, wParam, lParam);
}

Applet::Applet(HINSTANCE hInstance)
  :BaseApplet(hInstance, myName, false)
{
  // Set current row and column
  currentRow = 0;
  currentColumn = 0;

  // Set old active window
  oldActiveWindow = NULL;
  selectedWindow = NULL;
  oldTipWindow = NULL;

  lButtonDown = false;
}

Applet::~Applet()
{
  PostMessage(ELGetCoreWindow(), EMERGE_UNREGISTER, (WPARAM)mainWnd, (LPARAM)EMERGE_VWM);

  // Kill Task timer
  KillTimer(mainWnd, NEW_TASK_TIMER);

  // Gather all the windows onto one desktop
  SwitchDesktop(0, 0, true);

  // Cleanup the icon vectors
  taskList.clear();

  // Remove the tooltip region
  SendMessage(toolWnd, TTM_DELTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  // Cleanup the icon vectors
  if (!taskList.empty())
    taskList.clear();
}

GUIINFO Applet::GetGUIInfo()
{
  return guiInfo;
}

UINT Applet::Initialize()
{
  pSettings = std::tr1::shared_ptr<Settings>(new Settings());
  UINT ret = BaseApplet::Initialize(WindowProcedure, this, pSettings);
  if (ret == 0)
    return ret;

  // Set the window transparency
  UpdateGUI();

  // Build the sticky list only after reading the settings (in UpdateGUI())
  pSettings->BuildStickyList();

  // Register to recieve the specified Emerge Desktop messages
  PostMessage(ELGetCoreWindow(), EMERGE_REGISTER, (WPARAM)mainWnd, (LPARAM)EMERGE_VWM);

  // Gather any stranded Windows
  EnumWindows(BuildTasksList, (LPARAM)0);
  SwitchDesktop(0, 0, true);

  // Check to see if there are any new tasks and remove any defunt tasks
  SetTimer(mainWnd, NEW_TASK_TIMER, NEW_TASK_POLL_TIME, NULL);

  return ret;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	BuildTasksList
// Requires:	HWND hwnd - window handle
// 		LPARAM lParam - not used
// Returns:	BOOL
// Purpose:	Enumerates all the existing tasks
//----  --------------------------------------------------------------------------------------------------------
BOOL CALLBACK Applet::BuildTasksList(HWND hwnd, LPARAM lParam)
{
  Applet *pApplet = reinterpret_cast<Applet*>(lParam);
  if (pApplet)
    return pApplet->AddTasks(hwnd);

  return FALSE;
}

bool Applet::AddTasks(HWND hwnd)
{
  std::vector< std::tr1::shared_ptr<Task> >::iterator iter;
  bool found = false;

  if (!IsWindowValidTask(hwnd))
    return true;

  iter = taskList.begin();

  while (iter != taskList.end())
    {
      if ((*iter)->GetTaskWnd() == hwnd)
        {
          if (((*iter)->GetTaskWnd() == GetForegroundWindow()) &&
              ((*iter)->GetTaskWnd() != oldActiveWindow))
            {
              int row, column;

              if ((*iter)->GetMinimized())
                {
                  (*iter)->SetMinimized(false);
                  MakeCurrent((*iter)->GetTaskWnd());
                }
              else if (GetTaskRowColumn((*iter)->GetTaskWnd(), &row, &column))
                {
                  if ((row != currentRow) || (column != currentColumn))
                    SwitchDesktop(row, column, false);
                }

              oldActiveWindow = (*iter)->GetTaskWnd();

              taskList.erase(iter);
            }
          else if (IsIconic((*iter)->GetTaskWnd()))
            {
              (*iter)->SetMinimized(true);
              found = true;
            }
          else
            {
              (*iter)->UpdateDimensions(currentColumn, currentRow, pSettings->GetDesktopColumns(),
                                        pSettings->GetDesktopRows(), guiInfo);
              found = true;
            }

          break;
        }

      iter++;
    }

  if (!found)
    {
      Task *task = new Task(hwnd, mainWnd, mainInst, currentRow, currentColumn,
                            pSettings->GetDesktopRows(), pSettings->GetDesktopColumns(), guiInfo);
      taskList.push_back(std::tr1::shared_ptr<Task>(task));
    }

  return true;
}

void Applet::MakeCurrent(HWND taskWnd)
{
  int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  int screenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
  int screenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);
  int taskRow, taskColumn;
  RECT r;

  if (GetTaskRowColumn(taskWnd, &taskRow, &taskColumn))
    {
      if ((taskRow == currentRow) && (taskColumn == currentColumn))
        return;
    }

  GetWindowRect(taskWnd, &r);

  while (r.top < screenTop)
    r.top += screenHeight;
  while (r.top > (screenTop + screenHeight))
    r.top -= screenHeight;

  if (IsZoomed(taskWnd))
    r.top -= screenHeight;

  while (r.left < screenLeft)
    r.left += screenWidth;
  while (r.left > (screenLeft + screenWidth))
    r.left -= screenWidth;

  if (IsZoomed(taskWnd))
    r.left -= screenWidth;

  SetWindowPos(taskWnd, NULL, r.left, r.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	IsWindowValidTask
// Required:	HWND task - handle to the task's window
// Returns:	bool
// Purpose:	Check to see if the task is valid
//----  --------------------------------------------------------------------------------------------------------
bool Applet::IsWindowValidTask(HWND hwnd)
{
  WCHAR windowClass[MAX_LINE_LENGTH];
  RealGetWindowClass(hwnd, windowClass, MAX_LINE_LENGTH);

  // Ignore hidden windows
  if (!IsWindowVisible(hwnd))
    return false;

  if (ELIsApplet(hwnd))
    return false;

  // Ignore toolwindows
  /*  if((GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) == WS_EX_TOOLWINDOW)
      return false;*/

  // Ignore menus
  if (_wcsicmp(windowClass, TEXT("#32768")) == 0)
    return false;

  // Ignore tooltips
  if (_wcsicmp(windowClass, TOOLTIPS_CLASS) == 0)
    return false;

  // Ignore emergeDesktop window
  /*  if (hwnd == ELGetDesktopWindow())
      return false;*/

  return true;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	SwitchDesktop
// Required:	UINT row - target row
// 		UINT column - target column
// 		bool gather - gather windows?
// Returns:	Nothing
// Purpose:	Switches to target desktop
//----  --------------------------------------------------------------------------------------------------------
void Applet::SwitchDesktop(int row, int column, bool gather)
{
  if (((row > (pSettings->GetDesktopRows() - 1)) || (column > (pSettings->GetDesktopColumns() - 1)) ||
       (row == currentRow && column == currentColumn)) && !gather)
    return;

  HDWP dwp;
  std::vector< std::tr1::shared_ptr<Task> >::iterator iter;
  RECT r;
  int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  int screenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
  int screenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);
  int rowOffset, columnOffset, maxColumnShift, maxRowShift;

  maxColumnShift = pSettings->GetDesktopColumns() * screenWidth;
  maxRowShift = pSettings->GetDesktopRows() * screenHeight;

  rowOffset = row - currentRow;
  rowOffset = rowOffset * (screenHeight + 10);
  columnOffset = column - currentColumn;
  columnOffset = columnOffset * (screenWidth + 10);

  currentRow = row;
  currentColumn = column;

  if (!gather)
    {
      dwp = BeginDeferWindowPos((int)taskList.size());

      iter = taskList.begin();
      while (iter != taskList.end())
        {
          // VERY IMPORTANT: Only modify a valid window
          if (GetWindowRect((*iter)->GetTaskWnd(), &r))
            {
              // Ignore the window if it's sticky
              if (!pSettings->CheckSticky((*iter)->GetAppName()))
                {
                  r.left += maxColumnShift;
                  r.top += maxRowShift;

                  dwp = DeferWindowPos(dwp, (*iter)->GetTaskWnd(), NULL,
                                       r.left,	r.top,
                                       0, 0,
                                       SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
                }
            }

          iter++;
        }

      EndDeferWindowPos(dwp);
    }

  dwp = BeginDeferWindowPos((int)taskList.size());

  iter = taskList.begin();
  while (iter != taskList.end())
    {
      // VERY IMPORTANT: Only modify a valid window
      if (GetWindowRect((*iter)->GetTaskWnd(), &r))
        {
          // Ignore the window if it's sticky
          if (!pSettings->CheckSticky((*iter)->GetAppName()))
            {
              if (gather)
                {
                  RECT deskRect, tmpRect;
                  deskRect.left = screenLeft;
                  deskRect.right = deskRect.left + screenWidth;
                  deskRect.top = screenTop;
                  deskRect.bottom = deskRect.top + screenHeight;

                  if (!IntersectRect(&tmpRect, &r, &deskRect) && !IsZoomed((*iter)->GetTaskWnd()))
                    {
                      while (r.left < screenLeft)
                        r.left += screenWidth;
                      while (r.left > (screenLeft + screenWidth))
                        r.left -= screenWidth;

                      while (r.top < screenTop)
                        r.top += screenHeight;
                      while (r.top  > (screenTop + screenHeight))
                        r.top -= screenHeight;
                    }
                }
              else
                {
                  r.left -= maxColumnShift;
                  r.left -= columnOffset;
                  r.top -= maxRowShift;
                  r.top -= rowOffset;
                }

              dwp = DeferWindowPos(dwp, (*iter)->GetTaskWnd(), NULL,
                                   r.left,	r.top,
                                   0, 0,
                                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
            }
        }

      iter++;
    }

  oldActiveWindow = NULL;

  EndDeferWindowPos(dwp);

  DrawAlphaBlend();
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	DesktopMouseEvent
// Required:	UINT message - action to handle
// 		LPARAM lParam - location of the mouse
// Returns:	LRESULT
// Purpose:	Forwards the mouse message to the icon window
//----  --------------------------------------------------------------------------------------------------------
LRESULT Applet::DesktopMouseEvent(HWND hwnd, UINT message, LPARAM lParam)
{
  int row = 0, column = 0;
  POINT pt;
  RECT clientRect, desktopRect;
  bool found = false;
  HWND popupWnd;
  UINT dragBorder = guiInfo.dragBorder + guiInfo.bevelWidth + guiInfo.padding;
  int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN),
                    screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

  pt.x = LOWORD(lParam);
  pt.y = HIWORD(lParam);

  GetClientRect(hwnd, &clientRect);

  std::vector< std::tr1::shared_ptr<Task> >::reverse_iterator iter;

  for (iter = taskList.rbegin(); iter != taskList.rend(); iter++)
    {
      if ((PtInRect((*iter)->GetRect(), pt)) || ((*iter)->GetTaskWnd() == selectedWindow))
        {
          found = true;
          break;
        }
    }

  if (found)
    {
      if (message == WM_LBUTTONDOWN)
        {
          selectedWindow = (*iter)->GetTaskWnd();
          popupWnd = GetLastActivePopup(selectedWindow);
          referencePt = pt;
          SetForegroundWindow(popupWnd);
        }

      if (message == WM_RBUTTONUP)
        (*iter)->DisplayMenu(mainWnd);

      if (message == WM_MOUSEMOVE)
        {
          if (ELIsKeyDown(VK_LBUTTON))
            {
              float xOffset, yOffset, rowScalar, columnScalar;
              RECT windowRect, clientRect;

              GetWindowRect(selectedWindow, &windowRect);
              GetClientRect(mainWnd, &clientRect);

              rowScalar = (float)(clientRect.bottom - (2 * dragBorder)) /
                          (float)pSettings->GetDesktopRows();
              rowScalar = (float)screenHeight / rowScalar;
              columnScalar = (float)(clientRect.right - (2 * dragBorder)) /
                             (float)pSettings->GetDesktopColumns();
              columnScalar = (float)screenWidth / columnScalar;

              xOffset = (float)pt.x - (float)referencePt.x;
              xOffset *= columnScalar;
              yOffset = (float)pt.y - (float)referencePt.y;
              yOffset *= rowScalar;

              SetWindowPos(selectedWindow, NULL,
                           (windowRect.left + (int)xOffset),
                           (windowRect.top + (int)yOffset),
                           0, 0,
                           SWP_NOSIZE | SWP_NOACTIVATE);

              referencePt = pt;
            }
          else if ((*iter)->GetTaskWnd() != oldTipWindow)
            {
              oldTipWindow = (*iter)->GetTaskWnd();
              SendMessage(toolWnd, TTM_UPDATE, 0, 0);
            }
        }

      if (message == WM_LBUTTONUP)
        {
          oldActiveWindow = NULL;
          selectedWindow = NULL;
          SetForegroundWindow(selectedWindow);
        }

      return 0;
    }
  else
    {
      if (message == WM_MOUSEMOVE)
        {
          if (oldTipWindow != NULL)
            {
              SendMessage(toolWnd, TTM_UPDATE, 0, 0);
              oldTipWindow = NULL;
              return 0;
            }
        }

      for (row = 0; row < pSettings->GetDesktopRows(); row++)
        {
          for (column = 0; column < pSettings->GetDesktopColumns(); column++)
            {
              float rowScalar = (float)(clientRect.bottom - (2 * dragBorder)) /
                                (float)pSettings->GetDesktopRows();
              float columnScalar = (float)(clientRect.right - (2 * dragBorder)) /
                                   (float)pSettings->GetDesktopColumns();

              desktopRect.top = dragBorder + (int)(row * rowScalar);
              desktopRect.bottom = desktopRect.top + (int)rowScalar;
              desktopRect.left = dragBorder + (int)(column * columnScalar);
              desktopRect.right = desktopRect.left + (int)columnScalar;

              if (PtInRect(&desktopRect, pt))
                {
                  found = true;
                  break;
                }
            }

          if (found)
            break;
        }

      if (found)
        {
          if (message == WM_LBUTTONDOWN)
            SwitchDesktop(row, column, false);

          if (message == WM_LBUTTONDBLCLK)
            ELExecute((WCHAR*)TEXT("desk.cpl,,4"));

          return 0;
        }
    }

  return 1;
}

LRESULT Applet::PaintContent(HDC hdc, RECT clientrt)
{
  RECT desktoprt;   // used for getting window dimensions
  std::vector< std::tr1::shared_ptr<Task> >::iterator iter;
  int row, column;
  float columnScalar, rowScalar;
  int height = clientrt.bottom - clientrt.top;
  int width = clientrt.right - clientrt.left;

  rowScalar = (float)height / (float)pSettings->GetDesktopRows();
  columnScalar = (float)width / (float)pSettings->GetDesktopColumns();

  for (row = 0; row < pSettings->GetDesktopRows(); row++)
    {
      for (column = 0; column < pSettings->GetDesktopColumns(); column++)
        {
          desktoprt.top = clientrt.top + (int)(row * rowScalar);
          desktoprt.bottom = desktoprt.top + (int)rowScalar;
          desktoprt.left = clientrt.left + (int)(column * columnScalar);
          desktoprt.right = desktoprt.left + (int)columnScalar;

          if ((row == currentRow) && (column == currentColumn))
            EGFillRect(hdc, &desktoprt, guiInfo.alphaSelected, guiInfo.colorSelected);
        }
    }

  for (iter = taskList.begin(); iter != taskList.end(); iter++)
    {
      if (PaintTask((*iter).get()))
        {
          if ((*iter)->GetRect()->top < clientrt.top)
            (*iter)->GetRect()->top = clientrt.top;
          if ((*iter)->GetRect()->bottom > clientrt.bottom)
            (*iter)->GetRect()->bottom = clientrt.bottom;
          if ((*iter)->GetRect()->left < clientrt.left)
            (*iter)->GetRect()->left = clientrt.left;
          if ((*iter)->GetRect()->right > clientrt.right)
            (*iter)->GetRect()->right = clientrt.right;
          EGFillRect(hdc, (*iter)->GetRect(), guiInfo.alphaForeground, guiInfo.colorForeground);
          EGFrameRect(hdc, (*iter)->GetRect(), guiInfo.alphaFrame, guiInfo.colorFrame, 1);
        }
    }

  for (row = 0; row < pSettings->GetDesktopRows(); row++)
    {
      for (column = 0; column < pSettings->GetDesktopColumns(); column++)
        {
          desktoprt.top = clientrt.top + (int)(row * rowScalar);
          desktoprt.bottom = desktoprt.top + (int)rowScalar;
          desktoprt.left = clientrt.left + (int)(column * columnScalar);
          desktoprt.right = desktoprt.left + (int)columnScalar;

          EGFrameRect(hdc, &desktoprt, guiInfo.alphaFrame, guiInfo.colorFrame, 1);
        }
    }

  return 0;
}

bool Applet::PaintTask(Task *task)
{
  if (IsIconic(task->GetTaskWnd()))
    return false;

  if (pSettings->CheckSticky(task->GetAppName()) && pSettings->GetHideSticky())
    return false;

  return true;
}

LRESULT Applet::DoTimer(UINT timerID)
{
  if (BaseApplet::DoTimer(timerID) == 0)
    return 0;

  if (timerID == NEW_TASK_TIMER)
    {
      KillTimer(mainWnd, NEW_TASK_TIMER);

      // Remove and invalid tasks
      std::vector< std::tr1::shared_ptr<Task> >::iterator iter = taskList.begin(), tmpIter;

      while (iter != taskList.end())
        {
          if ((!IsWindow((*iter)->GetTaskWnd())) ||
              (!IsWindowVisible((*iter)->GetTaskWnd())))
            {
              tmpIter = taskList.erase(iter);
              iter = tmpIter;
            }
          else
            iter++;
        }

      // Add new tasks
      EnumWindows(BuildTasksList, (LPARAM)this);

      DrawAlphaBlend();

      SetTimer(mainWnd, NEW_TASK_TIMER, NEW_TASK_POLL_TIME, NULL);

      return 0;
    }

  return 1;
}

void Applet::ShowConfig()
{
  Config config(mainInst, mainWnd, pSettings);
  int res = config.Show();

  if ((res == IDOK) || (res == IDCANCEL))
    UpdateGUI();
}

LRESULT Applet::MySize()
{
  std::vector< std::tr1::shared_ptr<Task> >::iterator iter;
  RECT emptyRect;

  emptyRect.top = emptyRect.bottom = 0;
  emptyRect.left = emptyRect.right = 0;

  for (iter = taskList.begin(); iter != taskList.end(); iter++)
    (*iter)->SetReferenceRect(emptyRect);

  return 0;
}

LRESULT Applet::DoEmergeNotify(UINT messageClass, UINT message)
{
  //This modified code simply removed the edges of the VWM
  //When at an edge, it now loops back to the first or last virtual desktop
  //Modified by Jason "Teshadael" Price, 28/02/07
  if (messageClass & EMERGE_VWM)
    {
      if (message >= VWM_1 && message <= VWM_9)
        {
          UINT windowsNumber = pSettings->GetDesktopRows() * pSettings->GetDesktopColumns() ;
          UINT selectedWindow = (message-VWM_1) % windowsNumber ;
          UINT newRow = (UINT)(selectedWindow / pSettings->GetDesktopColumns()) ;
          UINT newColumn = selectedWindow % pSettings->GetDesktopColumns() ;
          SwitchDesktop(newRow, newColumn, false);
        }
      else if (message == VWM_UP)
        {
          if (currentRow > 0)
            SwitchDesktop(currentRow - 1, currentColumn, false);
          else
            SwitchDesktop((pSettings->GetDesktopRows() - 1), currentColumn, false);
        }
      else if (message == VWM_DOWN)
        {
          if (currentRow < (pSettings->GetDesktopRows() - 1))
            SwitchDesktop(currentRow + 1, currentColumn, false);
          else
            SwitchDesktop(0, currentColumn, false);  //If 0 doesn't work, perhaps try currentRow - currentRow?
        }
      else if (message == VWM_LEFT)
        {
          if (currentColumn > 0)
            SwitchDesktop(currentRow, currentColumn -1, false);
          else
            SwitchDesktop(currentRow, (pSettings->GetDesktopColumns() - 1), false);
        }
      else if (message == VWM_RIGHT)
        {
          if (currentColumn < (pSettings->GetDesktopColumns() - 1))
            SwitchDesktop(currentRow, currentColumn + 1, false);
          else
            SwitchDesktop(currentRow, 0, false);  //If 0 doesn't work, perhaps try currentColumn - currentColumn?
        }
      else if (message == VWM_GATHER)
        SwitchDesktop(currentRow, currentColumn, true);

      return 0;
    }

  return 1;
}

void Applet::AppletUpdate()
{
  UINT dragBorder = guiInfo.dragBorder + guiInfo.bevelWidth + guiInfo.padding;

  // fill in the TOOLINFO structure
  ZeroMemory(&ti, sizeof(TOOLINFO));
  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.hwnd = mainWnd;
  ti.uId = (ULONG_PTR)mainWnd;
  ti.hinst =  mainInst;
  ti.uFlags = TTF_SUBCLASS;
  ti.lpszText = LPSTR_TEXTCALLBACK;
  //ti.lpszText = (WCHAR*)L"Test Text";

  pSettings->ReadSettings();

  // Remove the tooltip region
  SendMessage(toolWnd, TTM_DELTOOL, 0, (LPARAM)&ti);

  ti.rect.top = dragBorder;
  ti.rect.left = dragBorder;
  ti.rect.right = ti.rect.left + pSettings->GetWidth();
  ti.rect.bottom = ti.rect.top + pSettings->GetHeight();

  // Add the main window as a tooltip region
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

LRESULT Applet::DoNotify(HWND hwnd, LPARAM lParam)
{
  LPNMHDR pnmh = (LPNMHDR)lParam;
  RECT rt;
  POINT pt;
  WCHAR windowTitle[TIP_SIZE];

  // Fetch tooltip text
  if (pnmh->code == TTN_NEEDTEXT)
    {
      LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT) lParam ;
      std::vector< std::tr1::shared_ptr<Task> >::reverse_iterator iter;

      GetWindowRect(hwnd, &rt);

      for (iter = taskList.rbegin(); iter != taskList.rend(); iter++)
        {
          GetCursorPos(&pt);
          pt.x -= rt.left;
          pt.y -= rt.top;

          if (PtInRect((*iter)->GetRect(), pt))
            {
              if (GetWindowText((*iter)->GetTaskWnd(), windowTitle, TIP_SIZE) != 0)
                {
                  lpttt->lpszText = windowTitle;
                  SetWindowPos(toolWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
                }
              else
                lpttt->lpszText = (WCHAR*)TEXT("\0");

              return 0;
            }
        }

      lpttt->lpszText = (WCHAR*)TEXT("\0");
    }

  return 1;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetTaskRowColumn
// Required:	HWND hwnd - window handle of task
//	 	UINT row (out) - target row
// 		UINT column (out) - target column
// Returns:	bool
// Purpose:	Gets the row and column the window is in
//
// Note:	Many thanx go to RabidCow for the suggestion of using the window title dimenions
// 		to determine the application desktop
//----  --------------------------------------------------------------------------------------------------------
bool Applet::GetTaskRowColumn(HWND hwnd, int *row, int *column)
{
  RECT winRect, desktopRect, tmpRect;
  int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  int screenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
  int screenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);
  int yborder = GetSystemMetrics(SM_CYDLGFRAME) + GetSystemMetrics(SM_CYEDGE);
  int rowDifferential = 0, columnDifferential = 0;
  int titleHeight = GetSystemMetrics(SM_CYSIZE);
  int titleWidth, titleVerticalCentre, titleHorizontalCentre;

  GetWindowRect(hwnd, &winRect);
  desktopRect.left = screenLeft;
  desktopRect.right = screenLeft + screenWidth;
  desktopRect.top = screenTop;
  desktopRect.bottom = screenTop + screenHeight;

  if (IsIconic(hwnd))
    return false;

  if (IntersectRect(&tmpRect, &winRect, &desktopRect))
    {
      *row = currentRow;
      *column = currentColumn;
      return true;
    }

  titleWidth = winRect.right - winRect.left;
  titleHorizontalCentre = winRect.left + (titleWidth / 2);
  titleVerticalCentre = winRect.top + yborder + (titleHeight / 2);

  while ((currentColumn + columnDifferential > 0) &&
         (titleHorizontalCentre <= screenLeft))
    {
      columnDifferential--;
      titleHorizontalCentre += screenWidth;
    }

  while ((currentColumn + columnDifferential < pSettings->GetDesktopColumns()) &&
         (titleHorizontalCentre >= (screenLeft + screenWidth)))
    {
      columnDifferential++;
      titleHorizontalCentre -= screenWidth;
    }

  while ((currentRow + rowDifferential > 0) &&
         (titleVerticalCentre <= screenTop))
    {
      rowDifferential--;
      titleVerticalCentre += screenHeight;
    }

  while ((currentRow + rowDifferential < pSettings->GetDesktopRows()) &&
         (titleVerticalCentre >= (screenTop + screenHeight)))
    {
      rowDifferential++;
      titleVerticalCentre -= screenHeight;
    }

  *row = currentRow + rowDifferential;
  if ((*row > (pSettings->GetDesktopRows() - 1)) || (*row < 0))
    *row = currentRow;

  *column = currentColumn + columnDifferential;
  if ((*column > (pSettings->GetDesktopColumns() - 1)) || (*column < 0))
    *column = currentColumn;

  return true;
}
