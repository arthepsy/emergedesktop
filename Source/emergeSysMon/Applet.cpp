// vim:tags+=../emergeLib/tags,../emergeBaseClasses/tags,../emergeGraphics/tags
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

#include "Applet.h"
#include "Settings.h"
#include "Config.h"

WCHAR myName[] = TEXT("emergeSysMon");

//----  --------------------------------------------------------------------------------------------------------
// Function:	WindowProcedure
// Required:	HWND hwnd - window handle that message was sent to
// 		UINT message - action to handle
// 		WPARAM wParam - dependant on message
// 		LPARAM lParam - dependant on message
// Returns:	LRESULT
// Purpose:	Handles messages sent from DispatchMessage
//----  --------------------------------------------------------------------------------------------------------
LRESULT CALLBACK Applet::WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  CREATESTRUCT *cs;
  static Applet *pApplet = NULL;

  if (message == WM_CREATE)
    {
      // Register to recieve the specified Emerge Desktop messages
      PostMessage(ELGetCoreWindow(), EMERGE_REGISTER, (WPARAM)hwnd, (LPARAM)EMERGE_CORE);

      cs = (CREATESTRUCT*)lParam;
      pApplet = reinterpret_cast<Applet*>(cs->lpCreateParams);
      return DefWindowProc(hwnd, message, wParam, lParam);
    }

  if (pApplet == NULL)
    return DefWindowProc(hwnd, message, wParam, lParam);

  switch (message)
    {
    case WM_COPYDATA:
      return pApplet->DoCopyData((COPYDATASTRUCT *)lParam);

      // Needed to handle changing the system colors.  It forces
      // a repaint of the window as well as the frame.
    case WM_SYSCOLORCHANGE:
      return pApplet->DoSysColorChange();

      // Allow for window dragging via Ctrl - Left - Click dragging
    case WM_NCLBUTTONDOWN:
      pApplet->DoNCLButtonDown();
      return DefWindowProc(hwnd, message, wParam, lParam);

      // Display the mainMenu via Ctrl - Right - Click
    case WM_NCRBUTTONUP:
      return pApplet->DoNCRButtonUp();

      // Reset the cursor back to the standard arrow after dragging
    case WM_NCLBUTTONUP:
      pApplet->DoNCLButtonUp();
      return DefWindowProc(hwnd, message, wParam, lParam);

    case WM_SETCURSOR:
      pApplet->DoSetCursor();
      return DefWindowProc(hwnd, message, wParam, lParam);

      // Handles the resizing of the window
    case WM_NCHITTEST:
      return pApplet->DoHitTest(lParam);

      // Repaint the icons as the window size is changing
    case WM_WINDOWPOSCHANGING:
      return pApplet->DoWindowPosChanging((WINDOWPOS *)lParam);

    case WM_ENTERSIZEMOVE:
      return pApplet->DoEnterSizeMove(hwnd);

    case WM_EXITSIZEMOVE:
      return pApplet->DoExitSizeMove(hwnd);

    case WM_SIZING:
      return pApplet->DoSizing(hwnd, (UINT)wParam, (LPRECT)lParam);

    case WM_MOVING:
      return pApplet->DoMoving(hwnd, (LPRECT)lParam);

    case WM_DISPLAYCHANGE:
      return pApplet->DoDisplayChange(hwnd);

    case WM_SYSCOMMAND:
      return pApplet->DoSysCommand(hwnd, message, wParam, lParam);

    case WM_TIMER:
      return pApplet->DoTimer((UINT_PTR)wParam);

    case WM_DESTROY:
    case WM_NCDESTROY:
      pApplet->StopTimer();

      // Unregister the specified Emerge Desktop messages
      PostMessage(ELGetCoreWindow(), EMERGE_UNREGISTER, (WPARAM)hwnd, (LPARAM)EMERGE_CORE);

      PostQuitMessage(0);
      break;

      // If not handled just forward the message on
    default:
      return pApplet->DoDefault(hwnd, message, wParam, lParam);
    }

  return 0;
}

Applet::Applet(HINSTANCE hInstance)
:BaseApplet(hInstance, myName, false, true)
{
  requiredTextHeight = 0;
  requiredTextWidth = 0;
  usedMBytes = 0;
  pagefile = 0;
  physicalUsage = 0;
  commitCharge = 0;
  numberOfProcessors = 0;
  cursorOffsetX = 0;
  cursorOffsetY = 0;

  mainFont = NULL;
}

Applet::~Applet()
{
  if (mainFont)
    DeleteObject(mainFont);
}

