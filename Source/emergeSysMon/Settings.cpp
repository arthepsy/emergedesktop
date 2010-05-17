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

#include "Settings.h"
#include <stdio.h>

Settings::Settings(): BaseSettings(false)
{
  ZeroMemory(MemGradientMethod, MAX_LINE_LENGTH);
  ZeroMemory(CPUGradientMethod, MAX_LINE_LENGTH);
  ZeroMemory(fontString, MAX_LINE_LENGTH);
  ZeroMemory(numberPosition, MAX_LINE_LENGTH);
  ZeroMemory(barDirection, MAX_LINE_LENGTH);
  showNumbers = false;
  historyMode = false;
  monitorPagefile = false;
  monitorPhysicalMem = false;
  monitorCommitCharge = false;
  monitorCPU = false;
  updateInterval = 0;
}

void Settings::ResetDefaults()
{
  BaseSettings::ResetDefaults();
  updateInterval = 2000;
  monitorCPU = true;
  monitorCommitCharge = true;
  monitorPhysicalMem = false;
  monitorPagefile = false;
  historyMode = true;
  showNumbers = false;
  wcscpy(barDirection, (WCHAR*)TEXT("right"));
  wcscpy(numberPosition, (WCHAR*)TEXT("left"));
  wcscpy(fontString, (WCHAR*)TEXT("Tahoma-12"));
  wcscpy(CPUGradientMethod, (WCHAR*)TEXT("Vertical"));
  CPUGradientFrom = RGB(255, 255, 255);
  CPUGradientTo = RGB(66, 66, 255);
  wcscpy(MemGradientMethod, (WCHAR*)TEXT("Vertical"));
  MemGradientFrom = RGB(255, 0, 0);
  MemGradientTo = RGB(113, 255, 113);
  x = 104;
  y = 104;
}

void Settings::DoReadSettings(IOHelper& helper)
{
  BaseSettings::DoReadSettings(helper);
  helper.ReadInt(TEXT("UpdateInterval"), updateInterval, 2000);
  if (updateInterval < MINIMUM_UPDATE_INTERVAL)
    updateInterval = MINIMUM_UPDATE_INTERVAL;
  helper.ReadBool(TEXT("MonitorCPU"), monitorCPU, true);
  helper.ReadBool(TEXT("MonitorCommitCharge"), monitorCommitCharge, true);
  helper.ReadBool(TEXT("MonitorPhysicalMem"), monitorPhysicalMem, false);
  helper.ReadBool(TEXT("MonitorPagefile"), monitorPagefile, false);
  helper.ReadBool(TEXT("HistoryMode"), historyMode, true);
  helper.ReadBool(TEXT("ShowNumbers"), showNumbers, false);
  helper.ReadString(TEXT("BarDirection"), barDirection, TEXT("right"));
  helper.ReadString(TEXT("NumberPosition"), numberPosition, TEXT("left"));
  helper.ReadString(TEXT("Font"), fontString, TEXT("Tahoma-12"));
  helper.ReadString(TEXT("CPUGradientMethod"), CPUGradientMethod, TEXT("Vertical"));
  helper.ReadColor(TEXT("CPUGradientFrom"), CPUGradientFrom, RGB(255, 255, 255));
  helper.ReadColor(TEXT("CPUGradientTo"), CPUGradientTo, RGB(66, 66, 255));
  helper.ReadString(TEXT("MemGradientMethod"), MemGradientMethod, TEXT("Vertical"));
  helper.ReadColor(TEXT("MemGradientFrom"), MemGradientFrom, RGB(255, 0, 0));
  helper.ReadColor(TEXT("MemGradientTo"), MemGradientTo, RGB(113, 255, 113));
}

void Settings::DoWriteSettings(IOHelper& helper)
{
  BaseSettings::DoWriteSettings(helper);
  helper.WriteInt(TEXT("UpdateInterval"), updateInterval);
  helper.WriteBool(TEXT("MonitorCPU"), monitorCPU);
  helper.WriteBool(TEXT("MonitorCommitCharge"), monitorCommitCharge);
  helper.WriteBool(TEXT("MonitorPhysicalMem"), monitorPhysicalMem);
  helper.WriteBool(TEXT("MonitorPagefile"), monitorPagefile);
  helper.WriteBool(TEXT("HistoryMode"), historyMode);
  helper.WriteBool(TEXT("ShowNumbers"), showNumbers);
  helper.WriteString(TEXT("BarDirection"), barDirection);
  helper.WriteString(TEXT("NumberPosition"), numberPosition);
  EGFontToString(logFont, fontString);
  helper.WriteString(TEXT("Font"), fontString);
  helper.WriteString(TEXT("CPUGradientMethod"), CPUGradientMethod);
  helper.WriteColor(TEXT("CPUGradientFrom"), CPUGradientFrom);
  helper.WriteColor(TEXT("CPUGradientTo"), CPUGradientTo);
  helper.WriteString(TEXT("MemGradientMethod"), MemGradientMethod);
  helper.WriteColor(TEXT("MemGradientFrom"), MemGradientFrom);
  helper.WriteColor(TEXT("MemGradientTo"), MemGradientTo);
}

