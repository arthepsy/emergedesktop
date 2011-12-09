// vim: tags+=../emergeLib/tags,../emergeGraphics/tags
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
      return pApplet->DoCopyData((COPYDATASTRUCT *)lParam);

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
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
      return pApplet->ItemMouseEvent(message, lParam);

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
      pApplet->DoDisplayChange(hwnd);

    case WM_SYSCOMMAND:
      return pApplet->DoSysCommand(hwnd, message, wParam, lParam);

    case WM_TIMER:
      return pApplet->DoTimer((UINT)wParam);

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
  :BaseApplet(hInstance, TEXT("emergeLauncher"), true, true)
{
  activeWnd = NULL;
  iconSize = 16;
}

Applet::~Applet()
{
}

UINT Applet::Initialize()
{
  pSettings = std::tr1::shared_ptr<Settings>(new Settings());
  UINT ret = BaseApplet::Initialize(WindowProcedure, this, pSettings);
  if (ret == 0)
    return ret;

  // Set the window transparency
  UpdateGUI();
  InitLiveFolderMap();

  return ret;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	PaintIcons
// Required:	HDC hdc - Device contect of calling window
// 		RECT r - client area of the calling window
// Returns:	LRESULT
// Purpose:	Paints the icons on the calling window
//----  --------------------------------------------------------------------------------------------------------
bool Applet::PaintItem(HDC hdc, size_t index, int x, int y, RECT rect)
{
  Item *item = pSettings->GetItem(index);

  if (item->GetType() == IT_LIVE_FOLDER)
    return false;

  item->SetRect(rect);
  UpdateTip(index);
  item->CreateNewIcon(guiInfo.alphaForeground, guiInfo.alphaBackground);

  InflateRect(&rect, 1, 1);
  if (item->GetActive())
    EGFillRect(hdc, &rect, guiInfo.alphaSelected, guiInfo.colorSelected);

  // Draw the indexcon
  DrawIconEx(hdc, x, y,
             item->GetIcon(), pSettings->GetIconSize(),
             pSettings->GetIconSize(), 0, NULL, DI_NORMAL);

  return true;
}

size_t Applet::GetVisibleIconCount()
{
  size_t visibleIconCount = 0;

  for (size_t i = 0; i < pSettings->GetItemListSize(); i++)
    {
      if (pSettings->GetItem(i)->GetType() != IT_LIVE_FOLDER)
        visibleIconCount++;
    }

  return visibleIconCount;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ItemMouseEvent
// Required:	UINT message - action to handle
// 		LPARAM lParam - location of the mouse
// Returns:	LRESULT
// Purpose:	Forwards the mouse message to the icon window
//----  --------------------------------------------------------------------------------------------------------
LRESULT Applet::ItemMouseEvent(UINT message, LPARAM lParam)
{
  UINT i;
  POINT pt;

  pt.x = LOWORD(lParam);
  pt.y = HIWORD(lParam);

  // Traverse the valid icon vector to see if the mouse is in the bounding rectangle
  // of the current icon
  for (i = 0; i < pSettings->GetItemListSize(); i++)
    {
      if (PtInRect(pSettings->GetItem(i)->GetRect(), pt))
        {
          if ((message == WM_LBUTTONDOWN) ||
              (message == WM_LBUTTONDBLCLK))
            {
              pSettings->GetItem(i)->SetActive(true);
              UINT timerID = i + 1000;
              SetTimer(mainWnd, timerID, ACTIVE_DELAY_TIME, (TIMERPROC)ActiveTimerProc);
              DrawAlphaBlend();

              switch (pSettings->GetItem(i)->GetType())
                {
                case IT_EXECUTABLE:
                case IT_LIVE_FOLDER_ITEM:
                  ELExecute(pSettings->GetItem(i)->GetApp(), pSettings->GetItem(i)->GetWorkingDir());
                  break;
                case IT_INTERNAL_COMMAND:
                  ELExecuteInternal(pSettings->GetItem(i)->GetApp());
                  break;
                case IT_SPECIAL_FOLDER:
                  ELExecuteSpecialFolder(pSettings->GetItem(i)->GetApp());
                  break;
                }
            }

          return 0;
        }
    }

  return 1;
}

VOID CALLBACK Applet::ActiveTimerProc(HWND hwnd, UINT uMsg UNUSED, UINT_PTR idEvent, DWORD dwTime UNUSED)
{
  Applet *pApplet = reinterpret_cast<Applet*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  pApplet->ClearActive(idEvent - 1000);
  KillTimer(hwnd, idEvent);
}

void Applet::ClearActive(UINT index)
{
  pSettings->GetItem(index)->SetActive(false);
  DrawAlphaBlend();
}

void Applet::ShowConfig()
{
  Config config(mainInst, mainWnd, appletName, pSettings);
  if (config.Show() == IDOK)
    {
      UpdateGUI();
      InitLiveFolderMap();
    }
}

LRESULT Applet::DoSizing(HWND hwnd, UINT edge, LPRECT rect)
{
  UpdateIcons();
  return BaseApplet::DoSizing(hwnd, edge, rect);
}

void Applet::InitLiveFolderMap()
{
  DWORD threadState, threadID;
  LiveFolderMap::iterator iter;
  std::wstring liveFolderPath;
  HANDLE thread;

  if (!liveFolderMap.empty())
    {
      iter = liveFolderMap.begin();

      thread = OpenThread(THREAD_TERMINATE|THREAD_QUERY_INFORMATION, FALSE, iter->first);

      GetExitCodeThread(thread, &threadState);
      if (threadState == STILL_ACTIVE)
        TerminateThread(thread, 0);

      liveFolderMap.erase(iter);
    }

  for (UINT i = 0; i < pSettings->GetItemListSize(); i++)
    {
      if (pSettings->GetItem(i)->GetType() == IT_LIVE_FOLDER)
        {
          liveFolderPath = pSettings->GetItem(i)->GetApp();
          liveFolderPath = ELExpandVars(liveFolderPath);

          thread = CreateThread(NULL, 0, LiveFolderThreadProc, this,
                                CREATE_SUSPENDED, &threadID);
          if (thread != NULL)
            {
              liveFolderMap.insert(std::pair<DWORD, std::wstring>(threadID,
                                   liveFolderPath));
              ResumeThread(thread);
            }
        }
    }
}

void Applet::AppletUpdate()
{
  for (UINT i = 0; i < pSettings->GetItemListSize(); i++)
    DeleteTip(i);

  pSettings->DeleteItems(false);
  pSettings->PopulateItems();
}

std::wstring Applet::GetFolderForThreadId(DWORD threadID)
{
  std::wstring result;
  LiveFolderMap::iterator iter = liveFolderMap.find(threadID);
  if (iter != liveFolderMap.end())
    result = iter->second;

  return result;
}

void Applet::UpdateIcons()
{
  for (UINT i = 0; i < pSettings->GetItemListSize(); i++)
    pSettings->GetItem(i)->SetIcon(pBaseSettings->GetIconSize(), pSettings->GetDirectionOrientation());
}

DWORD WINAPI Applet::LiveFolderThreadProc(LPVOID lpParameter)
{
  Applet *pApplet = reinterpret_cast< Applet* >(lpParameter);
  std::wstring watchFolder = pApplet->GetFolderForThreadId(GetCurrentThreadId());

  // Initialize COM or the icon extraction will fail
  OleInitialize(NULL);

  // Start a watch on the thread's assigned folder for file writes in order to detect change
  HANDLE folderWatch = FindFirstChangeNotification(watchFolder.c_str(), FALSE,
                       FILE_NOTIFY_CHANGE_FILE_NAME |
                       FILE_NOTIFY_CHANGE_DIR_NAME |
                       FILE_NOTIFY_CHANGE_LAST_WRITE);
  if (folderWatch != INVALID_HANDLE_VALUE)
    {
      // Start an infinite loop to watch the directory
      while (true)
        {
          // Wait for a file write to occur
          if (WaitForSingleObject(folderWatch, INFINITE) == WAIT_OBJECT_0)
            {
              // Once it has, force an applet update
              pApplet->UpdateGUI();
              // Restart the directory watch
              if (FindNextChangeNotification(folderWatch) == FALSE)
                break;
            }
          else
            break;
        }
    }

  OleUninitialize();
  return 0;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	UpdateTip
// Requires:    UINT index - index of the item
// Returns:	Nothing
// Purpose:	Updates the window handler with the task tooltip.  If
// 		the tip already exists, its updated.  If not, it is
// 		created.
//----  --------------------------------------------------------------------------------------------------------
void Applet::UpdateTip(UINT index)
{
  TOOLINFO ti;
  ZeroMemory(&ti, sizeof(TOOLINFO));

  bool exists;

  // fill in the TOOLINFO structure
  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = mainWnd;
  ti.uId = index;

  // Check to see if the tooltip exists
  exists = SendMessage(toolWnd, TTM_GETTOOLINFO, 0,(LPARAM) (LPTOOLINFO) &ti) ? true : false;

  //  complete the rest of the TOOLINFO structure
  ti.hinst =  mainInst;
  ti.lpszText = pSettings->GetItem(index)->GetTip();
  ti.rect = (*pSettings->GetItem(index)->GetRect());

  // If it exists, modify the tooltip, if not add it
  if (exists)
    SendMessage(toolWnd, TTM_SETTOOLINFO, 0, (LPARAM)(LPTOOLINFO)&ti);
  else
    SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	DeleteTip
// Requires:    UINT index - index of item
// Returns:	Nothing
// Purpose:	Deletes the task tooltip from the tooltip window
//----  --------------------------------------------------------------------------------------------------------
void Applet::DeleteTip(UINT index)
{
  TOOLINFO ti;
  ZeroMemory(&ti, sizeof(TOOLINFO));

  bool exists;

  // fill in the TOOLINFO structure
  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.hwnd = mainWnd;
  ti.uId = index;

  // Check to see if the tooltip exists
  exists = SendMessage(toolWnd, TTM_GETTOOLINFO, 0,(LPARAM) (LPTOOLINFO) &ti) ? true : false;

  if (exists)
    SendMessage(toolWnd, TTM_DELTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
}

size_t Applet::GetIconCount()
{
  return pSettings->GetItemListSize();
}

