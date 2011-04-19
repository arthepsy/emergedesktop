// vim:tags+=../emergeLib/tags
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
// Note:
//
// The LoadSSO and UnloadSSO code are from TrayManager.cpp which is part
// of the LiteStep code base.  Original inspiration for the icon handling code
// was curtousy of the GeoShell TrayService code.
//
// Many thanx go to Jaykul (of GeoShell fame) and Message (of LiteStep fame)
// for helping wrap my head around Windows deals with the systray.
//
//----  --------------------------------------------------------------------------------------------------------

#include "Applet.h"

/**< Tray related class names */
WCHAR szTrayName[ ] = TEXT("Shell_TrayWnd");
WCHAR szNotifyName[ ] = TEXT("TrayNotifyWnd");
WCHAR szReBarName[ ] = TEXT("ReBarWindow32");
WCHAR szClockName[ ] = TEXT("TrayClockWClass");
WCHAR szTaskSwName[ ] = TEXT("MSTaskSwWClass");

WCHAR myName[] = TEXT("emergeTray");

LRESULT CALLBACK Applet::TrayProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
      // Handle the icon messages
    case WM_COPYDATA:
      cpData = (COPYDATASTRUCT *)lParam;

      switch (cpData->dwData)
        {
        case TRAY_MESSAGE:
          return pApplet->TrayIconEvent(cpData);
        case APPBAR_MESSAGE:
          return pApplet->AppBarEvent(cpData);
        case ICONIDENTIFIER_MESSAGE:
          return pApplet->IconIdentifierEvent(cpData);
        }
      break;

      // If not handled just forward the message on
    default:
      return DefWindowProc(hwnd, message, wParam, lParam);
    }

  return 0;
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
    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MOUSEMOVE:
      return pApplet->TrayMouseEvent(message, lParam);

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

      // Write the X and Y settings to the registry
    case WM_MOVE:
      return pApplet->MyMove();

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

      // If not handled just forward the message on
    default:
      return pApplet->DoDefault(hwnd, message, wParam, lParam);
    }

  return 0;
}

Applet::Applet(HINSTANCE hInstance)
:BaseApplet(hInstance, myName, true)
{
  mainInst = hInstance;

  baseClassRegistered = false;
  trayClassRegistered = false;
  notifyClassRegistered = false;
  movesizeinprogress = false;

  autoHideLeft = false;
  autoHideRight = false;
  autoHideBottom = false;
  autoHideTop = false;

  activeIcon = NULL;
}

Applet::~Applet()
{
  // A quit message was recieved, so unload the 2K/XP system icons
  UnloadSSO();
  DestroyWindow(trayWnd);

  // Cleanup the icon vectors
  while (!trayIconList.empty())
    {
      trayIconList.front()->DeleteTip();
      trayIconList.erase(trayIconList.begin());
    }
}

UINT Applet::Initialize()
{
  WNDCLASSEX wincl;
  ZeroMemory(&wincl, sizeof(WNDCLASSEX));

  pSettings = std::tr1::shared_ptr<Settings>(new Settings(reinterpret_cast<LPARAM>(this)));
  UINT ret = BaseApplet::Initialize(WindowProcedure, this, pSettings);
  if (ret == 0)
    return ret;

  // Register the window class
  wincl.hInstance = mainInst;
  wincl.lpszClassName = szTrayName;
  wincl.lpfnWndProc = TrayProcedure;
  wincl.cbSize = sizeof (WNDCLASSEX);
  wincl.style = CS_DBLCLKS;
  wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
  wincl.hbrBackground = NULL;

  if (!RegisterClassEx (&wincl))
    return 0;

  wincl.lpszClassName = szNotifyName;
  if (!RegisterClassEx (&wincl))
    return 0;

  wincl.lpszClassName = szClockName;
  if (!RegisterClassEx (&wincl))
    return 0;

  wincl.lpszClassName = szTaskSwName;
  if (!RegisterClassEx (&wincl))
    return 0;

  wincl.lpszClassName = szReBarName;
  if (!RegisterClassEx (&wincl))
    return 0;

  trayWnd = CreateWindowEx(WS_EX_TOOLWINDOW, szTrayName, NULL, WS_POPUP,
                           0, 0, 0, 0, NULL, NULL, mainInst, reinterpret_cast<LPVOID>(this));
  if (!trayWnd)
    return 0;

  notifyWnd = CreateWindowEx(0, szNotifyName, NULL, WS_CHILDWINDOW,
                             0, 0, 0, 0, trayWnd, NULL, mainInst, reinterpret_cast<LPVOID>(this));
  if (!notifyWnd)
    return 0;

  clockWnd = CreateWindowEx(0, szClockName, NULL, WS_CHILDWINDOW,
                            0, 0, 0, 0, notifyWnd, NULL, mainInst, reinterpret_cast<LPVOID>(this));
  if (!clockWnd)
    return 0;

  rebarWnd = CreateWindowEx(0, szReBarName, NULL, WS_CHILDWINDOW,
                            0, 0, 0, 0, trayWnd, NULL, mainInst, reinterpret_cast<LPVOID>(this));
  if (!rebarWnd)
    return 0;

  taskWnd = CreateWindowEx(0, szTaskSwName, NULL, WS_CHILDWINDOW,
                           0, 0, 0, 0, rebarWnd, NULL, mainInst, reinterpret_cast<LPVOID>(this));
  if (!rebarWnd)
    return 0;

  SetProp(trayWnd, TEXT("AllowConsentToStealFocus"), (HANDLE)1);
  SetProp(trayWnd, TEXT("TaskBandHWND"), trayWnd);

  movesizeinprogress = false;

  // Set the window transparency
  UpdateGUI();

  // Build the sticky list only after reading the settings (in UpdateGUI())
  pSettings->BuildHideList();

  // Tell the applications that a systray was created
  SendNotifyMessage(HWND_BROADCAST, RegisterWindowMessage(TEXT("TaskbarCreated")), 0, 0);

  // Load the 2K/XP system icons
  LoadSSO();

  return 1;
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
  TrayIcon *pTrayIcon = trayIconList[index].get();

  // ignore hidden or invalid icons
  if (!IsIconVisible(pTrayIcon))
    return false;

  pTrayIcon->SetRect(rect);
  // Update the tooltip
  pTrayIcon->UpdateTip();

  // Convert the icon
  pTrayIcon->CreateNewIcon(guiInfo.alphaForeground, guiInfo.alphaBackground);

  // Draw the icon
  DrawIconEx(hdc, x, y, pTrayIcon->GetIcon(),
             ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);

  return true;
}

