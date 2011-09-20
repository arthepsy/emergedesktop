// vim:tags+=../emergeLib/tags,../emergeAppletEngine/tags,../emergeGraphics/tags,../emergeStyleEngine/tags
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

#include "BaseApplet.h"
#include "../emergeAppletEngine/emergeAppletEngine.h"
#include <stdio.h>

std::wstring instanceManagementPath = TEXT("%EmergeDir%\\files\\InstanceManagement.xml");

BaseApplet::BaseApplet(HINSTANCE hInstance, const WCHAR *appletName, bool allowAutoSize, bool allowMultipleInstances)
{
  mainInst = hInstance;
  wcscpy(this->appletName, appletName);
  wcscpy(this->baseAppletName, appletName);
  mouseOver = false;
  fullScreen = false;
  appletHidden = false;
  firstRun = true;
  this->allowAutoSize = allowAutoSize;
  this->allowMultipleInstances = allowMultipleInstances;

  activeBackgroundDC = NULL;
  inactiveBackgroundDC = NULL;

  displayChangeThread = NULL;
  fullScreenThread = NULL;

  ZeroMemory(&oldrt, sizeof(RECT));
}

BaseApplet::~BaseApplet()
{
  // Unregister the specified Emerge Desktop messages
  PostMessage(ELGetCoreWindow(), EMERGE_UNREGISTER, (WPARAM)mainWnd, (LPARAM)EMERGE_CORE);

  // Unregister as a Shell window
  if (activeBackgroundDC != NULL)
    DeleteDC(activeBackgroundDC);
  if (inactiveBackgroundDC != NULL)
    DeleteDC(inactiveBackgroundDC);

  CloseHandle(multiInstanceLock);

  // Close the InstanceManagement.xml file mutex
  CloseHandle(instanceManagementMutex);

  OleUninitialize();

  ELClearEmergeVars();
}

UINT BaseApplet::Initialize(WNDPROC WindowProcedure, LPVOID lpParam, std::tr1::shared_ptr<BaseSettings> pSettings)
{
  // Set the Emerge Vars irregardless of them being set already to handle the scenario where
  // the applets are started from another application that has the ThemeDir set previously.
  if (!ELSetEmergeVars())
    {
      ELMessageBox(GetDesktopWindow(), TEXT("Failed to initialize Environment variables."),
                   baseAppletName, ELMB_ICONERROR|ELMB_MODAL|ELMB_OK);
      return 0;
    }

  // Create the InstanceManagement.xml mutex initially as un-owned
  instanceManagementMutex = CreateMutex(NULL, FALSE, TEXT("InstanceManagement"));
  multiInstanceLock = CreateMutex(NULL, false, baseAppletName);
  DWORD err = GetLastError();

  if (allowMultipleInstances)
    {
      if (err != ERROR_ALREADY_EXISTS)
        {
          WriteAppletCount(-1, false); /*this is the first instance of this applet, so reset its applet count */
          RenameSettingsFiles();
        }

      if (!SpawnInstance())
        return 0;
    }
  else
    {
      if (err == ERROR_ALREADY_EXISTS)
        {
          CloseHandle(multiInstanceLock);
          return 0;
        }
    }

  // Set the non-critical evnvironment variables - do not popup warning messages.
  ELSetEnvironmentVars(false);

  if (FAILED(OleInitialize(NULL)))
    {
      ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("COM initialization failed"), baseAppletName,
                   ELMB_ICONERROR|ELMB_OK|ELMB_MODAL);
      return 0;
    }

  mainWnd = EAEInitializeAppletWindow(mainInst, WindowProcedure, lpParam);

  // If the window failed to get created, unregister the class and quit the program
  if (!IsWindow(mainWnd))
    return 0;

  toolWnd = EAEInitializeTooltipWindow(mainInst);

  // Disable menu animation, as it overrides the alpha blending
  SystemParametersInfo(SPI_SETMENUANIMATION, 0, (PVOID)false, SPIF_SENDCHANGE);

  if (ELRegisterShellHook(mainWnd, RSH_TASKMGR))
    ShellMessage = RegisterWindowMessage(TEXT("SHELLHOOK"));
  else
    {
      ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("Failed to register as a shell window"), baseAppletName,
                   ELMB_ICONERROR|ELMB_OK|ELMB_MODAL);
      return 0;
    }

  pBaseSettings = pSettings;
  pBaseSettings->Init(mainWnd, appletName, appletCount);
  pBaseAppletMenu = std::tr1::shared_ptr<BaseAppletMenu>(new BaseAppletMenu(mainWnd, mainInst, appletName, allowMultipleInstances));
  pBaseAppletMenu->Initialize();

  // Register to recieve the specified Emerge Desktop messages
  PostMessage(ELGetCoreWindow(), EMERGE_REGISTER, (WPARAM)mainWnd, (LPARAM)EMERGE_CORE);

  return 1;
}

bool BaseApplet::SpawnInstance()
{
  appletCount = ReadAppletCount(-1) + 1;
  if (appletCount > 0)
    swprintf(appletName, TEXT("%s%d"), appletName, appletCount);

  std::wstring tempSettingsFile;
  tempSettingsFile = TEXT("%ThemeDir%\\");
  tempSettingsFile += appletName;
  tempSettingsFile += TEXT(".xml");
  tempSettingsFile = ELExpandVars(tempSettingsFile);
  if ((appletCount != 0) && (!ELPathFileExists(tempSettingsFile.c_str())))
    return false;

  WriteAppletCount(appletCount);

  WCHAR appletPath[MAX_PATH];
  if (GetModuleFileName(0, appletPath, MAX_PATH))
    ELExecute(appletPath);

  return true;
}

