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
// Note: This file is based on DDEService class component of the LiteStep shell.
//
//----  --------------------------------------------------------------------------------------------------------

#include "DDEService.h"
#include "../emergeLib/emergeLib.h"

unsigned int DDEService::m_uThreadId;
HANDLE DDEService::m_hThread;
HANDLE DDEService::m_hStartEvent;
DDEWorker DDEService::m_DDEWorker;
HSZ DDEService::m_hszProgman;
HSZ DDEService::m_hszGroups;
HSZ DDEService::m_hszFolders;
HSZ DDEService::m_hszAppProperties;
DWORD DDEService::m_dwDDEInst;

DDEService::DDEService()
{
  m_hszProgman = NULL;
  m_hszGroups = NULL;
  m_hszFolders = NULL;
  m_hszAppProperties = NULL;
  m_dwDDEInst = 0;
  m_hStartEvent = NULL;
}

DDEService::~DDEService()
{}

unsigned int __stdcall DDEService::_DDEThreadProc(void* pvService)
{
  DDEService* pService = reinterpret_cast<DDEService*>(pvService);

  bool bStarted = pService->_DoStart();

  SetEvent(pService->m_hStartEvent);

  if (bStarted)
    {
      MSG msg;
      while (GetMessage(&msg, 0, 0, 0))
        {
          try
            {
              // Window message
              TranslateMessage(&msg);
              DispatchMessage(&msg);
            }
          catch (...)
            {
              // Quietly ignore exceptions?
            }
        }
    }

  pService->_DoStop();

  return 0;
}

HRESULT DDEService::Start()
{
  HRESULT hr = E_FAIL;

  // If m_dwDDEInst is not NULL, then we have already been started, just
  // return S_FALSE which will still pass SUCCEEDED()
  if (!m_dwDDEInst)
    {
      m_hStartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
      m_hThread = (HANDLE)_beginthreadex(NULL, 0, _DDEThreadProc, this,
                                         0, &m_uThreadId);

      WaitForSingleObject(m_hStartEvent, INFINITE);
      CloseHandle(m_hStartEvent);
      hr = S_OK;
    }
  else
    {
      hr = S_FALSE;
    }


  return hr;
}


HRESULT DDEService::Stop()
{
  HRESULT hr = S_OK;

  if (m_hThread)
    {
      PostThreadMessage(m_uThreadId, WM_QUIT, 0, 0);

      if (WaitForSingleObject(m_hThread, 3000) == WAIT_TIMEOUT)
        {
          TerminateThread(m_hThread, 0);
          hr = S_FALSE;
        }

      CloseHandle(m_hThread);
    }

  return hr;
}

bool DDEService::_DoStart()
{
  bool bReturn = false;

  if (!m_dwDDEInst)
    {
      OleInitialize(NULL);

      UINT uInitReturn = DdeInitialize(&m_dwDDEInst, (PFNCALLBACK)DdeCallback,
                                       APPCLASS_STANDARD | CBF_FAIL_POKES | CBF_FAIL_SELFCONNECTIONS |
                                       CBF_SKIP_ALLNOTIFICATIONS, 0L);

      if (uInitReturn == DMLERR_NO_ERROR)
        bReturn = SUCCEEDED(_RegisterDDE());
    }

  return bReturn;
}


void DDEService::_DoStop()
{
  DdeNameService(m_dwDDEInst, 0L, 0L, DNS_UNREGISTER);
  if (m_hszProgman)
    {
      DdeFreeStringHandle(m_dwDDEInst, m_hszProgman);
    }
  if (m_hszGroups)
    {
      DdeFreeStringHandle(m_dwDDEInst, m_hszGroups);
    }
  if (m_hszFolders)
    {
      DdeFreeStringHandle(m_dwDDEInst, m_hszFolders);
    }
  if (m_hszAppProperties)
    {
      DdeFreeStringHandle(m_dwDDEInst, m_hszAppProperties);
    }
  DdeUninitialize(m_dwDDEInst);

  m_dwDDEInst = 0;

  OleUninitialize();
}

