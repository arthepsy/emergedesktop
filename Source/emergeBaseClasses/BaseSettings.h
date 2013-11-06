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

#ifndef __EB_BASESETTINGS_H
#define __EB_BASESETTINGS_H

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#ifdef EMERGEBASECLASSES_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x501

#include <shlwapi.h>
#include "../emergeStyleEngine/emergeStyleEngine.h"
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeUtilityLib.h"
#include "../emergeLib/emergeWindowLib.h"
#include "../tinyxml/tinyxml.h"

typedef struct _SORTINFO
{
  bool ascending;
  int subItem;
} SORTINFO, *PSORTINFO;

class DLL_EXPORT BaseSettings
{
public:
  class DLL_EXPORT IOHelper
  {
  private:
    std::wstring subkeyName;
    TiXmlElement* section, *item, *target;
    int itemIndex, readIndex;
  public:
    IOHelper(TiXmlElement* sec);
    ~IOHelper();

    bool GetElement();
    void* GetElement(std::wstring name);
    bool SetElement(std::wstring name);
    bool GetElementText(std::wstring text);
    bool RemoveElement(std::wstring name);
    void* GetTarget();
    void Clear();
    TiXmlElement* GetSection();
    COLORREF ReadColor(std::wstring name, COLORREF defaultValue);
    bool ReadBool(std::wstring name, bool defaultValue);
    int ReadInt(std::wstring name, int defaultValue);
    float ReadFloat(std::wstring name, float defaultValue);
    std::wstring ReadString(std::wstring name, std::wstring defaultValue);
    RECT ReadRect(std::wstring name, RECT defaultValue);
    bool WriteColor(std::wstring name, COLORREF color);
    bool WriteBool(std::wstring name, bool data);
    bool WriteInt(std::wstring name, int data);
    bool WriteFloat(std::wstring name, float data);
    bool WriteString(std::wstring name, std::wstring data);
    bool WriteRect(std::wstring name, RECT data);
  };

  BaseSettings(bool allowAutoSize);
  virtual ~BaseSettings();
  void Init(HWND appletWnd, std::wstring appletName, int appletCount);
  void ReadSettings();
  int GetIconSize();
  int GetIconSpacing();
  virtual int GetX();
  virtual int GetY();
  int GetWidth();
  int GetHeight();
  int GetAppletMonitor();
  std::wstring GetZPosition();
  std::wstring GetHorizontalDirection();
  std::wstring GetVerticalDirection();
  std::wstring GetDirectionOrientation();
  std::wstring GetAnchorPoint();
  bool GetAutoSize();
  bool GetSnapMove();
  bool GetSnapSize();
  bool GetDynamicPositioning();
  std::wstring GetStyleFile();
  int GetClickThrough();
  LOGFONT* GetTitleBarFont();
  std::wstring GetTitleBarText();
  bool SetPosition();
  bool SetSize(int width, int height);
  bool SetZPosition(std::wstring zPosition);
  bool SetHorizontalDirection(std::wstring horizontalDirection);
  bool SetVerticalDirection(std::wstring verticalDirection);
  bool SetDirectionOrientation(std::wstring directionOrientation);
  bool SetAnchorPoint(std::wstring anchorPoint);
  bool SetAutoSize(bool autoSize);
  bool SetSnapMove(bool snapMove);
  bool SetSnapSize(bool snapSize);
  bool SetDynamicPositioning(bool dynamicPositioning);
  bool SetStyleFile(std::wstring styleFile);
  bool SetIconSize(int iconSize);
  bool SetIconSpacing(int iconSpacing);
  RECT* GetResolution();
  bool SetResolution(RECT* deskRect);
  bool SetClickThrough(int clickThrough);
  bool WriteSettings();
  bool SetAppletMonitor(int monitor);
  bool SetTitleBarFont(LOGFONT* titleBarFont);
  void SetTitleBarText(std::wstring titleBarText);
  bool ModifiedCheck();
  void SetModified();
  void ClearModified();
  bool GetModified();
  bool CopyTheme();
  bool CopyStyle();
  bool GetSortInfo(std::wstring editorName, PSORTINFO sortInfo);
  bool SetSortInfo(std::wstring editorName, PSORTINFO sortInfo);
  POINT InstancePosition(SIZE appletSize);
  bool GetStartHidden();
  bool SetStartHidden(bool startHidden);
  int GetAutoSizeLimit();
  bool SetAutoSizeLimit(int autoSizeLimit);

protected:
  virtual void DoReadSettings(IOHelper& helper);
  virtual void DoWriteSettings(IOHelper& helper);
  virtual void DoInitialize();
  virtual void ResetDefaults();
  int x, y;
  int width, height, iconSize, iconSpacing, clickThrough, appletMonitor;
  std::wstring horizontalDirection, verticalDirection, directionOrientation;
  bool autoSize, snapMove, snapSize, dynamicPositioning;
  std::wstring anchorPoint;
  std::wstring zPosition;
  std::wstring appletName;
  int appletCount;
  bool startHidden;
  std::wstring titleBarText;
  LOGFONT titleBarFont;
  int autoSizeLimit;

private:
  HWND appletWnd;
  std::wstring keyString;
  bool allowAutoSize;
  std::wstring styleFile;
  bool modifiedFlag;
  std::wstring oldTheme, defaultTheme, GBRYTheme;
  std::wstring titleBarFontString;
};

#endif

