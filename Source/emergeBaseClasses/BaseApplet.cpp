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

BaseApplet *pBaseApplet = NULL;

BaseApplet::BaseApplet(HINSTANCE hInstance, WCHAR *appletName, bool allowAutoSize)
{
  mainInst = hInstance;
  wcscpy((*this).appletName, appletName);
  mouseOver = false;
  fullScreen = false;
  appletHidden = false;
  (*this).allowAutoSize = allowAutoSize;

  activeBackgroundDC = NULL;
  inactiveBackgroundDC = NULL;

  pBaseApplet = this;

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
                   appletName, ELMB_ICONERROR|ELMB_MODAL|ELMB_OK);
      return 0;
    }

  if (FAILED(OleInitialize(NULL)))
    {
      ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("COM initialization failed"), appletName,
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
      ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("Failed to register as a shell window"), appletName,
                   ELMB_ICONERROR|ELMB_OK|ELMB_MODAL);
      return 0;
    }

  pBaseSettings = pSettings;
  pBaseSettings->Init(mainWnd, appletName);
  pBaseAppletMenu = std::tr1::shared_ptr<BaseAppletMenu>(new BaseAppletMenu(mainWnd, mainInst, appletName));
  pBaseAppletMenu->Initialize();

  // Register to recieve the specified Emerge Desktop messages
  PostMessage(ELGetCoreWindow(), EMERGE_REGISTER, (WPARAM)mainWnd, (LPARAM)EMERGE_CORE);

  return 1;
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
  GetWindowRect(hwnd, &referenceRect);
  anchor = ELGetAnchorPoint(hwnd);

  SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
               SWP_NOSIZE|SWP_NOMOVE|SWP_NOSENDCHANGING);

  return 0;
}