void Settings::DoInitialize()
{
  BaseSettings::DoInitialize();
  EGStringToFont(fontString, logFont);
}

void Settings::SetUpdateInterval(int interval)
{
  if (interval < MINIMUM_UPDATE_INTERVAL)
    interval = MINIMUM_UPDATE_INTERVAL;
  if (interval != updateInterval)
    {
      updateInterval = interval;
      SetModified();
    }
}

void Settings::SetMonitorCPU(bool cpu)
{
  if (monitorCPU != cpu)
    {
      monitorCPU = cpu;
      SetModified();
    }
}

void Settings::SetMonitorCommitCharge(bool mem)
{
  if (monitorCommitCharge != mem)
    {
      monitorCommitCharge = mem;
      SetModified();
    }
}

void Settings::SetMonitorPhysicalMem(bool mem)
{
  if (monitorPhysicalMem != mem)
    {
      monitorPhysicalMem = mem;
      SetModified();
    }
}

void Settings::SetMonitorPagefile(bool mem)
{
  if (monitorPagefile != mem)
    {
      monitorPagefile = mem;
      SetModified();
    }
}

void Settings::SetHistoryMode(bool b)
{
  if (historyMode != b)
    {
      historyMode = b;
      SetModified();
    }
}

void Settings::SetShowNumbers(bool b)
{
  if (showNumbers != b)
    {
      showNumbers = b;
      SetModified();
    }
}

void Settings::SetBarDirection(WCHAR *dir)
{
  if (_wcsicmp(barDirection, dir) != 0)
    {
      wcscpy(barDirection, dir);
      SetModified();
    }
}

void Settings::SetNumberPosition(WCHAR* str)
{
  if (_wcsicmp(numberPosition, str) != 0)
    {
      wcscpy(numberPosition, str);
      SetModified();
    }
}

void Settings::SetFont(LOGFONT* font)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  EGFontToString(*font, tmp);
  if (!EGEqualLogFont(this->logFont, *font))
    {
      wcscpy(fontString, tmp);
      CopyMemory(&logFont, font, sizeof(LOGFONT));
      SetModified();
    }
}

void Settings::SetCPUGradientMethod(const WCHAR* str)
{
  if (_wcsicmp(CPUGradientMethod, str) != 0)
    {
      wcscpy(CPUGradientMethod, str);
      SetModified();
    }
}

void Settings::SetCPUGradientFrom(COLORREF col)
{
  if (CPUGradientFrom != col)
    {
      CPUGradientFrom = col;
      SetModified();
    }
}

void Settings::SetCPUGradientTo(COLORREF col)
{
  if (CPUGradientTo != col)
    {
      CPUGradientTo = col;
      SetModified();
    }
}

void Settings::SetMemGradientMethod(const WCHAR* str)
{
  if (_wcsicmp(MemGradientMethod, str) != 0)
    {
      wcscpy(MemGradientMethod, str);
      SetModified();
    }
}

void Settings::SetMemGradientFrom(COLORREF col)
{
  if (MemGradientFrom != col)
    {
      MemGradientFrom = col;
      SetModified();
    }
}

void Settings::SetMemGradientTo(COLORREF col)
{
  if (MemGradientTo != col)
    {
      MemGradientTo = col;
      SetModified();
    }
}

int Settings::GetUpdateInterval()
{
  return updateInterval;
}

bool Settings::GetMonitorCPU()
{
  return monitorCPU;
}

bool Settings::GetMonitorCommitCharge()
{
  return monitorCommitCharge;
}

bool Settings::GetMonitorPhysicalMem()
{
  return monitorPhysicalMem;
}

bool Settings::GetMonitorPagefile()
{
  return monitorPagefile;
}

bool Settings::GetHistoryMode()
{
  return historyMode;
}

bool Settings::GetShowNumbers()
{
  return showNumbers;
}

const WCHAR *Settings::GetBarDirection()
{
  return barDirection;
}

const WCHAR *Settings::GetNumberPosition()
{
  return numberPosition;
}

LOGFONT* Settings::GetFont()
{
  return &logFont;
}

const WCHAR *Settings::GetCPUGradientMethod()
{
  return CPUGradientMethod;
}

COLORREF Settings::GetCPUGradientFrom()
{
  return CPUGradientFrom;
}

COLORREF Settings::GetCPUGradientTo()
{
  return CPUGradientTo;
}

const WCHAR *Settings::GetMemGradientMethod()
{
  return MemGradientMethod;
}

COLORREF Settings::GetMemGradientFrom()
{
  return MemGradientFrom;
}

COLORREF Settings::GetMemGradientTo()
{
  return MemGradientTo;
}
