// vim:tags+=../emergeLib/tags
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

#include "BaseSettings.h"
#include <stdio.h>

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

BaseSettings::BaseSettings(bool allowAutoSize)
{
  this->allowAutoSize = allowAutoSize;
  defaultTheme = TEXT("default");
  GBRYTheme = TEXT("GBRY");
}

BaseSettings::~BaseSettings()
{}

void BaseSettings::Init(HWND appletWnd, WCHAR *appletName, int appletCount)
{
  wcscpy(this->appletName, appletName);
  swprintf(keyString, TEXT("%s\\Settings"), appletName);
  this->appletWnd = appletWnd;
  this->appletCount = appletCount;
}

void BaseSettings::ReadSettings()
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section;
  bool readSettings = false;
  std::wstring xmlFile = TEXT("%ThemeDir%\\");
  xmlFile += appletName;
  xmlFile += TEXT(".xml");
  std::wstring theme = ELGetThemeName();
  theme = ELToLower(theme);

  oldTheme = theme;

  if (theme != defaultTheme)
    {
      configXML = ELOpenXMLConfig(xmlFile, false);
      if (configXML)
        {
          section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Settings"), false);

          if (section)
            {
              readSettings = true;

              IOHelper helper(section);
              DoReadSettings(helper);
              ClearModified();
              if (ELPathIsRelative(styleFile))
                ELConvertThemePath(styleFile, CTP_FULL);
            }
        }
    }

  // In the case where there is an issue accessing the XML file, use default values;
  if (!readSettings)
    ResetDefaults();

  DoInitialize();
}

bool BaseSettings::ModifiedCheck()
{
  std::wstring theme = ELGetThemeName();

  if (!ELIsModifiedTheme(theme))
    // Perserve the old theme name
    oldTheme = theme;

  return ELSetModifiedTheme(theme);
}

bool BaseSettings::CopyTheme()
{
  std::wstring oldThemePath, newThemePath;

  oldThemePath = TEXT("%EmergeDir%\\themes\\") + oldTheme;
  oldThemePath += TEXT("\\*");
  newThemePath = TEXT("%ThemeDir%");

  if (!PathIsDirectory(newThemePath.c_str()))
    ELCreateDirectory(newThemePath);

  ELFileOp(appletWnd, FO_COPY, oldThemePath, newThemePath);

  return CopyStyle();
}

bool BaseSettings::WriteSettings()
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section;
  std::wstring xmlFile = TEXT("%ThemeDir%\\");
  xmlFile += appletName;
  xmlFile += TEXT(".xml");
  bool ret = false;

  if (GetModified())
    {
      if (ModifiedCheck())
        CopyTheme();

      std::wstring theme = ELGetThemeName();
      theme = ELToLower(theme);
      if ((theme != defaultTheme) && (theme != GBRYTheme))
        {
          configXML = ELOpenXMLConfig(xmlFile, true);
          if (configXML)
            {
              section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Settings"), true);

              if (section)
                {
                  IOHelper helper(section);
                  DoWriteSettings(helper);

                  ret = ELWriteXMLConfig(configXML.get());
                  oldTheme = ELGetThemeName();
                  ClearModified();
                }
            }
        }
    }

  return ret;
}

void BaseSettings::DoReadSettings(IOHelper& helper)
{
  helper.ReadInt(TEXT("Monitor"), appletMonitor, 0);
  helper.ReadInt(TEXT("X"), x, 0);
  helper.ReadInt(TEXT("Y"), y, 0);
  helper.ReadInt(TEXT("Width"), width, 100);
  helper.ReadInt(TEXT("Height"), height, 100);
  helper.ReadString(TEXT("ZPosition"), zPosition, TEXT("normal"));
  helper.ReadString(TEXT("HorizontalDirection"), horizontalDirection, TEXT("right"));
  helper.ReadString(TEXT("VerticalDirection"), verticalDirection, TEXT("down"));
  helper.ReadString(TEXT("DirectionOrientation"), directionOrientation, (WCHAR*)TEXT("down"));
  helper.ReadBool(TEXT("AutoSize"), autoSize, false);
  helper.ReadInt(TEXT("IconSize"), iconSize, 16);
  helper.ReadInt(TEXT("IconSpacing"), iconSpacing, 1);
  if (!helper.ReadString(TEXT("Style"), styleFile, TEXT("\0")))
    helper.ReadString(TEXT("Scheme"), styleFile, TEXT("\0"));
  helper.ReadBool(TEXT("SnapMove"), snapMove, true);
  helper.ReadBool(TEXT("SnapSize"), snapSize, true);
  helper.ReadBool(TEXT("DynamicPositioning"), dynamicPositioning, true);
  helper.ReadInt(TEXT("ClickThrough"), clickThrough, 0);
  helper.ReadString(TEXT("AnchorPoint"), anchorPoint, (WCHAR*)TEXT("TopLeft"));
}