LRESULT Applet::DoTimer(UINT timerID)
{
  LRESULT res;

  if (movesizeinprogress)
    return 1;

  if (timerID == MOUSE_TIMER)
    CleanTray();

  res = BaseApplet::DoTimer(timerID);
  if ((res == 0) && !mouseOver)
    ShowHiddenIcons(false, false);

  return res;
}

void Applet::ShowConfig()
{
  Config config(mainInst, mainWnd, pSettings);
  if (config.Show() == IDOK)
    {
      ShowHiddenIcons(false, true);
      SortIcons();
      UpdateGUI();
    }
}

LRESULT Applet::DoSetCursor()
{
  if (!mouseOver)
    ShowHiddenIcons(true, false);

  return BaseApplet::DoSetCursor();
}

LRESULT Applet::DoEnterSizeMove(HWND hwnd)
{
  movesizeinprogress = true;
  return BaseApplet::DoEnterSizeMove(hwnd);
}

void Applet::UpdateIcons()
{
  for (UINT i = 0; i < trayIconList.size(); i++)
    trayIconList[i]->UpdateIcon();
}

LRESULT Applet::DoSizing(HWND hwnd, UINT edge, LPRECT rect)
{
  UpdateIcons();
  return BaseApplet::DoSizing(hwnd, edge, rect);
}

LRESULT Applet::MyMove()
{
  RECT winRect;
  movesizeinprogress = false;

  GetWindowRect(mainWnd, &winRect);
  SetWindowPos(trayWnd, NULL, winRect.left, winRect.top, (winRect.right - winRect.left), (winRect.bottom - winRect.top), SWP_NOZORDER | SWP_NOACTIVATE);

  return 0;
}

LRESULT Applet::MySize()
{
  movesizeinprogress = false;
  return 0;
}