LRESULT BaseApplet::DoMoving(HWND hwnd, RECT *lpRect)
{
  SNAPMOVEINFO smi;

  if (pBaseSettings->GetSnapMove())
    {
      smi.AppletWindow = hwnd;
      smi.origin = anchor;
      smi.AppletRect = lpRect;

      ELSnapMove(&smi);
    }

  return 0;
}

LRESULT BaseApplet::DoEnterSizeMove(HWND hwnd)
{
  ELGetWindowRect(hwnd, &referenceRect);
  anchor = ELGetAnchorPoint(hwnd);

  SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
               SWP_NOSIZE|SWP_NOMOVE|SWP_NOSENDCHANGING);

  return 0;
}

LRESULT BaseApplet::DoExitSizeMove(HWND hwnd)
{
  RECT currentRect;
  int currentHeight, currentWidth, referenceHeight, referenceWidth;

  ELGetWindowRect(hwnd, &currentRect);

  currentWidth = currentRect.right - currentRect.left;
  currentHeight = currentRect.bottom - currentRect.top;
  referenceWidth = referenceRect.right - referenceRect.left;
  referenceHeight = referenceRect.bottom - referenceRect.top;

  if ((currentRect.left != referenceRect.left) || (currentRect.top != referenceRect.top))
    DoMove();

  if ((currentWidth != referenceWidth) || (currentHeight != referenceHeight))
    DoSize(currentWidth, currentHeight);

  if (_wcsicmp(pBaseSettings->GetZPosition(), TEXT("Top")) == 0)
    SetWindowPos(mainWnd, HWND_TOPMOST, 0 , 0, 0, 0,
                 SWP_NOSIZE|SWP_NOMOVE|SWP_NOSENDCHANGING);
  else
    {
      SetWindowPos(mainWnd, HWND_NOTOPMOST, 0 , 0, 0, 0,
                   SWP_NOSIZE|SWP_NOMOVE|SWP_NOSENDCHANGING);
      if (_wcsicmp(pBaseSettings->GetZPosition(), TEXT("Bottom")) == 0)
        SetWindowPos(mainWnd, ELGetDesktopWindow(), 0 , 0, 0, 0,
                     SWP_NOSIZE|SWP_NOMOVE|SWP_NOSENDCHANGING);
    }


  pBaseSettings->WriteSettings();

  return 0;
}

void BaseApplet::UpdateGUI(WCHAR *styleFile)
{
  int dragBorder;
  HWND hWndInsertAfter;
  RECT wndRect;
  UINT SWPFlags = 0;

  pBaseSettings->ReadSettings();
  if (styleFile == NULL)
    styleFile = pBaseSettings->GetStyleFile();
  ESELoadStyle(styleFile, &guiInfo);

  if (pBaseSettings->GetClickThrough() == 2)
    guiInfo.alphaBackground = 0;

  dragBorder = guiInfo.dragBorder + guiInfo.bevelWidth + guiInfo.padding;

  // Hide the window to allow the shadow effect to toggle
  ShowWindow(mainWnd, SW_HIDE);

  hWndInsertAfter = EAEUpdateGUI(mainWnd, guiInfo.windowShadow, pBaseSettings->GetZPosition());

  if (firstRun)
    {
      firstRun = false;
      appletHidden = pBaseSettings->GetStartHidden();
    }

  AppletUpdate(); // Call any applet specific update

  if (pBaseSettings->GetAutoSize())
    {
      wndRect.top = pBaseSettings->GetY();
      wndRect.bottom = wndRect.top;
      wndRect.left = pBaseSettings->GetX();
      wndRect.right = wndRect.left;
      AdjustRect(&wndRect);
      if ((GetVisibleIconCount() > 0) && !appletHidden && !fullScreen)
        SWPFlags |= SWP_SHOWWINDOW;
    }
  else
    {
      if (_wcsicmp(pBaseSettings->GetVerticalDirection(), TEXT("up")) == 0)
        wndRect.top = pBaseSettings->GetY() - (pBaseSettings->GetHeight() + (2 * dragBorder));
      else if (_wcsicmp(pBaseSettings->GetVerticalDirection(), TEXT("center")) == 0)
        wndRect.top = pBaseSettings->GetY() - ((pBaseSettings->GetHeight() + (2 * dragBorder)) / 2);
      else
        wndRect.top = pBaseSettings->GetY();

      if (_wcsicmp(pBaseSettings->GetHorizontalDirection(), TEXT("left")) == 0)
        wndRect.left = pBaseSettings->GetX() - (pBaseSettings->GetWidth() + (2 * dragBorder));
      else if (_wcsicmp(pBaseSettings->GetHorizontalDirection(), TEXT("center")) == 0)
        wndRect.left = pBaseSettings->GetX() - ((pBaseSettings->GetWidth() + (2 * dragBorder)) / 2);
      else
        wndRect.left = pBaseSettings->GetX();

      wndRect.bottom = wndRect.top + pBaseSettings->GetHeight() + (2 * dragBorder);
      wndRect.right = wndRect.left + pBaseSettings->GetWidth() + (2 * dragBorder);

      if (!appletHidden && !fullScreen)
        SWPFlags |= SWP_SHOWWINDOW;
    }

  // Seems like Windows 7 has an issue setting HWND_TOPMOST unless the window
  // is the active window
  ELStealFocus(mainWnd);
  SetWindowPos(mainWnd, hWndInsertAfter, wndRect.left, wndRect.top,
               wndRect.right - wndRect.left, wndRect.bottom - wndRect.top, SWPFlags);

  if (pBaseAppletMenu)
    pBaseAppletMenu->UpdateHook(guiInfo.alphaMenu);

  ZeroMemory(&oldrt, sizeof(RECT));

  DrawAlphaBlend();
}

void BaseApplet::AppletUpdate()
{
  return;
}