void BaseSettings::DoWriteSettings(IOHelper& helper)
{
  helper.WriteInt(TEXT("X"), x);
  helper.WriteInt(TEXT("Y"), y);
  helper.WriteInt(TEXT("Width"), width);
  helper.WriteInt(TEXT("Height"), height);
  helper.WriteString(TEXT("ZPosition"), zPosition);
  helper.WriteString(TEXT("HorizontalDirection"), horizontalDirection);
  helper.WriteString(TEXT("VerticalDirection"), verticalDirection);
  helper.WriteString(TEXT("DirectionOrientation"), directionOrientation);
  helper.WriteBool(TEXT("AutoSize"), autoSize);
  helper.WriteInt(TEXT("IconSize"), iconSize);
  helper.WriteInt(TEXT("IconSpacing"), iconSpacing);
  helper.WriteString(TEXT("Style"), styleFile);
  helper.WriteBool(TEXT("SnapMove"), snapMove);
  helper.WriteBool(TEXT("SnapSize"), snapSize);
  helper.WriteBool(TEXT("DynamicPositioning"), dynamicPositioning);
  helper.WriteInt(TEXT("ClickThrough"), clickThrough);
  helper.WriteInt(TEXT("Monitor"), appletMonitor);
  helper.WriteString(TEXT("AnchorPoint"), anchorPoint);
}

void BaseSettings::DoInitialize()
{
  // nothing to be initialized here. Maybe in the derived classes.
}

void BaseSettings::ResetDefaults()
{
  x = 0;
  y = 0;
  width = 100;
  height = 100;
  wcscpy(zPosition, (WCHAR*)TEXT("normal"));
  wcscpy(horizontalDirection, (WCHAR*)TEXT("right"));
  wcscpy(verticalDirection, (WCHAR*)TEXT("down"));
  wcscpy(directionOrientation, (WCHAR*)TEXT("down"));
  autoSize = false;
  iconSize = 16;
  iconSpacing = 1;
  wcscpy(styleFile, (WCHAR*)TEXT("\0"));
  snapMove = true;
  snapSize = true;
  dynamicPositioning = true;
  clickThrough = 0;
  appletMonitor = 0;
  wcscpy(anchorPoint, (WCHAR*)TEXT("TopLeft"));
}

void BaseSettings::SetModified()
{
  modifiedFlag = true;
}

bool BaseSettings::GetModified()
{
  return modifiedFlag;
}

void BaseSettings::ClearModified()
{
  modifiedFlag = false;
}

bool BaseSettings::SetSize(int width, int height)
{
  if (this->width != width || this->height != height)
    {
      this->width = width;
      this->height = height;
      SetModified();
    }
  return true;
}

bool BaseSettings::SetIconSize(int iconSize)
{
  if (this->iconSize != iconSize)
    {
      this->iconSize = iconSize;
      SetModified();
    }
  return true;
}

bool BaseSettings::SetAppletMonitor(int monitor)
{
  if (this->appletMonitor != monitor)
    {
      this->appletMonitor = monitor;
      SetModified();
    }
  return true;
}

bool BaseSettings::SetIconSpacing(int iconSpacing)
{
  if (this->iconSpacing != iconSpacing)
    {
      this->iconSpacing = iconSpacing;
      SetModified();
    }
  return true;
}

