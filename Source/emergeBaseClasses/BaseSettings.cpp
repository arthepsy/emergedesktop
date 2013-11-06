// vim:tags+=../emergeLib/tags
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
  x = 0;
  y = 0;
  width = 0;
  height = 0;
  iconSize = 16;
  iconSpacing = 0;
  clickThrough = false;
  appletMonitor = 0;
  horizontalDirection = TEXT("");
  verticalDirection = TEXT("");
  directionOrientation = TEXT("");
  autoSize = false;
  snapMove = false;
  snapSize = false;
  dynamicPositioning = false;
  anchorPoint = TEXT("");
  zPosition = TEXT("");
  appletName = TEXT("");
  appletCount = 0;
  startHidden = false;
  titleBarText = TEXT("");
  appletWnd = NULL;
  keyString = TEXT("");
  styleFile = TEXT("");
  modifiedFlag = false;
  titleBarFontString = TEXT("");
  autoSizeLimit = 0;
}

BaseSettings::~BaseSettings()
{}

void BaseSettings::Init(HWND appletWnd, std::wstring appletName, int appletCount)
{
  this->appletName = appletName;
  keyString = appletName;
  keyString = keyString + TEXT("\\Settings");
  this->appletWnd = appletWnd;
  this->appletCount = appletCount;
}

void BaseSettings::ReadSettings()
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* section;
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
        {
          styleFile = ELGetAbsolutePath(styleFile, TEXT("%ThemeDir%\\"));
        }
      }
    }
  }

  // In the case where there is an issue accessing the XML file, use default values;
  if (!readSettings)
  {
    ResetDefaults();
  }

  DoInitialize();
}

bool BaseSettings::ModifiedCheck()
{
  std::wstring theme = ELGetThemeName();

  if (!ELIsModifiedTheme(theme))
    // Perserve the old theme name
  {
    oldTheme = theme;
  }

  return ELSetModifiedTheme(theme);
}

bool BaseSettings::CopyTheme()
{
  std::wstring oldThemePath, newThemePath;

  oldThemePath = TEXT("%EmergeDir%\\themes\\") + oldTheme;
  oldThemePath += TEXT("\\*");
  newThemePath = TEXT("%ThemeDir%");

  if ((ELGetFileSpecialFlags(newThemePath) & SF_DIRECTORY) != SF_DIRECTORY)
  {
    if (ELCreateDirectory(newThemePath))
    {
      if (ELFileOp(appletWnd, false, FO_COPY, oldThemePath, newThemePath))
      {
        return CopyStyle();
      }
    }
  }

  return false;
}