void BaseApplet::Activate()
{
  return;
}

void BaseApplet::AdjustRect(RECT *wndRect)
{
  AUTOSIZEINFO autoSizeInfo;

  ZeroMemory(&autoSizeInfo, sizeof(AUTOSIZEINFO));

  autoSizeInfo.hwnd = mainWnd;
  autoSizeInfo.rect = wndRect;
  if (wcslen(pBaseSettings->GetTitleBarText()) > 0)
    {
      HFONT mainFont = CreateFontIndirect(pBaseSettings->GetTitleBarFont());
      EGGetTextRect(pBaseSettings->GetTitleBarText(), mainFont, &autoSizeInfo.titleBarRect, 0);
      if (mainFont != NULL)
        DeleteObject(mainFont);
    }
  autoSizeInfo.dragBorder = guiInfo.dragBorder + guiInfo.bevelWidth + guiInfo.padding;
  autoSizeInfo.iconSize = pBaseSettings->GetIconSize();
  autoSizeInfo.iconSpacing = pBaseSettings->GetIconSpacing();
  autoSizeInfo.visibleIconCount = (UINT)GetVisibleIconCount();

  if (_wcsicmp(pBaseSettings->GetDirectionOrientation(), TEXT("vertical")) == 0)
    autoSizeInfo.orientation = ASI_VERTICAL;

  if (_wcsicmp(pBaseSettings->GetVerticalDirection(), TEXT("up")) == 0)
    autoSizeInfo.verticalDirection = ASI_UP;
  else if (_wcsicmp(pBaseSettings->GetVerticalDirection(), TEXT("center")) == 0)
    autoSizeInfo.verticalDirection = ASI_CENTER;

  if (_wcsicmp(pBaseSettings->GetHorizontalDirection(), TEXT("left")) == 0)
    autoSizeInfo.horizontalDirection = ASI_LEFT;
  else if (_wcsicmp(pBaseSettings->GetHorizontalDirection(), TEXT("center")) == 0)
    autoSizeInfo.horizontalDirection = ASI_MIDDLE;

  EAEAutoSize(autoSizeInfo);
}

LRESULT BaseApplet::DoWindowPosChanging(WINDOWPOS *windowPos)
{
  if (_wcsicmp(pBaseSettings->GetZPosition(), TEXT("bottom")) == 0)
    {
      windowPos->flags |= SWP_NOACTIVATE;
      windowPos->flags &= ~SWP_NOZORDER;
      windowPos->hwndInsertAfter = ELGetDesktopWindow();
    }

  return 0;
}

LRESULT BaseApplet::DoMove()
{
  if (!pBaseSettings->SetPosition())
    return 1;

  pBaseSettings->SetAppletMonitor(ELGetAppletMonitor(mainWnd));

  return 0;
}

LRESULT BaseApplet::DoSizing(HWND hwnd, UINT edge, LPRECT rect)
{
  int height;
  int width;
  int minSize = 2 * (guiInfo.dragBorder + guiInfo.bevelWidth + guiInfo.padding) + pBaseSettings->GetIconSize();
  if (minSize < MIN_APPLET_SIZE)
    minSize = MIN_APPLET_SIZE;

  height = (*rect).bottom - (*rect).top;
  width = (*rect).right - (*rect).left;

  if (!pBaseSettings->GetAutoSize() && pBaseSettings->GetSnapSize())
    {
      SNAPSIZEINFO ssi;
      ssi.AppletWindow = hwnd;
      ssi.AppletEdge = edge;
      ssi.AppletRect = rect;

      ELSnapSize(&ssi);
    }

  if ((edge == WMSZ_TOPRIGHT) || (edge == WMSZ_TOP) || (edge == WMSZ_RIGHT))
    {
      if (height < minSize)
        (*rect).top = (*rect).bottom - minSize;

      if (width < minSize)
        (*rect).right = (*rect).left + minSize;
    }

  if ((edge == WMSZ_BOTTOMRIGHT) || (edge == WMSZ_BOTTOM))
    {
      if (height < minSize)
        (*rect).bottom = (*rect).top + minSize;

      if (width < minSize)
        (*rect).right = (*rect).left + minSize;
    }

  if ((edge == WMSZ_TOPLEFT) || (edge == WMSZ_LEFT))
    {
      if (height < minSize)
        (*rect).top = (*rect).bottom - minSize;

      if (width < minSize)
        (*rect).left = (*rect).right - minSize;
    }

  if (edge == WMSZ_BOTTOMLEFT)
    {
      if (height < minSize)
        (*rect).bottom = (*rect).top + minSize;

      if (width < minSize)
        (*rect).left = (*rect).right - minSize;
    }

  DrawAlphaBlend();

  return 0;
}

LRESULT BaseApplet::DoSize(DWORD width, DWORD height)
{
  int dragBorder = guiInfo.dragBorder + guiInfo.bevelWidth + guiInfo.padding;

  if (!IsWindowVisible(mainWnd))
    return 1;

  if (pBaseSettings->GetAutoSize())
    return 1;

  pBaseSettings->SetSize(width - (2 * dragBorder), height - (2 * dragBorder));

  return 0;
}

DWORD WINAPI BaseApplet::DisplayChangeThreadProc(LPVOID lpParameter)
{
  // reinterpret lpParameter as a BaseApplet*
  BaseApplet *pBaseApplet = reinterpret_cast< BaseApplet* >(lpParameter);

  // Pause the thread for 500 ms to mitigate an fullscreen app changing the
  // display resolution prior to taking full screen
  WaitForSingleObject(GetCurrentThread(), DISPLAYCHANGE_WAIT_TIME);

  // If not and Dynamic Positioning is enabled, adjust the applet position
  if (pBaseApplet->pBaseSettings->GetDynamicPositioning() && !pBaseApplet->GetFullScreen())
    pBaseApplet->UpdateGUI();

  return 0;
}