bool BaseSettings::SetPosition()
{
  int tmpx, tmpy;
  RECT appletRect;

  if (!ELGetWindowRect(appletWnd, &appletRect))
    return false;

  if (_wcsicmp(verticalDirection, TEXT("up")) == 0)
    tmpy = appletRect.bottom;
  else if (_wcsicmp(verticalDirection, TEXT("center")) == 0)
    tmpy = appletRect.top + ((appletRect.bottom - appletRect.top) / 2);
  else
    tmpy = appletRect.top;

  if (_wcsicmp(horizontalDirection, TEXT("left")) == 0)
    tmpx = appletRect.right;
  else if (_wcsicmp(horizontalDirection, TEXT("center")) == 0)
    tmpx = appletRect.left + ((appletRect.right - appletRect.left) / 2);
  else
    tmpx = appletRect.left;

  if (dynamicPositioning)
    {
      RECT monitorRect = ELGetMonitorRect(ELGetAppletMonitor(appletWnd));
      tmpx -= monitorRect.left;
      tmpy -= monitorRect.top;
      int width = monitorRect.right - monitorRect.left;
      int height = monitorRect.bottom - monitorRect.top;

      if (wcsstr(anchorPoint, TEXT("Center")) != NULL)
        tmpy -= (height / 2);
      if (wcsstr(anchorPoint, TEXT("Bottom")) != NULL)
        {
          tmpy -= height;
          if (tmpy == 0)
            tmpy = -1;
        }

      if (wcsstr(anchorPoint, TEXT("Middle")) != NULL)
        tmpx -= (width / 2);
      if (wcsstr(anchorPoint, TEXT("Right")) != NULL)
        {
          tmpx -= width;
          if (tmpx == 0)
            tmpx = -1;
        }
    }

  if ((this->x != tmpx) || (this->y != tmpy))
    {
      this->x = tmpx;
      this->y = tmpy;
      SetModified();
    }

  return true;
}

int BaseSettings::GetX()
{
  if (dynamicPositioning)
    {
      RECT monitorRect = ELGetMonitorRect(appletMonitor);
      if (wcsstr(anchorPoint, TEXT("Left")) != NULL)
        return (monitorRect.left + x);
      if (wcsstr(anchorPoint, TEXT("Middle")) != NULL)
        return (((monitorRect.right - monitorRect.left) / 2) + x);
      if (wcsstr(anchorPoint, TEXT("Right")) != NULL)
        {
          if (x == -1)
            return monitorRect.right;
          else
            return (monitorRect.right + x);
        }
    }

  return x;
}

int BaseSettings::GetAppletMonitor()
{
  return appletMonitor;
}

int BaseSettings::GetIconSize()
{
  return iconSize;
}

int BaseSettings::GetIconSpacing()
{
  return iconSpacing;
}

int BaseSettings::GetY()
{
  if (dynamicPositioning)
    {
      RECT monitorRect = ELGetMonitorRect(appletMonitor);
      if (wcsstr(anchorPoint, TEXT("Top")) != NULL)
        return (monitorRect.top + y);
      if (wcsstr(anchorPoint, TEXT("Center")) != NULL)
        return (((monitorRect.bottom - monitorRect.top) / 2) + y);
      if (wcsstr(anchorPoint, TEXT("Bottom")) != NULL)
        {
          if (y == -1)
            return monitorRect.bottom;
          else
            return (monitorRect.bottom + y);
        }
    }

  return y;
}

int BaseSettings::GetWidth()
{
  return width;
}

int BaseSettings::GetHeight()
{
  return height;
}

WCHAR *BaseSettings::GetAnchorPoint()
{
  return anchorPoint;
}

WCHAR *BaseSettings::GetZPosition()
{
  return zPosition;
}

WCHAR *BaseSettings::GetHorizontalDirection()
{
  return horizontalDirection;
}

WCHAR *BaseSettings::GetVerticalDirection()
{
  return verticalDirection;
}

WCHAR *BaseSettings::GetDirectionOrientation()
{
  return directionOrientation;
}

int BaseSettings::GetClickThrough()
{
  return clickThrough;
}

