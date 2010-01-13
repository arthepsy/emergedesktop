// vim:tags+=../emergeLib/tags,../emergeBaseClasses/tags
//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2009  The Emerge Desktop Development Team
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
#include "Settings.h"
#include "Config.h"

// winamp defines
#define W2_PREV             40044
#define W2_PLAY             40045
#define W2_PAUSE            40046
#define W2_STOP             40047
#define W2_NEXT             40048
#define W2_FILEPLAY         40029

// foobar defines
#define FOOBAR_STOP					40010
#define FOOBAR_PAUSE				40044
#define FOOBAR_PLAY 				40009
#define FOOBAR_NEXT 				40011
#define FOOBAR_PREV                 40051
#define FOOBAR_OPENFILE				40049

// classic media player
#define ID_PLAY_PLAY                    155
#define ID_PLAY_PAUSE                   156
#define ID_PLAY_STOP                    158
#define ID_PLAY_DECRATE                 159
#define ID_PLAY_INCRATE                 160
#define ID_NAVIGATE_SKIPBACK            164
#define ID_NAVIGATE_SKIPFORWARD         165
#define ID_FILE_OPENMEDIA               131

// WMP
#define WMP_PAUSE 18808
#define WMP_STOP  18809
#define WMP_PREV  18810
#define WMP_NEXT  18811

#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))

extern Applet *pApplet;
Settings *pSettings = NULL;
Config *pConfig = NULL;

WCHAR myName[] = TEXT("emergeMediaCtrl");

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

  if ((pApplet == NULL) || (pSettings == NULL))
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

      // Reset the cursor back to the standard arrow after dragging
    case WM_NCLBUTTONUP:
      pApplet->DoNCLButtonUp();
      return DefWindowProc(hwnd, message, wParam, lParam);

    case WM_LBUTTONDOWN:
      pApplet->DoLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
      break;

    case WM_LBUTTONUP:
      pApplet->DoLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
      break;

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
  mainFont = NULL;
}

Applet::~Applet()
{
  // stop timer
  UINT mediaID = (ULONG_PTR)mainWnd + 100;
  KillTimer(mainWnd, mediaID);

  // Unregister the specified Emerge Desktop messages
  PostMessage(ELGetCoreWindow(), EMERGE_UNREGISTER, (WPARAM)mainWnd, (LPARAM)EMERGE_VWM);

  // Destroy class pointers
  delete pConfig;

  delete pPrevButton;
  delete pPauseButton;
  delete pNextButton;

  if (mainFont)
    DeleteObject(mainFont);
}

UINT Applet::Initialize()
{
  UINT ret = BaseApplet::Initialize(WindowProcedure, this);
  if (ret == 0)
    return ret;

  // Initialize Settings, Config and AppletMenu classes
  pSettings = reinterpret_cast<Settings*>(pBaseSettings);
  pConfig = new Config(mainInst, mainWnd);

  pPrevButton = new Button(*this, pSettings, mainFont, guiInfo, mainWnd);
  pPauseButton = new Button(*this, pSettings, mainFont, guiInfo, mainWnd, true);
  pNextButton = new Button(*this, pSettings, mainFont, guiInfo, mainWnd);
  pPrevButton->SetTitle(TEXT("<"));
  pNextButton->SetTitle(TEXT(">"));

  // Set the window transparency
  UpdateGUI();

  // setup timer
  UINT mediaID = (ULONG_PTR)mainWnd + 100;
  SetTimer(mainWnd, mediaID, pSettings->GetUpdateInterval(), NULL);
  UpdateStatus();

  return ret;
}

BaseSettings *Applet::createSettings()
{
  return new Settings();
}

#define MIN(x, y) ((x) < (y) ? (x) : (y))

