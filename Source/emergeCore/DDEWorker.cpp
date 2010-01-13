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

#include "DDEWorker.h"
#include "../emergeLib/emergeLib.h"

DDEWorker::DDEWorker()
{
  ZeroMemory(m_szCurrentGroup, MAX_PATH);

  SC_HANDLE hSC = OpenSCManager(NULL, NULL, GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE);
  if (hSC)
    m_bIsUserAnAdmin = TRUE;
  else
    m_bIsUserAnAdmin = FALSE;

  SHFindFiles = (BOOL (__stdcall *)(LPCITEMIDLIST, LPCITEMIDLIST))GetProcAddress(GetModuleHandle(TEXT("SHELL32.DLL")), (LPSTR)((long)0x5A));
}


DDEWorker::~DDEWorker()
{}


// Handle incoming DDE requests and pass to relevant functions
BOOL DDEWorker::ParseRequest(LPCWSTR pszRequest)
{
  // 10 is the Maximum number of parameters passed in PROGMAN DDE call (AddItem)
  LPWSTR pszParamList[10];      // This holds a list of pointers to args
  BOOL bReturn = FALSE;

  DWORD dwRequest = _MatchRequest(pszRequest);
  if (dwRequest)
    {
      size_t stLength = wcslen(pszRequest) + 1;
      LPWSTR pszWorkRequest = new WCHAR[stLength];

      wcsncpy(pszWorkRequest, pszRequest, stLength);
      pszWorkRequest[stLength - 3] = '\0';
      pszWorkRequest = StrChr(pszWorkRequest, '(');
      pszWorkRequest++;

      pszParamList[0] = pszWorkRequest;

      int nCurIndex = 1;
      LPWSTR pszTmp = StrChr(pszWorkRequest, ','); // find a delimiter
      while (NULL != pszTmp)
        {
          *(pszTmp++) = '\0'; // null terminate and cut.
          pszParamList[nCurIndex++] = pszTmp; // put it in the pointer list
          pszTmp = StrChr(pszTmp, ','); // get the next one
        }

      // Set initial state of the group to act on:
      // bCommon ? Perform function on a common group : Perform function on a private group
      BOOL bCommon = m_bIsUserAnAdmin;

      switch (dwRequest)
        {
          // Working
          // DDE call: e.g. ExploreFolder(idlist[,object])
        case DDE_REQUEST_EXPLOREFOLDER:
        {
          WCHAR szParam[MAX_LINE_LENGTH];
          wcsncpy(szParam, pszParamList[0], MAX_LINE_LENGTH);
          PathUnquoteSpaces(szParam);
          if (PathIsDirectory(szParam) || PathIsUNC(szParam))
            swprintf(szParam, TEXT("explorer.exe /e,%s"), pszParamList[0]);
          else
            swprintf(szParam, TEXT("explorer.exe /e,/idlist,%s"), pszParamList[1]);
          bReturn = (ELExecute(szParam) == true);
        }
        break;

        // Working
        // DDE call: e.g. ViewFolder(idlist[,object])
        case DDE_REQUEST_VIEWFOLDER:
        {
          WCHAR szParam[MAX_LINE_LENGTH];
          wcsncpy(szParam, pszParamList[0], MAX_LINE_LENGTH);
          PathUnquoteSpaces(szParam);
          if (PathIsDirectory(szParam) || PathIsUNC(szParam))
            swprintf(szParam, TEXT("explorer.exe %s"), pszParamList[0]);
          else
            swprintf(szParam, TEXT("explorer.exe /idlist,%s"), pszParamList[1]);
          bReturn = (ELExecute(szParam) == true);
          // Will return too soon if we don't sleep here... funky
          Sleep(500);
        }
        break;

        // DDE call: e.g. FindFolder(idlist[,object])
        case DDE_REQUEST_FINDFOLDER:
          bReturn = _FindFiles(pszParamList[0], TRUE);
          break;

          // DDE call: e.g. OpenFindFile(idlist[,object])
        case DDE_REQUEST_OPENFINDFILE:
          bReturn = _FindFiles(pszParamList[0], FALSE);
          break;

          // Working
          // DDE call: e.g. CreateGroup(GroupName[,CommonGroupFlag])
        case DDE_REQUEST_CREATEGROUP:
        {
          if (2 == nCurIndex) // second parameter forces common/private
            bCommon = _wtoi(pszParamList[1]);
          bReturn = _CreateGroup(pszParamList[0], bCommon);
        }
        break;

        // DDE call: e.g. DeleteGroup(GroupName[,CommonGroupFlag])
        case DDE_REQUEST_DELETEGROUP:
        {
          if (2 == nCurIndex) // second parameter forces common/private
            bCommon = _wtoi(pszParamList[1]);
          bReturn = _DeleteGroup(pszParamList[0], bCommon);
        }
        break;

        // Working
        // DDE call: e.g. ShowGroup(GroupName,ShowCommand[,CommonGroupFlag])
        case DDE_REQUEST_SHOWGROUP:
        {
          if (nCurIndex >= 2) // show command: maps to SW_
            {
              if (3 == nCurIndex) // third parameter forces common/private
                {
                  bCommon = _wtoi(pszParamList[2]);
                }
              int nShow = _wtoi(pszParamList[1]);
              bReturn = _ShowGroup(pszParamList[0], nShow, bCommon);
            }
        }
        break;

        // Working
        // DDE call: e.g. DeleteItem(ItemName) : applies to current group
        case DDE_REQUEST_DELETEITEM:
        {
          bReturn = _DeleteItem(pszParamList[0]);
        }
        break;

        // Working
        //DDE call: AddItem(CmdLine[,Name[,IconPath[,IconIndex[,xPos,yPos[,DefDir[,
        //HotKey[,fMinimize[fSeparateMemSpace] ] ] ] ] ] ]) : applies to current group
        case DDE_REQUEST_ADDITEM:
        {
          BOOL bSepMemSpace = FALSE;
          BOOL bMinimize = FALSE;
          WORD dwHotKey = 0;
          LPCWSTR pszDefDir = NULL;
          int nIconIndex = 0;
          LPWSTR pszIconPath = NULL;
          LPWSTR pszDescription = NULL;

          switch (nCurIndex)
            {
            case 10:  // SeparateMemSpace ignored for now
            case 9:
            {
              // Minimize
              bMinimize = (BOOL)StrToInt(pszParamList[8]);
            }
            case 8:
            {
              // HotKey
              dwHotKey = (WORD)StrToInt(pszParamList[7]);
            }
            case 7:
            {
              // DefDir
              pszDefDir = pszParamList[6];
            }
            case 6:  //xPos and yPos ignored. Not necessary
            case 5:
            case 4:
            {
              // IconIndex
              nIconIndex = StrToInt(pszParamList[3]);
            }
            case 3:
            {
              // IconPath
              pszIconPath = pszParamList[2];
            }
            case 2:
            {
              // Description
              pszDescription = pszParamList[1];
            }

            default:
              break;
            }
          bReturn = _AddItem(pszParamList[0], pszDescription, pszIconPath, nIconIndex, pszDefDir, dwHotKey, bMinimize, bSepMemSpace);
        }
        break;

        default:
          break;
        }
    }
  return bReturn;
}