bool BaseSettings::GetAutoSize()
{
  if (!allowAutoSize)
    return false;

  return autoSize;
}

bool BaseSettings::GetSnapMove()
{
  return snapMove;
}

bool BaseSettings::GetDynamicPositioning()
{
  return dynamicPositioning;
}

bool BaseSettings::GetSnapSize()
{
  return snapSize;
}

WCHAR *BaseSettings::GetStyleFile()
{
  return styleFile;
}

bool BaseSettings::SetZPosition(WCHAR *zPosition)
{
  if (_wcsicmp(this->zPosition, zPosition) != 0)
    {
      wcscpy(this->zPosition, zPosition);
      SetModified();
    }
  return true;
}

bool BaseSettings::SetHorizontalDirection(WCHAR *horizontalDirection)
{
  if (_wcsicmp(this->horizontalDirection, horizontalDirection) != 0)
    {
      wcscpy(this->horizontalDirection, horizontalDirection);
      SetModified();
    }
  return true;
}

bool BaseSettings::SetVerticalDirection(WCHAR *verticalDirection)
{
  if (_wcsicmp(this->verticalDirection, verticalDirection) != 0)
    {
      wcscpy(this->verticalDirection, verticalDirection);
      SetModified();
    }
  return true;
}

bool BaseSettings::SetDirectionOrientation(WCHAR *directionOrientation)
{
  if (_wcsicmp(this->directionOrientation, directionOrientation) != 0)
    {
      wcscpy(this->directionOrientation, directionOrientation);
      SetModified();
    }
  return true;
}

bool BaseSettings::SetAnchorPoint(WCHAR *anchorPoint)
{
  if (_wcsicmp(this->anchorPoint, anchorPoint) != 0)
    {
      wcscpy(this->anchorPoint, anchorPoint);
      SetPosition();
      SetModified();
    }
  return true;
}

bool BaseSettings::SetClickThrough(int clickThrough)
{
  if (this->clickThrough != clickThrough)
    {
      this->clickThrough = clickThrough;
      SetModified();
    }
  return true;
}

bool BaseSettings::SetAutoSize(bool autoSize)
{
  if (this->autoSize != autoSize)
    {
      this->autoSize = autoSize;
      SetModified();
    }
  return true;
}

bool BaseSettings::SetSnapMove(bool snapMove)
{
  if (this->snapMove != snapMove)
    {
      this->snapMove = snapMove;
      SetModified();
    }
  return true;
}

bool BaseSettings::SetDynamicPositioning(bool dynamicPositioning)
{
  if (this->dynamicPositioning != dynamicPositioning)
    {
      this->dynamicPositioning = dynamicPositioning;
      SetModified();
    }
  return true;
}

bool BaseSettings::SetSnapSize(bool snapSize)
{
  if (this->snapSize != snapSize)
    {
      this->snapSize = snapSize;
      SetModified();
    }
  return true;
}

bool BaseSettings::SetStyleFile(const WCHAR *styleFile)
{
  if (_wcsicmp(this->styleFile, styleFile) != 0)
    {
      wcscpy(this->styleFile, styleFile);
      SetModified();
    }
  return true;
}

bool BaseSettings::CopyStyle()
{
  std::wstring workingStyle = styleFile, destStyle;

  if (workingStyle.find(L"\\Styles\\") == std::wstring::npos)
    {
      destStyle = L"%ThemeDir%\\Styles";
      if (!PathIsDirectory(destStyle.c_str()))
        ELCreateDirectory(destStyle);

      if (ELFileOp(NULL, FO_COPY, workingStyle, destStyle) && !workingStyle.empty())
        {
          destStyle += workingStyle.substr(workingStyle.rfind(L"\\"));
          wcscpy(styleFile, destStyle.c_str());
        }
      else
        return false;
    }

  return true;
}