LRESULT BaseApplet::DoDisplayChange(HWND hwnd UNUSED)
{
  DWORD threadID, threadState;

  GetExitCodeThread(displayChangeThread, &threadState);
  if (threadState != STILL_ACTIVE)
    displayChangeThread = CreateThread(NULL, 0, DisplayChangeThreadProc, this, 0, &threadID);

  return 0;
}

bool BaseApplet::ClickThrough()
{
  if (ELIsKeyDown(VK_SHIFT))
    return false;

  if (pBaseSettings->GetClickThrough() != 1)
    return false;

  return true;
}

LRESULT BaseApplet::DoNCLButtonDown()
{
  if (ELIsKeyDown(VK_CONTROL))
    SetCursor(LoadCursor(NULL, IDC_SIZEALL));

  return 0;
}

LRESULT BaseApplet::DoNCLButtonUp()
{
  SetCursor(LoadCursor(NULL, IDC_ARROW));

  return 0;
}

void BaseApplet::DrawAlphaBlend()
{
  HDC hdc;
  RECT clientrt, contentrt, customcontentrt;
  POINT srcPt;
  SIZE wndSz;
  BLENDFUNCTION bf;
  int dragBorder = guiInfo.dragBorder + guiInfo.bevelWidth + guiInfo.padding;

  if (!IsWindowVisible(mainWnd))
    return;

  if (!GetClientRect(mainWnd, &clientrt))
    return;

  hdc = EGBeginPaint(mainWnd);

  if (!EqualRect(&clientrt, &oldrt))
    {
      CopyRect(&oldrt, &clientrt);
      if (activeBackgroundDC != NULL)
        DeleteDC(activeBackgroundDC);
      if (inactiveBackgroundDC != NULL)
        DeleteDC(inactiveBackgroundDC);
      activeBackgroundDC = ESEPaintBackground(clientrt, &guiInfo, true);
      inactiveBackgroundDC = ESEPaintBackground(clientrt, &guiInfo, false);

      if (EGIsCompositionEnabled())
        EGBlurWindow(mainWnd, guiInfo.windowBlur);
    }

  if (mouseOver)
    BitBlt(hdc, clientrt.left, clientrt.top, clientrt.right - clientrt.left, clientrt.bottom - clientrt.top,
           activeBackgroundDC, 0, 0, SRCCOPY);
  else
    BitBlt(hdc, clientrt.left, clientrt.top, clientrt.right - clientrt.left, clientrt.bottom - clientrt.top,
           inactiveBackgroundDC, 0, 0, SRCCOPY);

  if (wcslen(pBaseSettings->GetTitleBarText()) > 0)
    {
      HFONT mainFont = CreateFontIndirect(pBaseSettings->GetTitleBarFont());
      RECT titleTextSizingRect;
      EGGetTextRect(pBaseSettings->GetTitleBarText(), mainFont, &titleTextSizingRect, 0);
      clientrt.top = clientrt.top + (titleTextSizingRect.bottom - titleTextSizingRect.top);
      if (mainFont != NULL)
        DeleteObject(mainFont);
    }

  CopyRect(&contentrt, &clientrt);

  //allow the applet to change the content area (so it can draw something in an area guaranteed to be empty)
  CopyRect(&customcontentrt, &contentrt);
  AdjustContentRect(&customcontentrt);
  if ((!IsRectEmpty(&customcontentrt)) && (!EqualRect(&customcontentrt, &contentrt)))
    CopyRect(&contentrt, &customcontentrt);

  InflateRect(&contentrt, -dragBorder, -dragBorder);
  PaintContent(hdc, contentrt); // Call the applet content paint routine

  ZeroMemory(&bf, sizeof(BLENDFUNCTION));
  bf.BlendOp = AC_SRC_OVER;
  if (guiInfo.alphaBackground < 0xff)
    bf.AlphaFormat = AC_SRC_ALPHA;  // use source alpha
  if (mouseOver)
    bf.SourceConstantAlpha = guiInfo.alphaActive;
  else
    bf.SourceConstantAlpha = guiInfo.alphaInactive;

  wndSz.cx = clientrt.right;
  wndSz.cy = clientrt.bottom;
  srcPt.x = 0;
  srcPt.y = 0;

  UpdateLayeredWindow(mainWnd, NULL, NULL, &wndSz, hdc, &srcPt, 0, &bf, ULW_ALPHA);

  // do cleanup
  EGEndPaint();
  DeleteDC(hdc);
}

void BaseApplet::AdjustContentRect(LPRECT contentRect UNUSED)
{
}