bool BaseSettings::WriteSettings()
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* section;
  std::wstring xmlFile = TEXT("%ThemeDir%\\");
  xmlFile += appletName;
  xmlFile += TEXT(".xml");
  bool ret = false;

  if (GetModified())
  {
    if (ModifiedCheck())
    {
      CopyTheme();
    }

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
  ZeroMemory(&titleBarFont, sizeof(LOGFONT));

  appletMonitor = helper.ReadInt(TEXT("Monitor"), 0);
  x = helper.ReadInt(TEXT("X"), 0);
  y = helper.ReadInt(TEXT("Y"), 0);
  width = helper.ReadInt(TEXT("Width"), 100);
  height = helper.ReadInt(TEXT("Height"), 100);
  zPosition = helper.ReadString(TEXT("ZPosition"), TEXT("normal"));
  horizontalDirection = helper.ReadString(TEXT("HorizontalDirection"), TEXT("right"));
  verticalDirection = helper.ReadString(TEXT("VerticalDirection"), TEXT("down"));
  directionOrientation = helper.ReadString(TEXT("DirectionOrientation"), (WCHAR*)TEXT("down"));
  autoSize = helper.ReadBool(TEXT("AutoSize"), false);
  autoSizeLimit = helper.ReadInt(TEXT("AutoSizeLimit"), 0);
  iconSize = helper.ReadInt(TEXT("IconSize"), 16);
  iconSpacing = helper.ReadInt(TEXT("IconSpacing"), 1);
  snapMove = helper.ReadBool(TEXT("SnapMove"), true);
  snapSize = helper.ReadBool(TEXT("SnapSize"), true);
  dynamicPositioning = helper.ReadBool(TEXT("DynamicPositioning"), true);
  clickThrough = helper.ReadInt(TEXT("ClickThrough"), 0);
  anchorPoint = helper.ReadString(TEXT("AnchorPoint"), (WCHAR*)TEXT("TopLeft"));
  startHidden = helper.ReadBool(TEXT("StartHidden"), false);
  titleBarFontString = helper.ReadString(TEXT("TitleBarFont"), TEXT("Arial-16"));
  titleBarText = helper.ReadString(TEXT("TitleBarText"), TEXT(""));
  // Check for a Style entry...
  styleFile = helper.ReadString(TEXT("Style"), TEXT(""));
  if (styleFile.empty())
  {
    // ... if not found, look for a Scheme entry.  If found...
    styleFile = helper.ReadString(TEXT("Scheme"), TEXT(""));
    if (!styleFile.empty())
    {
      // ... remove it ...
      if (helper.RemoveElement(TEXT("Scheme")))
      {
        // .. if removed, add a Style entry.
        if (helper.WriteString(TEXT("Style"), styleFile))
        {
          ELWriteXMLConfig(ELGetXMLConfig(helper.GetSection()));
        }
      }
    }
  }
  else
  {
    // ... if found, check for a 'Scheme' entry...
    if (!helper.ReadString(TEXT("Scheme"), TEXT("")).empty())
    {
      // ... and remove it.
      if (helper.RemoveElement(TEXT("Scheme")))
      {
        ELWriteXMLConfig(ELGetXMLConfig(helper.GetSection()));
      }
    }

  }
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
  helper.WriteInt(TEXT("AutoSizeLimit"), autoSizeLimit);
  helper.WriteInt(TEXT("IconSize"), iconSize);
  helper.WriteInt(TEXT("IconSpacing"), iconSpacing);
  helper.WriteString(TEXT("Style"), styleFile);
  helper.WriteBool(TEXT("SnapMove"), snapMove);
  helper.WriteBool(TEXT("SnapSize"), snapSize);
  helper.WriteBool(TEXT("DynamicPositioning"), dynamicPositioning);
  helper.WriteInt(TEXT("ClickThrough"), clickThrough);
  helper.WriteInt(TEXT("Monitor"), appletMonitor);
  helper.WriteString(TEXT("AnchorPoint"), anchorPoint);
  helper.WriteBool(TEXT("StartHidden"), startHidden);
  titleBarFontString = EGFontToString(titleBarFont);
  helper.WriteString(TEXT("TitleBarFont"), titleBarFontString);
  helper.WriteString(TEXT("TitleBarText"), titleBarText);
}

void BaseSettings::DoInitialize()
{
  if (!titleBarFontString.empty())
  {
    titleBarFont = EGStringToFont(titleBarFontString);
  }
}

POINT BaseSettings::InstancePosition(SIZE appletSize)
{
  HMONITOR cursorMonitor;
  MONITORINFO cursorMonitorInfo;
  POINT cursorPt;

  // Get the current position of the cursor
  GetCursorPos(&cursorPt);

  // Determine which monitor the cursor is on
  cursorMonitor = MonitorFromPoint(cursorPt, MONITOR_DEFAULTTONEAREST);

  // Get the dimentions of the monitor
  cursorMonitorInfo.cbSize = sizeof(MONITORINFO);
  GetMonitorInfo(cursorMonitor, &cursorMonitorInfo);

  // Initially set the x and y values to be the cursor position,
  // incorporating the dimensions of the applet
  cursorPt.x -= (appletSize.cx / 2);
  cursorPt.y -= (appletSize.cy / 2);

  // Make sure the applet is on the same monitor as the cursor
  if ((cursorPt.x + appletSize.cx) > cursorMonitorInfo.rcMonitor.right)
  {
    cursorPt.x = cursorMonitorInfo.rcMonitor.right - appletSize.cx;
  }
  if (cursorPt.x < cursorMonitorInfo.rcMonitor.left)
  {
    cursorPt.x = cursorMonitorInfo.rcMonitor.left;
  }
  if ((cursorPt.y + appletSize.cy) > cursorMonitorInfo.rcMonitor.bottom)
  {
    x = cursorMonitorInfo.rcMonitor.bottom - appletSize.cy;
  }
  if (cursorPt.y < cursorMonitorInfo.rcMonitor.top)
  {
    cursorPt.y = cursorMonitorInfo.rcMonitor.top;
  }

  return cursorPt;
}