bool DDEWorker::GetShellFolderPath(int nFolder, LPTSTR ptzPath, size_t cchPath UNUSED)
{
  IMalloc* pMalloc;
  bool bReturn = false;

  // SHGetSpecialFolderPath is not available on Win95
  // use SHGetSpecialFolderLocation and SHGetPathFromIDList instead
  if (SUCCEEDED(SHGetMalloc(&pMalloc)))
    {
      LPITEMIDLIST pidl;

      if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, nFolder, &pidl)))
        {
          bReturn = SHGetPathFromIDList(pidl, ptzPath) ? true : false;

          if (bReturn)
            PathAddBackslash(ptzPath);

          pMalloc->Free(pidl);
        }

      pMalloc->Release();
    }

  return bReturn;
}

// List the program groups
BOOL DDEWorker::ListGroups(LPVOID& pGroupList, UINT& ulSize)
{
  WCHAR szPath[MAX_LINE_LENGTH];
  pGroupList = NULL;
  ulSize = 0;
  LPWSTR pszTemp = NULL;
  BOOL bReturn = FALSE;

  // Get user specific folders
  if (GetShellFolderPath(CSIDL_PROGRAMS, szPath, MAX_LINE_LENGTH))
    {
      PathAppend(szPath, TEXT("*.*"));

      HANDLE hHeap = GetProcessHeap();
      if (hHeap)
        {
          if (_ListGroupsHelper(hHeap, szPath, pGroupList, ulSize))
            {
              // Get common folders
              if (GetShellFolderPath(CSIDL_COMMON_PROGRAMS, szPath, MAX_LINE_LENGTH))
                {
                  PathAppend(szPath, TEXT("*.*"));

                  if (_ListGroupsHelper(hHeap, szPath, pGroupList, ulSize))
                    {
                      // add null terminator
                      pszTemp = (WCHAR*) HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pGroupList, ulSize + 1);
                      if (pszTemp != NULL)
                        {
                          pszTemp[ulSize] = '\0';
                          pGroupList = (LPVOID) pszTemp;
                          ulSize += 1;
                          bReturn = TRUE;
                        }
                    }
                }
            }
        }
    }

  return bReturn;
}