UINT Applet::Initialize()
{
  pSettings = std::tr1::shared_ptr<Settings>(new Settings());

  UINT ret = BaseApplet::Initialize(WindowProcedure, this, pSettings);
  if (ret == 0)
    return ret;

  // Initialize number of processors and usageHistory
  SYSTEM_INFO sysinf;
  GetSystemInfo(&sysinf);
  numberOfProcessors = sysinf.dwNumberOfProcessors;
  for (UINT i = 0; i < numberOfProcessors + 3; ++i)
    {
      if (i < numberOfProcessors)
        CPUUsages.push_back(0);
      // placeholder for memory usage stats
      usageHistory.push_back(std::vector<BYTE>());
      usageHistory.push_back(std::vector<BYTE>());
      usageHistory.push_back(std::vector<BYTE>());
    }
  commitCharge = physicalUsage = pagefile = 0;

  // Set the window transparency
  UpdateGUI();

  // setup timer
  StartTimer();
  UpdateBars();

  return ret;
}

void Applet::StartTimer()
{
  UINT sysMonID = (ULONG_PTR)mainWnd + 100;
  SetTimer(mainWnd, sysMonID, pSettings->GetUpdateInterval(), NULL);
}

void Applet::StopTimer()
{
  UINT sysMonID = (ULONG_PTR)mainWnd + 100;
  KillTimer(mainWnd, sysMonID);
}

LRESULT Applet::PaintContent(HDC hdc, RECT clientrt)
{
  // use minimum 1 pixel between bars
  int padding = (guiInfo.padding > 0 ? guiInfo.padding : 1);

  RECT r = clientrt;
  int totalNumOfBars = (pSettings->GetMonitorCPU() ? CPUUsages.size() : 0) +
    (pSettings->GetMonitorCommitCharge() ? 1 : 0) +
    (pSettings->GetMonitorPhysicalMem() ? 1 : 0) +
    (pSettings->GetMonitorPagefile() ? 1 : 0);
  if (totalNumOfBars == 0)
    return 0;

  // calculate the room for one bar
  bool vertical = (_wcsicmp(pSettings->GetDirectionOrientation(), TEXT("vertical")) == 0);
  int roomForBar = ((vertical ? r.bottom - r.top : r.right - r.left) - padding * (totalNumOfBars - 1)) /
    totalNumOfBars;

  // rect offsets for calculating the next rect
  int offsetX = (vertical ? 0 : roomForBar + padding);
  int offsetY = (vertical ? roomForBar + padding : 0);
  if (vertical)
    r.bottom = r.top + roomForBar;
  else
    r.right = r.left + roomForBar;

  bool gradient = (_wcsicmp(pSettings->GetCPUGradientMethod(), TEXT("solid")) != 0);

  // draw bars
  if (pSettings->GetMonitorCPU())
    {
      for (unsigned int i = 0; i < CPUUsages.size(); ++i)
        {
          RECT tmpRect = r;
          if (pSettings->GetShowNumbers())
            DrawTextAndAdjustRect(hdc, CPUUsages[i], tmpRect);
          DrawItem(hdc, tmpRect, CPUUsages[i], &usageHistory[i], gradient, true, pSettings);
          OffsetRect(&r, offsetX, offsetY);
        }
    }

  gradient = (_wcsicmp(pSettings->GetMemGradientMethod(), TEXT("solid")) != 0);
  if (pSettings->GetMonitorCommitCharge())
    {
      RECT tmpRect = r;
      if (pSettings->GetShowNumbers())
        DrawTextAndAdjustRect(hdc, commitCharge, tmpRect);
      DrawItem(hdc, tmpRect, commitCharge, &usageHistory[numberOfProcessors], gradient, false, pSettings);
      OffsetRect(&r, offsetX, offsetY);
    }
  if (pSettings->GetMonitorPhysicalMem())
    {
      RECT tmpRect = r;
      if (pSettings->GetShowNumbers())
        DrawTextAndAdjustRect(hdc, physicalUsage, tmpRect);
      DrawItem(hdc, tmpRect, physicalUsage, &usageHistory[numberOfProcessors + 1], gradient, false, pSettings);
      OffsetRect(&r, offsetX, offsetY);
    }
  if (pSettings->GetMonitorPagefile())
    {
      RECT tmpRect = r;
      if (pSettings->GetShowNumbers())
        DrawTextAndAdjustRect(hdc, pagefile, tmpRect);
      DrawItem(hdc, tmpRect, pagefile, &usageHistory[numberOfProcessors + 2], gradient, false, pSettings);
      OffsetRect(&r, offsetX, offsetY);
    }

  return 0;
}

