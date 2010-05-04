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

#ifndef __BASESETTINGS_H
#define __BASESETTINGS_H

#define UNICODE 1

#ifdef EMERGEBASECLASSES_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif

#include "../emergeLib/emergeLib.h"
#include "../emergeSchemeEngine/emergeSchemeEngine.h"
#include <shlwapi.h>

class DLL_EXPORT BaseSettings
{
public:
  class DLL_EXPORT IOHelper
  {
  private:
    HKEY key, targetKey;
    std::wstring subkeyName;
    TiXmlElement *section, *item, *target;
    int itemIndex, readIndex;
  public:
    IOHelper(TiXmlElement *sec);
    IOHelper(HKEY key);
    ~IOHelper();

    bool GetElement();
    void *GetElement(WCHAR *name);
    bool SetElement(const WCHAR *name);
    bool GetElementText(WCHAR *text);
    void *GetTarget();
    void Clear();
    bool ReadColor(const WCHAR* name, COLORREF& color, COLORREF def);
    bool ReadBool(const WCHAR* name, bool& data, bool def);
    bool ReadInt(const WCHAR* name, int& data, int def);
    bool ReadFloat(const WCHAR* name, float& data, float def);
    bool ReadString(const WCHAR* name, WCHAR* data, const WCHAR* def);
    bool ReadRect(const WCHAR* name, RECT& data, RECT& def);
    bool WriteColor(const WCHAR* name, COLORREF color);
    bool WriteBool(const WCHAR* name, bool data);
    bool WriteInt(const WCHAR* name, int data);
    bool WriteFloat(const WCHAR* name, float data);
    bool WriteString(const WCHAR* name, WCHAR* data);
    bool WriteRect(const WCHAR* name, RECT& data);
  };

  BaseSettings(bool allowAutoSize);
  virtual ~BaseSettings();
  void Init(HWND appletWnd, WCHAR *appletName);
  void ReadSettings();
  int GetIconSize();
  int GetIconSpacing();
  int GetX();
  int GetY();
  int GetWidth();
  int GetHeight();
  int GetAppletMonitor();
  WCHAR *GetZPosition();
  WCHAR *GetHorizontalDirection();
  WCHAR *GetVerticalDirection();
  WCHAR *GetDirectionOrientation();
  WCHAR *GetAnchorPoint();
  bool GetAutoSize();
  bool GetSnapMove();
  bool GetSnapSize();
  bool GetDynamicPositioning();
  WCHAR *GetSchemeFile();
  int GetClickThrough();
  bool SetPosition();
  bool SetSize(int width, int height);
  bool SetZPosition(WCHAR *zPosition);
  bool SetHorizontalDirection(WCHAR *horizontalDirection);
  bool SetVerticalDirection(WCHAR *verticalDirection);
  bool SetDirectionOrientation(WCHAR *directionOrientation);
  bool SetAnchorPoint(WCHAR *anchorPoint);
  bool SetAutoSize(bool autoSize);
  bool SetSnapMove(bool snapMove);
  bool SetSnapSize(bool snapSize);
  bool SetDynamicPositioning(bool dynamicPositioning);
  bool SetSchemeFile(const WCHAR *schemeFile);
  bool SetIconSize(int iconSize);
  bool SetIconSpacing(int iconSpacing);
  RECT *GetResolution();
  bool SetResolution(RECT *deskRect);
  bool SetClickThrough(int clickThrough);
  bool WriteSettings();
  bool SetAppletMonitor(int monitor);
  bool ModifiedCheck();
  void SetModified();
  void ClearModified();
  bool GetModified();
  bool CopyTheme();

protected:
  virtual void DoReadSettings(IOHelper& helper);
  virtual void DoWriteSettings(IOHelper& helper);
  virtual void DoInitialize();
  virtual void ResetDefaults();
  int x, y;

private:
  HWND appletWnd;
  int width, height, iconSize, iconSpacing, clickThrough, appletMonitor;
  WCHAR zPosition[MAX_LINE_LENGTH];
  WCHAR keyString[MAX_LINE_LENGTH];
  WCHAR horizontalDirection[MAX_LINE_LENGTH], verticalDirection[MAX_LINE_LENGTH],
  directionOrientation[MAX_LINE_LENGTH], appletName[MAX_LINE_LENGTH];
  bool autoSize, snapMove, snapSize, dynamicPositioning, allowAutoSize;
  WCHAR schemeFile[MAX_PATH];
  WCHAR anchorPoint[MAX_LINE_LENGTH];
  bool modifiedFlag;
  std::wstring oldTheme, defaultTheme, GBRYTheme;
};

#endif