LRESULT BaseApplet::PaintContent(HDC hdc, RECT clientrt)
{
  UINT i;
  RECT rect;
  int x, y;
  int xdefault, ydefault;
  int width = clientrt.right - clientrt.left;
  int height = clientrt.bottom - clientrt.top;
  size_t maxHorizontal = (width + pBaseSettings->GetIconSpacing()) / (pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing());
  size_t maxVertical = (height + pBaseSettings->GetIconSpacing()) / (pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing());
  size_t multiplier, paintedIcons = 0;
  size_t visibleIconCount = GetVisibleIconCount();

  if ((visibleIconCount == 0) || (maxHorizontal == 0) || (maxVertical == 0))
    return 0;

  if (_wcsicmp(pBaseSettings->GetHorizontalDirection(), TEXT("left")) == 0)
    xdefault = clientrt.right - pBaseSettings->GetIconSize();
  else if (_wcsicmp(pBaseSettings->GetHorizontalDirection(), TEXT("center")) == 0)
    {
      xdefault = clientrt.left;
      xdefault += (width / 2);

      if (_wcsicmp(pBaseSettings->GetDirectionOrientation(), TEXT("horizontal")) == 0)
        {
          if (visibleIconCount < maxHorizontal)
            multiplier = visibleIconCount;
          else
            multiplier = maxHorizontal;
        }
      else
        {
          if (visibleIconCount == maxVertical)
            multiplier = 1;
          else
            multiplier = visibleIconCount / maxVertical + 1;
        }

      xdefault -= ((int)multiplier * (int)pBaseSettings->GetIconSize()) / 2;

      if (maxHorizontal > 1)
        {
          xdefault -= ((int)multiplier * (int)pBaseSettings->GetIconSpacing()) / 2;
          xdefault += (int)pBaseSettings->GetIconSpacing();
        }
    }
  else
    xdefault = clientrt.left;

  x = xdefault;

  if (_wcsicmp(pBaseSettings->GetVerticalDirection(), TEXT("up")) == 0)
    ydefault = clientrt.bottom - pBaseSettings->GetIconSize();
  else if (_wcsicmp(pBaseSettings->GetVerticalDirection(), TEXT("center")) == 0)
    {
      ydefault = clientrt.top;
      ydefault += (height / 2);

      if (_wcsicmp(pBaseSettings->GetDirectionOrientation(), TEXT("vertical")) == 0)
        {
          if (visibleIconCount < maxVertical)
            multiplier = visibleIconCount;
          else
            multiplier = maxVertical;
        }
      else
        {
          if (visibleIconCount == maxHorizontal)
            multiplier = 1;
          else
            multiplier = visibleIconCount / maxHorizontal + 1;
        }

      ydefault -= ((int)multiplier * (int)pBaseSettings->GetIconSize()) / 2;

      if (maxVertical > 1)
        {
          ydefault -= ((int)multiplier * (int)pBaseSettings->GetIconSpacing()) / 2;
          ydefault += (int)pBaseSettings->GetIconSpacing();
        }
    }
  else
    ydefault = clientrt.top;

  y = ydefault;

  // Traverse the valid icon vector, painting each icon
  for (i = 0; i < GetIconCount(); i++)
    {
      // Update the bounding rectangle of the icon
      rect.left = x;
      rect.right = rect.left + pBaseSettings->GetIconSize();
      rect.top = y;
      rect.bottom = rect.top + pBaseSettings->GetIconSize();

      if (!PaintItem(hdc, i, x, y, rect))
        continue;
      else
        paintedIcons++;

      if (_wcsicmp(pBaseSettings->GetDirectionOrientation(), TEXT("vertical")) == 0)
        {
          // Set X and Y for the next icon
          if (_wcsicmp(pBaseSettings->GetVerticalDirection(), TEXT("up")) == 0)
            {
              y -= pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();

              if (y < clientrt.top)
                {
                  y = ydefault;

                  if (_wcsicmp(pBaseSettings->GetHorizontalDirection(), TEXT("left")) == 0)
                    x -= pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();
                  else
                    x += pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();
                }
            }
          else if (_wcsicmp(pBaseSettings->GetVerticalDirection(), TEXT("center")) == 0)
            {
              y += pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();

              if (paintedIcons % maxVertical == 0)
                {
                  multiplier = visibleIconCount - paintedIcons;
                  if (multiplier > maxVertical)
                    multiplier = maxVertical;
                  y = height / 2;
                  y -= ((int)multiplier * (int)pBaseSettings->GetIconSize()) / 2;
                  y -= ((int)multiplier * (int)pBaseSettings->GetIconSpacing()) / 2;

                  if (_wcsicmp(pBaseSettings->GetHorizontalDirection(), TEXT("left")) == 0)
                    x -= pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();
                  else
                    x += pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();
                }
            }
          else
            {
              y += pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();

              if (y > (clientrt.bottom - (int)pBaseSettings->GetIconSize()))
                {
                  y = ydefault;

                  if (_wcsicmp(pBaseSettings->GetHorizontalDirection(), TEXT("left")) == 0)
                    x -= pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();
                  else
                    x += pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();
                }
            }
        }
      else
        {
          // Set X and Y for the next icon
          if (_wcsicmp(pBaseSettings->GetHorizontalDirection(), TEXT("left")) == 0)
            {
              x -= pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();

              if (x < clientrt.left)
                {
                  x = xdefault;

                  if (_wcsicmp(pBaseSettings->GetVerticalDirection(), TEXT("up")) == 0)
                    y -= pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();
                  else
                    y += pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();
                }
            }
          else if (_wcsicmp(pBaseSettings->GetHorizontalDirection(), TEXT("center")) == 0)
            {
              x += pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();

              if (paintedIcons % maxHorizontal == 0)
                {
                  multiplier = visibleIconCount - paintedIcons;
                  if (multiplier > maxHorizontal)
                    multiplier = maxHorizontal;
                  x = width / 2;
                  x -= ((int)multiplier * (int)pBaseSettings->GetIconSize()) / 2;
                  x -= ((int)multiplier * (int)pBaseSettings->GetIconSpacing()) / 2;

                  if (_wcsicmp(pBaseSettings->GetVerticalDirection(), TEXT("up")) == 0)
                    y -= pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();
                  else
                    y += pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();
                }
            }
          else
            {
              x += pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();

              if (x > (clientrt.right - (int)pBaseSettings->GetIconSize()))
                {
                  x = xdefault;

                  if (_wcsicmp(pBaseSettings->GetVerticalDirection(), TEXT("up")) == 0)
                    y -= pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();
                  else
                    y += pBaseSettings->GetIconSize() + pBaseSettings->GetIconSpacing();
                }
            }
        }
    }

  if (wcslen(pBaseSettings->GetTitleBarText()) > 0)
    {
      CLIENTINFO clientInfo;
      FORMATINFO formatInfo;
      RECT titleTextSizingRect;
      HFONT mainFont = CreateFontIndirect(pBaseSettings->GetTitleBarFont());

      EGGetTextRect(pBaseSettings->GetTitleBarText(), mainFont, &titleTextSizingRect, 0);
      RECT titleBarRect = {0, 0, clientrt.right, (titleTextSizingRect.bottom - titleTextSizingRect.top)};

      formatInfo.horizontalAlignment = EGDAT_HCENTER;
      formatInfo.verticalAlignment = EGDAT_VCENTER;

      formatInfo.font = mainFont;
      formatInfo.color = guiInfo.colorFont;
      formatInfo.flags = 0;

      clientInfo.hdc = hdc;
      CopyRect(&clientInfo.rt, &titleBarRect);
      clientInfo.bgAlpha = guiInfo.alphaBackground;

      EGDrawAlphaText(guiInfo.alphaText, clientInfo, formatInfo, pBaseSettings->GetTitleBarText());

      if (mainFont != NULL)
        DeleteObject(mainFont);
    }

  return 0;
}