void Applet::DrawItem(HDC hdc, RECT rect, BYTE value, std::vector<BYTE>* history, bool gradient, bool cpu, std::tr1::shared_ptr<Settings> pSettings)
{
  RECT bmRect = rect;
  int width = rect.right - rect.left;
  int displayHeight = rect.bottom - rect.top;

  int alpha = 255;
  COLORREF color = (cpu ? pSettings->GetCPUGradientFrom() : pSettings->GetMemGradientFrom());

  HDC maskDC = hdc;
  HDC fillDC = NULL;
  HBITMAP mask = NULL, fillBMP = NULL;
  HGDIOBJ maskObj = NULL, fillObj = NULL;
  if (gradient)
    {
      bmRect.left = bmRect.top = 0;
      bmRect.right = width;
      bmRect.bottom = displayHeight;

      // create fill and mask bitmap
      fillDC = CreateCompatibleDC(NULL);

      fillBMP = EGCreateBitmap(0x00, 0, bmRect);
      fillObj = SelectObject(fillDC, fillBMP);

      maskDC = CreateCompatibleDC(NULL);
      mask = EGCreateBitmap(0x00, RGB(0, 0, 0), bmRect);
      maskObj = SelectObject(maskDC, mask);

      // draw gradient
      HDC bkDC = CreateCompatibleDC(fillDC);
      HBITMAP gradientBMP = CreateCompatibleBitmap(fillDC, bmRect.right - bmRect.left, bmRect.bottom - bmRect.top);
      HGDIOBJ bkObj = SelectObject(bkDC, gradientBMP);
      EGGradientFillRect(bkDC, &bmRect, guiInfo.alphaForeground,
                         (cpu ? pSettings->GetCPUGradientFrom() : pSettings->GetMemGradientFrom()),
                         (cpu ? pSettings->GetCPUGradientTo() : pSettings->GetMemGradientTo()),
                         0, (WCHAR*)(cpu ? pSettings->GetCPUGradientMethod() : pSettings->GetMemGradientMethod()));
      BitBlt(fillDC, bmRect.left, bmRect.top, bmRect.right - bmRect.left, bmRect.bottom - bmRect.top,
             bkDC, 0, 0, SRCCOPY);
      SelectObject(bkDC, bkObj);
      DeleteDC(bkDC);
      DeleteObject(gradientBMP);

      // change drawing color
      alpha = 255;
      color = RGB(255, 255, 255);
    }

  if (pSettings->GetHistoryMode())
    DrawHistoryGraph(maskDC, bmRect, *history, alpha, color);
  else
    DrawBar(maskDC, bmRect, value, alpha, color);

  if (gradient)
    {
      // fill := fill & mask
      BitBlt(fillDC, 0, 0, width, displayHeight, maskDC, 0, 0, SRCAND);

      // invert mask
      BitBlt(maskDC, 0, 0, width, displayHeight, NULL, 0, 0, DSTINVERT);

      // dest := dest & (inverted)mask
      BitBlt(hdc, rect.left, rect.top, width, displayHeight, maskDC, 0, 0, SRCAND);

      // dest := dest | (masked)fill
      BitBlt(hdc, rect.left, rect.top, width, displayHeight, fillDC, 0, 0, SRCPAINT);

      SelectObject(maskDC, maskObj);
      DeleteDC(maskDC);
      DeleteObject(mask);
      SelectObject(fillDC, fillObj);
      DeleteDC(fillDC);
      DeleteObject(fillBMP);
    }
}

void Applet::DrawBar(HDC hdc, RECT& rect, int percentage, int alpha, COLORREF color)
{
  RECT r = rect;
  if (_wcsicmp(pSettings->GetBarDirection(), TEXT("right")) == 0)
    r.right = r.left + (r.right - r.left) * percentage / 100;
  else if (_wcsicmp(pSettings->GetBarDirection(), TEXT("left")) == 0)
    r.left = r.right - (r.right - r.left) * percentage / 100;
  else if (_wcsicmp(pSettings->GetBarDirection(), TEXT("down")) == 0)
    r.bottom = r.top + (r.bottom - r.top) * percentage / 100;
  else if (_wcsicmp(pSettings->GetBarDirection(), TEXT("up")) == 0)
    r.top = r.bottom - (r.bottom - r.top) * percentage / 100;
  EGFillRect(hdc, &r, alpha, color);
}

