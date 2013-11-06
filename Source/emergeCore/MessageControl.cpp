//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2012  The Emerge Desktop Development Team
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
// Note: The message handling routine is based on MessageManager which is part
// of the BB4Win source code copyright 2001 - 2007 The Blackbox for Windows
// Development Team.
//
//----  --------------------------------------------------------------------------------------------------------

#include "MessageControl.h"

MessageControl::MessageControl()
{
  EmergeDesktopTypes[0] = EMERGE_CORE;
  EmergeDesktopTypes[1] = EMERGE_VWM;
  types.insert( std::pair<UINT, std::tr1::shared_ptr<WindowSet> >
                (EmergeDesktopTypes[0], std::tr1::shared_ptr<WindowSet>(new WindowSet)));
  types.insert( std::pair<UINT, std::tr1::shared_ptr<WindowSet> >
                (EmergeDesktopTypes[1], std::tr1::shared_ptr<WindowSet>(new WindowSet)));
}

MessageControl::~MessageControl()
{
  while (types.empty())
  {
    types.erase(types.begin());
  }

  types.clear();
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	AddType
// Requires:	HWND window - window to recieve the message
//              UINT type - applet type
// Returns:	Nothing
// Purpose:	Adds the window to the applet type list
//----  --------------------------------------------------------------------------------------------------------
void MessageControl::AddType(HWND window, UINT type)
{
  for (UINT i = 0; i < 2; i++)
  {
    if ((type & EmergeDesktopTypes[i]) == EmergeDesktopTypes[i])
    {
      DoAdd(window, EmergeDesktopTypes[i]);
    }
  }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	DoAdd
// Requires:	HWND window - window to recieve the message
//              UINT type - applet type
// Returns:	Nothing
// Purpose:	Adds the window to the applet type list
//----  --------------------------------------------------------------------------------------------------------
void MessageControl::DoAdd(HWND window, UINT type)
{
  TypeMap::iterator iter;

  iter = types.find(type);

  if (iter != types.end())
  {
    iter->second->insert(iter->second->end(), window);
  }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	RemoveType
// Requires:	HWND window - window to be removed
//              UINT type - applet type
// Returns:	Nothing
// Purpose:	Removes a window from an applet type so it will not
// 		continue to recieve the associated messages
//----  --------------------------------------------------------------------------------------------------------
void MessageControl::RemoveType(HWND window, UINT type)
{
  for (UINT i = 0; i < 2; i++)
  {
    if ((type & EmergeDesktopTypes[i]) == EmergeDesktopTypes[i])
    {
      DoRemove(window, EmergeDesktopTypes[i]);
    }
  }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	RemoveType
// Requires:	HWND window - window to be removed
//              UINT type - applet type
// Returns:	Nothing
// Purpose:	Removes a window from an applet type so it will not
// 		continue to recieve the associated messages
//----  --------------------------------------------------------------------------------------------------------
void MessageControl::DoRemove(HWND window, UINT type)
{
  TypeMap::iterator iter;

  iter = types.find(type);

  if (iter != types.end())
  {
    iter->second->erase(window);
  }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	DispatchMessage
// Requires:    UINT type - applet type
//              UINT message - message to send
// Returns:	Nothing
// Purpose:	Sends the message to the windows that have requested it
//----  --------------------------------------------------------------------------------------------------------
void MessageControl::Dispatch_Message(UINT type, UINT message, WCHAR* instanceName)
{
  WindowSet* winSet;
  TypeMap::iterator iter1;
  WindowSet::iterator iter2;
  UINT Msg = 0;
  WPARAM wParam = 0;
  LPARAM lParam = 0;
  NOTIFYINFO notifyInfo;
  COPYDATASTRUCT cds;

  iter1 = types.find(type);

  if (iter1 == types.end())
  {
    return;
  }

  if (type == EMERGE_CORE && message == CORE_QUIT)
  {
    Msg = WM_NCDESTROY;
  }
  else
  {
    Msg = WM_COPYDATA;

    ZeroMemory(&notifyInfo, sizeof(notifyInfo));
    notifyInfo.Type = type;
    notifyInfo.Message = message;
    if ((instanceName != NULL) && wcslen(instanceName))
    {
      wcsncpy(notifyInfo.InstanceName, instanceName, MAX_PATH - 1);
    }

    cds.dwData = EMERGE_NOTIFY;
    cds.cbData = sizeof(notifyInfo);
    cds.lpData = &notifyInfo;

    lParam = reinterpret_cast<LPARAM>(&cds);
  }

  winSet = iter1->second.get();
  iter2 = winSet->begin();
  HWND coreWnd = ELGetCoreWindow();
  while (iter2 != winSet->end())
  {
    // if CORE_QUIT, bypass coreWnd so that all the other applets are sent
    // the WM_NCDESTORY message.
    if ((Msg == WM_NCDESTROY) && (((HWND)*iter2) == coreWnd))
    {
      iter2++;
      continue;
    }
    SendMessageTimeout((HWND)*iter2, Msg, wParam, lParam, SMTO_ABORTIFHUNG,
                       500, NULL);
    iter2++;
  }

  // if CORE_QUIT, kill coreWnd after all other applets have been passed the
  // WM_NCDESTORY message.
  if (Msg == WM_NCDESTROY)
    SendMessageTimeout(coreWnd, Msg, wParam, lParam, SMTO_ABORTIFHUNG, 500,
                       NULL);
}