LRESULT Applet::PaintContent(HDC hdc, RECT clientrt)
{
  RECT r = clientrt;
  int widthThird = (r.right - r.left + 2) / 3;
  int height = r.bottom - r.top;
  RECT leftBox = clientrt;
  leftBox.right = leftBox.left + MIN(height, widthThird);
  RECT rightBox = clientrt;
  rightBox.left = rightBox.right - MIN(height, widthThird);
  RECT middleBox = clientrt;
  middleBox.left = leftBox.right + guiInfo.padding;
  middleBox.right = rightBox.left - guiInfo.padding;

  pPrevButton->SetBounds(leftBox);
  pPauseButton->SetBounds(middleBox);
  pNextButton->SetBounds(rightBox);

  pPrevButton->Paint(hdc);
  pPauseButton->Paint(hdc);
  pNextButton->Paint(hdc);

  return 0;
}

LRESULT Applet::DoTimer(UINT_PTR timerID)
{
  if (timerID == MOUSE_TIMER)
    return BaseApplet::DoTimer(timerID);
  else
    UpdateStatus();

  return 1;
}

void CutFromText(std::wstring& text, const std::wstring& section)
{
  size_t n = 0;
  if ((n = text.find(section)) != std::wstring::npos)
    {
      text.erase(text.begin() + n, text.begin() + n + section.size());
    }
}

void Applet::UpdateStatus()
{
  // get the title of played content
  HWND playerWnd = NULL;
  WCHAR title[MAX_LINE_LENGTH];
  if (!(playerWnd = FindWindow(TEXT("Winamp v1.x"), NULL)))
    {
      if (!(playerWnd = FindWindow(TEXT("FOOBAR2000_CLASS"), NULL)))
        {
          if (!(playerWnd = FindWindow(TEXT("MediaPlayerClassicW"), NULL)))
            {
              playerWnd = FindWindow(TEXT("WMPlayerApp"), TEXT("Windows Media Player"));
            }
        }
    }

  if (playerWnd)
    {
      GetWindowText(playerWnd, title, MAX_LINE_LENGTH);
      if (wcslen(title) == 0)
        wcscpy(title, TEXT("none"));
    }
  else
    wcscpy(title, TEXT("Player not found"));

  std::wstring tmp = title;
  CutFromText(tmp, TEXT(" - Winamp"));
  CutFromText(tmp, TEXT(" - Media Player Classic"));
  pPauseButton->SetTitle(tmp);
  pPauseButton->NextIteration();

  // update tooltip
  TOOLINFO ti;
  ZeroMemory(&ti, sizeof(TOOLINFO));
  RECT rect;
  bool exists;

  GetClientRect(mainWnd, &rect);

  // fill in the TOOLINFO structure
  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = mainWnd;
  ti.uId = (ULONG_PTR)toolWnd;

  // Check to see if the tooltip exists
  exists = SendMessage(toolWnd, TTM_GETTOOLINFO, 0,(LPARAM) (LPTOOLINFO) &ti) ? true : false;

  //  complete the rest of the TOOLINFO structure
  ti.hinst = mainInst;
  ti.lpszText = title;
  ti.rect = rect;

  // If it exists, modify the tooltip, if not add it
  if (exists)
    SendMessage(toolWnd, TTM_SETTOOLINFO, 0, (LPARAM)(LPTOOLINFO)&ti);
  else
    SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  DrawAlphaBlend();
}

void Applet::DoPause()
{
  HWND playerWnd = NULL;
  if ((playerWnd = FindWindow(TEXT("Winamp v1.x"), NULL)) != NULL)
    PostMessage(playerWnd, WM_COMMAND, W2_PAUSE, 0);
  else if((playerWnd = FindWindow(TEXT("FOOBAR2000_CLASS"), NULL)) != NULL)
    PostMessage(playerWnd, WM_COMMAND, FOOBAR_PAUSE, 0);
  else if((playerWnd = FindWindow(TEXT("MediaPlayerClassicW"), NULL)) != NULL)
    PostMessage(playerWnd, WM_COMMAND, ID_PLAY_PAUSE, 0);
  else if((playerWnd = FindWindow(TEXT("WMPlayerApp"), TEXT("Windows Media Player"))) != NULL)
    PostMessage(playerWnd, WM_COMMAND, WMP_PAUSE, 0);
}

