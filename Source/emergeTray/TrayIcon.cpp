// vim: tags+=../emergeLib/tags
//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2010  The Emerge Desktop Development Team
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
//---

#include "Balloon.h"
#include "TrayIcon.h"

//-----
// Function:	Constructor
// Requires:	HINSTANCE appInstance - application instance
// 		HWND wnd - window handle of icon
// 		HICON icon - icon handle of icon
// 		UINT id - icon id
// 		UINT callbackMessage - icon callback message
// 		WCHAR *tip - icon tooltip
// 		UINT flags - icon flags
// 		bool hidden - hidden state of the icon
// Returns:	Nothing
// Purpose:	Creates TrayIcon Class Object
//-----
TrayIcon::TrayIcon(HINSTANCE appInstance, HWND wnd, UINT id, HWND mainWnd, HWND toolWnd, Settings *pSettings)
{
  this->appInstance = appInstance;
  this->wnd = wnd;
  this->id = id;
  iconVersion = 0;
  this->mainWnd = mainWnd;
  this->toolWnd = toolWnd;
  origIcon = NULL;
  newIcon = NULL;
  origIconSource = NULL;
  callbackMessage = 0;
  wcscpy(tip, TEXT("\0"));
  flags = 0;
  rect.left = 0;
  rect.right = 0;
  rect.top = 0;
  rect.bottom = 0;
  hidden = false;
  shared = false;

  convertIcon = true;

  pBalloon = std::tr1::shared_ptr<Balloon>(new Balloon(appInstance, this, pSettings));
  pBalloon->Initialize();
}

void TrayIcon::CreateNewIcon(BYTE foregroundAlpha)
{
  HICON tmpIcon = NULL;

  if (origIcon == NULL)
    return;

  if (convertIcon)
    {
      convertIcon = false;

      tmpIcon = EGConvertIcon(origIcon, foregroundAlpha);
      if (tmpIcon != NULL)
        {
          if (newIcon != NULL)
            DestroyIcon(newIcon);
          newIcon = CopyIcon(tmpIcon);
          DestroyIcon(tmpIcon);
        }
    }
}

//-----
// Function:	Deconstructor
// Requires:	Nothing
// Returns:	Nothing
// Purpose:	Deletes the icon
//-----
TrayIcon::~TrayIcon()
{
  DestroyIcon(origIcon);
  DestroyIcon(newIcon);
}

//-----
// Function:	GetWnd
// Requires:	Nothing
// Returns:	HWND
// Purpose:	Retrieves the icon window
//-----
HWND TrayIcon::GetWnd()
{
  return wnd;
}

//-----
// Function:	GetIcon
// Requires:	Nothing
// Returns:	HICON
// Purpose:	Retrieves the icon
//-----
HICON TrayIcon::GetIcon()
{
  return newIcon;
}

//-----
// Function:	GetID
// Requires:	Nothing
// Returns:	UINT
// Purpose:	Retrieves icon ID
//-----
UINT TrayIcon::GetID()
{
  return id;
}

UINT TrayIcon::GetIconVersion()
{
  return iconVersion;
}

//-----
// Function:	GetCallback
// Requires:	Nothing
// Returns:	UINT
// Purpose:	Retrieves the icon callback message
//-----
UINT TrayIcon::GetCallback()
{
  return callbackMessage;
}

//-----
// Function:	GetTip
// Requires:	Nothing
// Returns:	WCHAR*
// Purpose:	Retrieves the icon's tooltip
//-----
WCHAR *TrayIcon::GetTip()
{
  return tip;
}

//-----
// Function:	GetFlags
// Requires:	Nothing
// Returns:	UINT
// Purpose:	Retrieves the icon flags
//-----
UINT TrayIcon::GetFlags()
{
  return flags;
}

//-----
// Function:	GetRect
// Requires:	Nothing
// Returns:	RECT*
// Purpose:	Retrieves the bounding rectangle of the icon
//-----
RECT *TrayIcon::GetRect()
{
  return &rect;
}

//-----
// Function:	GetHidden
// Requires:	Nothing
// Returns:	bool
// Purpose:	Retrieves hidden state of the icon
//-----
bool TrayIcon::GetHidden()
{
  return hidden;
}

//-----
// Function:	GetShared
// Requires:	Nothing
// Returns:	bool
// Purpose:	Retrieves shared state of the icon
//-----
bool TrayIcon::GetShared()
{
  return shared;
}

void TrayIcon::SetIconVersion(UINT iconVersion)
{
  (*this).iconVersion = iconVersion;
}

//-----
// Function:	SetIcon
// Requires:	HICON icon - new icon
// Returns:	true if icon was updated, false otherwise
// Purpose:	Replaces existing icon with new icon
//-----
bool TrayIcon::SetIcon(HICON icon)
{
  if (origIconSource != icon)
    {
      if (origIcon)
        DestroyIcon(origIcon);

      origIconSource = icon;
      origIcon = CopyIcon(icon);
      convertIcon = true;

      // changed
      return true;
    }

  return false;
}

