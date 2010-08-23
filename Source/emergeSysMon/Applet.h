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

#ifndef __ESM_APPLET_H
#define __ESM_APPLET_H

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

// Define required for menu animation
#undef WINVER
#define WINVER 0x0501

// Defines required for tooltip
#undef _WIN32_IE
#define _WIN32_IE 0x0600

#include "../emergeStyleEngine/emergeStyleEngine.h"
#include "../emergeAppletEngine/emergeAppletEngine.h"
#include "../emergeBaseClasses/BaseApplet.h"
#include <vector>
#include <process.h>
#include <psapi.h>
#include <commctrl.h>
#ifdef __MINGW32__
#include <w32api.h>
#endif

#define TIP_SIZE 256

#define MODIFY_POLL_TIME  100

#ifndef _W64
#if (__W32API_MAJOR_VERSION == 3 && __W32API_MINOR_VERSION < 15)
typedef struct _PERFORMANCE_INFORMATION
{
  DWORD  cb;
  SIZE_T CommitTotal;
  SIZE_T CommitLimit;
  SIZE_T CommitPeak;
  SIZE_T PhysicalTotal;
  SIZE_T PhysicalAvailable;
  SIZE_T SystemCache;
  SIZE_T KernelTotal;
  SIZE_T KernelPaged;
  SIZE_T KernelNonpaged;
  SIZE_T PageSize;
  DWORD  HandleCount;
  DWORD  ProcessCount;
  DWORD  ThreadCount;
} PERFORMANCE_INFORMATION, *PPERFORMANCE_INFORMATION;
#endif

typedef struct _ENUM_PAGE_FILE_INFORMATION
{
  DWORD  cb;
  DWORD  Reserved;
  SIZE_T TotalSize;
  SIZE_T TotalInUse;
  SIZE_T PeakUsage;
} ENUM_PAGE_FILE_INFORMATION, *PENUM_PAGE_FILE_INFORMATION;
#endif

typedef BOOL (WINAPI *GetPerformanceInfoFunc)(
                                              PPERFORMANCE_INFORMATION pPerformanceInformation,
                                              DWORD cb
                                             );

typedef BOOL(CALLBACK *PENUM_PAGE_CALLBACK)(LPVOID,PENUM_PAGE_FILE_INFORMATION,LPCTSTR);
typedef BOOL (WINAPI *EnumPageFilesFunc)(
                                         PENUM_PAGE_CALLBACK pCallbackRoutine,
                                         LPVOID lpContext
                                        );

typedef struct _PAGEFILEINFO
{
  SIZE_T TotalSize;
  SIZE_T CurrentUsage;
} PAGEFILEINFO;

#define SystemProcessorPerformanceInformation 0x8
#define MAX_NUM_PROCESSORS 32
typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
{
  LARGE_INTEGER IdleTime;
  LARGE_INTEGER KernelTime;
  LARGE_INTEGER UserTime;
  LARGE_INTEGER DpcTime;
  LARGE_INTEGER InterruptTime;
  ULARGE_INTEGER InterruptCount;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

typedef LONG (WINAPI *NtQuerySystemInformationFunc)(UINT, PVOID, ULONG, PULONG);

class Settings;

class Applet: public BaseApplet
{
private:
  int cursorOffsetX, cursorOffsetY;
  static LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
  void UpdateBars();
  void GetCPUUsages(std::vector<BYTE>& usages);
  void GetMemUsage(int& commitCharge, int& physical, int& pagefile, int& usedMBytes);
  void DrawBar(HDC hdc, RECT& rect, int percentage, int alpha, COLORREF color);
  void DrawHistoryGraph(HDC hdc, RECT& rect, const std::vector<BYTE>& history, int alpha, COLORREF color);
  void DrawItem(HDC hdc, RECT rect, BYTE value, std::vector<BYTE>* history, bool gradient, bool cpu, std::tr1::shared_ptr<Settings> pSettings);
  void DrawTextAndAdjustRect(HDC hdc, int percent, RECT& rect);
  void StartTimer();
  void StopTimer();
  void AddToHistory(int CPU, int value);
  unsigned int numberOfProcessors;
  std::vector<BYTE> CPUUsages;
  int commitCharge;
  int physicalUsage;
  int pagefile;
  int usedMBytes;
  std::vector< std::vector<BYTE> > usageHistory;
  std::tr1::shared_ptr<Settings> pSettings;
  HFONT mainFont;
  int requiredTextWidth;
  int requiredTextHeight;

public:
  Applet(HINSTANCE hInstance);
  ~Applet();
  UINT Initialize();
  LRESULT DoTimer(UINT_PTR timerID);
  LRESULT DoEmergeNotify(UINT messageClass, UINT message);
  LRESULT DoNCRButtonUp();
  LRESULT DoMoving(HWND hwnd, RECT *lpRect);
  LRESULT DoSizing(HWND hwnd, UINT edge, LPRECT rect);
  void ShowConfig();
  LRESULT PaintContent(HDC hdc, RECT clientrt);
  void AppletUpdate();
};

#endif