void Applet::DrawHistoryGraph(HDC hdc, RECT& r, const std::vector<BYTE>& history, int alpha, COLORREF color)
{
  int from = r.right - history.size();
  if (from < r.left)
    from = r.left;
  for (int i = from; i < r.right; ++i)
    {
      int height = (r.bottom - r.top) * history[i - r.right + history.size()] / 100;
      RECT rect;
      rect.left = i;
      rect.right = i + 1;
      rect.bottom = r.bottom;
      rect.top = r.bottom - height;
      EGFillRect(hdc, &rect, alpha, color);
    }
}

void Applet::DrawTextAndAdjustRect(HDC hdc, int percent, RECT& rect)
{
  // calculate textRect
  RECT textRect = rect;
  if (_wcsicmp(pSettings->GetNumberPosition(), TEXT("right")) == 0)
    textRect.left = MAX(textRect.left, textRect.right - requiredTextWidth);
  else if (_wcsicmp(pSettings->GetNumberPosition(), TEXT("left")) == 0)
    textRect.right = MIN(textRect.right, textRect.left + requiredTextWidth);
  else if (_wcsicmp(pSettings->GetNumberPosition(), TEXT("down")) == 0)
    textRect.top = MAX(textRect.top, textRect.bottom - requiredTextHeight);
  else if (_wcsicmp(pSettings->GetNumberPosition(), TEXT("up")) == 0)
    textRect.bottom = MIN(textRect.bottom, textRect.top + requiredTextHeight);

  // draw text
  CLIENTINFO clientInfo;
  FORMATINFO formatInfo;
  formatInfo.horizontalAlignment = EGDAT_HCENTER;
  formatInfo.verticalAlignment = EGDAT_VCENTER;
  formatInfo.font = mainFont;
  formatInfo.color = guiInfo.colorFont;
  formatInfo.flags = 0;

  clientInfo.hdc = hdc;
  CopyRect(&clientInfo.rt, &textRect);
  clientInfo.bgAlpha = guiInfo.alphaBackground;
  WCHAR text[30];
  _itow(percent, text, 10);
  wcscat(text, TEXT("%"));
  EGDrawAlphaText(guiInfo.alphaText, clientInfo, formatInfo, text);

  // adjust rect
  if (rect.left != textRect.left)
    rect.right = textRect.left;
  else if (rect.right != textRect.right)
    rect.left = textRect.right;
  else if (rect.top != textRect.top)
    rect.bottom = textRect.top;
  else if (rect.bottom != textRect.bottom)
    rect.top = textRect.bottom;
}