DWORD DDEWorker::_MatchRequest(LPCWSTR pszCommand)
{
  DWORD dwReturn = DDE_REQUEST_NONE;

  if (StrStr(pszCommand, TEXT("[ExploreFolder(")) == pszCommand)
    {
      dwReturn = DDE_REQUEST_EXPLOREFOLDER;
    }
  else if (StrStr(pszCommand, TEXT("[ViewFolder(")) == pszCommand)
    {
      dwReturn = DDE_REQUEST_VIEWFOLDER;
    }
  else if (StrStr(pszCommand, TEXT("[FindFolder(")) == pszCommand)
    {
      dwReturn = DDE_REQUEST_FINDFOLDER;
    }
  else if (StrStr(pszCommand, TEXT("[OpenFindFile(")) == pszCommand)
    {
      dwReturn = DDE_REQUEST_OPENFINDFILE;
    }
  else if (StrStr(pszCommand, TEXT("[CreateGroup(")) == pszCommand)
    {
      dwReturn = DDE_REQUEST_CREATEGROUP;
    }
  else if (StrStr(pszCommand, TEXT("[DeleteGroup(")) == pszCommand)
    {
      dwReturn = DDE_REQUEST_DELETEGROUP;
    }
  else if (StrStr(pszCommand, TEXT("[ShowGroup(")) == pszCommand)
    {
      dwReturn = DDE_REQUEST_SHOWGROUP;
    }
  else if (StrStr(pszCommand, TEXT("[AddItem(")) == pszCommand)
    {
      dwReturn = DDE_REQUEST_ADDITEM;
    }
  else if (StrStr(pszCommand, TEXT("[DeleteItem(")) == pszCommand)
    {
      dwReturn = DDE_REQUEST_DELETEITEM;
    }

  return dwReturn;
}


BOOL DDEWorker::_FindFiles(LPWSTR pszPath, BOOL bFindFolder)
{
  LPMALLOC pMalloc = NULL;
  BOOL bReturn = FALSE;
  WCHAR wzPath[MAX_LINE_LENGTH] = { 0 };

  PathUnquoteSpaces(pszPath);
  wcsncpy(wzPath, pszPath, MAX_LINE_LENGTH);

  HRESULT hr = SHGetMalloc(&pMalloc);
  if (SUCCEEDED(hr))
    {
      IShellFolder* psfParent = NULL;

      hr = SHGetDesktopFolder(&psfParent);
      if (SUCCEEDED(hr))
        {
          LPITEMIDLIST pidl = NULL;

          hr = psfParent->ParseDisplayName(NULL, NULL, wzPath, NULL, &pidl, NULL);
          if (SUCCEEDED(hr))
            {
              if (bFindFolder) // FindFolder
                {
                  bReturn = (BOOL)SHFindFiles(pidl, NULL);
                }
              else // OpenFindFile
                {
                  bReturn = (BOOL)SHFindFiles(NULL, pidl);
                }

              pMalloc->Free(pidl);
            }

          psfParent->Release();
        }

      pMalloc->Release();
    }

  return bReturn;
}