bool BaseSettings::GetSortInfo(WCHAR *editorName, PSORTINFO sortInfo)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section, *editor, *applet;
  bool readSettings = false;
  std::wstring xmlFile = TEXT("%EmergeDir%\\files\\SortOrder.xml");

  configXML = ELOpenXMLConfig(xmlFile, false);
  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Settings"), false);
      if (section)
        {
          applet = ELGetFirstXMLElementByName(section, appletName, false);
          if (applet)
            {
              editor = ELGetFirstXMLElementByName(applet, editorName, false);
              if (editor)
                {
                  readSettings = true;

                  ELReadXMLIntValue(editor, TEXT("SubItem"), &sortInfo->subItem, 0);
                  ELReadXMLBoolValue(editor, TEXT("Ascending"), &sortInfo->ascending, true);
                }
            }
        }
    }

  if (!readSettings)
    {
      sortInfo->subItem = 0;
      sortInfo->ascending = true;
    }

  return readSettings;
}

bool BaseSettings::SetSortInfo(WCHAR *editorName, PSORTINFO sortInfo)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section, *applet, *editor;
  std::wstring xmlFile = TEXT("%EmergeDir%\\files\\SortOrder.xml");

  configXML = ELOpenXMLConfig(xmlFile, true);
  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Settings"), true);
      if (section)
        {
          applet = ELGetFirstXMLElementByName(section, appletName, true);
          if (applet)
            {
              editor = ELGetFirstXMLElementByName(applet, editorName, true);
              if (editor)
                {
                  ELWriteXMLIntValue(editor, TEXT("SubItem"), sortInfo->subItem);
                  ELWriteXMLBoolValue(editor, TEXT("Ascending"), sortInfo->ascending);
                  return ELWriteXMLConfig(configXML.get());
                }
            }
        }
    }

  return false;
}

BaseSettings::IOHelper::IOHelper(TiXmlElement *sec)
{
  key = NULL;
  section = sec;
  item = NULL;
  itemIndex = 0;
  readIndex = 0;
  target = section; // Set the target element to the section initially
  targetKey = key; // Set the target key to the main key initially
}

BaseSettings::IOHelper::IOHelper(HKEY key)
{
  (*this).key = key;
  section = NULL;
  item = NULL;
  itemIndex = 0;
  readIndex = 0;
  target = section; // Set the target element to the section initially
  targetKey = key; // Set the target key to the main key initially
}

BaseSettings::IOHelper::~IOHelper()
{
}

void BaseSettings::IOHelper::Clear()
{
  if (!section)
    return;

  TiXmlElement *tmpItem;

  tmpItem = ELGetFirstXMLElement(section);
  while (tmpItem)
    {
      ELRemoveXMLElement(tmpItem);
      tmpItem = ELGetFirstXMLElement(section);
    }

  itemIndex = 0;
}