LRESULT BaseApplet::DoExitSizeMove(HWND hwnd)
{
  RECT currentRect;
  int currentHeight, currentWidth, referenceHeight, referenceWidth;

  GetWindowRect(hwnd, &currentRect);

  currentWidth = currentRect.right - currentRect.left;
  currentHeight = currentRect.bottom - currentRect.top;
  referenceWidth = referenceRect.right - referenceRect.left;
  referenceHeight = referenceRect.bottom - referenceRect.top;

  if ((currentRect.left != referenceRect.left) || (currentRect.top != referenceRect.top))
    DoMove();

  if ((currentWidth != referenceWidth) || (currentHeight != referenceHeight))
    DoSize(currentWidth, currentHeight);

  if (_wcsicmp(pBaseSettings->GetZPosition(), TEXT("Top")) != 0)
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
  UINT SWPFlags = SWP_FRAMECHANGED;

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

  AppletUpdate(); // Call any applet specific update

  if (pBaseSettings->GetAutoSize())
    {
      wndRect.top = pBaseSettings->GetY();
      wndRect.bottom = wndRect.top;
      wndRect.left = pBaseSettings->GetX();
      wndRect.right = wndRect.left;
      AdjustRect(&wndRect);
      if ((GetVisibleIconCount() > 0) && !appletHidden)
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

      if (!appletHidden)
        SWPFlags |= SWP_SHOWWINDOW;
    }

  // Set focus to mainWnd to fix the 'top' z-order issue
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

void BaseApplet::AdjustRect(RECT *wndRect)
{
  AUTOSIZEINFO autoSizeInfo;

  ZeroMemory(&autoSizeInfo, sizeof(AUTOSIZEINFO));

  autoSizeInfo.hwnd = mainWnd;
  autoSizeInfo.rect = wndRect;
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

LRESULT BaseApplet::DoDisplayChange(HWND hwnd)
{
  if (!ELIsFullScreen(hwnd, GetForegroundWindow()))
    {
      if (pBaseSettings->GetDynamicPositioning())
        UpdateGUI();
    }

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
  RECT clientrt, contentrt;
  POINT srcPt;
  SIZE wndSz;
  BLENDFUNCTION bf;
  int dragBorder = guiInfo.dragBorder + guiInfo.bevelWidth + guiInfo.padding;
  bool blurWindow = EGIsCompositionEnabled() && guiInfo.windowBlur;

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
    }

  if (mouseOver)
    BitBlt(hdc, clientrt.left, clientrt.top, clientrt.right - clientrt.left, clientrt.bottom - clientrt.top,
           activeBackgroundDC, 0, 0, SRCCOPY);
  else
    BitBlt(hdc, clientrt.left, clientrt.top, clientrt.right - clientrt.left, clientrt.bottom - clientrt.top,
           inactiveBackgroundDC, 0, 0, SRCCOPY);

  CopyRect(&contentrt, &clientrt);

  InflateRect(&contentrt, -dragBorder, -dragBorder);
  PaintContent(hdc, contentrt); // Call the applet content paint routine

  bf.BlendOp = AC_SRC_OVER;
  bf.BlendFlags = 0;
  bf.AlphaFormat = AC_SRC_ALPHA;  // use source alpha
  if (blurWindow)
    bf.SourceConstantAlpha = 255;
  else
    {
      if (mouseOver)
        bf.SourceConstantAlpha = guiInfo.alphaActive;
      else
        bf.SourceConstantAlpha = guiInfo.alphaInactive;
    }


  wndSz.cx = clientrt.right;
  wndSz.cy = clientrt.bottom;
  srcPt.x = 0;
  srcPt.y = 0;

  UpdateLayeredWindow(mainWnd, NULL, NULL, &wndSz, hdc, &srcPt, 0, &bf, ULW_ALPHA);

  if (blurWindow)
    EGBlurWindow(mainWnd);

  // do cleanup
  EGEndPaint();
  DeleteDC(hdc);
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
  std::wstring theme = reinterpret_cast<WCHAR*>(cds->lpData);

  if (cds->dwData == EMERGE_MESSAGE)
    {
      SetEnvironmentVariable(TEXT("ThemeDir"), theme.c_str());
      return 1;
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
      GetWindowRect(mainWnd, &rc);
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
    }

  return 0;
}

void BaseApplet::ShowConfig()
{
  return;
}

LRESULT BaseApplet::DoEmergeNotify(UINT messageClass, UINT message)
{
  if ((messageClass & EMERGE_CORE) == EMERGE_CORE)
    {
      switch (message)
        {
        case CORE_HIDE:
          if (!appletHidden)
            {
              appletHidden = true;
              SetWindowPos(mainWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER |
                           SWP_NOACTIVATE | SWP_HIDEWINDOW);
            }
          break;

        case CORE_SHOW:
          if (appletHidden)
            {
              appletHidden = false;
              SetWindowPos(mainWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER |
                           SWP_NOACTIVATE | SWP_SHOWWINDOW);
            }
          break;

        case CORE_REFRESH:
          ZeroMemory(&oldrt, sizeof(RECT));
          UpdateGUI(ESEGetStyle());
          break;

        case CORE_REPOSITION:
            {
              HWND hwndInsertBehind = NULL;
              if (_wcsicmp(pBaseSettings->GetZPosition(), TEXT("top")) != 0)
                hwndInsertBehind = ELGetDesktopWindow();
              SetWindowPos(mainWnd, hwndInsertBehind, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
          break;

        case CORE_RECONFIGURE:
          UpdateGUI();
          break;

        case CORE_WRITESETTINGS:
          pBaseSettings->SetModified();
          pBaseSettings->WriteSettings();
          break;
        }

      return 0;
    }

  return 1;
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

VOID CALLBACK BaseApplet::FullScreenTimer(HWND hwnd, UINT uMsg UNUSED, UINT_PTR idEvent UNUSED, DWORD dwTime UNUSED)
{
  KillTimer(hwnd, FULLSCREEN_TIMER);
  if (ELIsFullScreen(hwnd, GetForegroundWindow()))
    {
      pBaseApplet->SetFullScreen(true);
      pBaseApplet->DoEmergeNotify(EMERGE_CORE, CORE_HIDE);
    }
  else if (pBaseApplet->GetFullScreen())
    {
      pBaseApplet->SetFullScreen(false);
      pBaseApplet->DoEmergeNotify(EMERGE_CORE, CORE_SHOW);
    }
  SetTimer(hwnd, FULLSCREEN_TIMER, FULLSCREEN_POLL_TIME, (TIMERPROC)FullScreenTimer);
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
  fullScreen = value;
}

bool BaseApplet::GetFullScreen()
{
  return fullScreen;
}

LRESULT BaseApplet::DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  UINT shellMessage = (UINT)wParam;

  if (message == ShellMessage)
    {
      switch (shellMessage)
        {
          // A "task" was activated
        case HSHELL_RUDEAPPACTIVATED:
          SetTimer(mainWnd, FULLSCREEN_TIMER, FULLSCREEN_POLL_TIME, (TIMERPROC)FullScreenTimer);
          return 1;

        case HSHELL_WINDOWACTIVATED:
          if (fullScreen)
            {
              KillTimer(mainWnd, FULLSCREEN_TIMER);
              fullScreen = false;
              DoEmergeNotify(EMERGE_CORE, CORE_SHOW);
            }
          return 1;

        default:
          return 0;
        }
    }

  if (message == EMERGE_NOTIFY)
    return DoEmergeNotify(shellMessage, (UINT)lParam);

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