void BaseSettings::ResetDefaults()
{
  x = 0;
  y = 0;
  width = 100;
  height = 100;
  zPosition = TEXT("normal");
  horizontalDirection = TEXT("right");
  verticalDirection = TEXT("down");
  directionOrientation = TEXT("down");
  autoSize = false;
  autoSizeLimit = 0;
  iconSize = 16;
  iconSpacing = 1;
  styleFile = TEXT("");
  snapMove = true;
  snapSize = true;
  dynamicPositioning = true;
  clickThrough = 0;
  appletMonitor = 0;
  anchorPoint = TEXT("TopLeft");
  startHidden = false;
  titleBarFontString = TEXT("Arial-16");
  titleBarText = TEXT("");
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

  appletRect = ELGetWindowRect(appletWnd);
  if (IsRectEmpty(&appletRect))
  {
    return false;
  }

  if (ELToLower(verticalDirection) == TEXT("up"))
  {
    tmpy = appletRect.bottom;
  }
  else if (ELToLower(verticalDirection) == TEXT("center"))
  {
    tmpy = appletRect.top + ((appletRect.bottom - appletRect.top) / 2);
  }
  else
  {
    tmpy = appletRect.top;
  }

  if (ELToLower(horizontalDirection) == TEXT("left"))
  {
    tmpx = appletRect.right;
  }
  else if (ELToLower(horizontalDirection) == TEXT("center"))
  {
    tmpx = appletRect.left + ((appletRect.right - appletRect.left) / 2);
  }
  else
  {
    tmpx = appletRect.left;
  }

  if (dynamicPositioning)
  {
    RECT monitorRect = ELGetMonitorRect(ELGetAppletMonitor(appletWnd));
    tmpx -= monitorRect.left;
    tmpy -= monitorRect.top;
    int width = monitorRect.right - monitorRect.left;
    int height = monitorRect.bottom - monitorRect.top;

    if (anchorPoint.find(TEXT("Center")) != std::wstring::npos)
    {
      tmpy -= (height / 2);
    }
    if (anchorPoint.find(TEXT("Bottom")) != std::wstring::npos)
    {
      tmpy -= height;
      if (tmpy == 0)
      {
        tmpy = -1;
      }
    }

    if (anchorPoint.find(TEXT("Middle")) != std::wstring::npos)
    {
      tmpx -= (width / 2);
    }
    if (anchorPoint.find(TEXT("Right")) != std::wstring::npos)
    {
      tmpx -= width;
      if (tmpx == 0)
      {
        tmpx = -1;
      }
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
  int xpos = x;
  RECT appletRect;
  GetClientRect(appletWnd, &appletRect);

  if (dynamicPositioning)
  {
    RECT monitorRect = ELGetMonitorRect(appletMonitor);
    if (anchorPoint.find(TEXT("Left")) != std::wstring::npos)
    {
      xpos = monitorRect.left + x;
    }
    if (anchorPoint.find(TEXT("Middle")) != std::wstring::npos)
    {
      xpos = ((monitorRect.right - monitorRect.left) / 2) + x;
    }
    if (anchorPoint.find(TEXT("Right")) != std::wstring::npos)
    {
      if (x == -1)
      {
        xpos =  monitorRect.right;
      }
      else
      {
        xpos = monitorRect.right + x;
      }
    }
  }

  // Make sure that the applet is within the virtual screen (i.e. does not
  // display off screen)
  if (xpos < GetSystemMetrics(SM_XVIRTUALSCREEN))
  {
    xpos = GetSystemMetrics(SM_XVIRTUALSCREEN);
  }
  if (xpos > GetSystemMetrics(SM_CXVIRTUALSCREEN))
  {
    xpos = GetSystemMetrics(SM_CXVIRTUALSCREEN) - MAX(appletRect.right, iconSize);
  }

  return xpos;
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
  int ypos = y;

  if (dynamicPositioning)
  {
    RECT monitorRect = ELGetMonitorRect(appletMonitor);
    if (anchorPoint.find(TEXT("Top")) != std::wstring::npos)
    {
      ypos = monitorRect.top + y;
    }
    if (anchorPoint.find(TEXT("Center")) != std::wstring::npos)
    {
      ypos = ((monitorRect.bottom - monitorRect.top) / 2) + y;
    }
    if (anchorPoint.find(TEXT("Bottom")) != std::wstring::npos)
    {
      if (y == -1)
      {
        ypos = monitorRect.bottom;
      }
      else
      {
        ypos = monitorRect.bottom + y;
      }
    }
  }

  // Make sure that the applet is within the virtual screen (i.e. does not
  // display off screen)
  if (ypos < GetSystemMetrics(SM_YVIRTUALSCREEN))
  {
    ypos = GetSystemMetrics(SM_YVIRTUALSCREEN);
  }
  if (ypos > GetSystemMetrics(SM_CYVIRTUALSCREEN))
  {
    ypos = GetSystemMetrics(SM_CYVIRTUALSCREEN) - iconSize;
  }

  return ypos;
}

int BaseSettings::GetWidth()
{
  return width;
}

int BaseSettings::GetHeight()
{
  return height;
}

std::wstring BaseSettings::GetAnchorPoint()
{
  return anchorPoint;
}

bool BaseSettings::GetStartHidden()
{
  return startHidden;
}

std::wstring BaseSettings::GetZPosition()
{
  return zPosition;
}

std::wstring BaseSettings::GetHorizontalDirection()
{
  return horizontalDirection;
}

std::wstring BaseSettings::GetVerticalDirection()
{
  return verticalDirection;
}

std::wstring BaseSettings::GetDirectionOrientation()
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
  {
    return false;
  }

  return autoSize;
}

int BaseSettings::GetAutoSizeLimit()
{
  return autoSizeLimit;
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

std::wstring BaseSettings::GetStyleFile()
{
  return styleFile;
}

LOGFONT* BaseSettings::GetTitleBarFont()
{
  return &titleBarFont;
}

std::wstring BaseSettings::GetTitleBarText()
{
  return titleBarText;
}

bool BaseSettings::SetZPosition(std::wstring zPosition)
{
  if (ELToLower(this->zPosition) != ELToLower(zPosition))
  {
    this->zPosition = zPosition;
    SetModified();
  }
  return true;
}

bool BaseSettings::SetHorizontalDirection(std::wstring horizontalDirection)
{
  if (ELToLower(this->horizontalDirection) != ELToLower(horizontalDirection))
  {
    this->horizontalDirection = horizontalDirection;
    SetModified();
  }
  return true;
}

bool BaseSettings::SetVerticalDirection(std::wstring verticalDirection)
{
  if (ELToLower(this->verticalDirection) != verticalDirection)
  {
    this->verticalDirection = verticalDirection;
    SetModified();
  }
  return true;
}

bool BaseSettings::SetDirectionOrientation(std::wstring directionOrientation)
{
  if (ELToLower(this->directionOrientation) != directionOrientation)
  {
    this->directionOrientation = directionOrientation;
    SetModified();
  }
  return true;
}

bool BaseSettings::SetAnchorPoint(std::wstring anchorPoint)
{
  if (ELToLower(this->anchorPoint) != anchorPoint)
  {
    this->anchorPoint = anchorPoint;
    SetPosition();
    SetModified();
  }
  return true;
}

bool BaseSettings::SetStartHidden(bool startHidden)
{
  if (this->startHidden != startHidden)
  {
    this->startHidden = startHidden;
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

bool BaseSettings::SetAutoSizeLimit(int autoSizeLimit)
{
  if (this->autoSizeLimit != autoSizeLimit)
  {
    this->autoSizeLimit = autoSizeLimit;
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

bool BaseSettings::SetStyleFile(std::wstring styleFile)
{
  if (ELToLower(this->styleFile) != styleFile)
  {
    this->styleFile = styleFile;
    SetModified();
  }
  return true;
}

bool BaseSettings::SetTitleBarFont(LOGFONT* titleBarFont)
{
  if (!EGEqualLogFont(this->titleBarFont, *titleBarFont))
  {
    titleBarFontString = EGFontToString(*titleBarFont);
    CopyMemory(&this->titleBarFont, titleBarFont, sizeof(LOGFONT));
    SetModified();
  }
  return true;
}

void BaseSettings::SetTitleBarText(std::wstring titleBarText)
{
  if (ELToLower(this->titleBarText) != titleBarText)
  {
    this->titleBarText = titleBarText;
    SetModified();
  }
}

bool BaseSettings::CopyStyle()
{
  std::wstring workingStyle = styleFile, destStyle;

  if (workingStyle.find(TEXT("\\Styles\\")) == std::wstring::npos)
  {
    destStyle = TEXT("%ThemeDir%\\Styles");
    if ((ELGetFileSpecialFlags(destStyle) & SF_DIRECTORY) != SF_DIRECTORY)
      if (!ELCreateDirectory(destStyle))
      {
        return false;
      }

    if (ELFileOp(NULL, false, FO_COPY, workingStyle, destStyle) && !workingStyle.empty())
    {
      destStyle += workingStyle.substr(workingStyle.rfind(TEXT("\\")));
      styleFile = destStyle;
    }
    else
    {
      return false;
    }
  }

  return true;
}

bool BaseSettings::GetSortInfo(std::wstring editorName, PSORTINFO sortInfo)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* section, *editor, *applet;
  bool readSettings = false;
  std::wstring xmlFile = TEXT("%EmergeDir%\\files\\SortOrder.xml");

  configXML = ELOpenXMLConfig(xmlFile, false);
  if (configXML)
  {
    section = ELGetXMLSection(configXML.get(), TEXT("Settings"), false);
    if (section)
    {
      applet = ELGetFirstXMLElementByName(section, appletName, false);
      if (applet)
      {
        editor = ELGetFirstXMLElementByName(applet, editorName, false);
        if (editor)
        {
          readSettings = true;

          sortInfo->subItem = ELReadXMLIntValue(editor, TEXT("SubItem"), 0);
          sortInfo->ascending = ELReadXMLBoolValue(editor, TEXT("Ascending"), true);
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

bool BaseSettings::SetSortInfo(std::wstring editorName, PSORTINFO sortInfo)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* section, *applet, *editor;
  std::wstring xmlFile = TEXT("%EmergeDir%\\files\\SortOrder.xml");

  configXML = ELOpenXMLConfig(xmlFile, true);
  if (configXML)
  {
    section = ELGetXMLSection(configXML.get(), TEXT("Settings"), true);
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

BaseSettings::IOHelper::IOHelper(TiXmlElement* sec)
{
  section = sec;
  item = NULL;
  itemIndex = 0;
  readIndex = 0;
  target = section; // Set the target element to the section initially
}

BaseSettings::IOHelper::~IOHelper()
{
}

void BaseSettings::IOHelper::Clear()
{
  if (!section)
  {
    return;
  }

  TiXmlElement* tmpItem;

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
  TiXmlElement* tmpItem = NULL;

  if (section)
  {
    if (item == NULL)
    {
      item = ELGetFirstXMLElement(section);
    }
    else
    {
      tmpItem = item;
      item = ELGetSiblingXMLElement(tmpItem);
    }

    // If item is found, set it as the target element
    if (item)
    {
      target = item;
    }

    return (item != NULL);
  }

  return false;
}

bool BaseSettings::IOHelper::SetElement(std::wstring name)
{
  bool ret = false;

  if (section)
  {
    item = ELSetFirstXMLElementByName(section, name);
    if (item)
    {
      ret = true;
      target = item;
    }
  }

  return ret;
}

void* BaseSettings::IOHelper::GetTarget()
{
  if (section)
  {
    return reinterpret_cast<void*>(target);
  }

  return NULL;
}

void* BaseSettings::IOHelper::GetElement(std::wstring name)
{
  if (section)
  {
    item = ELGetFirstXMLElementByName(section, name, false);

    // If item is found, set it as the target element
    if (item)
    {
      target = item;
    }

    return reinterpret_cast<void*>(item);
  }

  return NULL;
}

bool BaseSettings::IOHelper::GetElementText(std::wstring text)
{
  if (section)
  {
    return ELGetXMLElementText(item, text);
  }

  return false;
}

bool BaseSettings::IOHelper::RemoveElement(std::wstring name)
{
  if (section)
  {
    TiXmlElement* itemToRemove = ELGetFirstXMLElementByName(section, name, false);
    if (itemToRemove)
    {
      return ELRemoveXMLElement(itemToRemove);
    }
  }

  return false;
}

TiXmlElement* BaseSettings::IOHelper::GetSection()
{
  return section;
}

bool BaseSettings::IOHelper::ReadBool(std::wstring name, bool defaultValue)
{
  if (target)
  {
    return ELReadXMLBoolValue(target, name, defaultValue);
  }

  return defaultValue;
}

int BaseSettings::IOHelper::ReadInt(std::wstring name, int defaultValue)
{
  if (target)
  {
    return ELReadXMLIntValue(target, name, defaultValue);
  }

  return defaultValue;
}

float BaseSettings::IOHelper::ReadFloat(std::wstring name, float defaultValue)
{
  if (target)
  {
    return ELReadXMLFloatValue(target, name, defaultValue);
  }

  return defaultValue;
}

std::wstring BaseSettings::IOHelper::ReadString(std::wstring name, std::wstring defaultValue)
{
  if (target)
  {
    return ELReadXMLStringValue(target, name, defaultValue);
  }

  return defaultValue;
}

RECT BaseSettings::IOHelper::ReadRect(std::wstring name, RECT defaultValue)
{
  if (target)
  {
    return ELReadXMLRectValue(target, name, defaultValue);
  }

  return defaultValue;
}

COLORREF BaseSettings::IOHelper::ReadColor(std::wstring name, COLORREF defaultValue)
{
  if (target)
  {
    return ELReadXMLColorValue(target, name, defaultValue);
  }

  return defaultValue;
}

bool BaseSettings::IOHelper::WriteBool(std::wstring name, bool data)
{
  return ELWriteXMLBoolValue(target, name, data);
}

bool BaseSettings::IOHelper::WriteInt(std::wstring name, int data)
{
  return ELWriteXMLIntValue(target, name, data);
}

bool BaseSettings::IOHelper::WriteFloat(std::wstring name, float data)
{
  return ELWriteXMLFloatValue(target, name, data);
}

bool BaseSettings::IOHelper::WriteString(std::wstring name, std::wstring data)
{
  return ELWriteXMLStringValue(target, name, data);
}

bool BaseSettings::IOHelper::WriteRect(std::wstring name, RECT data)
{
  return ELWriteXMLRectValue(target, name, data);
}

bool BaseSettings::IOHelper::WriteColor(std::wstring name, COLORREF data)
{
  return ELWriteXMLColorValue(target, name, data);
}