BOOL DDEWorker::_ShowGroup(LPCTSTR strGroupName, int nShow, BOOL bCommon)
{
  WCHAR szFullPath[MAX_LINE_LENGTH];
  WCHAR szPath[MAX_LINE_LENGTH];
  BOOL bReturn = FALSE;

  // Get the program group path
  if (GetShellFolderPath(bCommon ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS, szPath, MAX_LINE_LENGTH))
    {
      wsprintf(szFullPath, TEXT("%s%s\\"), szPath, strGroupName);
//      PathQuoteSpaces(szFullPath);

      if (PathIsDirectory(szFullPath))
        {
          // open it up!
          ELExecute(szFullPath, (WCHAR*)TEXT("\0"), nShow);

          // set our current group to this one, as per Progman DDE
          wcsncpy(m_szCurrentGroup, szFullPath, MAX_LINE_LENGTH);

          bReturn = TRUE;
        }
    }

  return bReturn;
}

// Create the program group
BOOL DDEWorker::_CreateGroup(LPCTSTR strGroupName, BOOL bCommon)
{
  WCHAR szPath[MAX_LINE_LENGTH];
  WCHAR szFullPath[MAX_LINE_LENGTH];
  BOOL bReturn = FALSE;

  // Get the program group path
  if (GetShellFolderPath(bCommon ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS, szPath, MAX_LINE_LENGTH))
    {
      wsprintf(szFullPath, TEXT("%s%s"), szPath, strGroupName);
//      PathQuoteSpaces(szFullPath);

      // standard create directory call
      if (CreateDirectory(szFullPath, NULL))
        {
          wcsncpy(m_szCurrentGroup, szFullPath, MAX_LINE_LENGTH);

          // Hmmm should we show the group here???? Maybe
          // but not for now.
          //ShellExecute(NULL, "open", szPath, NULL, NULL, nShow);

          // Tell the shell that something changed
          SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, szFullPath, 0);

          bReturn = TRUE;
        }
      else
        {
          if (ERROR_ALREADY_EXISTS == GetLastError())
            {
              bReturn = TRUE;
            }
        }
    }

  return bReturn;
}

// Remove the group
BOOL DDEWorker::_DeleteGroup(LPCTSTR strGroupName, BOOL bCommon)
{
  WCHAR szTemp[MAX_LINE_LENGTH];
  WCHAR szPath[MAX_LINE_LENGTH];
  WIN32_FIND_DATA FindData;
  BOOL bFindFile = TRUE;
  BOOL bReturn = FALSE;

  // Get the program group path
  if (GetShellFolderPath(bCommon ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS, szPath, MAX_LINE_LENGTH))
    {
      // Append \*.* for FindFirstFile
//      wcsncpy(szTemp, TEXT("\\"), MAX_LINE_LENGTH);
//      wcscat(szTemp, strGroupName);
//      wcscat(szPath, szTemp);

      wcscat(szPath, strGroupName);

      wcsncpy(szTemp, szPath, MAX_LINE_LENGTH);
      wcscat(szTemp, TEXT("\\*.*"));

      // Use FindFirstFile to list dir. contents
      HANDLE hFind = FindFirstFile(szTemp, &FindData);

      // kill them all off
      while ((INVALID_HANDLE_VALUE != hFind) && bFindFile)
        {
          if (*(FindData.cFileName) != '.')
            {
              wcsncpy(szTemp, szPath, MAX_LINE_LENGTH);
              PathAppend(szTemp, FindData.cFileName);
              DeleteFile(szTemp);
            }
          bFindFile = FindNextFile(hFind, &FindData);
        }

      FindClose(hFind);

      bReturn = RemoveDirectory(szPath);
    }

  return bReturn;
}

// helper function to do allocation for the list of program groups
BOOL DDEWorker::_ListGroupsHelper(HANDLE hHeap, WCHAR* szPath, LPVOID& pGroupList, UINT& ulSize)
{
  WIN32_FIND_DATA FindData;
  BOOL bFindFile = TRUE;
  WCHAR* pszTemp = NULL;
  BOOL bReturn = FALSE;

  // allocate the group list with base zero terminator if NULL
  if (NULL == pGroupList)
    {
      pGroupList = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 1);
    }

  // check allocation
  if (pGroupList)
    {
      // Get the first one
      HANDLE hFind = FindFirstFile(szPath, &FindData);

      // iterate through the files
      // copy their names into a string in which each field is terminated
      // by \r\n
      while ((INVALID_HANDLE_VALUE != hFind) && bFindFile)
        {
          if (*(FindData.cFileName) != '.' && (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
              int stLen = lstrlen(FindData.cFileName);

              pszTemp = (WCHAR*) HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pGroupList, ulSize + stLen + 2);
              if (pszTemp != NULL)
                {
                  CopyMemory(&pszTemp[ulSize], FindData.cFileName, stLen);
                  CopyMemory(&pszTemp[ulSize + stLen], "\r\n", 2);
                  pGroupList = (LPVOID) pszTemp;
                  ulSize += (stLen + 2);
                  bReturn = TRUE;
                }
              else
                {
                  // Cleanup
                  FindClose(hFind);
                  bReturn = FALSE;
                  break;
                }
            }
          // continue to next
          bFindFile = FindNextFile(hFind, &FindData);
        }

      // Cleanup
      FindClose(hFind);
    }
  return bReturn;
}