void Applet::AppletUpdate()
{
  UpdateIcons();
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	LoadSSO
// Required:	Nothing
// Returns:	Nothing
// Purpose:	Loads the 2K/XP system icons
//----  --------------------------------------------------------------------------------------------------------
void Applet::LoadSSO()
{
  HKEY key;
  int i = 0;
  WCHAR valueName[32];
  WCHAR data[40];
  DWORD valueSize;
  DWORD dataSize;
  DWORD dwDataType;
  CLSID clsid, trayclsid;
  IOleCommandTarget *target;

  CLSIDFromString((WCHAR*)TEXT("{35CEC8A3-2BE6-11D2-8773-92E220524153}"), &trayclsid);

  target = ELStartSSO(trayclsid);
  if (target)
    ssoIconList.push_back(target);

  valueSize = 32 * sizeof(WCHAR);
  dataSize = 40 * sizeof(WCHAR);
  i = 0;

  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\ShellServiceObjectDelayLoad"),
                   0, KEY_READ, &key) == ERROR_SUCCESS)
    {
      while (RegEnumValue(key, i, valueName, &valueSize, 0, &dwDataType, (LPBYTE) data, &dataSize) == ERROR_SUCCESS)
        {
          CLSIDFromString(data, &clsid);
          if (clsid != trayclsid)
            {
              target = ELStartSSO(clsid);
              if (target)
                ssoIconList.push_back(target);
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
void Applet::UnloadSSO()
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

void Applet::ShowHiddenIcons(bool cmd, bool force)
{
  std::vector< std::tr1::shared_ptr<TrayIcon> >::iterator iter;
  RECT wndRect;

  if (((pSettings->GetUnhideIcons())|| (force)) && (!movesizeinprogress))
    {
      // Go through each of the elements in the trayIcons array
      for (iter = trayIconList.begin(); iter != trayIconList.end(); iter++)
        {
          if (pSettings->CheckHide((*iter)->GetTip()))
            {
              (*iter)->SetHidden(!cmd);
              if (cmd)
                (*iter)->UpdateTip();
              else
                (*iter)->DeleteTip();
            }
        }

      // Added call here to ensure the icons would be sorted
      // even if one was modified during mouseover (unhide)
      SortIcons();

      if (pSettings->GetAutoSize())
        {
          if (GetWindowRect(mainWnd, &wndRect))
            {
              AdjustRect(&wndRect);
              SetWindowPos(mainWnd, NULL, wndRect.left, wndRect.top,
                           wndRect.right - wndRect.left, wndRect.bottom - wndRect.top,
                           SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }
      DrawAlphaBlend();
    }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	CleanTray
// Required:	Nothing
// Returns:	Nothing
//----  --------------------------------------------------------------------------------------------------------
void Applet::CleanTray()
{
  RECT wndRect;
  bool hidden, removed = false;
  TrayIcon *pTrayIcon;
  UINT SWPFlags = SWP_NOZORDER | SWP_NOACTIVATE;

  if (trayIconList.empty())
    return;

  // Go through each of the elements in the trayIcons array
  for (size_t i =0; i < trayIconList.size(); i++)
    {
      pTrayIcon = trayIconList[i].get();

      // If the icon does not have a valid window handle, remove it
      if (!IsWindow(pTrayIcon->GetWnd()))
        {
          hidden = pTrayIcon->GetHidden();

          pTrayIcon->DeleteTip();
          removed = true;

          if (!hidden)
            {
              if (pSettings->GetAutoSize())
                {
                  if (GetWindowRect(mainWnd, &wndRect))
                    {
                      AdjustRect(&wndRect);
                      if (GetVisibleIconCount() == 0)
                        SWPFlags |= SWP_HIDEWINDOW;
                      SetWindowPos(mainWnd, NULL, wndRect.left, wndRect.top,
                                   wndRect.right - wndRect.left,
                                   wndRect.bottom - wndRect.top,
                                   SWPFlags);
                    }
                }

            }

          trayIconList.erase(trayIconList.begin() + i);
          i--;
        }
    }

  if (removed)
    DrawAlphaBlend();
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	FindTrayIcon
// Required:	NOTIFYICONDATA *iconData - extracted icon data
// Returns:	trayIconItem
// Purpose:	Determines if an icon is already in the valid icon
// 		vector and returns it
//----  --------------------------------------------------------------------------------------------------------
TrayIcon* Applet::FindTrayIcon(HWND hwnd, UINT uID)
{
  std::vector< std::tr1::shared_ptr<TrayIcon> >::iterator iter;

  // Traverse the trayIcons array
  for (iter = trayIconList.begin(); iter != trayIconList.end(); iter++)
    {
      // If the window and uID match an entry in the trayIcons array, return its position
      if ((*iter)->GetWnd() == hwnd && (*iter)->GetID() == uID)
        break;
    }

  if (iter == trayIconList.end())
    return NULL;

  return (*iter).get();
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	RemoveTrayIcon
// Required:	trayIconItem icon - pointer to icon in valid icon vector
// Returns:	LRESULT
// Purpose:	Removes an icon from the valid icon vector
//----  --------------------------------------------------------------------------------------------------------
LRESULT Applet::RemoveTrayIcon(HWND hwnd, UINT uID)
{
  TrayIcon *pTrayIcon = FindTrayIcon(hwnd, uID);
  RECT wndRect;
  UINT SWPFlags = SWP_NOZORDER | SWP_NOACTIVATE;

  if (!pTrayIcon)
    return 0;

  bool hidden = pTrayIcon->GetHidden();

  /*  if (activeIcon != NULL)
      {
      if (activeIcon->GetWnd() == (*iter)->GetWnd())
      {
      SendMessage(activeIcon->GetWnd(), activeIcon->GetCallback(), MAKEWPARAM(0, 0),
      MAKELPARAM(NIN_POPUPCLOSE, activeIcon->GetID()));
      activeIcon = NULL;
      }
      }*/

  pTrayIcon->DeleteTip();
  pTrayIcon->HideBalloon();
  RemoveTrayIconListItem(pTrayIcon);

  if (!hidden)
    {
      if (pSettings->GetAutoSize())
        {
          if (GetWindowRect(mainWnd, &wndRect))
            {
              AdjustRect(&wndRect);
              if (GetVisibleIconCount() == 0)
                SWPFlags |= SWP_HIDEWINDOW;
              SetWindowPos(mainWnd, NULL, wndRect.left, wndRect.top,
                           wndRect.right - wndRect.left, wndRect.bottom - wndRect.top,
                           SWPFlags);
            }
        }

      DrawAlphaBlend();
    }

  return 1;
}

LRESULT Applet::SetTrayIconVersion(HWND hwnd, UINT uID, UINT iconVersion)
{
  TrayIcon *pTrayIcon = FindTrayIcon(hwnd, uID);

  if (!pTrayIcon)
    return 0;

  pTrayIcon->SetIconVersion(iconVersion);
  if ((iconVersion == NOTIFYICON_VERSION_4) && ((pTrayIcon->GetFlags() & NIF_SHOWTIP) != NIF_SHOWTIP))
    pTrayIcon->DeleteTip();

  return 1;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ModifyTrayIcon
// Required:	trayIconItem icon - pointer to icon in valid icon vector
// 		NOTIFYICONDATA *iconData - extracted icon data
// 		LPTSTR newTip - new icon tip
// Returns:	LRESULT
// Purpose:	Updated the existing valid icon with the new data
//----  --------------------------------------------------------------------------------------------------------
LRESULT Applet::ModifyTrayIcon(HWND hwnd, UINT uID, UINT uFlags, UINT uCallbackMessage,
                               HICON icon, LPTSTR newTip, LPTSTR newInfo,
                               LPTSTR newInfoTitle, DWORD newInfoFlags, bool hidden,
                               bool shared)
{
  CleanTray();

  bool adjust = false;
  bool changed = false;
  TrayIcon *pTrayIcon = FindTrayIcon(hwnd, uID);
  RECT wndRect;
  UINT SWPFlags = SWP_NOZORDER | SWP_NOACTIVATE;

  if (!pTrayIcon)
    return 0;

  bool isHide = pSettings->CheckHide(newTip);

  if ((uFlags & NIF_ICON) == NIF_ICON)
    {
      if (pTrayIcon->SetIcon(icon))
        {
          pTrayIcon->SetFlags(pTrayIcon->GetFlags() | NIF_ICON);
          adjust = true;
          changed = true;
        }
    }

  if (!isHide)
    {
      if (hidden != pTrayIcon->GetHidden())
        {
          pTrayIcon->SetHidden(hidden);
          adjust = true;
          changed = true;
        }
    }
  /* Due to some tray icons getting their tip text delayed (i.e. network icons
   * use the isHidden check to force them hidden if appropriate.
   */
  else
    {
      if (wcslen(pTrayIcon->GetTip()) == 0)
        {
          pTrayIcon->SetHidden(true);
          adjust = true;
          changed = true;
        }
    }

  pTrayIcon->SetShared(shared);

  if ((uFlags & NIF_MESSAGE) == NIF_MESSAGE)
    {
      if (pTrayIcon->SetCallback(uCallbackMessage))
        {
          pTrayIcon->SetFlags(pTrayIcon->GetFlags() | NIF_MESSAGE);
          changed = true;
        }
    }

  if ((uFlags & NIF_INFO) == NIF_INFO)
    {
      pTrayIcon->ShowBalloon(newInfoTitle, newInfo, newInfoFlags, icon);
      pTrayIcon->SetFlags(pTrayIcon->GetFlags() | NIF_INFO);
    }

  if ((uFlags & NIF_TIP) == NIF_TIP)
    {
      if (!isHide || (isHide && (wcslen(newTip) != 0)))
        {
          if (pTrayIcon->SetTip(newTip))
            {
              pTrayIcon->SetFlags(pTrayIcon->GetFlags() | NIF_TIP);
              changed = true;
            }
        }
    }

  if (adjust)
    {
      SortIcons();
      if (pSettings->GetAutoSize())
        {
          if (GetWindowRect(mainWnd, &wndRect))
            {
              AdjustRect(&wndRect);
              if ((GetVisibleIconCount() > 0) && !appletHidden)
                SWPFlags |= SWP_SHOWWINDOW;
              SetWindowPos(mainWnd, NULL, wndRect.left, wndRect.top,
                           wndRect.right - wndRect.left,
                           wndRect.bottom - wndRect.top,
                           SWPFlags);
            }
        }
    }

  if (changed)
    DrawAlphaBlend();

  return 1;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	AddTrayIcon
// Required:	NOTIFYICONDATA *iconData - extracted icon data
// 		LPTSTR szTip - icon tip
// Returns:	LRESULT
// Purpose:	Adds a valid icon to the valid icon array
//----  --------------------------------------------------------------------------------------------------------
LRESULT Applet::AddTrayIcon(HWND hwnd, UINT uID, UINT uFlags, UINT uCallbackMessage,
                            HICON icon, LPTSTR szTip, LPTSTR szInfo, LPTSTR szInfoTitle,
                            DWORD dwInfoFlags, bool hidden, bool shared)
{
  CleanTray();

  TrayIcon *pTrayIcon = FindTrayIcon(hwnd, uID);
  RECT wndRect;
  UINT SWPFlags = SWP_NOZORDER | SWP_NOACTIVATE;

  if (pTrayIcon)
    return 0;

  pTrayIcon = new TrayIcon(mainInst, hwnd, uID, mainWnd, toolWnd, pSettings.get());

  if ((uFlags & NIF_MESSAGE) == NIF_MESSAGE)
    pTrayIcon->SetCallback(uCallbackMessage);

  if ((uFlags & NIF_INFO) == NIF_INFO)
    pTrayIcon->ShowBalloon(szInfoTitle, szInfo, dwInfoFlags, icon);

  if ((uFlags & NIF_TIP) == NIF_TIP)
    pTrayIcon->SetTip(szTip);
  else
    pTrayIcon->SendMessage(LPARAM(WM_MOUSEMOVE));

  pTrayIcon->SetFlags(uFlags);
  if (pSettings->CheckHide(szTip))
    pTrayIcon->SetHidden(true);
  else
    pTrayIcon->SetHidden(hidden);
  pTrayIcon->SetShared(shared);

  if ((uFlags & NIF_ICON) == NIF_ICON)
    pTrayIcon->SetIcon(icon);

  trayIconList.push_back( std::tr1::shared_ptr<TrayIcon>(pTrayIcon) );

  SortIcons();

  if (!hidden)
    {
      if (pSettings->GetAutoSize())
        {
          if (GetWindowRect(mainWnd, &wndRect))
            {
              AdjustRect(&wndRect);
              if ((GetVisibleIconCount() > 0) && !appletHidden)
                SWPFlags |= SWP_SHOWWINDOW;
              SetWindowPos(mainWnd, NULL, wndRect.left, wndRect.top,
                           wndRect.right - wndRect.left, wndRect.bottom - wndRect.top,
                           SWPFlags);
            }
        }

      DrawAlphaBlend();
    }

  return 1;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	TrayMouseEvent
// Required:	UINT message - action to handle
// 		LPARAM lParam - location of the mouse
// Returns:	bool
// Purpose:	Forwards the mouse message to the icon window
//----  --------------------------------------------------------------------------------------------------------
bool Applet::TrayMouseEvent(UINT message, LPARAM lParam)
{
  std::vector< std::tr1::shared_ptr<TrayIcon> >::iterator iter;
  POINT lparamPT, cursorPT;
  DWORD processID = 0;

  WCHAR tmp[MAX_LINE_LENGTH], error[MAX_LINE_LENGTH];
  ZeroMemory(tmp, MAX_LINE_LENGTH);

  lparamPT.x = LOWORD(lParam);
  lparamPT.y = HIWORD(lParam);
  GetCursorPos(&cursorPT);

  // Traverse the valid icon vector to see if the mouse is in the bounding rectangle
  // of the current icon
  for (iter = trayIconList.begin(); iter != trayIconList.end(); iter++)
    {
      if (PtInRect((*iter)->GetRect(), lparamPT) && !(*iter)->GetHidden())
        {
          //  If holding down ALT while left-click, add tip to
          //  hideList and hide the icon from the tray.
          if (ELIsKeyDown(VK_MENU) && (message == WM_LBUTTONDOWN))
            {
              if (wcscmp((*iter)->GetTip(), TEXT("\0")) == 0) //((*iter)->GetTip())
                {
                  swprintf(error, TEXT("This icon cannot be hidden because it has no text (tooltip)."), tmp);
                  ELMessageBox(GetDesktopWindow(), error, (WCHAR*)TEXT("emergeTray"),
                               ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
                  return 0;
                }
              if (!pSettings->CheckHide((*iter)->GetTip()))
                {
                  pSettings->AddHideListItem((*iter)->GetTip());
                  pSettings->WriteHideList();
                }

              if (!pSettings->GetUnhideIcons())
                {
                  (*iter)->SetHidden(true);
                  (*iter)->DeleteTip();
                  UpdateGUI();
                }

              ShowHiddenIcons(false, true);
              SortIcons();
              UpdateGUI();

              return 0;
            }

          GetWindowThreadProcessId((*iter)->GetWnd(), &processID);
          AllowSetForegroundWindow(processID);

          switch (message)
            {
            case WM_MOUSEMOVE:
              if ((ELVersionInfo() >= 6.0) && (((*iter)->GetFlags() & NIF_INFO) == NIF_INFO))
                {
                  if (activeIcon != NULL)
                    {
                      if( activeIcon->GetWnd() != (*iter)->GetWnd())
                        {
                          activeIcon->SendMessage(NIN_POPUPCLOSE);
                          activeIcon = (*iter).get();
                        }
                    }
                  message = NIN_POPUPOPEN;
                }
              break;

              /**< For some reason some icons require WM_USER being passed to activate the left click action */
            case WM_LBUTTONUP:
              (*iter)->SendMessage(WM_USER);
              break;

            case WM_RBUTTONUP:
              (*iter)->SendMessage(message);
              message = WM_CONTEXTMENU;
              break;
            }

          (*iter)->SendMessage(message);
          return 0;
        }
    }

  return 1;
}

bool Applet::RemoveAppBar(AppBar *pAppBar)
{
  std::vector< std::tr1::shared_ptr<AppBar> >::iterator iter;
  bool ret = false;

  for (iter = barList.begin(); iter != barList.end(); iter++)
    {
      if ((*iter).get() == pAppBar)
        {
          barList.erase(iter);
          ret = true;
          break;
        }
    }

  return ret;
}

bool Applet::RemoveTrayIconListItem(TrayIcon *pTrayIcon)
{
  std::vector< std::tr1::shared_ptr<TrayIcon> >::iterator iter;
  bool ret = false;

  for (iter = trayIconList.begin(); iter != trayIconList.end(); iter++)
    {
      if ((*iter).get() == pTrayIcon)
        {
          trayIconList.erase(iter);
          ret = true;
          break;
        }
    }

  return ret;
}

AppBar* Applet::FindAppBar(APPBARDATA abd)
{
  std::vector< std::tr1::shared_ptr<AppBar> >::iterator iter;

  for (iter = barList.begin(); iter != barList.end(); iter++)
    {
      if ((*iter)->IsEqual(abd))
        break;
    }

  if (iter == barList.end())
    return NULL;

  return (*iter).get();
}

LRESULT Applet::IconIdentifierEvent(COPYDATASTRUCT *cpData)
{
  POINT cursorPos;
  ICONIDENTIFIERDATA *iid;

  GetCursorPos(&cursorPos);

  iid = (PICONIDENTIFIERDATA)cpData->lpData;
  switch (iid->dwMessage)
    {
    case 2:
      return MAKELONG(16, 16);
    default:
      return MAKELONG(cursorPos.x, cursorPos.y);
    }

  return 0;
}

bool Applet::SetAutoHideEdge(UINT edge)
{
  if ((edge == ABE_TOP) && !autoHideTop)
    {
      autoHideTop = true;
      return true;
    }
  else if ((edge == ABE_BOTTOM) && !autoHideBottom)
    {
      autoHideBottom = true;
      return true;
    }
  else if ((edge == ABE_LEFT) && !autoHideLeft)
    {
      autoHideLeft = true;
      return true;
    }
  else if ((edge == ABE_RIGHT) && !autoHideRight)
    {
      autoHideRight = true;
      return true;
    }

  return false;
}

bool Applet::ClearAutoHideEdge(UINT edge)
{
  if (edge == ABE_TOP)
    autoHideTop = false;
  else if (edge == ABE_BOTTOM)
    autoHideBottom = false;
  else if (edge == ABE_LEFT)
    autoHideLeft = false;
  else if (edge == ABE_RIGHT)
    autoHideRight = false;

  return true;
}

LRESULT Applet::AppBarEvent(COPYDATASTRUCT *cpData)
{
  DWORD message = 0, dataSize;
  APPBARDATA abd;
  APPBARDATA_WOW32* abd32 = NULL;
  AppBar* pAppBar;
  std::vector< std::tr1::shared_ptr<AppBar> >::iterator iter;

  dataSize = *(DWORD*)cpData->lpData;
  if (dataSize == sizeof(APPBARDATA))
    CopyMemory(&abd, cpData->lpData, sizeof(APPBARDATA));
  else if (dataSize == sizeof(APPBARDATA_WOW32))
    {
      abd32 = (APPBARDATA_WOW32*)cpData->lpData;
      abd.cbSize = abd32->cbSize;
      abd.hWnd = (HWND)abd32->hWnd;
      abd.uCallbackMessage = abd32->uCallbackMessage;
      abd.uEdge = abd32->uEdge;
      abd.rc = abd32->rc;
      abd.lParam = abd32->lParam;
    }
  else
    return 0;

  message = *(DWORD *) (((BYTE *)cpData->lpData) + dataSize);

  switch (message)
    {
    case ABM_ACTIVATE:
      return 1;

    case ABM_GETSTATE:
        {
          LRESULT result = 0;

          if (!IsWindowVisible(mainWnd))
            result = ABS_AUTOHIDE;

          if (ELVersionInfo() >= 7.0)
            result |= ABS_ALWAYSONTOP;
          else
            {
              if (_wcsicmp(pSettings->GetZPosition(), TEXT("Top")) == 0)
                result |= ABS_ALWAYSONTOP;
            }

          return result;
        }

    case ABM_SETSTATE:
      return 1;

    case ABM_WINDOWPOSCHANGED:
      return 1;

    case ABM_GETAUTOHIDEBAR:
      for (iter = barList.begin(); iter != barList.end(); iter++)
        {
          if ((*iter)->IsAutoHide() && ((*iter)->GetEdge() == abd.uEdge))
            return (LRESULT)(*iter)->GetWnd();
        }
      return 0;

    case ABM_SETAUTOHIDEBAR:
      pAppBar = FindAppBar(abd);
      if (pAppBar)
        {
          if (SetAutoHideEdge(abd.uEdge))
            {
              barList.push_back( std::tr1::shared_ptr<AppBar>(new AppBar(mainWnd, abd, true)) );
              return 1;
            }
        }
      return 0;

    case ABM_REMOVE:
      pAppBar = FindAppBar(abd);
      if (pAppBar)
        {
          if (pAppBar->IsAutoHide())
            ClearAutoHideEdge(pAppBar->GetEdge());
          RemoveAppBar(pAppBar);
          return 1;
        }
      return 0;

    case ABM_NEW:
      pAppBar = FindAppBar(abd);
      if (!pAppBar)
        {
          barList.push_back( std::tr1::shared_ptr<AppBar>(new AppBar(mainWnd, abd, false)) );
          return 1;
        }
      return 0;

    case ABM_QUERYPOS:
      pAppBar = FindAppBar(abd);
      if (pAppBar)
        {
          if (abd.uEdge == pAppBar->GetEdge())
            {
              if (dataSize == sizeof(APPBARDATA_WOW32))
                ((APPBARDATA_WOW32*)cpData->lpData)->rc = pAppBar->GetRect();
              else
                ((APPBARDATA*)cpData->lpData)->rc = pAppBar->GetRect();
            }

          return 1;
        }
      return 0;

    case ABM_SETPOS:
      pAppBar = FindAppBar(abd);
      if (pAppBar)
        {
          pAppBar->SetRect(abd.rc);
          pAppBar->SetEdge(abd.uEdge);

          return 1;
        }
      return 0;

    case ABM_GETTASKBARPOS:
      pAppBar = FindAppBar(abd);
      if (pAppBar)
        {
          if (dataSize == sizeof(APPBARDATA_WOW32))
            ((APPBARDATA_WOW32*)cpData->lpData)->rc = pAppBar->GetRect();
          else
            ((APPBARDATA*)cpData->lpData)->rc = pAppBar->GetRect();

          return 1;
        }
      return 0;
    }

  return 0;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	TrayIconEvent
// Required:	LPARAM lParam - icon data
// Returns:	LRESULT
// Purpose:	Extracts the icon data and passes it to the appropriate function
//----  --------------------------------------------------------------------------------------------------------
LRESULT Applet::TrayIconEvent(COPYDATASTRUCT *cpData)
{
  bool hidden = false, shared = false;
  std::wstring wideString;
  WCHAR iconTip[TIP_SIZE], iconInfo[TIP_SIZE], infoTitle[TIP_SIZE];
  ZeroMemory(iconTip, TIP_SIZE);
  ZeroMemory(iconInfo, TIP_SIZE);
  ZeroMemory(infoTitle, TIP_SIZE);
  NID_5W *iconData5W;
  NID_5A *iconData5A;
  NID_4W *iconData4W;
  NID_4A *iconData4A;
  NID_5W_WOW32 *iconData5W32;
  NID_5A_WOW32 *iconData5A32;
  NID_4W_WOW32 *iconData4W32;
  NID_4A_WOW32 *iconData4A32;
  UINT iconVersion = 0, uID = 0, uCallbackMessage = 0, uFlags = 0;
  HICON icon = NULL;
  HWND hwnd = NULL;
  DWORD message = 0, infoFlags = 0;

  DWORD iconDataSize = 0;
  void* iconData = NULL;

  //OutputDebugString(TEXT("TrayIconEvent with tooltip: "));
  if (IsWindow(((PSHELLTRAYDATA)cpData->lpData)->iconData.hWnd))
    {
      message = ((PSHELLTRAYDATA)cpData->lpData)->dwMessage;
      iconDataSize = ((PSHELLTRAYDATA)cpData->lpData)->iconData.cbSize;
      iconData = &((PSHELLTRAYDATA)cpData->lpData)->iconData;
    }
  else if (IsWindow((HWND)((PSHELLTRAYDATAWOW32)cpData->lpData)->iconData.hWnd))
    {
      message = ((PSHELLTRAYDATAWOW32)cpData->lpData)->dwMessage;
      iconDataSize = ((PSHELLTRAYDATAWOW32)cpData->lpData)->iconData.cbSize;
      iconData = &((PSHELLTRAYDATAWOW32)cpData->lpData)->iconData;
    }
  else
    return 0;

  switch (iconDataSize)
    {
    case sizeof(NID_5W):
    case sizeof(NID_6W):
    case sizeof(NID_7W):
      iconData5W = (NID_5W*)iconData;
      iconVersion = iconData5W->uVersion;
      hwnd = (HWND)iconData5W->hWnd;
      uID = iconData5W->uID;
      uCallbackMessage = iconData5W->uCallbackMessage;
      uFlags = iconData5W->uFlags;

      if ((uFlags & NIF_INFO) == NIF_INFO)
        {
          wcscpy(iconInfo, iconData5W->szInfo);
          iconInfo[TIP_SIZE - 1] = '0';
          wcscpy(infoTitle, iconData5W->szInfoTitle);
          infoTitle[TIP_SIZE - 1] = '0';
          infoFlags = iconData5W->dwInfoFlags;
        }

      if ((uFlags & NIF_TIP) == NIF_TIP)
        {
          wcscpy(iconTip, iconData5W->szTip);
          iconTip[TIP_SIZE - 1] = '0';
        }

      if ((uFlags & NIF_ICON) == NIF_ICON)
        icon = (HICON)iconData5W->hIcon;

      if ((uFlags & NIF_STATE) == NIF_STATE)
        {
          if ((iconData5W->dwState & iconData5W->dwStateMask & NIS_HIDDEN) == NIS_HIDDEN)
            hidden = true;
          if ((iconData5W->dwState & iconData5W->dwStateMask & NIS_SHAREDICON) == NIS_SHAREDICON)
            shared = true;
        }
      break;

    case sizeof(NID_5A):
    case sizeof(NID_6A):
    case sizeof(NID_7A):
      iconData5A = (NID_5A*)iconData;
      iconVersion = iconData5A->uVersion;
      hwnd = (HWND)iconData5A->hWnd;
      uID = iconData5A->uID;
      uCallbackMessage = iconData5A->uCallbackMessage;
      uFlags = iconData5A->uFlags;

      if ((uFlags & NIF_INFO) == NIF_INFO)
        {
          wideString = ELstringTowstring(iconData5A->szInfo);
          wcsncpy(iconInfo, wideString.c_str(), TIP_SIZE);
          wideString = ELstringTowstring(iconData5A->szInfoTitle);
          wcsncpy(infoTitle, wideString.c_str(), TIP_SIZE);
          infoFlags = iconData5A->dwInfoFlags;
        }

      if ((uFlags & NIF_TIP) == NIF_TIP)
        {
          wideString = ELstringTowstring(iconData5A->szTip);
          wcsncpy(iconTip, wideString.c_str(), TIP_SIZE);
        }

      if ((uFlags & NIF_ICON) == NIF_ICON)
        icon = (HICON)iconData5A->hIcon;

      if ((uFlags & NIF_STATE) == NIF_STATE)
        {
          if ((iconData5A->dwState & iconData5A->dwStateMask & NIS_HIDDEN) == NIS_HIDDEN)
            hidden = true;
          if ((iconData5A->dwState & iconData5A->dwStateMask & NIS_SHAREDICON) == NIS_SHAREDICON)
            shared = true;
        }
      break;

    case sizeof(NID_4W):
      iconData4W = (NID_4W*)iconData;
      hwnd = (HWND)iconData4W->hWnd;
      uID = iconData4W->uID;
      uCallbackMessage = iconData4W->uCallbackMessage;
      uFlags = iconData4W->uFlags;

      if ((uFlags & NIF_ICON) == NIF_ICON)
        icon = (HICON)iconData4W->hIcon;

      if ((uFlags & NIF_TIP) == NIF_TIP)
        {
          wcsncpy(iconTip, iconData4W->szTip, TIP_SIZE);
          iconTip[TIP_SIZE - 1]='\0';
        }
      break;

    case sizeof(NID_4A):
      iconData4A = (NID_4A*)iconData;
      hwnd = (HWND)iconData4A->hWnd;
      uID = iconData4A->uID;
      uCallbackMessage = iconData4A->uCallbackMessage;
      uFlags = iconData4A->uFlags;

      if ((uFlags & NIF_ICON) == NIF_ICON)
        icon = (HICON)iconData4A->hIcon;

      if ((uFlags & NIF_TIP) == NIF_TIP)
        {
          wideString = ELstringTowstring(iconData4A->szTip);
          wcsncpy(iconTip, wideString.c_str(), TIP_SIZE);
        }
      break;
    }

  // If the icon notification type was not determined, attempt to check against 32-bit types.
  // This is required since x64 Windows makes use of both 32-bit and 64-bit notifications.
  if ((uID == 0) && (uCallbackMessage == 0) && (uFlags == 0) && (hwnd == NULL))
    {
      switch (iconDataSize)
        {
        case sizeof(NID_5W_WOW32):
        case sizeof(NID_6W_WOW32):
        case sizeof(NID_7W_WOW32):
          iconData5W32 = (NID_5W_WOW32*)iconData;
          iconVersion = iconData5W32->uVersion;
          hwnd = (HWND)iconData5W32->hWnd;
          uID = iconData5W32->uID;
          uCallbackMessage = iconData5W32->uCallbackMessage;
          uFlags = iconData5W32->uFlags;

          if ((uFlags & NIF_INFO) == NIF_INFO)
            {
              wcscpy(iconInfo, iconData5W32->szInfo);
              iconInfo[TIP_SIZE - 1] = '0';
              wcscpy(infoTitle, iconData5W32->szInfoTitle);
              infoTitle[TIP_SIZE - 1] = '0';
              infoFlags = iconData5W32->dwInfoFlags;
            }

          if ((uFlags & NIF_TIP) == NIF_TIP)
            {
              wcscpy(iconTip, iconData5W32->szTip);
              iconTip[TIP_SIZE - 1] = '0';
            }

          if ((uFlags & NIF_ICON) == NIF_ICON)
            icon = (HICON)iconData5W32->hIcon;

          if ((uFlags & NIF_STATE) == NIF_STATE)
            {
              if ((iconData5W32->dwState & iconData5W32->dwStateMask & NIS_HIDDEN) == NIS_HIDDEN)
                hidden = true;
              if ((iconData5W32->dwState & iconData5W32->dwStateMask & NIS_SHAREDICON) == NIS_SHAREDICON)
                shared = true;
            }
          break;

        case sizeof(NID_5A_WOW32):
        case sizeof(NID_6A_WOW32):
        case sizeof(NID_7A_WOW32):
          iconData5A32 = (NID_5A_WOW32*)iconData;
          iconVersion = iconData5A32->uVersion;
          hwnd = (HWND)iconData5A32->hWnd;
          uID = iconData5A32->uID;
          uCallbackMessage = iconData5A32->uCallbackMessage;
          uFlags = iconData5A32->uFlags;

          if ((uFlags & NIF_INFO) == NIF_INFO)
            {
              wideString = ELstringTowstring(iconData5A32->szInfo);
              wcsncpy(iconInfo, wideString.c_str(), TIP_SIZE);
              wideString = ELstringTowstring(iconData5A32->szInfoTitle);
              wcsncpy(infoTitle, wideString.c_str(), TIP_SIZE);
              infoFlags = iconData5A32->dwInfoFlags;
            }

          if ((uFlags & NIF_TIP) == NIF_TIP)
            {
              wideString = ELstringTowstring(iconData5A32->szTip);
              wcsncpy(iconTip, wideString.c_str(), TIP_SIZE);
            }

          if ((uFlags & NIF_ICON) == NIF_ICON)
            icon = (HICON)iconData5A32->hIcon;

          if ((uFlags & NIF_STATE) == NIF_STATE)
            {
              if ((iconData5A32->dwState & iconData5A32->dwStateMask & NIS_HIDDEN) == NIS_HIDDEN)
                hidden = true;
              if ((iconData5A32->dwState & iconData5A32->dwStateMask & NIS_SHAREDICON) == NIS_SHAREDICON)
                shared = true;
            }
          break;

        case sizeof(NID_4W_WOW32):
          iconData4W32 = (NID_4W_WOW32*)iconData;
          hwnd = (HWND)iconData4W32->hWnd;
          uID = iconData4W32->uID;
          uCallbackMessage = iconData4W32->uCallbackMessage;
          uFlags = iconData4W32->uFlags;

          if ((uFlags & NIF_ICON) == NIF_ICON)
            icon = (HICON)iconData4W32->hIcon;

          if ((uFlags & NIF_TIP) == NIF_TIP)
            {
              wcsncpy(iconTip, iconData4W32->szTip, TIP_SIZE);
              iconTip[TIP_SIZE - 1]='\0';
            }
          break;

        case sizeof(NID_4A_WOW32):
          iconData4A32 = (NID_4A_WOW32*)iconData;
          hwnd = (HWND)iconData4A32->hWnd;
          uID = iconData4A32->uID;
          uCallbackMessage = iconData4A32->uCallbackMessage;
          uFlags = iconData4A32->uFlags;

          if ((uFlags & NIF_ICON) == NIF_ICON)
            icon = (HICON)iconData4A32->hIcon;

          if ((uFlags & NIF_TIP) == NIF_TIP)
            {
              wideString = ELstringTowstring(iconData4A32->szTip);
              wcsncpy(iconTip, wideString.c_str(), TIP_SIZE);
            }
          break;

        default:
          return 0;
        }
    }

  switch (message)
    {
      // Since icons don't always send the appropriate message,
      // do a little checking
    case NIM_ADD:
      return AddTrayIcon(hwnd, uID, uFlags, uCallbackMessage, icon, iconTip,
                         iconInfo, infoTitle, infoFlags, hidden, shared);
    case NIM_SETVERSION:
      return SetTrayIconVersion(hwnd, uID, iconVersion);
    case NIM_MODIFY:
      return ModifyTrayIcon(hwnd, uID, uFlags, uCallbackMessage, icon, iconTip,
                            iconInfo, infoTitle, infoFlags, hidden, shared);
    case NIM_DELETE:
      return RemoveTrayIcon(hwnd, uID);
    }

  return 0;
}

size_t Applet::GetTrayIconListSize()
{
  return trayIconList.size();
}

TrayIcon *Applet::GetTrayIconListItem(UINT index)
{
  return trayIconList[index].get();
}

// Used to sort the visible icons first, then the hidden ones
void Applet::SortIcons()
{

  std::vector< std::tr1::shared_ptr<TrayIcon> >::iterator iter;
  std::vector< std::tr1::shared_ptr<TrayIcon> > trayIconListTmp;
  int insertCount;

  insertCount = 0;

  // Traverse the valid icon vector, painting each icon
  for (iter = trayIconList.begin(); iter != trayIconList.end(); iter++)
    {

      if ((*iter)->GetHidden())
        {
          trayIconListTmp.push_back((*iter));
        }
      else
        {
          trayIconListTmp.insert(trayIconListTmp.begin() + insertCount,(*iter));
          insertCount++;
        }
    }

  trayIconList.assign(trayIconListTmp.begin(),trayIconListTmp.end());
}

size_t Applet::GetVisibleIconCount()
{
  size_t visibleIconCount = 0;

  for (size_t i = 0; i < trayIconList.size(); i++)
    {
      if (IsIconVisible(trayIconList[i].get()))
        visibleIconCount++;
    }

  return visibleIconCount;
}

bool Applet::IsIconVisible(TrayIcon *pTrayIcon)
{
  return (!pTrayIcon->GetHidden() &&
          ((pTrayIcon->GetFlags() & NIF_ICON) == NIF_ICON));
}

size_t Applet::GetIconCount()
{
  return trayIconList.size();
}