bool BaseApplet::PaintItem(HDC hdc UNUSED, UINT index UNUSED, int x UNUSED, int y UNUSED, RECT rect UNUSED)
{
  return true;
}

LRESULT BaseApplet::DoSetCursor()
{
  if (!mouseOver)
    {
      mouseOver = true;
      DrawAlphaBlend();
      SetTimer(mainWnd, MOUSE_TIMER, MOUSE_POLL_TIME, NULL);
    }

  return 0;
}

LRESULT BaseApplet::DoCopyData(COPYDATASTRUCT *cds)
{
  if (cds->dwData == EMERGE_MESSAGE)
    {
      std::wstring theme = reinterpret_cast<WCHAR*>(cds->lpData);
      SetEnvironmentVariable(TEXT("ThemeDir"), theme.c_str());
      return 1;
    }

  if ((cds->dwData == EMERGE_NOTIFY) && (cds->cbData == sizeof(NOTIFYINFO)))
    {
      LPNOTIFYINFO notifyInfo = reinterpret_cast<LPNOTIFYINFO>(cds->lpData);

      if ((notifyInfo->Type & EMERGE_CORE) == EMERGE_CORE)
        {
          switch (notifyInfo->Message)
            {
            case CORE_ACTIVATE:
              if ((notifyInfo->InstanceName != NULL) && wcslen(notifyInfo->InstanceName))
                {
                  if (_wcsicmp(notifyInfo->InstanceName, appletName) == 0)
                    Activate();
                }
              break;

            case CORE_SHOWAPPLET:
            {
              bool toggle = IsWindowVisible(mainWnd);
              if ((notifyInfo->InstanceName != NULL) && wcslen(notifyInfo->InstanceName))
                {
                  if (_wcsicmp(notifyInfo->InstanceName, appletName) == 0)
                    HideApplet(toggle);
                }
              else
                HideApplet(toggle);
            }
            break;

            case CORE_HIDE:
              if ((notifyInfo->InstanceName != NULL) && wcslen(notifyInfo->InstanceName))
                {
                  if (_wcsicmp(notifyInfo->InstanceName, appletName) == 0)
                    HideApplet(true);
                }
              else
                HideApplet(true);
              break;

            case CORE_SHOW:
              if ((notifyInfo->InstanceName != NULL) && wcslen(notifyInfo->InstanceName))
                {
                  if (_wcsicmp(notifyInfo->InstanceName, appletName) == 0)
                    HideApplet(false);
                }
              else
                HideApplet(false);
              break;

            case CORE_REFRESH:
              if ((notifyInfo->InstanceName != NULL) && wcslen(notifyInfo->InstanceName))
                {
                  if (_wcsicmp(notifyInfo->InstanceName, appletName) == 0)
                    {
                      ZeroMemory(&oldrt, sizeof(RECT));
                      UpdateGUI(ESEGetStyle());
                    }
                }
              else
                {
                  ZeroMemory(&oldrt, sizeof(RECT));
                  UpdateGUI(ESEGetStyle());
                }
              break;

            case CORE_REPOSITION:
            {
              HWND hwndInsertBehind = NULL;
              if (_wcsicmp(pBaseSettings->GetZPosition(), TEXT("top")) != 0)
                hwndInsertBehind = ELGetDesktopWindow();
              SetWindowPos(mainWnd, hwndInsertBehind, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
            break;

            case CORE_SHOWCONFIG:
              if ((notifyInfo->InstanceName != NULL) && wcslen(notifyInfo->InstanceName))
                {
                  if (_wcsicmp(notifyInfo->InstanceName, appletName) == 0)
                    ShowConfig();
                }
              break;

            case CORE_RECONFIGURE:
              if (allowMultipleInstances)
                {
                  // Kill any instances other than the initial instance.
                  if (appletCount > 0)
                    PostQuitMessage(0);
                  else
                    {
                      UpdateGUI(); // Update initial instance
                      WriteAppletCount(0, false); // Reset the Applet Count
                      WCHAR appletPath[MAX_PATH]; // Execute the next instance
                      if (GetModuleFileName(0, appletPath, MAX_PATH))
                        ELExecute(appletPath);
                    }
                }
              else
                UpdateGUI();
              break;

            case CORE_WRITESETTINGS:
              pBaseSettings->SetModified();
              pBaseSettings->WriteSettings();
              break;
            }

          return 1;
        }
    }

  return 0;
}

LRESULT BaseApplet::DoTimer(UINT_PTR timerID)
{
  POINT pt;
  RECT rc;

  if (timerID == MOUSE_TIMER)
    {
      GetCursorPos(&pt);
      ELGetWindowRect(mainWnd, &rc);
      if (!PtInRect(&rc, pt))
        {
          mouseOver = false;
          DrawAlphaBlend();
          KillTimer(mainWnd, MOUSE_TIMER);
        }

      // Make sure to clear the transparent style if set
      LONG_PTR windowStyle = GetWindowLongPtr(mainWnd, GWL_EXSTYLE);
      if ((!mouseOver && ((windowStyle & WS_EX_TRANSPARENT) == WS_EX_TRANSPARENT)) || ELIsKeyDown(VK_SHIFT))
        SetWindowLongPtr(mainWnd, GWL_EXSTYLE, windowStyle & ~WS_EX_TRANSPARENT);

      return 0;
    }

  return 1;
}

LRESULT BaseApplet::DoNCRButtonUp()
{
  POINT pt;
  WCHAR styleFile[MAX_LINE_LENGTH];

  WCHAR strAppletCount[sizeof(int) + sizeof(WCHAR)];
  std::wstring tempSettingsFile;
  int tempAppletCount;


  GetCursorPos(&pt);
  switch (pBaseAppletMenu->ActivateMenu(pt.x, pt.y, styleFile))
    {
    case EBC_LOADSTYLE:
      pBaseSettings->SetStyleFile(styleFile);
      pBaseSettings->WriteSettings();
      UpdateGUI();
      break;

    case EBC_RELOADSTYLE:
      UpdateGUI();
      break;

    case EBC_CONFIGURE:
      ShowConfig();
      break;

    case EBC_NEWINSTANCE:
      // Set the theme to modified
      pBaseSettings->SetModified();
      pBaseSettings->WriteSettings();

      tempAppletCount = ReadAppletCount(-1) + 1;
      swprintf(strAppletCount, TEXT("%d"), tempAppletCount);

      tempSettingsFile = TEXT("%ThemeDir%\\");
      tempSettingsFile += baseAppletName;
      tempSettingsFile += strAppletCount;
      tempSettingsFile += TEXT(".xml");
      tempSettingsFile = ELExpandVars(tempSettingsFile);

      if (!ELPathFileExists(tempSettingsFile.c_str()))
        {
          //create a new settings file for the new applet instance
          HANDLE newSettingsFile = CreateFile(tempSettingsFile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
          CloseHandle(newSettingsFile);
        }

      WCHAR appletPath[MAX_PATH];
      if (GetModuleFileName(0, appletPath, MAX_PATH))
        ELExecute(appletPath);

      break;

    case EBC_DELETEINSTANCE:
      // Set the theme to modified
      pBaseSettings->SetModified();
      pBaseSettings->WriteSettings();

      tempSettingsFile = TEXT("%ThemeDir%\\");
      tempSettingsFile += baseAppletName;
      if (appletCount > 0)
        {
          swprintf(strAppletCount, TEXT("%d"), appletCount);
          tempSettingsFile += strAppletCount;
        }
      tempSettingsFile += TEXT(".xml");
      tempSettingsFile = ELExpandVars(tempSettingsFile);

      if (ELPathFileExists(tempSettingsFile.c_str()))
        DeleteFile(tempSettingsFile.c_str());
      PostQuitMessage(0);
    }

  return 0;
}

void BaseApplet::ShowConfig()
{
  return;
}

LRESULT BaseApplet::DoSysColorChange()
{
  UpdateGUI();
  return 0;
}

LRESULT BaseApplet::DoHitTest(LPARAM lParam UNUSED)
{
  LONG_PTR windowStyle = GetWindowLongPtr(mainWnd, GWL_EXSTYLE);
  if (!ClickThrough())
    return EAEHitTest(mainWnd, guiInfo.dragBorder, pBaseSettings->GetAutoSize(),
                      LOWORD(lParam), HIWORD(lParam));
  else
    SetWindowLongPtr(mainWnd, GWL_EXSTYLE, windowStyle | WS_EX_TRANSPARENT);

  return 0;
}

HWND BaseApplet::GetMainWnd()
{
  return mainWnd;
}

void BaseApplet::HideApplet(bool hide)
{
  if (hide)
    {
      if (IsWindowVisible(mainWnd))
        {
          //appletHidden = true;
          SetWindowPos(mainWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER |
                       SWP_NOACTIVATE | SWP_HIDEWINDOW);
        }
    }
  else
    {
      if (!IsWindowVisible(mainWnd))
        {
          //appletHidden = false;
          SetWindowPos(mainWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER |
                       SWP_NOACTIVATE | SWP_SHOWWINDOW);
          // If there is not active or inactive background, create one
          if (!activeBackgroundDC || !inactiveBackgroundDC)
            DrawAlphaBlend();
        }
    }

  appletHidden = hide;
}

WCHAR *BaseApplet::GetInstanceName()
{
  return appletName;
}

DWORD WINAPI BaseApplet::FullScreenThreadProc(LPVOID lpParameter)
{
  // reinterpret lpParameter as a BaseApplet*
  BaseApplet *pBaseApplet = reinterpret_cast< BaseApplet* >(lpParameter);

  // loop infinitely
  while (true)
    {
      // Pause the current thread for FULLSCREEN_POLL_TIME
      WaitForSingleObject(GetCurrentThread(), FULLSCREEN_WAIT_TIME);

      // Check if the current foreground window is full screen...
      if (ELIsFullScreen(pBaseApplet->GetMainWnd(), GetForegroundWindow()))
        // if so set fullscreen to true...
        pBaseApplet->SetFullScreen(true);
      // If not and in fullscreen mode...
      else if (pBaseApplet->GetFullScreen())
        // set fullscreen false...
        pBaseApplet->SetFullScreen(false);
      else
        // Fail safe to kill runaway threads
        ExitThread(0);
    }

  return 0;
}

LRESULT BaseApplet::DoSysCommand(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (wParam)
    {
    case SC_CLOSE:
    case SC_MAXIMIZE:
    case SC_MINIMIZE:
      break;
    default:
      return DefWindowProc(hwnd, message, wParam, lParam);
    }
  return 0;
}

void BaseApplet::SetFullScreen(bool value)
{
  if (value)
    {
      if (IsWindowVisible(mainWnd))
        {
          SetWindowPos(mainWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER |
                       SWP_NOACTIVATE | SWP_HIDEWINDOW);
        }
    }
  else
    {
      if (!IsWindowVisible(mainWnd) && !appletHidden)
        {
          SetWindowPos(mainWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER |
                       SWP_NOACTIVATE | SWP_SHOWWINDOW);
          // If there is not active or inactive background, create one
          if (!activeBackgroundDC || !inactiveBackgroundDC)
            DrawAlphaBlend();
        }
    }

  fullScreen = value;
}

bool BaseApplet::GetFullScreen()
{
  return fullScreen;
}

LRESULT BaseApplet::DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  UINT shellMessage = (UINT)wParam;
  DWORD threadID, threadState;

  if (message == ShellMessage)
    {
      switch (shellMessage)
        {
          // A "task" was activated
        case HSHELL_RUDEAPPACTIVATED:
          // Kill the display change thread
          TerminateThread(displayChangeThread, 0);
          // Create the fullscreen thread
          GetExitCodeThread(fullScreenThread, &threadState);
          if (threadState != STILL_ACTIVE)
            fullScreenThread = CreateThread(NULL, 0, FullScreenThreadProc, this, 0, &threadID);
          return 1;

        case HSHELL_WINDOWACTIVATED:
          // Kill the fullscreen thread
          TerminateThread(fullScreenThread, 0);
          // If in fullscreen mode...
          if (fullScreen)
            // and show the applet
            SetFullScreen(false);
          return 1;

        case HSHELL_WINDOWCREATED:
          if (_wcsicmp(pBaseSettings->GetZPosition(), TEXT("Top")) == 0)
            SetWindowPos(mainWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
          else if (_wcsicmp(pBaseSettings->GetZPosition(), TEXT("Bottom")) == 0)
            SetWindowPos(mainWnd, ELGetDesktopWindow(), 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
          return 1;

        default:
          return 0;
        }
    }

  return DefWindowProc(hwnd, message, wParam, lParam);
}

size_t BaseApplet::GetVisibleIconCount()
{
  return GetIconCount();
}

size_t BaseApplet::GetIconCount()
{
  return 0;
}

int BaseApplet::ReadAppletCount(int defaultValue)
{
  int tempAppletCount = defaultValue;
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section = NULL;

  // Take control of the InstanceManagement.xml Mutex
  WaitForSingleObject(instanceManagementMutex, INFINITE);

  instanceManagementPath = ELExpandVars(instanceManagementPath);

  if (ELPathFileExists(instanceManagementPath.c_str()))
    {
      configXML = ELOpenXMLConfig(instanceManagementPath, false);
      if (configXML)
        {
          section = ELGetXMLSection(configXML.get(), baseAppletName, false);
          if (section)
            ELReadXMLIntValue(section, TEXT("AppletCount"), &tempAppletCount, defaultValue);
        }
    }

  // Release the InstanceManagement.xml Mutex
  ReleaseMutex(instanceManagementMutex);
  return tempAppletCount;
}

bool BaseApplet::WriteAppletCount(int value, bool forceCreate)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section = NULL;
  bool ret = false;

  // Take control of the InstanceManagement.xml Mutex
  WaitForSingleObject(instanceManagementMutex, INFINITE);

  instanceManagementPath = ELExpandVars(instanceManagementPath);

  configXML = ELOpenXMLConfig(instanceManagementPath, forceCreate);
  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), baseAppletName, forceCreate);
      if (section)
        {
          if (ELWriteXMLIntValue(section, TEXT("AppletCount"), value))
            {
              if (ELWriteXMLConfig(configXML.get()) == true)
                ret = true;
            }
        }
    }

  // Release the InstanceManagement.xml Mutex
  ReleaseMutex(instanceManagementMutex);
  return ret;
}