void Applet::DoNext()
{
  HWND playerWnd = NULL;
  if ((playerWnd = FindWindow(TEXT("Winamp v1.x"), NULL)) != NULL)
    PostMessage(playerWnd, WM_COMMAND, W2_NEXT, 0);
  else if((playerWnd = FindWindow(TEXT("FOOBAR2000_CLASS"), NULL)) != NULL)
    PostMessage(playerWnd, WM_COMMAND, FOOBAR_NEXT, 0);
  else if((playerWnd = FindWindow(TEXT("MediaPlayerClassicW"), NULL)) != NULL)
    PostMessage(playerWnd, WM_COMMAND, ID_NAVIGATE_SKIPFORWARD, 0);
  else if((playerWnd = FindWindow(TEXT("WMPlayerApp"), TEXT("Windows Media Player"))) != NULL)
    PostMessage(playerWnd, WM_COMMAND, WMP_NEXT, 0);
}

void Applet::DoPrev()
{
  HWND playerWnd = NULL;
  if ((playerWnd = FindWindow(TEXT("Winamp v1.x"), NULL)) != NULL)
    PostMessage(playerWnd, WM_COMMAND, W2_PREV, 0);
  else if((playerWnd = FindWindow(TEXT("FOOBAR2000_CLASS"), NULL)) != NULL)
    PostMessage(playerWnd, WM_COMMAND, FOOBAR_PREV, 0);
  else if((playerWnd = FindWindow(TEXT("MediaPlayerClassicW"), NULL)) != NULL)
    PostMessage(playerWnd, WM_COMMAND, ID_NAVIGATE_SKIPBACK, 0);
  else if((playerWnd = FindWindow(TEXT("WMPlayerApp"), TEXT("Windows Media Player"))) != NULL)
    PostMessage(playerWnd, WM_COMMAND, WMP_PREV, 0);
}

LRESULT Applet::DoEmergeNotify(UINT messageClass, UINT message)
{
  if (messageClass & EMERGE_VWM)
    {
      DrawAlphaBlend();

      return 0;
    }

  return BaseApplet::DoEmergeNotify(messageClass, message);
}

void Applet::AppletUpdate()
{
  if (mainFont != NULL)
    DeleteObject(mainFont);
  mainFont = CreateFontIndirect(pSettings->GetFont());
}

void Applet::ShowConfig()
{
  if (pConfig->Show() == IDOK)
    {
      // restart timer
      UINT mediaID = (ULONG_PTR)mainWnd + 100;
      KillTimer(mainWnd, mediaID);
      SetTimer(mainWnd, mediaID, pSettings->GetUpdateInterval(), NULL);

      UpdateGUI();
    }
}

LRESULT Applet::DoLButtonDown(int x, int y, int modifiers)
{
  if ((modifiers & (MK_CONTROL | MK_SHIFT)) == 0)
    {
      if (pPrevButton->LButtonDown(x, y))
        DrawAlphaBlend();
      if (pPauseButton->LButtonDown(x, y))
        DrawAlphaBlend();
      if (pNextButton->LButtonDown(x, y))
        DrawAlphaBlend();
    }
  return 0;
}

LRESULT Applet::DoLButtonUp(int x, int y, int modifiers)
{
  if ((modifiers & (MK_CONTROL | MK_SHIFT)) == 0)
    {
      if (pPrevButton->LButtonUp(x, y))
        DoPrev();
      else if (pPauseButton->LButtonUp(x, y))
        DoPause();
      else if (pNextButton->LButtonUp(x, y))
        DoNext();
      DrawAlphaBlend();
    }
  return 0;
}