//-----
// Function:	SetCallback
// Requires:	UINT callbackMessage
// Returns:	true if callback was updated, false otherwise
// Purpose:	Replaces the existing callback message with the new one
//-----
bool TrayIcon::SetCallback(UINT callbackMessage)
{
  if ((*this).callbackMessage != callbackMessage)
    {
      (*this).callbackMessage = callbackMessage;

      return true;
    }

  return false;
}

//-----
// Function:	SetTip
// Requires:	WCHAR *tip - new tooltip text
// Returns:	true if tooltip was updated, false otherwise
// Purpose:	Replaces the existing tooltip text
//-----
bool TrayIcon::SetTip(WCHAR *tip)
{
  if (wcscmp((*this).tip, tip) != 0)
    {
      wcscpy((*this).tip, tip);
      UpdateTip();

      // changed
      return true;
    }

  return false;
}

// Function:	SetInfo
// Requires:	WCHAR *info - new info text
// Returns:	true if info was updated, false otherwise
// Purpose:	Replaces the existing info text
//-----
void TrayIcon::ShowBalloon(WCHAR *infoTitle, WCHAR *info, DWORD infoFlags, HICON icon)
{
  POINT balloonPt;
  balloonPt.x = rect.left;
  balloonPt.y = rect.top;

  if (ClientToScreen(mainWnd, &balloonPt))
  {
    pBalloon->SetInfoFlags(infoFlags, icon);
    pBalloon->SetInfoTitle(infoTitle);
    pBalloon->SetInfo(info);
    pBalloon->Show(balloonPt);
  }
}

void TrayIcon::HideBalloon()
{
  pBalloon->Hide();
}

BOOL TrayIcon::SendMessage(LPARAM lParam)
{
  if (iconVersion == NOTIFYICON_VERSION_4)
    {
      POINT cursorPos;
      GetCursorPos(&cursorPos);

      return SendNotifyMessage(wnd, callbackMessage, MAKEWPARAM(rect.left, rect.top),
                               MAKELPARAM(lParam, id));
    }
  else
    return SendNotifyMessage(wnd, callbackMessage, WPARAM(id), lParam);
}

//-----
// Function:	SetFlags
// Requires:	UINT flags - new icon flags
// Returns:	Nothing
// Purpose:	Replaces existing icon flags
//-----
void TrayIcon::SetFlags(UINT flags)
{
  (*this).flags = flags;
}

//-----
// Function:	SetRect
// Requires:	RECT rect - new bounding rectangle
// Returns:	Nothing
// Purpose:	Replaces the bounding rectangle of the icon
//-----
void TrayIcon::SetRect(RECT rect)
{
  if (!EqualRect(&(*this).rect, &rect))
    {
      (*this).rect = rect;
      convertIcon = true;
    }
}

//-----
// Function:	SetHidden
// Requires:	bool hidden - the hidden state of the icon
// Returns:	Nothing
// Purpose:	Replaces the hidden state of the icon
//-----
void TrayIcon::SetHidden(bool hidden)
{
  (*this).hidden = hidden;
}

//-----
// Function:	SetShared
// Requires:	bool shared - the shared state of the icon
// Returns:	Nothing
// Purpose:	Replaces the shared state of the icon
//-----
void TrayIcon::SetShared(bool shared)
{
  (*this).shared = shared;
}

void TrayIcon::UpdateIcon()
{
  convertIcon = true;
}

//-----
// Function:	UpdateTip
// Requires:	HWND mainWnd - handle of calling window
// 		HWND toolWnd - handle to the tooltip window
// Returns:	Nothing
// Purpose:	Updates the window handler with the icon tooltip.  If
// 		the tip already exists, its updated.  If not, it is
// 		created.
//-----
void TrayIcon::UpdateTip()
{
  TOOLINFO ti;
  ZeroMemory(&ti, sizeof(TOOLINFO));

  bool exists;

  // fill in the TOOLINFO structure
  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = mainWnd;
  ti.uId = id + (ULONG_PTR)wnd;

  // Check to see if the tooltip exists
  exists = ::SendMessage(toolWnd, TTM_GETTOOLINFO, 0,(LPARAM) (LPTOOLINFO) &ti) ? true : false;

  //  complete the rest of the TOOLINFO structure
  ti.hinst =  appInstance;
  ti.lpszText = tip;
  ti.rect = rect;

  // If it exists, modify the tooltip, if not add it
  if (exists)
    ::SendMessage(toolWnd, TTM_SETTOOLINFO, 0, (LPARAM)(LPTOOLINFO)&ti);
  else
    ::SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
}

//-----
// Function:	DeleteTip
// Requires:	HWND mainWnd - handle of calling window
// 		HWND toolWnd - handle to the tooltip window
// Returns:	Nothing
// Purpose:	Deletes the icon tooltip from the tooltip window
//-----
void TrayIcon::DeleteTip()
{
  TOOLINFO ti;
  ZeroMemory(&ti, sizeof(TOOLINFO));

  bool exists;

  // fill in the TOOLINFO structure
  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.hwnd = mainWnd;
  ti.uId = id + (ULONG_PTR)wnd;

  // Check to see if the tooltip exists
  exists = ::SendMessage(toolWnd, TTM_GETTOOLINFO, 0,(LPARAM) (LPTOOLINFO) &ti) ? true : false;

  if (exists)
    ::SendMessage(toolWnd, TTM_DELTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
}