// kill an item
BOOL DDEWorker::_DeleteItem(LPCTSTR strItem)
{
  WCHAR szPath[MAX_LINE_LENGTH];
  BOOL bReturn = FALSE;

  if (m_szCurrentGroup[0])
    {
      wsprintf(szPath, TEXT("%s\\%s"), m_szCurrentGroup, strItem);
      bReturn = DeleteFile(szPath);
    }

  return bReturn;
}

// Add an item
BOOL DDEWorker::_AddItem(LPWSTR strCmdLine, LPWSTR strDescription, LPWSTR strIconPath, int nIconIndex, LPCTSTR strDefDir, WORD dwHotKey, BOOL bMinimize, BOOL bSepMemSpace UNUSED)
{
  WCHAR szPath[MAX_LINE_LENGTH];
  IShellLink* pShellLink = NULL;
  IPersistFile* pPersistFile = NULL;
  BOOL bReturn = FALSE;
  LPVOID lpVoid;

  // check that we've set a current group via showgroup or creategroup
  if (m_szCurrentGroup[0])
    {
      WCHAR szDesc[MAX_LINE_LENGTH];
      WCHAR szArgs[MAX_LINE_LENGTH];
      WCHAR szCmd[MAX_LINE_LENGTH];

      PathQuoteSpaces(strCmdLine);

      LPTSTR strArgs = PathGetArgs(strCmdLine);
      if (strArgs)
        {
          wcsncpy(szArgs, strArgs, MAX_LINE_LENGTH);
          PathRemoveArgs(strCmdLine);
        }

      PathUnquoteSpaces(strCmdLine);
      wcsncpy(szCmd, strCmdLine, MAX_LINE_LENGTH);

      if (strDescription)
        {
          wcsncpy(szDesc, PathFindFileName(strCmdLine), MAX_LINE_LENGTH);
          PathRemoveExtension(szDesc);
        }
      else
        {
          PathUnquoteSpaces(strDescription);
          wcsncpy(szDesc, strDescription, MAX_LINE_LENGTH);
        }

      wsprintf(szPath, TEXT("%s\\%s.lnk"), m_szCurrentGroup, szDesc);

      lpVoid = pShellLink;
      HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                    IID_IShellLink, &lpVoid);
      if (SUCCEEDED(hr))
        {
          // Set the shortcut info.
          pShellLink->SetDescription(szDesc);
          pShellLink->SetHotkey(dwHotKey);
          pShellLink->SetWorkingDirectory(strDefDir);
          pShellLink->SetShowCmd(bMinimize ? SW_SHOWMINIMIZED : SW_SHOWNORMAL);
          pShellLink->SetArguments(szArgs);
          pShellLink->SetPath(szCmd);

          if (strIconPath)
            {
              pShellLink->SetIconLocation(strIconPath, nIconIndex);
            }

          // Save it.
          lpVoid = pPersistFile;
          hr = pShellLink->QueryInterface(IID_IPersistFile, &lpVoid);
          if (SUCCEEDED(hr))
            {
              WCHAR wsz[MAX_LINE_LENGTH];

              // Ensure that the string is ANSI.
              wcsncpy(wsz, szPath, MAX_LINE_LENGTH);

              // Save the link by calling IPersistFile::Save.
              hr = pPersistFile->Save(wsz, TRUE);
              if (SUCCEEDED(hr))
                {
                  SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, m_szCurrentGroup, 0);
                  SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, szPath, 0);
                  bReturn = TRUE;
                }
              pPersistFile->Release();
            }

          pShellLink->Release();
        }
    }

  return bReturn;
}