LRESULT Applet::DoTimer(UINT_PTR timerID)
{
  if (timerID == MOUSE_TIMER)
    {
      return BaseApplet::DoTimer(timerID);
    }
  else
    {
      UpdateBars();
    }

  return 1;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	UpdateBars
// Purpose:	Updates the values of the monitored parameters and redraws the bars
//---- void Applet::UpdateBars() -------------------------------------------------------------------------------
void Applet::UpdateBars()
{
  GetCPUUsages(CPUUsages);
  GetMemUsage(commitCharge, physicalUsage, pagefile, usedMBytes);
  for (UINT i = 0; i < CPUUsages.size(); ++i)
    AddToHistory(i, CPUUsages[i]);
  AddToHistory(numberOfProcessors, commitCharge);
  AddToHistory(numberOfProcessors + 1, physicalUsage);
  AddToHistory(numberOfProcessors + 2, pagefile);

  // create tooltip text
  WCHAR tip[MAX_PATH];
  WCHAR buf[MAX_PATH];
  tip[0] = 0;
  if (pSettings->GetMonitorCPU())
    {
      for (unsigned int i = 0; i < CPUUsages.size(); ++i)
        {
          wcscat(tip, L"CPU");
          if (CPUUsages.size() > 1)
            {
              _itow(i, buf, 10);
              wcscat(tip, buf);
            }
          wcscat(tip, L": ");
          _itow(CPUUsages[i], buf, 10);
          wcscat(tip, buf);
          wcscat(tip, L"%\n");
        }
    }
  if (pSettings->GetMonitorCommitCharge())
    {
      wcscat(tip, L"Commit charge: ");
      _itow(commitCharge, buf, 10);
      wcscat(tip, buf);
      wcscat(tip, L"% (");
      _itow(usedMBytes, buf, 10);
      wcscat(tip, buf);
      wcscat(tip, L"MB)\n");
    }
  if (pSettings->GetMonitorPhysicalMem())
    {
      wcscat(tip, L"Physical: ");
      _itow(physicalUsage, buf, 10);
      wcscat(tip, buf);
      wcscat(tip, L"%\n");
    }
  if (pSettings->GetMonitorPagefile())
    {
      wcscat(tip, L"Pagefile: ");
      _itow(pagefile, buf, 10);
      wcscat(tip, buf);
      wcscat(tip, L"%");
    }


  // update tooltip
  TOOLINFO ti;
  ZeroMemory(&ti, sizeof(TOOLINFO));
  RECT rect;
  bool exists;

  GetClientRect(mainWnd, &rect);

  // fill in the TOOLINFO structure
  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = mainWnd;
  ti.uId = (ULONG_PTR)toolWnd;

  // Check to see if the tooltip exists
  exists = SendMessage(toolWnd, TTM_GETTOOLINFO, 0,(LPARAM) (LPTOOLINFO) &ti) ? true : false;

  //  complete the rest of the TOOLINFO structure
  ti.hinst = mainInst;
  ti.lpszText = tip;
  ti.rect = rect;

  // If it exists, modify the tooltip, if not add it
  if (exists)
    SendMessage(toolWnd, TTM_SETTOOLINFO, 0, (LPARAM)(LPTOOLINFO)&ti);
  else
    SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  DrawAlphaBlend();
}

void Applet::ShowConfig()
{
  Config config(mainInst, mainWnd, appletName, pSettings);
  StopTimer();
  if (config.Show() == IDOK)
    UpdateGUI();
  StartTimer();
}

LRESULT Applet::DoNCRButtonUp()
{
  StopTimer();
  BaseApplet::DoNCRButtonUp();
  StartTimer();
  return 0;
}

LRESULT Applet::DoMoving(HWND hwnd, RECT *lpRect)
{
  StopTimer();
  BaseApplet::DoMoving(hwnd, lpRect);
  StartTimer();
  return 0;
}

LRESULT Applet::DoSizing(HWND hwnd, UINT edge, LPRECT rect)
{
  StopTimer();
  BaseApplet::DoSizing(hwnd, edge, rect);
  StartTimer();
  return 0;
}

void Applet::AppletUpdate()
{
  if (mainFont != NULL)
    DeleteObject(mainFont);
  // create font
  mainFont = CreateFontIndirect(pSettings->GetFont());
  // get required text dimensions
  HDC hdc = CreateCompatibleDC(NULL);
  HGDIOBJ obj = SelectObject(hdc, mainFont);
  RECT r;
  r.left = 0;
  r.top = 0;
  requiredTextHeight = DrawTextEx(hdc, (WCHAR*)TEXT("100%"), 4, &r, DT_CENTER | DT_VCENTER | DT_CALCRECT, NULL);
  requiredTextWidth = r.right;
  SelectObject(hdc, obj);
  DeleteDC(hdc);
}

BOOL CALLBACK PageFileEnum(LPVOID lpContext, PENUM_PAGE_FILE_INFORMATION pPageFileInfo, LPCTSTR lpFilename UNUSED)
{
  PAGEFILEINFO *pfi = (PAGEFILEINFO*)lpContext;

  pfi->TotalSize += pPageFileInfo->TotalSize;
  pfi->CurrentUsage += pPageFileInfo->TotalInUse;

  return TRUE;
}

void Applet::GetMemUsage(int& commitCharge, int& physical, int& pagefile, int& usedMBytes)
{
  static GetPerformanceInfoFunc GetPerformanceInfo = 0;
  static EnumPageFilesFunc EnumPageFiles = 0;

  // find API function
  if(!GetPerformanceInfo)
    {
      GetPerformanceInfo = (GetPerformanceInfoFunc)GetProcAddress(ELLoadSystemLibrary(L"psapi.dll"), "GetPerformanceInfo");
      if(!GetPerformanceInfo)
        {
          ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("Cannot find system information function!"),
                       (WCHAR*)TEXT("emergeSysMon"), ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
          PostQuitMessage(1);
          return;
        }
    }

  // find API function
  if(!EnumPageFiles)
    {
      EnumPageFiles = (EnumPageFilesFunc)GetProcAddress(ELLoadSystemLibrary(L"psapi.dll"), "EnumPageFilesW");
      if(!EnumPageFiles)
        {
          ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("Cannot find system information function!"),
                       (WCHAR*)TEXT("emergeSysMon"), ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
          PostQuitMessage(1);
          return;
        }
    }

  // ask memory details
  PERFORMANCE_INFORMATION pi;
  PAGEFILEINFO pfi;
  ZeroMemory(&pfi, sizeof(pfi));
  pi.cb = sizeof(pi);
  commitCharge = 1;
  physical = 0;
  pagefile = 0;
  usedMBytes = 1;
  if (GetPerformanceInfo(&pi, sizeof(pi)) && EnumPageFiles(PageFileEnum, (void*)&pfi))
    {
      physical = (int)((pi.PhysicalTotal - pi.PhysicalAvailable) * 100 / pi.PhysicalTotal);
      SIZE_T currentPageFileUsage = (pfi.CurrentUsage * pi.PageSize) / 1024 / 1024;
      SIZE_T totalPageFileSize = (pfi.TotalSize * pi.PageSize) / 1024 / 1024;
      if (totalPageFileSize > 0)
        pagefile = (int)(currentPageFileUsage * 100 / totalPageFileSize);
      DWORDLONG total = (DWORDLONG)pi.CommitTotal * (DWORDLONG)pi.PageSize;
      DWORDLONG limit = (DWORDLONG)pi.CommitLimit * (DWORDLONG)pi.PageSize;
      int totalMB = (int)(total / 1024 / 1024);
      int limitMB = (int)(limit / 1024 / 1024);
      commitCharge = totalMB * 100 / limitMB;
      usedMBytes = totalMB;
    }
}

void Applet::GetCPUUsages(std::vector<BYTE>& usages)
{
  usages.clear();
  static ULARGE_INTEGER  sys_idle_old[32];
  static ULARGE_INTEGER  sys_kernel_old[32];
  static ULARGE_INTEGER  sys_user_old[32];

  static NtQuerySystemInformationFunc NtQuerySystemInformation = 0;

  unsigned long returnlength = 0;
  SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION sppi[MAX_NUM_PROCESSORS];
  memset(sppi, 0, sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * MAX_NUM_PROCESSORS);

  // find API function
  if(!NtQuerySystemInformation)
    {
      NtQuerySystemInformation = (NtQuerySystemInformationFunc)GetProcAddress(ELLoadSystemLibrary(L"ntdll.dll"), "NtQuerySystemInformation");
      if(!NtQuerySystemInformation)
        {
          ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("Cannot find system information function!"),
                       (WCHAR*)TEXT("emergeSysMon"), ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
          PostQuitMessage(1);
          return;
        }

      // init static part
      ZeroMemory(sys_idle_old, sizeof(ULARGE_INTEGER) * 32);
      ZeroMemory(sys_kernel_old, sizeof(ULARGE_INTEGER) * 32);
      ZeroMemory(sys_user_old, sizeof(ULARGE_INTEGER) * 32);
    }

  if (0 == NtQuerySystemInformation(SystemProcessorPerformanceInformation, sppi,
                                    (sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * numberOfProcessors), &returnlength))
    {
      for(unsigned int i = 0; i < numberOfProcessors; ++i)
        {
          ULARGE_INTEGER sys_idle;
          ULARGE_INTEGER sys_kernel;
          ULARGE_INTEGER sys_user;

          sys_idle.QuadPart   = sppi[i].IdleTime.QuadPart;
          sys_kernel.QuadPart = sppi[i].KernelTime.QuadPart;
          sys_user.QuadPart   = sppi[i].UserTime.QuadPart;

          if (sys_idle_old[i].QuadPart != 0)
            {
              BYTE usage;
              int kernelUserTime = ((sys_kernel.QuadPart +  sys_user.QuadPart) -
                                    (sys_kernel_old[i].QuadPart + sys_user_old[i].QuadPart));
              if (kernelUserTime > 0)
                usage = (BYTE) (100 - (((sys_idle.QuadPart - sys_idle_old[i].QuadPart) * 100) / kernelUserTime));
              else
                usage = 0;

              usages.push_back(usage);
            }

          sys_idle_old[i].QuadPart = sys_idle.QuadPart;
          sys_user_old[i].QuadPart = sys_user.QuadPart;
          sys_kernel_old[i].QuadPart = sys_kernel.QuadPart;
        }
    }
}

void Applet::AddToHistory(int CPU, int value)
{
  RECT rect;
  GetClientRect(mainWnd, &rect);
  usageHistory[CPU].push_back(value);
  // do not let usageHistory to grow infinitely
  if (usageHistory[CPU].size() > (UINT)(rect.right - rect.left) && usageHistory[CPU].size() > 0)
    usageHistory[CPU].erase(usageHistory[CPU].begin());
}