HDDEDATA CALLBACK DDEService::DdeCallback(
  UINT wType,
  UINT wFmt,
  HCONV hConv UNUSED,
  HSZ hszTopic,
  HSZ hszItem,
  HDDEDATA hData,
  DWORD lData1 UNUSED,
  DWORD lData2 UNUSED)
{
  HDDEDATA hReturn = (HDDEDATA)FALSE;

  switch (wType)
    {
    case XTYP_CONNECT:
      hReturn = (HDDEDATA)TRUE;
      break;

    case XTYP_WILDCONNECT:
    {
      HSZPAIR FAR *phszp;
      DWORD cb;

      if ((!hszTopic || hszTopic == m_hszProgman) &&
          (!hszItem || hszItem == m_hszProgman))
        {
          HDDEDATA hData = DdeCreateDataHandle(m_dwDDEInst, NULL,
                                               2 * sizeof(HSZPAIR), 0L, 0, 0, 0);

          if (hData)
            {
              phszp = (HSZPAIR FAR *)DdeAccessData(hData, &cb);
              phszp[0].hszSvc = m_hszProgman;
              phszp[0].hszTopic = m_hszProgman;
              phszp[1].hszSvc = phszp[1].hszTopic = 0;
              DdeUnaccessData(hData);
              hReturn = hData;
            }
        }
    }
    break;

    case XTYP_EXECUTE:
    {
      if ((hszTopic == m_hszGroups) || (hszTopic == m_hszAppProperties) || (hszTopic == m_hszProgman))
        {
          TCHAR tzBuf[MAX_LINE_LENGTH];
          DdeGetData(hData, (LPBYTE)tzBuf, MAX_LINE_LENGTH, 0);
          tzBuf[MAX_LINE_LENGTH - 1] = '\0';
          if (m_DDEWorker.ParseRequest(tzBuf))
            {
              hReturn = (HDDEDATA)DDE_FACK;
            }
        }
    }
    break;

    case XTYP_ADVSTART:
    case XTYP_ADVSTOP:
    {
      if (wFmt == CF_TEXT)
        {
          hReturn = (HDDEDATA)TRUE;
        }
    }
    break;

    case XTYP_REQUEST:
    case XTYP_ADVREQ:
    {
      if ((wFmt == CF_TEXT) && ((hszTopic == m_hszProgman) && (hszItem == m_hszGroups)))
        {
          LPVOID pList = NULL;
          UINT ulLen = 0;

          if (m_DDEWorker.ListGroups(pList, ulLen))
            {
              hReturn = DdeCreateDataHandle(m_dwDDEInst, (LPBYTE)pList, ulLen, 0L,
                                            m_hszGroups, CF_TEXT, 0);
              HeapFree(GetProcessHeap(), 0, pList);
            }
        }
    }
    break;

    default:
      break;
    }

  return hReturn;
}

HRESULT DDEService::_RegisterDDE()
{
  HRESULT hr = E_FAIL;

  m_hszProgman = DdeCreateStringHandle(m_dwDDEInst, TEXT("PROGMAN"), CP_WINUNICODE);
  if (m_hszProgman != 0L)
    {
      m_hszGroups = DdeCreateStringHandle(m_dwDDEInst, TEXT("Groups"), CP_WINUNICODE);
      if (m_hszGroups != 0L)
        {
          m_hszFolders = DdeCreateStringHandle(m_dwDDEInst, TEXT("Folders"), CP_WINUNICODE);
          if (m_hszFolders != 0L)
            {
              m_hszAppProperties = DdeCreateStringHandle(m_dwDDEInst, TEXT("AppProperties"), CP_WINUNICODE);
              if (m_hszAppProperties != 0L)
                {
                  if (DdeNameService(m_dwDDEInst, m_hszProgman, 0L, DNS_REGISTER) != 0L)
                    {
                      if (DdeNameService(m_dwDDEInst, m_hszFolders, 0L, DNS_REGISTER) != 0L)
                        hr = S_OK;
                    }
                }
            }
        }
    }

  return hr;
}