bool BaseSettings::IOHelper::GetElement()
{
  TiXmlElement *tmpItem = NULL;
  WCHAR tmpkeyName[MAX_LINE_LENGTH];
  DWORD size = 0;

  if (section)
    {
      if (item == NULL)
        item = ELGetFirstXMLElement(section);
      else
        {
          tmpItem = item;
          item = ELGetSiblingXMLElement(tmpItem);
        }

      // If item is found, set it as the target element
      if (item)
        target = item;

      return (item != NULL);
    }
  if (key)
    {
      size = sizeof(WCHAR) * MAX_LINE_LENGTH;
      if (RegEnumKeyEx(key, readIndex, tmpkeyName, &size, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
          if (RegOpenKeyEx(key, tmpkeyName, 0, KEY_READ, &targetKey) == ERROR_SUCCESS)
            {
              subkeyName = tmpkeyName;
              readIndex++;
              return true;
            }
        }
    }

  return false;
}

bool BaseSettings::IOHelper::SetElement(const WCHAR *name)
{
  bool ret = false;

  if (section)
    {
      item = ELSetFirstXMLElement(section, name);
      if (item)
        {
          ret = true;
          target = item;
        }
    }

  return ret;
}

void *BaseSettings::IOHelper::GetTarget()
{
  if (section)
    return reinterpret_cast<void*>(target);

  if (key)
    return reinterpret_cast<void*>(targetKey);

  return NULL;
}

void *BaseSettings::IOHelper::GetElement(WCHAR *name)
{
  if (section)
    {
      item = ELGetFirstXMLElementByName(section, name, false);

      // If item is found, set it as the target element
      if (item)
        target = item;

      return reinterpret_cast<void*>(item);
    }

  if (key)
    {
      if (RegOpenKeyEx(key, name, 0, KEY_READ, &targetKey) == ERROR_SUCCESS)
        return reinterpret_cast<void*>(targetKey);
    }

  return NULL;
}

bool BaseSettings::IOHelper::GetElementText(WCHAR *text)
{
  if (section)
    return ELGetXMLElementText(item, text);

  if (key)
    {
      if (!subkeyName.empty())
        {
          wcscpy(text, subkeyName.c_str());
          return true;
        }
    }

  return false;
}

bool BaseSettings::IOHelper::ReadBool(const WCHAR* name, bool& data, bool def)
{
  bool ret = false;
  DWORD tmpData;

  if (target)
    return ELReadXMLBoolValue(target, name, &data, def);
  else if (targetKey)
    {
      DWORD tmpDef = 0;
      data = false;
      if (def)
        tmpDef = 1;
      ret = ELReadRegDWord(targetKey, name, &tmpData, tmpDef);
      if (tmpData == 1)
        data = true;
    }

  return ret;
}

bool BaseSettings::IOHelper::ReadInt(const WCHAR* name, int& data, int def)
{
  if (target)
    return ELReadXMLIntValue(target, name, &data, def);
  else if (targetKey)
    return ELReadRegDWord(targetKey, name, (DWORD*)&data, def);

  return false;
}

bool BaseSettings::IOHelper::ReadFloat(const WCHAR* name, float& data, float def)
{
  if (target)
    return ELReadXMLFloatValue(target, name, &data, def);

  return false;
}

bool BaseSettings::IOHelper::ReadString(const WCHAR* name, WCHAR* data, const WCHAR* def)
{
  if (target)
    return ELReadXMLStringValue(target, name, data, def);
  else if (targetKey)
    return ELReadRegString(targetKey, name, data, def);

  return false;
}

bool BaseSettings::IOHelper::ReadRect(const WCHAR* name, RECT& data, RECT& def)
{
  if (target)
    return ELReadXMLRectValue(target, name, &data, def);
  else if (targetKey)
    return ELReadRegRect(targetKey, name, &data, &def);

  return false;
}

bool BaseSettings::IOHelper::ReadColor(const WCHAR* name, COLORREF& data, COLORREF def)
{
  bool ret = false;
  int red = 0, green = 0, blue = 0;

  if (target)
    return ELReadXMLColorValue(target, name, &data, def);
  else if (targetKey)
    {
      WCHAR tmpDef[MAX_LINE_LENGTH], tmpData[MAX_LINE_LENGTH];
      swprintf(tmpDef, TEXT("%d,%d,%d"), red, green, blue);
      ret = ELReadRegString(targetKey, name, tmpData, tmpDef);
      if (swscanf(tmpData, TEXT("%d,%d,%d"), &red, &green, &blue) == 3)
        {
          if ((red >= 0 && red <= 255) &&
              (green >= 0 && green <= 255) &&
              (blue >= 0 && blue <= 255))
            data = RGB(red, green, blue);
        }
    }

  return ret;
}

bool BaseSettings::IOHelper::WriteBool(const WCHAR* name, bool data)
{
  return ELWriteXMLBoolValue(target, name, data);
}

bool BaseSettings::IOHelper::WriteInt(const WCHAR* name, int data)
{
  return ELWriteXMLIntValue(target, name, data);
}

bool BaseSettings::IOHelper::WriteFloat(const WCHAR* name, float data)
{
  return ELWriteXMLFloatValue(target, name, data);
}

bool BaseSettings::IOHelper::WriteString(const WCHAR* name, WCHAR* data)
{
  return ELWriteXMLStringValue(target, name, data);
}

bool BaseSettings::IOHelper::WriteRect(const WCHAR* name, RECT& data)
{
  return ELWriteXMLRectValue(target, name, data);
}

bool BaseSettings::IOHelper::WriteColor(const WCHAR* name, COLORREF data)
{
  return ELWriteXMLColorValue(target, name, data);
}
