//----  --------------------------------------------------------------------------------------------------------
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
//----  --------------------------------------------------------------------------------------------------------
//
// Note: This file is based on DDEService class component of the LiteStep shell.
//
//----  --------------------------------------------------------------------------------------------------------

#ifndef __EC_DDESERVICE_H
#define __EC_DDESERVICE_H

#define UNICODE 1

#include "DDEWorker.h"
#include <process.h>

class DDEService
{
public:
  DDEService();
  ~DDEService();

  HRESULT Start();
  HRESULT Stop();

  static HDDEDATA CALLBACK DdeCallback(UINT wType, UINT wFmt, HCONV hConv, HSZ hszTopic,
                                       HSZ hszItem, HDDEDATA hData, DWORD lData1, DWORD lData2);

private:
  HRESULT _RegisterDDE();

  bool _DoStart();
  void _DoStop();

  static unsigned int __stdcall _DDEThreadProc(void* pvService);

  static unsigned int m_uThreadId;
  static HANDLE m_hThread;
  static HANDLE m_hStartEvent;

  static DDEWorker m_DDEWorker;
  static HSZ m_hszProgman;
  static HSZ m_hszGroups;
  static HSZ m_hszFolders;
  static HSZ m_hszAppProperties;
  static DWORD m_dwDDEInst;
};

#endif // __DDESERVICE_H

