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

#ifndef __ET_APPLET_H
#define __ET_APPLET_H

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

// Define required for menu animation
#undef WINVER
#define WINVER 0x0501

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include <vector>
#include <map>
#include <stdlib.h>
#include <time.h>
#include <process.h>
#include "Task.h"
#include "Config.h"
#include "Settings.h"
#include "../emergeStyleEngine/emergeStyleEngine.h"
#include "../emergeAppletEngine/emergeAppletEngine.h"
#include "../emergeBaseClasses/BaseApplet.h"

#define MODIFY_POLL_TIME  100

typedef std::tr1::shared_ptr<Task> TaskPtr;
typedef std::vector<TaskPtr> TaskVector;

class Applet: public BaseApplet
{
private:
  std::tr1::shared_ptr<Settings> pSettings;
  TaskVector taskList;
  std::map<HWND, UINT> flashMap;
  std::map<HWND, DWORD> modifyMap;
  std::map<HWND, HANDLE> thumbnailMap;
  HWND activeWnd;
  void ResetTaskIcons();
  void UpdateIcons();
  static LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
  static BOOL CALLBACK EnumTasksList(HWND hwnd, LPARAM lParam);
  static VOID CALLBACK FlashTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
  CRITICAL_SECTION vectorLock;
  LRESULT DoTaskIcon(HWND task, HICON icon);
  static DWORD WINAPI ModifyThreadProc(LPVOID lpParameter);
  static DWORD WINAPI UpdateThumbnailThreadProc(LPVOID lpParameter);

protected:
  virtual std::tr1::shared_ptr<BaseSettings> createSettings();

public:
  Applet(HINSTANCE hInstance);
  ~Applet();
  UINT Initialize();
  LRESULT DoTimer(UINT_PTR timerID);
  LRESULT DoSizing(HWND hwnd, UINT edge, LPRECT rect);
  LRESULT DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT TaskMouseEvent(UINT message, LPARAM lParam);
  LRESULT AddTask(HWND task);
  LRESULT ModifyTask(HWND task);
  LRESULT RemoveTask(HWND task);
  LRESULT SetFlash(HWND task, bool flash);
  LRESULT DoCopyData(COPYDATASTRUCT *cds);
  void CreateBrushes();
  void DestroyBrushes();
  bool CleanTasks();
  void BuildTasksList();
  TaskVector::iterator FindTask(HWND hwnd);
  void AppletUpdate();
  void SetActive(HWND hwnd);
  void ShowConfig();
  bool PaintItem(HDC hdc, UINT index, int x, int y, RECT rect);
  size_t GetIconCount();
  void DoTaskFlash(UINT id);
  LRESULT ModifyTaskByThread(DWORD threadID);
};

#endif
