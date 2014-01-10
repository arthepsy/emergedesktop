//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2013  The Emerge Desktop Development Team
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
  MemGradientMethod = TEXT("");
  CPUGradientMethod = TEXT("");
  fontString = TEXT("");
  numberPosition = TEXT("");
  barDirection = TEXT("");
  showNumbers = false;
  historyMode = false;
  monitorPagefile = false;
  monitorPhysicalMem = false;
  monitorCommitCharge = false;
  monitorCPU = false;
  updateInterval = 0;
  CPUGradientFrom = RGB(0, 0, 0);
  CPUGradientTo = RGB(0, 0, 0);
  MemGradientFrom = RGB(0, 0, 0);
  MemGradientTo = RGB(0, 0, 0);
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
  barDirection = TEXT("right");
  numberPosition = TEXT("left");
  fontString = TEXT("Tahoma-12");
  CPUGradientMethod = TEXT("Vertical");
  CPUGradientFrom = RGB(255, 255, 255);
  CPUGradientTo = RGB(66, 66, 255);
  MemGradientMethod = TEXT("Vertical");
  MemGradientFrom = RGB(255, 0, 0);
  MemGradientTo = RGB(113, 255, 113);
  // If appletCount > 0 assume this is a new instance and place it at the centre
  // of the screen.
  if (appletCount > 0)
  {
    POINT origin;
    SIZE appletSize;

    appletSize.cx = width;
    appletSize.cy = height;

    origin = InstancePosition(appletSize);
    x = origin.x;
    y = origin.y;
  }
  else
  {
    x = 108;
  }
}

void Settings::DoReadSettings(IOHelper& helper)
{
  // Clear the logFont on read so there isn't any font style transposing between themes
  ZeroMemory(&logFont, sizeof(LOGFONT));

  BaseSettings::DoReadSettings(helper);
  updateInterval = helper.ReadInt(TEXT("UpdateInterval"), 2000);
  if (updateInterval < MINIMUM_UPDATE_INTERVAL)
  {
    updateInterval = MINIMUM_UPDATE_INTERVAL;
  }
  monitorCPU = helper.ReadBool(TEXT("MonitorCPU"), true);
  monitorCommitCharge = helper.ReadBool(TEXT("MonitorCommitCharge"), true);
  monitorPhysicalMem = helper.ReadBool(TEXT("MonitorPhysicalMem"), false);
  monitorPagefile = helper.ReadBool(TEXT("MonitorPagefile"), false);
  historyMode = helper.ReadBool(TEXT("HistoryMode"), true);
  showNumbers = helper.ReadBool(TEXT("ShowNumbers"), false);
  barDirection = helper.ReadString(TEXT("BarDirection"), TEXT("right"));
  numberPosition = helper.ReadString(TEXT("NumberPosition"), TEXT("left"));
  fontString = helper.ReadString(TEXT("Font"), TEXT("Tahoma-12"));
  CPUGradientMethod = helper.ReadString(TEXT("CPUGradientMethod"), TEXT("Vertical"));
  CPUGradientFrom = helper.ReadColor(TEXT("CPUGradientFrom"), RGB(255, 255, 255));
  CPUGradientTo = helper.ReadColor(TEXT("CPUGradientTo"), RGB(66, 66, 255));
  MemGradientMethod = helper.ReadString(TEXT("MemGradientMethod"), TEXT("Vertical"));
  MemGradientFrom = helper.ReadColor(TEXT("MemGradientFrom"), RGB(255, 0, 0));
  MemGradientTo = helper.ReadColor(TEXT("MemGradientTo"), RGB(113, 255, 113));
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
  fontString = EGFontToString(logFont);
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
  logFont = EGStringToFont(fontString);
}

void Settings::SetUpdateInterval(int interval)
{
  if (interval < MINIMUM_UPDATE_INTERVAL)
  {
    interval = MINIMUM_UPDATE_INTERVAL;
  }
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

void Settings::SetBarDirection(std::wstring direction)
{
  if (ELToLower(barDirection) != ELToLower(direction))
  {
    barDirection = direction;
    SetModified();
  }
}

void Settings::SetNumberPosition(std::wstring position)
{
  if (ELToLower(numberPosition) != ELToLower(position))
  {
    numberPosition = position;
    SetModified();
  }
}

void Settings::SetFont(LOGFONT* font)
{
  if (!EGEqualLogFont(this->logFont, *font))
  {
    fontString = EGFontToString(*font);
    CopyMemory(&logFont, font, sizeof(LOGFONT));
    SetModified();
  }
}

void Settings::SetCPUGradientMethod(std::wstring method)
{
  if (ELToLower(CPUGradientMethod) != ELToLower(method))
  {
    CPUGradientMethod = method;
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

void Settings::SetMemGradientMethod(std::wstring method)
{
  if (ELToLower(MemGradientMethod) != ELToLower(method))
  {
    MemGradientMethod = method;
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

std::wstring Settings::GetBarDirection()
{
  return barDirection;
}

std::wstring Settings::GetNumberPosition()
{
  return numberPosition;
}

LOGFONT* Settings::GetFont()
{
  return &logFont;
}

std::wstring Settings::GetCPUGradientMethod()
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

std::wstring Settings::GetMemGradientMethod()
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