void BaseApplet::RenameSettingsFiles()
{
  std::wstring searchDirectory = ELExpandVars(TEXT("%ThemeDir%\\"));
  std::wstring searchFileName = searchDirectory;
  searchFileName += baseAppletName;
  searchFileName += TEXT("?.xml");
  WIN32_FIND_DATA fileInfo;
  HANDLE searchHandle;
  std::vector< std::wstring > fileList;
  UINT counter = 0;
  std::wstring srcFile;
  WCHAR dstFile[MAX_PATH];

  searchHandle = FindFirstFile(searchFileName.c_str(), &fileInfo);
  if (searchHandle == INVALID_HANDLE_VALUE)
    return;

  do
    {
      if ((wcscmp(fileInfo.cFileName, TEXT(".")) != 0) && (wcscmp(fileInfo.cFileName, TEXT("..")) != 0))
        fileList.push_back(fileInfo.cFileName);
    }
  while (FindNextFile(searchHandle, &fileInfo) != 0);
  FindClose(searchHandle);

  for (counter = 0; counter < fileList.size(); counter++)
    {
      srcFile = searchDirectory;
      srcFile += fileList[counter];
      swprintf(dstFile, TEXT("%s%s"), searchDirectory.c_str(), baseAppletName);
      if (counter)
        swprintf(dstFile, TEXT("%s%d"), dstFile, counter);
      swprintf(dstFile, TEXT("%s%s"), dstFile, TEXT(".xml"));

      MoveFile(srcFile.c_str(), dstFile);
    }

  fileList.clear();
}
