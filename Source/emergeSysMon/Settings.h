//---
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
//---

#ifndef __ESM_SETTINGS_H
#define __ESM_SETTINGS_H

#define UNICODE 1

#include <string>
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
  void SetBarDirection(std::wstring direction);
  void SetNumberPosition(std::wstring position);
  void SetFont(LOGFONT* font);
  void SetCPUGradientMethod(std::wstring method);
  void SetCPUGradientFrom(COLORREF col);
  void SetCPUGradientTo(COLORREF to);
  void SetMemGradientMethod(std::wstring method);
  void SetMemGradientFrom(COLORREF col);
  void SetMemGradientTo(COLORREF to);
  int GetUpdateInterval();
  bool GetMonitorCPU();
  bool GetMonitorCommitCharge();
  bool GetMonitorPhysicalMem();
  bool GetMonitorPagefile();
  bool GetHistoryMode();
  bool GetShowNumbers();
  std::wstring GetBarDirection();
  std::wstring GetNumberPosition();
  LOGFONT* GetFont();
  std::wstring GetCPUGradientMethod();
  COLORREF GetCPUGradientFrom();
  COLORREF GetCPUGradientTo();
  std::wstring GetMemGradientMethod();
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
  std::wstring barDirection;
  std::wstring numberPosition;
  std::wstring fontString;
  std::wstring CPUGradientMethod;
  COLORREF CPUGradientFrom;
  COLORREF CPUGradientTo;
  std::wstring MemGradientMethod;
  COLORREF MemGradientFrom;
  COLORREF MemGradientTo;
  LOGFONT logFont;
};

#endif
