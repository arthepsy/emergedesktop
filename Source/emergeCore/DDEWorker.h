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
// Note: This file is based on DDEWorker class component of the LiteStep shell.
//
//----  --------------------------------------------------------------------------------------------------------

#ifndef __DDEWORKER_H
#define __DDEWORKER_H

#define UNICODE 1

#include <windows.h>
#include <stdio.h>
#include <ddeml.h>
#include <shlobj.h>
#include <shlwapi.h>

#define DDE_REQUEST_NONE			0x0000
#define DDE_REQUEST_CREATEGROUP		0x0001
#define DDE_REQUEST_DELETEGROUP		0x0002
#define DDE_REQUEST_SHOWGROUP		0x0004
#define DDE_REQUEST_ADDITEM			0x0006
#define DDE_REQUEST_DELETEITEM		0x0008
#define DDE_REQUEST_EXPLOREFOLDER	0x0010
#define DDE_REQUEST_VIEWFOLDER		0x0020
#define DDE_REQUEST_FINDFOLDER		0x0040
#define DDE_REQUEST_OPENFINDFILE	0x0060

class DDEWorker
{
public:
  DDEWorker();
  ~DDEWorker();

  BOOL ParseRequest(LPCWSTR pszRequest);
  BOOL ListGroups(LPVOID& pGroupList, UINT& ulSize);

private:

  BOOL _FindFiles(LPWSTR pszPath, BOOL bFindFolder);
  BOOL _ShowGroup(LPCTSTR strGroupName, int nShow, BOOL bCommon);
  BOOL _CreateGroup(LPCTSTR strGroupName, BOOL bCommon);
  BOOL _DeleteGroup(LPCTSTR strGroupName, BOOL bCommon);
  BOOL _DeleteItem(LPCTSTR strItem);
  BOOL _AddItem(LPWSTR strCmdLine, LPWSTR pszDescription, LPWSTR pszIconPath, int nIconIndex, LPCTSTR pszDefDir, WORD dwHotKey, BOOL bMinimize, BOOL bSepMemSpace);
  BOOL _ListGroupsHelper(HANDLE hHeap, WCHAR* szPath, LPVOID& pGroupList, UINT& ulSize);
  DWORD _MatchRequest(LPCWSTR pszCommand);
  bool GetShellFolderPath(int nFolder, LPTSTR ptzPath, size_t cchPath);

  WCHAR m_szCurrentGroup[MAX_PATH];
  BOOL m_bIsUserAnAdmin;

  BOOL (__stdcall *SHFindFiles)(LPCITEMIDLIST, LPCITEMIDLIST);
};

#endif // __DDEWROKER_H

