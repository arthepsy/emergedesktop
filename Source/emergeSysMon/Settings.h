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

#ifndef __SETTINGS_H
#define __SETTINGS_H

#include "../emergeLib/emergeLib.h"
#include "../emergeBaseClasses/BaseSettings.h"
#include "resource.h"

class Settings: public BaseSettings
{
public:
  Settings();
  void SetUpdateInterval(int ival);
  void SetMonitorCPU(bool cpu);
  void SetMonitorCommitCharge(bool mem);
  void SetMonitorPhysicalMem(bool mem);
  void SetMonitorPagefile(bool mem);
  void SetHistoryMode(bool b);
  void SetShowNumbers(bool b);
  void SetBarDirection(WCHAR* dir);
  void SetNumberPosition(WCHAR* str);
  void SetFont(LOGFONT* font);
  void SetCPUGradientMethod(const WCHAR* str);
  void SetCPUGradientFrom(COLORREF col);
  void SetCPUGradientTo(COLORREF to);
  void SetMemGradientMethod(const WCHAR* str);
  void SetMemGradientFrom(COLORREF col);
  void SetMemGradientTo(COLORREF to);
  int GetUpdateInterval();
  bool GetMonitorCPU();
  bool GetMonitorCommitCharge();
  bool GetMonitorPhysicalMem();
  bool GetMonitorPagefile();
  bool GetHistoryMode();
  bool GetShowNumbers();
  const WCHAR *GetBarDirection();
  const WCHAR *GetNumberPosition();
  LOGFONT* GetFont();
  const WCHAR *GetCPUGradientMethod();
  COLORREF GetCPUGradientFrom();
  COLORREF GetCPUGradientTo();
  const WCHAR *GetMemGradientMethod();
  COLORREF GetMemGradientFrom();
  COLORREF GetMemGradientTo();

protected:
  virtual void DoReadSettings(IOHelper& helper);
  virtual void DoWriteSettings(IOHelper& helper);
  virtual void ResetDefaults();
  virtual void DoInitialize();

private:
  int updateInterval;
  bool monitorCPU;
  bool monitorCommitCharge;
  bool monitorPhysicalMem;
  bool monitorPagefile;
  bool historyMode;
  bool showNumbers;
  WCHAR barDirection[MAX_LINE_LENGTH];
  WCHAR numberPosition[MAX_LINE_LENGTH];
  WCHAR fontString[MAX_LINE_LENGTH];
  WCHAR CPUGradientMethod[MAX_LINE_LENGTH];
  COLORREF CPUGradientFrom;
  COLORREF CPUGradientTo;
  WCHAR MemGradientMethod[MAX_LINE_LENGTH];
  COLORREF MemGradientFrom;
  COLORREF MemGradientTo;
  LOGFONT logFont;
};

#endif
