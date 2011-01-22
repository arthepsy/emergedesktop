/*!
  @file main.cpp
  @brief Utility functions for Emerge Desktop
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2011  The Emerge Desktop Development Team

  @attention Emerge Desktop is free software; you can redistribute it and/or
  modify  it under the terms of the GNU General Public License as published
  by  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  @attention Emerge Desktop is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  @attention You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  @note 1)	For MSVC users, put the AggressiveOptimize.h header file (available from
http://www.nopcode.com) in your 'include' directory.  It cuts down on executable
filesize.
@note 2)	The ELExit code is based on a function from the LiteShell code base.

@note 3)  Using vectors for strings (taken from
http://www.codeguru.com/forum/archive/index.php/t-193852.html):
Whenever a function wants a constant pointer to a string (LPCSTR, LPWCSTR), you can
supply the string using the c_str() function. No problem since the buffer is not
modified by the function.

However, if the function wants to modify the string contents (an LPSTR or LPWSTR),
you can't use c_str() since c_str() returns a pointer to an unmodifiable buffer.
Even so, a std::(w)string may not store its contents in a contiguous buffer, which
is what the function requires. Instead you use a vector<WCHAR> or a vector<char> to
store your characters. You must make sure that the vector is sized (via reserve or
resize)appropriately when it is constructed.

A vector is guaranteed to have its contents stored in contiguous memory, and its
contents are modifiable, providing you have a sufficient number of elements in the
vector. You then pass the address of the first element of the vector -- this is no
different than passing the address of the first character. The advantage of this is
that you don't have to hard code your array sizes, since the constructor for a vector
takes any integer argument for the size.

This is a trick that you won't learn from Microsoft or MSDN -- how to use std::string,
vector<> and std::wstring with their API (or any function that requires pointers to
buffers). In the Scott Meyers book "Effective STL", he has a whole chapter on just this
topic alone.
*/

#undef WINVER
#define WINVER 0x0600

#include "Shutdown.h"
#include "MsgBox.h"
#include "zip.h"
#ifdef _W64
#include <mapinls.h>
#endif
#include <shellapi.h>
#include <shlwapi.h>
#include <stdio.h>
#include <string>
#include <shlobj.h>
#include <psapi.h>
#ifndef _W64
#include <WtsApi32.h>
#endif
#include <mmsystem.h>
#include <time.h>
#include <fstream>
#include <algorithm>
#include <powrprof.h>
#include <cctype>
#include <vector>
#include <deque>

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#ifdef __MINGW32__
#include "../MinGWInterfaces.h"
#else
#include <Mmdeviceapi.h>
#include <Endpointvolume.h>
#define _MMDeviceEnumerator MMDeviceEnumerator
#define _IMMDeviceEnumerator IMMDeviceEnumerator
#define _IAudioEndpointVolume IAudioEndpointVolume
#endif

#ifndef APPCOMMAND_BROWSER_BACKWARD
#define APPCOMMAND_BROWSER_BACKWARD 1
#endif
#ifndef FAPPCOMMAND_KEY
#define FAPPCOMMAND_KEY 0
#endif

#define CP_THEME  1
#define CP_USER   2
#define CP_APPLET 3

// Helper functions
bool ELCheckPathWithExtension(LPTSTR path);
bool IsClose(int side, int edge);
WCHAR *ReadValue(const WCHAR *fileName, WCHAR *keyword);
void stripQuotes(LPTSTR source);
bool SnapMoveToEdge(LPSNAPMOVEINFO snapMove, RECT rt);
bool SnapSizeToEdge(LPSNAPSIZEINFO snapSize, RECT rt);
bool WriteValue(const WCHAR *fileName, WCHAR *keyword, WCHAR *value);
BOOL CALLBACK FullscreenEnum(HWND hwnd, LPARAM lParam);
BOOL CALLBACK WindowIconEnum(HWND hwnd, LPARAM lParam);
bool GetSpecialFolderGUID(int folder, WCHAR *classID);
bool GetPIDLGUID(LPITEMIDLIST pidl, WCHAR *classID);
bool VistaVolumeControl(UINT command);
bool VolumeControl(UINT command);
bool PathTokenCheck(WCHAR *path);
std::tr1::shared_ptr<TiXmlDocument> OpenXMLConfig(std::string filename, bool create);
BOOL CALLBACK AppletMonitorEnum(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
BOOL CALLBACK MonitorRectEnum(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
bool ConvertPath(WCHAR *styleFile, DWORD flags, DWORD path);
BOOL CALLBACK ThemeEnum(HWND hwnd, LPARAM lParam);

typedef struct _APPLETMONITORINFO
{
  HWND appletWnd;
  HMONITOR appletMonitor;
  int appletMonitorNum;
  RECT appletMonitorRect;
} APPLETMONITORINFO, *LPAPPLETMONITORINFO;

// Globals
int enumCount = 0;
static HMODULE shell32DLL = NULL;
static HMODULE user32DLL = NULL;
static HMODULE kernel32DLL = NULL;
static HINSTANCE emergeLibInstance = NULL;

// Globally shared data
/*#ifdef __GNUC__
//bool showDesktop __attribute__((section ("shared"), shared)) = false;
std::deque <HWND> hwndDeque;
//std::deque <HWND> hwndDeque __attribute__((section ("shared"), shared));
#else
#pragma data_seg( ".shared" )
//bool showDesktop = false;
std::deque <HWND> hwndDeque;
#pragma data_seg()
#pragma comment(linker, "/SECTION:.shared,RWS")
#endif*/

// MS Run dialog
typedef void (__stdcall *lpfnMSRun)(HWND, HICON, LPCSTR, LPCSTR, LPCSTR, int);
static lpfnMSRun MSRun = NULL;

// MS SwitchToThisWindow
typedef void (__stdcall *lpfnMSSwitchToThisWindow)(HWND, BOOL);
static lpfnMSSwitchToThisWindow MSSwitchToThisWindow = NULL;

// MS RegisterShellHookWindow
typedef BOOL (WINAPI *lpfnMSRegisterShellHookWindow)(HWND hWnd, DWORD method);
static lpfnMSRegisterShellHookWindow MSRegisterShellHookWindow = NULL;

// MS IL Functions
typedef LPITEMIDLIST (WINAPI *fnILClone)(LPCITEMIDLIST);
static fnILClone MSILClone = NULL;

typedef LPITEMIDLIST (WINAPI *fnILFindLastID)(LPCITEMIDLIST);
static fnILFindLastID MSILFindLastID = NULL;

typedef BOOL (WINAPI *fnILRemoveLastID)(LPCITEMIDLIST);
static fnILRemoveLastID MSILRemoveLastID = NULL;

typedef void (WINAPI *lpfnILFree)(LPCITEMIDLIST);
static lpfnILFree MSILFree = NULL;

typedef BOOL (WINAPI *lpfnIsWow64Process)(HANDLE, PBOOL);
static lpfnIsWow64Process MSIsWow64Process = NULL;

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL UNUSED, DWORD fdwReason, LPVOID lpvReserved UNUSED)
{
  switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
      emergeLibInstance = hinstDLL;
      if (shell32DLL == NULL)
        shell32DLL = ELLoadSystemLibrary(TEXT("shell32.dll"));
      if (user32DLL == NULL)
        user32DLL = ELLoadSystemLibrary(TEXT("user32.dll"));
      if (kernel32DLL == NULL)
        kernel32DLL = ELLoadSystemLibrary(TEXT("kernel32.dll"));
      break;
    case DLL_PROCESS_DETACH:
      if (shell32DLL != NULL)
        {
          FreeLibrary(shell32DLL);
          shell32DLL = NULL;
        }
      if (user32DLL != NULL)
        {
          FreeLibrary(user32DLL);
          user32DLL = NULL;
        }
      if (kernel32DLL != NULL)
        {
          FreeLibrary(kernel32DLL);
          kernel32DLL = NULL;
        }
      break;
    }

  return TRUE;
}

std::tr1::shared_ptr<TiXmlDocument> OpenXMLConfig(std::string filename, bool create)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML(new TiXmlDocument(filename.c_str()));

  if (!configXML->LoadFile())
    {
      if (create)
        {
          TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "UTF-8", "yes");
          configXML->LinkEndChild(decl);
          if (!configXML->SaveFile())
            configXML.reset();
        }
      else
        configXML.reset();
    }

  return configXML;
}

bool ELPathFileExists(const WCHAR *file)
{
  std::wstring workingFile = file;
  workingFile = ELExpandVars(workingFile);

  return (PathFileExists(workingFile.c_str()) && !ELPathIsRelative(workingFile.c_str()));
}

// Don't use any EmergeVars in ELGetThemeInfo because it is used to derive the EmergeVars
void ELGetThemeInfo(LPTHEMEINFO themeInfo)
{
  WCHAR xmlPath[MAX_PATH];
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section;
  std::wstring userPath, workingPath;

  ELGetCurrentPath(xmlPath);
  workingPath = xmlPath;
  userPath = workingPath + TEXT("\\theme.xml");
  if (!ELPathFileExists(userPath.c_str()))
    {
      workingPath = TEXT("%AppData%\\Emerge Desktop");
      workingPath = ELExpandVars(workingPath);
      userPath = workingPath + TEXT("\\theme.xml");
    }
  wcscpy(themeInfo->path, workingPath.c_str());

  configXML = ELOpenXMLConfig(userPath, true);
  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Theme"), true);
      if (section)
        {
          if (!ELReadXMLStringValue(section, TEXT("Current"), themeInfo->theme, TEXT("Default")))
            {
              ELWriteXMLStringValue(section, (WCHAR*)TEXT("Current"), themeInfo->theme);
              ELWriteXMLConfig(configXML.get());
            }
          else
            {
              swprintf(themeInfo->themePath, TEXT("%s\\themes\\%s"),
                       themeInfo->path, themeInfo->theme);
              if (!PathIsDirectory(themeInfo->themePath))
                {
                  wcscpy(themeInfo->theme, TEXT("Default"));
                  ELWriteXMLStringValue(section, (WCHAR*)TEXT("Current"), themeInfo->theme);
                  ELWriteXMLConfig(configXML.get());
                }
            }
        }
    }

  if (_wcsicmp(themeInfo->theme, TEXT("Default")) != 0)
    {
      swprintf(themeInfo->themePath, TEXT("%s\\themes\\%s"), themeInfo->path, themeInfo->theme);
      workingPath = themeInfo->themePath;
      ELCreateDirectory(workingPath);
    }
  swprintf(themeInfo->userPath, TEXT("%s\\files"), themeInfo->path);
  workingPath = themeInfo->userPath;
  ELCreateDirectory(workingPath);
}

std::wstring ELGetThemeName()
{
  size_t i;
  std::wstring working = TEXT("%ThemeDir%");
  working = ELExpandVars(working);
  i = working.find_last_of(TEXT("\\"));
  return working.substr(i+1);
}

BOOL CALLBACK ThemeEnum(HWND hwnd, LPARAM lParam)
{
  WCHAR hwndClass[MAX_LINE_LENGTH], *test = reinterpret_cast<WCHAR*>(lParam);
  COPYDATASTRUCT cds;

  if (GetClassName(hwnd, hwndClass, MAX_LINE_LENGTH) == 0)
    return TRUE;

  if ((_wcsicmp(hwndClass, TEXT("EmergeDesktopApplet")) != 0) &&
      (_wcsicmp(hwndClass, TEXT("EmergeDesktopMenuBuilder")) != 0) &&
      (_wcsicmp(hwndClass, TEXT("EmergeDesktopCore")) != 0))
    return TRUE;

  cds.dwData = EMERGE_MESSAGE;
  cds.cbData = sizeof(WCHAR) * (wcslen(test) + 1);
  cds.lpData = test;

  SendMessage(hwnd, WM_COPYDATA, (WPARAM)0, (LPARAM)&cds);

  return TRUE;
}

bool ELSetTheme(std::wstring theme)
{
  bool ret = false;
  std::wstring emergeXML = TEXT("%EmergeDir%\\theme.xml");
  std::tr1::shared_ptr<TiXmlDocument> configXML = ELOpenXMLConfig(emergeXML, true);
  TiXmlElement *section;

  theme = ELExpandVars(theme);
  EnumWindows(ThemeEnum, (LPARAM)theme.c_str());
  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Theme"), true);

      if (section)
        {
          ELWriteXMLStringValue(section, (WCHAR*)TEXT("Current"), ELGetThemeName().c_str());
          ELWriteXMLConfig(configXML.get());

          ret = true;
        }
    }

  return ret;
}

std::tr1::shared_ptr<TiXmlDocument> ELOpenXMLConfig(std::wstring file, bool create)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  file = ELExpandVars(file);
  std::string ansiFile = ELwstringTostring(file, CP_ACP);

  configXML = OpenXMLConfig(ansiFile, create);

  return configXML;
}

bool ELCreateDirectory(std::wstring directory)
{
  size_t i = 0;

  directory = ELExpandVars(directory);

  i = directory.find_first_of(TEXT("\\"), i);
  while (i != std::wstring::npos)
    {
      if (!PathIsDirectory(directory.substr(0, i).c_str()))
        if (!CreateDirectory(directory.substr(0, i).c_str(), NULL))
          return false;
      i++;
      i = directory.find_first_of(TEXT("\\"), i);
    }

  if (!PathIsDirectory(directory.c_str()))
    {
      if (!CreateDirectory(directory.c_str(), NULL))
        return false;
    }

  return true;
}

bool ELReadXMLColorValue(TiXmlElement *section, const WCHAR *item, COLORREF *value, COLORREF defaultValue)
{
  bool ret;
  WCHAR stringValue[MAX_LINE_LENGTH], stringDefault[MAX_LINE_LENGTH];
  int red = 0, blue = 0, green = 0;

  swprintf(stringDefault, TEXT("%d,%d,%d"), GetRValue(defaultValue),
           GetGValue(defaultValue), GetBValue(defaultValue));

  ret = ELReadXMLStringValue(section, item, stringValue, stringDefault);

  if (swscanf(stringValue, TEXT("%d,%d,%d"), &red, &green, &blue) == 3)
    {
      if ((red >= 0 && red <= 255) &&
          (green >= 0 && green <= 255) &&
          (blue >= 0 && blue <= 255))
        (*value) = RGB(red, green, blue);
    }
  else
    (*value) = defaultValue;

  return ret;
}

bool ELReadXMLRectValue(TiXmlElement *section, const WCHAR *item, RECT *value, RECT defaultValue)
{
  bool ret;
  WCHAR stringValue[MAX_LINE_LENGTH], stringDefault[MAX_LINE_LENGTH];

  swprintf(stringDefault, TEXT("%d,%d,%d,%d"), defaultValue.top, defaultValue.left,
           defaultValue.bottom, defaultValue.right);

  ret = ELReadXMLStringValue(section, item, stringValue, stringDefault);

  if (swscanf(stringValue, TEXT("%d,%d,%d,%d"), &value->top, &value->left,
              &value->bottom, &value->right) != 4)
    {
      CopyRect(value, &defaultValue);
      ret = false;
    }

  return ret;
}

bool ELReadXMLBoolValue(TiXmlElement *section, const WCHAR *item, bool *value, bool defaultValue)
{
  bool ret;
  WCHAR stringValue[MAX_LINE_LENGTH], stringDefault[MAX_LINE_LENGTH];

  swprintf(stringDefault, TEXT("false"));
  if (defaultValue)
    swprintf(stringDefault, TEXT("true"));

  ret = ELReadXMLStringValue(section, item, stringValue, stringDefault);

  *value = (_wcsicmp(stringValue, TEXT("true")) == 0);

  return ret;
}

bool ELReadXMLIntValue(TiXmlElement *section, const WCHAR *item, int *value, int defaultValue)
{
  bool ret;
  WCHAR stringValue[MAX_LINE_LENGTH], stringDefault[MAX_LINE_LENGTH];

  swprintf(stringDefault, TEXT("%d"), defaultValue);

  ret = ELReadXMLStringValue(section, item, stringValue, stringDefault);

  swscanf(stringValue, TEXT("%d"), value);

  return ret;
}

bool ELReadXMLFloatValue(TiXmlElement *section, const WCHAR *item, float *value, float defaultValue)
{
  bool ret;
  WCHAR stringValue[MAX_LINE_LENGTH], stringDefault[MAX_LINE_LENGTH];

  swprintf(stringDefault, TEXT("%f"), defaultValue);

  ret = ELReadXMLStringValue(section, item, stringValue, stringDefault);

  swscanf(stringValue, TEXT("%f"), value);

  return ret;
}

bool ELReadXMLStringValue(TiXmlElement *section, const WCHAR *item, WCHAR *value, const WCHAR *defaultValue)
{
  std::string narrowItem = ELwstringTostring(item);
  std::wstring wideString;

  if (!section)
    return false;

  TiXmlElement *xmlItem = section->FirstChildElement(narrowItem.c_str());
  if (!xmlItem)
    {
      wcscpy(value, defaultValue);
      return false;
    }

  if (xmlItem->GetText() != NULL)
    {
      wideString = ELstringTowstring(xmlItem->GetText());
      wcscpy(value, wideString.c_str());
    }
  else
    {
      wcscpy(value, defaultValue);
      return false;
    }

  return true;
}

bool ELWriteXMLColorValue(TiXmlElement *section, const WCHAR *item, COLORREF value)
{
  WCHAR stringValue[MAX_LINE_LENGTH];

  swprintf(stringValue, TEXT("%d,%d,%d"), GetRValue(value), GetGValue(value),
           GetBValue(value));

  return ELWriteXMLStringValue(section, item, stringValue);
}

bool ELWriteXMLRectValue(TiXmlElement *section, const WCHAR *item, RECT value)
{
  WCHAR stringValue[MAX_LINE_LENGTH];

  swprintf(stringValue, TEXT("%d,%d,%d,%d"), value.top, value.left,
           value.bottom, value.right);

  return ELWriteXMLStringValue(section, item, stringValue);
}

bool ELWriteXMLBoolValue(TiXmlElement *section, const WCHAR *item, bool value)
{
  WCHAR stringValue[MAX_LINE_LENGTH];

  swprintf(stringValue, TEXT("false"));
  if (value)
    swprintf(stringValue, TEXT("true"));

  return ELWriteXMLStringValue(section, item, stringValue);
}

bool ELWriteXMLIntValue(TiXmlElement *section, const WCHAR *item, int value)
{
  WCHAR stringValue[MAX_LINE_LENGTH];

  swprintf(stringValue, TEXT("%d"), value);

  return ELWriteXMLStringValue(section, item, stringValue);
}

bool ELWriteXMLFloatValue(TiXmlElement *section, const WCHAR *item, float value)
{
  WCHAR stringValue[MAX_LINE_LENGTH];

  swprintf(stringValue, TEXT("%f"), value);

  return ELWriteXMLStringValue(section, item, stringValue);
}

bool ELWriteXMLStringValue(TiXmlElement *section, const WCHAR *item, const WCHAR *value)
{
  std::string narrowItem = ELwstringTostring(item), narrowValue = ELwstringTostring(value);

  if (!section)
    return false;

  TiXmlElement *xmlItem = section->FirstChildElement(narrowItem.c_str());
  if (!xmlItem)
    {
      xmlItem = new TiXmlElement(narrowItem.c_str());
      xmlItem->LinkEndChild(new TiXmlText(narrowValue.c_str()));
      return (section->LinkEndChild(xmlItem) != NULL);
    }
  else
    {
      TiXmlElement newItem(narrowItem.c_str());
      newItem.LinkEndChild(new TiXmlText(narrowValue.c_str()));
      return (section->ReplaceChild(xmlItem, newItem) != NULL);
    }
}

TiXmlElement *ELGetXMLSection(TiXmlDocument *configXML, WCHAR *section, bool createSection)
{
  std::string narrowSection = ELwstringTostring(section);

  TiXmlElement *xmlSection = configXML->FirstChildElement(narrowSection.c_str());
  if (!xmlSection && createSection)
    {
      xmlSection = new TiXmlElement(narrowSection.c_str());
      configXML->LinkEndChild(xmlSection);
      configXML->SaveFile();
    }

  return xmlSection;
}

TiXmlElement *ELGetFirstXMLElement(TiXmlElement *xmlSection)
{
  return xmlSection->FirstChildElement();
}

TiXmlElement *ELGetFirstXMLElementByName(TiXmlElement *xmlSection, WCHAR *elementName)
{
  std::string narrowElement = ELwstringTostring(elementName);
  TiXmlElement *child;

  child = xmlSection->FirstChildElement(narrowElement.c_str());
  if (!child)
    child = ELSetFirstXMLElement(xmlSection, elementName);

  return child;
}

TiXmlElement *ELSetFirstXMLElement(TiXmlElement *xmlSection, const WCHAR *elementName)
{
  std::string narrowElement = ELwstringTostring(elementName);
  TiXmlElement *child;

  child = new TiXmlElement(narrowElement.c_str());
  xmlSection->LinkEndChild(child);

  return child;
}

TiXmlElement *ELGetSiblingXMLElement(TiXmlElement *xmlElement)
{
  return xmlElement->NextSiblingElement();
}

TiXmlElement *ELSetSibilingXMLElement(TiXmlElement *xmlElement, const WCHAR *elementName)
{
  std::string narrowElement = ELwstringTostring(elementName);
  TiXmlElement *sibling, newSibling(narrowElement.c_str());

  sibling = xmlElement->Parent()->InsertAfterChild(xmlElement, newSibling)->ToElement();

  return sibling;
}

bool ELGetXMLElementText(TiXmlElement *xmlElement, WCHAR* xmlString)
{
  if (!xmlElement)
    return false;

  if (xmlElement->GetText() != NULL)
    {
      std::wstring wideString = ELstringTowstring(xmlElement->GetText());
      wcscpy(xmlString, wideString.c_str());
      return true;
    }

  return false;
}

bool ELGetXMLElementLabel(TiXmlElement *xmlElement, WCHAR* xmlString)
{
  if (xmlElement->Value() != NULL)
    {
      std::wstring wideString = ELstringTowstring(xmlElement->Value());
      wcscpy(xmlString, wideString.c_str());
      return true;
    }

  return false;
}

bool ELRemoveXMLElement(TiXmlElement *xmlElement)
{
  TiXmlNode *xmlSection = xmlElement->Parent();

  return xmlSection->RemoveChild(xmlElement);
}

bool ELWriteXMLConfig(TiXmlDocument *configXML)
{
  return configXML->SaveFile();
}

TiXmlDocument *ELGetXMLConfig(TiXmlElement *element)
{
  return element->GetDocument();
}

/*!
  @fn	bool ELReadRegDWord(HKEY key, LPTSTR value, DWORD *target, DWORD notFound)
  @brief Reads a DWORD from the specified key in the registry
  @param key key to read from
  @param value value to read from the key
  @param target pointer to DWORD to recieve value
  @param notFound value to use if not found
  */
bool ELReadRegDWord(HKEY key, LPCTSTR value, DWORD *target, DWORD notFound)
{
  DWORD dwSize = sizeof(DWORD);

  if (RegQueryValueEx(key, value, NULL, NULL, (LPBYTE)target, &dwSize) == ERROR_SUCCESS)
    return true;

  // Return notFound value
  *target = notFound;
  return false;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELWriteRegDWord
// Requires:	HKEY key - key to write to
// 		LPTSTR value - value to write to the key
// 		DWORD *source - pointer to DWORD to be written
// Returns:	BOOL
// Purpose:	Writes a DWORD to the specified key in the registry
//----  --------------------------------------------------------------------------------------------------------
bool ELWriteRegDWord(HKEY key, LPCTSTR value, DWORD *source)
{
  if (RegSetValueEx(key, value, 0, REG_DWORD, (BYTE*)source, sizeof(DWORD)) == ERROR_SUCCESS)
    return true;

  return false;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELReadRegRect
// Requires:	HKEY key - key to read from
// 		LPTSTR value - value to read from the key
// 		RECT *target - pointer to the rect that recieves the values
// 		LPTSTR notFound - value to use if not found
// Returns:	BOOL
// Purpose:	Reads a string from the specified key in the registry
//----  --------------------------------------------------------------------------------------------------------
bool ELReadRegRect(HKEY key, LPCTSTR value, RECT *target, RECT *notFound)
{
  DWORD dwSize = MAX_LINE_LENGTH;
  WCHAR tmp[MAX_LINE_LENGTH];

  if (RegQueryValueEx(key, value, NULL, NULL, (LPBYTE)tmp, &dwSize) != ERROR_SUCCESS)
    {
      CopyRect(target, notFound);
      return false;
    }

  // Return notFound value
  if (swscanf(tmp, TEXT("%d,%d,%d,%d"), &target->top, &target->left,
              &target->bottom, &target->right) != 4)
    {
      CopyRect(target, notFound);
      return false;
    }

  return true;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELReadRegString
// Requires:	HKEY key - key to read from
// 		LPTSTR value - value to read from the key
// 		LPTSTR target - pointer to the string that recieves the value
// 		LPTSTR notFound - value to use if not found
// Returns:	BOOL
// Purpose:	Reads a string from the specified key in the registry
//----  --------------------------------------------------------------------------------------------------------
bool ELReadRegString(HKEY key, LPCTSTR value, LPTSTR target, LPCTSTR notFound)
{
  DWORD dwSize = MAX_LINE_LENGTH;

  if (RegQueryValueEx(key, value, NULL, NULL, (LPBYTE)target, &dwSize) == ERROR_SUCCESS)
    return true;

  // Return notFound value
  wcscpy(target, notFound);
  return false;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELWriteRegRect
// Requires:	HKEY key - key to write to
// 		LPTSTR value - value to write to the key
// 		RECT* source - pointer to the rect to be written
// Returns:	BOOL
// Purpose:	Writes a rect string to the specified key in the registry
//----  --------------------------------------------------------------------------------------------------------
bool ELWriteRegRect(HKEY key, LPTSTR value, RECT *source)
{
  WCHAR tmp[MAX_LINE_LENGTH];

  swprintf(tmp, TEXT("%d,%d,%d,%d"), source->top, source->left, source->bottom, source->right);

  return ELWriteRegString(key, value, tmp);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELWriteRegString
// Requires:	HKEY key - key to write to
// 		LPTSTR value - value to write to the key
// 		LPTSTR source - pointer to the string to be written
// Returns:	BOOL
// Purpose:	Writes a string to the specified key in the registry
//----  --------------------------------------------------------------------------------------------------------
bool ELWriteRegString(HKEY key, LPTSTR value, LPTSTR source)
{
  if (RegSetValueEx(key, value, 0, REG_SZ, (BYTE*)source, (DWORD)wcslen(source) * sizeof(source[0])) ==
      ERROR_SUCCESS)
    return true;

  return false;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELOpenRegKey
// Requires:	LPTSTR subkey - subkey to open
// 		HKEY *key - pointer to the key that will recieve the
// 		handle
// Returns:	BOOL
// Purpose:	Opens the subkey specified in the current users
// 		Software\\Emerge Desktop key.  If the key does not exist,
// 		it is created.
//----  --------------------------------------------------------------------------------------------------------
bool ELOpenRegKey(LPCTSTR subkey, HKEY *key, bool createKey)
{
  DWORD result;
  WCHAR inputKey[MAX_LINE_LENGTH];

  wcscpy(inputKey, TEXT("Software\\Emerge Desktop\\"));
  wcscat(inputKey, subkey);

  if (createKey)
    {
      if (RegCreateKeyEx(HKEY_CURRENT_USER, inputKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                         NULL, key, &result) == ERROR_SUCCESS)
        return true;
    }
  else
    {
      if (RegOpenKeyEx(HKEY_CURRENT_USER, inputKey, 0, KEY_ALL_ACCESS, key) == ERROR_SUCCESS)
        return true;
    }

  return false;
}


//----  --------------------------------------------------------------------------------------------------------
// Function:	ELDeleteRegKey
// Requires:	LPTSTR subkey - subkey to open
// Returns:	BOOL
// Purpose:	Deletes the subkey specified in the current users
// 		Software\\Emerge Desktop key.
//----  --------------------------------------------------------------------------------------------------------
bool ELDeleteRegKey(LPTSTR subkey)
{
  WCHAR inputKey[MAX_LINE_LENGTH] = TEXT("Software\\Emerge Desktop\\");
  wcscat(inputKey, subkey);

  if (SHDeleteKey(HKEY_CURRENT_USER, inputKey) == ERROR_SUCCESS)
    return true;

  return false;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELCloseRegKey
// Requires:	HKEY key - key to close
// Returns:	BOOL
// Purpose:	Closes the specified key
//----  --------------------------------------------------------------------------------------------------------
bool ELCloseRegKey(HKEY key)
{
  if (RegCloseKey(key) == ERROR_SUCCESS)
    return true;

  return false;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELGetCoreWindow
// Requires:	Nothing
// Returns:	HWND
// Purpose:	Returns emergeCore's control window
//----  --------------------------------------------------------------------------------------------------------
HWND ELGetCoreWindow()
{
  return FindWindow(TEXT("EmergeDesktopCore"), NULL);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELGetDesktopWindow
// Requires:	Nothing
// Returns:	HWND
// Purpose:	Returns emergeCore's desktop window
//----  --------------------------------------------------------------------------------------------------------
HWND ELGetDesktopWindow()
{
  HWND deskWindow = FindWindow(TEXT("progman"), NULL);

  if (deskWindow == NULL)
    deskWindow = HWND_BOTTOM;
  else
    deskWindow = GetNextWindow(deskWindow, GW_HWNDPREV);

  return deskWindow;
}

std::string ELwstringTostring(std::wstring inString, UINT codePage)
{
  std::wstring wideString = inString;
  std::string returnString;

  size_t tmpStringLength = WideCharToMultiByte(codePage, 0, wideString.c_str(), wideString.length(), NULL, 0,
                                               NULL, NULL);
  if (tmpStringLength != 0)
    {
      LPSTR tmpString = new char[tmpStringLength + 1];
      size_t writtenBytes = WideCharToMultiByte(codePage, 0, wideString.c_str(), wideString.length(), tmpString,
                                                tmpStringLength, NULL, NULL);
      if (writtenBytes != 0)
        {
          if (writtenBytes <= tmpStringLength)
            tmpString[writtenBytes] = '\0';
          returnString = tmpString;
        }
      delete tmpString;
    }
  return returnString;
}

std::wstring ELstringTowstring(std::string inString, UINT codePage)
{
  std::string narrowString = inString;
  std::wstring returnString;

  size_t tmpStringLength = MultiByteToWideChar(codePage, 0, narrowString.c_str(), narrowString.length(), NULL, 0);
  if (tmpStringLength != 0)
    {
      LPWSTR tmpString = new WCHAR[tmpStringLength + 1];
      size_t writtenBytes = MultiByteToWideChar(codePage, 0, narrowString.c_str(), narrowString.length(), tmpString,
                                                tmpStringLength);
      if (writtenBytes != 0)
        {
          if (writtenBytes <= tmpStringLength)
            tmpString[writtenBytes] = '\0';
          returnString = tmpString;
        }
      delete tmpString;
    }

  return returnString;
}

bool ELFileOp(HWND appletWnd, UINT function, std::wstring source, std::wstring destination)
{
  SHFILEOPSTRUCT fileOpStruct;
  WCHAR *fromString = NULL, *toString = NULL;
  bool ret = false;

  source = ELExpandVars(source);
  destination = ELExpandVars(destination);
  ZeroMemory(&fileOpStruct, sizeof(fileOpStruct));

  fileOpStruct.hwnd = appletWnd;
  fileOpStruct.wFunc = function;
  fileOpStruct.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI;

  fromString = (WCHAR*)GlobalAlloc(GPTR, sizeof(WCHAR) * (source.length() + 2));
  wcscpy(fromString, source.c_str());
  fileOpStruct.pFrom = fromString;

  if (destination.length() != 0)
    {
      toString = (WCHAR*)GlobalAlloc(GPTR, sizeof(WCHAR) * (destination.length() + 2));
      wcscpy(toString, destination.c_str());
      fileOpStruct.pTo = toString;
    }

  if (SHFileOperation(&fileOpStruct) == 0)
    ret = true;

  if (destination.length() != 0)
    GlobalFree(toString);
  GlobalFree(fromString);

  return ret;
}

void ELWriteDebug(std::wstring debugText)
{
  std::wfstream out;
  std::string filename;
  std::wstring wideFilename;

  wideFilename = ELGetUserDataPath() + TEXT("\\Debug.txt");
  filename = ELwstringTostring(wideFilename);

  out.open(filename.c_str(), std::wfstream::out | std::wfstream::app);
  out << ELGetProcessIDApp(GetCurrentProcessId(), false) << TEXT(": ") << debugText << std::endl;
  out.close();
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELExecuteInternal
// Requires:	LPTSTR command - internal command
// Returns:	bool
// Purpose:	Executes internal Emerge Desktop commands
//----  --------------------------------------------------------------------------------------------------------
bool ELExecuteInternal(LPTSTR command)
{
  if (_wcsicmp(command, TEXT("RightDeskMenu")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, (WPARAM)EMERGE_CORE, (LPARAM)CORE_RIGHTMENU);
      return true;
    }
  else if (_wcsicmp(command, TEXT("MidDeskMenu")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, (WPARAM)EMERGE_CORE, (LPARAM)CORE_MIDMENU);
      return true;
    }
  else if (_wcsicmp(command, TEXT("Quit")) == 0)
    {
      ELQuit(true);
      return true;
    }
  else if (_wcsicmp(command, TEXT("Run")) == 0)
    {
      ELRun();
      return true;
    }
  else if (_wcsicmp(command, TEXT("Shutdown")) == 0)
    {
      ELShutdown(ELGetCoreWindow());
      return true;
    }
  else if (_wcsicmp(command, TEXT("Hide")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, (WPARAM)EMERGE_CORE, (LPARAM)CORE_HIDE);
      return true;
    }
  else if (_wcsicmp(command, TEXT("Show")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, (WPARAM)EMERGE_CORE, (LPARAM)CORE_SHOW);
      return true;
    }
  else if(_wcsicmp(command,TEXT("VWM_1")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(),EMERGE_DISPATCH, (WPARAM)EMERGE_VWM,(LPARAM)VWM_1);
      return true;
    }
  else if(_wcsicmp(command,TEXT("VWM_2")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(),EMERGE_DISPATCH, (WPARAM)EMERGE_VWM,(LPARAM)VWM_2);
      return true;
    }
  else if(_wcsicmp(command,TEXT("VWM_3")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(),EMERGE_DISPATCH, (WPARAM)EMERGE_VWM,(LPARAM)VWM_3);
      return true;
    }
  else if(_wcsicmp(command,TEXT("VWM_4")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(),EMERGE_DISPATCH, (WPARAM)EMERGE_VWM,(LPARAM)VWM_4);
      return true;
    }
  else if(_wcsicmp(command,TEXT("VWM_5")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(),EMERGE_DISPATCH, (WPARAM)EMERGE_VWM,(LPARAM)VWM_5);
      return true;
    }
  else if(_wcsicmp(command,TEXT("VWM_6")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(),EMERGE_DISPATCH, (WPARAM)EMERGE_VWM,(LPARAM)VWM_6);
      return true;
    }
  else if(_wcsicmp(command,TEXT("VWM_7")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(),EMERGE_DISPATCH, (WPARAM)EMERGE_VWM,(LPARAM)VWM_7);
      return true;
    }
  else if(_wcsicmp(command,TEXT("VWM_8")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(),EMERGE_DISPATCH, (WPARAM)EMERGE_VWM,(LPARAM)VWM_8);
      return true;
    }
  else if(_wcsicmp(command,TEXT("VWM_9")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(),EMERGE_DISPATCH, (WPARAM)EMERGE_VWM,(LPARAM)VWM_9);
      return true;
    }
  else if (_wcsicmp(command, TEXT("VWMUp")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, (WPARAM)EMERGE_VWM, (LPARAM)VWM_UP);
      return true;
    }
  else if (_wcsicmp(command, TEXT("VWMDown")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, (WPARAM)EMERGE_VWM, (LPARAM)VWM_DOWN);
      return true;
    }
  else if (_wcsicmp(command, TEXT("VWMLeft")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, (WPARAM)EMERGE_VWM, (LPARAM)VWM_LEFT);
      return true;
    }
  else if (_wcsicmp(command, TEXT("VWMRight")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, (WPARAM)EMERGE_VWM, (LPARAM)VWM_RIGHT);
      return true;
    }
  else if (_wcsicmp(command, TEXT("VWMGather")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, (WPARAM)EMERGE_VWM, (LPARAM)VWM_GATHER);
      return true;
    }
  else if (_wcsicmp(command, TEXT("EmptyBin")) == 0)
    {
      SHEmptyRecycleBin(NULL, NULL, 0);
      return true;
    }
  else if (_wcsicmp(command, TEXT("Lock")) == 0)
    return (LockWorkStation() == TRUE);
  else if (_wcsicmp(command, TEXT("Logoff")) == 0)  //allelimo 05/28/2004
    {
      ELExit(EMERGE_LOGOFF, true);
      return true;
    }
  else if (_wcsicmp(command, TEXT("Disconnect")) == 0)
    {
      ELExit(EMERGE_DISCONNECT, true);
      return true;
    }
  else if (_wcsicmp(command, TEXT("Reboot")) == 0)
    {
      ELExit(EMERGE_REBOOT, true);
      return true;
    }
  else if (_wcsicmp(command, TEXT("Halt")) == 0)
    {
      ELExit(EMERGE_HALT, true);
      return true;
    }
  else if (_wcsicmp(command, TEXT("Suspend")) == 0)
    {
      ELExit(EMERGE_SUSPEND, true);
      return true;
    }
  else if (_wcsicmp(command, TEXT("Hibernate")) == 0)
    {
      ELExit(EMERGE_HIBERNATE, true);
      return true;
    }
  else if (_wcsicmp(command, TEXT("ShowDesktop")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, (WPARAM)EMERGE_CORE, (LPARAM)CORE_DESKTOP);
      return true;
    }
  else if (_wcsicmp(command, TEXT("WorkspaceSettings")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, (WPARAM)EMERGE_CORE, (LPARAM)CORE_SETTINGS);
      return true;
    }
  else if (_wcsicmp(command, TEXT("CoreLaunchEditor")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, (WPARAM)EMERGE_CORE, (LPARAM)CORE_LAUNCH);
      return true;
    }
  else if (_wcsicmp(command, TEXT("CoreShellChanger")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, (WPARAM)EMERGE_CORE, (LPARAM)CORE_SHELL);
      return true;
    }
  else if (_wcsicmp(command, TEXT("CoreThemeSelector")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, (WPARAM)EMERGE_CORE, (LPARAM)CORE_THEMESELECT);
      return true;
    }
  else if (_wcsicmp(command, TEXT("CoreAbout")) == 0)
    {
      ELSwitchToThisWindow(ELGetCoreWindow());
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, (WPARAM)EMERGE_CORE, (LPARAM)CORE_ABOUT);
      return true;
    }
  else if (_wcsicmp(command, TEXT("VolumeUp")) == 0)
    {
      ELAdjustVolume(ELAV_VOLUMEUP);
      return true;
    }
  else if (_wcsicmp(command, TEXT("VolumeDown")) == 0)
    {
      ELAdjustVolume(ELAV_VOLUMEDOWN);
      return true;
    }
  else if (_wcsicmp(command, TEXT("VolumeMute")) == 0)
    {
      ELAdjustVolume(ELAV_MUTE);
      return true;
    }
  else if (_wcsicmp(command, TEXT("Homepage")) == 0)
    {
      ELExecute((WCHAR*)TEXT("http://emergedesktop.org"));
      return true;
    }
  else if (_wcsicmp(command, TEXT("Tutorial")) == 0)
    {
      ELExecute((WCHAR*)TEXT("http://sites.google.com/site/emergedesktop/Home/getting-started/the-basics"));
      return true;
    }
  /*  else if (_wcsicmp(command, TEXT("BrowserBack")) == 0)
      {
      ELControlBrowser(ELCB_BACK);
      return true;
      }*/

  return false;
}

int ELMakeZip(std::wstring zipFile, std::wstring zipRoot, std::wstring zipPath)
{
  std::wstring forwardSlash = TEXT("\\"), backSlash = TEXT("/");
  std::string narrowZipFile, narrowZipRoot, narrowZipPath;

  zipFile = ELExpandVars(zipFile);
  zipRoot = ELExpandVars(zipRoot);
  zipPath = ELExpandVars(zipPath);

  // switch to UNIX path separators since zip lib requires it
  zipFile = ELwstringReplace(zipFile, forwardSlash, backSlash, false);

  narrowZipFile = ELwstringTostring(zipFile);
  narrowZipRoot = ELwstringTostring(zipRoot);
  narrowZipPath = ELwstringTostring(zipPath);

  return MakeZip((char*)narrowZipFile.c_str(), (char*)narrowZipRoot.c_str(), (char*)narrowZipPath.c_str());
}

int ELExtractZip(std::wstring zipFile, std::wstring unzipPath)
{
  std::wstring forwardSlash = TEXT("\\"), backSlash = TEXT("/");
  std::string narrowZipFile, narrowUnzipPath;

  zipFile = ELExpandVars(zipFile);
  unzipPath = ELExpandVars(unzipPath);

  // switch to UNIX path separators since unzip lib requires it
  zipFile = ELwstringReplace(zipFile, forwardSlash, backSlash, false);

  narrowZipFile = ELwstringTostring(zipFile);
  narrowUnzipPath = ELwstringTostring(unzipPath);

  return ExtractZip((char*)narrowZipFile.c_str(), (char*)narrowUnzipPath.c_str());
}

void stripQuotes(LPTSTR source)
{
  WCHAR target[MAX_LINE_LENGTH];
  UINT sourceIndex = 0;
  UINT targetIndex = 0;
  UINT sourceLength = (UINT)wcslen(source);

  ZeroMemory(target, MAX_LINE_LENGTH);

  while (sourceIndex < sourceLength)
    {
      if (source[sourceIndex] != '\"')
        {
          target[targetIndex] = source[sourceIndex];
          targetIndex++;
        }

      sourceIndex++;
    }

  wcscpy(source, target);
}

bool ELParseCommand(const WCHAR *application, WCHAR *program, WCHAR *arguments)
{
  WCHAR buildTmp[MAX_LINE_LENGTH], pathTmp[MAX_LINE_LENGTH];
  ZeroMemory(buildTmp, MAX_LINE_LENGTH);
  wcscpy(program, TEXT("\0"));
  wcscpy(arguments, TEXT("\0"));
  bool valid = false;
  UINT argIndex = 0, appIndex = 0;
  size_t appLength = wcslen(application);
  std::wstring workingApp = application;

  if (!workingApp.empty())
    {
      if (workingApp.at(0) == '@')
        workingApp = workingApp.substr(1);
    }
  workingApp = ELExpandVars(workingApp);

  // Bail if workingApp is a directory, UNC Path or it exists
  if (PathIsDirectory(workingApp.c_str()) ||
      PathIsUNC(workingApp.c_str()) ||
      PathIsURL(workingApp.c_str()) ||
      ELPathFileExists(workingApp.c_str()))
    {
      wcscpy(program, workingApp.c_str());
      return true;
    }

  wcscpy(pathTmp, workingApp.c_str());
  if (ELCheckPathWithExtension(pathTmp))
    {
      wcscpy(program, pathTmp);
      return true;
    }
  ZeroMemory(pathTmp, MAX_LINE_LENGTH);

  while (appIndex < appLength)
    {
      if ((application[appIndex] == ' ') || (application[appIndex] == ','))
        {
          wcscpy(pathTmp, buildTmp);

          if (ELCheckPathWithExtension(pathTmp))
            {
              ZeroMemory(arguments, wcslen(arguments));
              argIndex = 0;
              wcscpy(program, pathTmp);
              valid = true;
            }
        }

      if (valid)
        {
          if ((appIndex + 1) <= appLength)
            arguments[argIndex] = application[appIndex + 1];
          argIndex++;
        }

      buildTmp[appIndex] = application[appIndex];

      appIndex++;
    }

  if (wcslen(program) > 0)
    return true;

  return false;
}

bool GetPIDLGUID(LPITEMIDLIST pidl, WCHAR *classID)
{
  IShellFolder *pDesktop, *pFolder;
  IPersistFolder *pPersist;
  CLSID clsID;
  LPVOID lpVoid;
  WCHAR *GUIDString;
  bool ret = false;

  if (SUCCEEDED(SHGetDesktopFolder(&pDesktop)))
    {
      if (SUCCEEDED(pDesktop->BindToObject(pidl, NULL, IID_IShellFolder, &lpVoid)))
        {
          pFolder = reinterpret_cast <IShellFolder*> (lpVoid);

          if (SUCCEEDED(pFolder->QueryInterface(IID_IPersistFolder, &lpVoid)))
            {
              pPersist = reinterpret_cast <IPersistFolder*> (lpVoid);

              if (SUCCEEDED(pPersist->GetClassID(&clsID)))
                {
                  if (SUCCEEDED(StringFromCLSID(clsID, &GUIDString)))
                    {
                      wcscpy(classID, GUIDString);
                      CoTaskMemFree(GUIDString);
                      ret = true;
                    }
                }

              pPersist->Release();
            }

          pFolder->Release();
        }

      pDesktop->Release();
    }

  return ret;
}

void ELILFree(LPITEMIDLIST pidl)
{
  if (MSILFree == NULL)
    MSILFree = (lpfnILFree)GetProcAddress(shell32DLL, (LPCSTR)155);
  if (MSILFree == NULL)
    return;

  MSILFree(pidl);
}

LPITEMIDLIST ELILClone(LPITEMIDLIST pidl)
{
  if (MSILClone == NULL)
    MSILClone = (fnILClone)GetProcAddress(shell32DLL, (LPCSTR)18);
  if (MSILClone == NULL)
    return NULL;

  return MSILClone(pidl);
}

LPITEMIDLIST ELILFindLastID(LPITEMIDLIST pidl)
{
  if (MSILFindLastID == NULL)
    MSILFindLastID = (fnILFindLastID)GetProcAddress(shell32DLL, (LPCSTR)16);
  if (MSILFindLastID == NULL)
    return NULL;

  return MSILFindLastID(pidl);
}

BOOL ELILRemoveLastID(LPITEMIDLIST pidl)
{
  if (MSILRemoveLastID == NULL)
    MSILRemoveLastID = (fnILRemoveLastID)GetProcAddress(shell32DLL, (LPCSTR)17);
  if (MSILRemoveLastID == NULL)
    return FALSE;

  return MSILRemoveLastID(pidl);
}

bool GetSpecialFolderGUID(int folder, WCHAR *classID)
{
  LPITEMIDLIST pidl;
  bool ret = false;

  if (SUCCEEDED(SHGetFolderLocation(NULL, folder, NULL, 0, &pidl)))
    {
      ret = GetPIDLGUID(pidl, classID);
      ELILFree(pidl);
    }

  return ret;
}

bool ELExecuteSpecialFolder(LPTSTR folder)
{
  WCHAR command[MAX_LINE_LENGTH], classID[MAX_PATH];
  wcscpy(command, TEXT("%windir%\\explorer.exe ::"));

  int specialFolder = ELIsSpecialFolder(folder);

  switch (specialFolder)
    {
    case 0:
      return false;

    case CSIDL_CONTROLS:
      if (ELVersionInfo() < 6.0)
        {
          if (GetSpecialFolderGUID(CSIDL_DRIVES, classID))
            {
              wcscat(command, classID);
              wcscat(command, TEXT("\\::"));
            }
        }
      if (GetSpecialFolderGUID(specialFolder, classID))
        wcscat(command, classID);
      break;
    case CSIDL_PERSONAL:
      if (ELVersionInfo() == 5.0)
        {
          wcscpy(command, TEXT("%Documents%"));
          break;
        }
    default:
      if (GetSpecialFolderGUID(specialFolder, classID))
        wcscat(command, classID);
    }

  return ELExecute(command);
}

BOOL CALLBACK WindowIconEnum(HWND hwnd, LPARAM lParam)
{
  SendMessage(hwnd, WM_SETICON, ICON_SMALL, lParam);
  SendMessage(hwnd, WM_SETICON, ICON_BIG, lParam);
  return TRUE;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELExecute
// Requires:	LPTSTR applicaiton - application to execute
// 		bool trackProcess - determines if the process handle
// 		should be returned
// Returns:	HANDLE
// Purpose:	Executes the application
//----  --------------------------------------------------------------------------------------------------------
bool ELExecute(LPTSTR application, LPTSTR workingDir, int nShow, WCHAR *verb)
{
  WCHAR program[MAX_PATH], directory[MAX_PATH], arguments[MAX_LINE_LENGTH];
  WCHAR command[MAX_LINE_LENGTH];
  WCHAR *commandDir = NULL;
  SHELLEXECUTEINFO sei;
  SHORTCUTINFO shortcutInfo;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  std::wstring workingString = application, shortcutPath;
  bool isShortcut = false;

  ZeroMemory(&sei, sizeof(sei));
  ZeroMemory(&si, sizeof(si));
  ZeroMemory(&pi, sizeof(pi));
  ZeroMemory(program, MAX_PATH);
  ZeroMemory(directory, MAX_PATH);
  ZeroMemory(arguments, MAX_LINE_LENGTH);

  if (!workingString.empty())
    {
      if (workingString.at(0) == '@')
        {
          verb = (WCHAR*)TEXT("runas");
          workingString = workingString.substr(1);
        }
    }
  workingString = ELExpandVars(workingString);
  shortcutPath = workingString;

  if (ELParseCommand(workingString.c_str(), program, arguments))
    workingString = program;
  shortcutInfo.flags = SI_PATH | SI_ARGUMENTS | SI_WORKINGDIR | SI_SHOW;
  if (ELParseShortcut(workingString.c_str(), &shortcutInfo))
    {
      isShortcut = true;
      wcscpy(program, shortcutInfo.Path);
      wcscpy(arguments, shortcutInfo.Arguments);
      wcscpy(directory, shortcutInfo.WorkingDirectory);
      nShow = shortcutInfo.ShowCmd;
    }
  else
    {
      if (wcslen(workingDir) > 0)
        wcscpy(directory, workingDir);
      else
        {
          wcscpy(directory, program);
          PathRemoveFileSpec(directory);
        }
    }

  // Expand variables again so as to not confuse CreateProcess and ShellExecuteEx
  workingString = program;
  workingString = ELExpandVars(workingString);
  wcscpy(program, workingString.c_str());

  // If the program doesn't exist or is not a URL return false so as to not
  // generate stray DDE calls
  if (!ELPathFileExists(program) && !PathIsURL(program))
    return false;

  if (!ELFileTypeCommand(program, arguments, command))
    swprintf(command, TEXT("\"%s\" %s"), program, arguments);

  if (wcslen(directory) > 0)
    {
      workingString = directory;
      workingString = ELExpandVars(workingString);
      commandDir = (WCHAR*)workingString.c_str();
    }

  if (wcslen(command) == 0)
    return false;

  si.cb = sizeof(si);
  if (isShortcut)
    {
      // 0x800 is an undocumented flag that tells the new process that lpTitle
      // is actually the path of a shortcut file. The new console process can
      // then extract the icon and title from the shortcut it was launched from.
      //
      // For console shortcuts, the title is key because that's what used to
      // determine window dimensions and buffers (similar to when you change the
      // default options you are given the choice to 'Save properties for future
      // windows with the same title'.
      si.dwFlags = 0x800;
      si.lpTitle = (WCHAR*)shortcutPath.c_str();
    }
  else
    {
      si.dwFlags = STARTF_USESHOWWINDOW;
      si.wShowWindow = nShow;
    }

  if (verb == NULL)
    {
      if (CreateProcess(NULL,
                        command,
                        NULL,
                        NULL,
                        FALSE,
                        NORMAL_PRIORITY_CLASS,
                        NULL,
                        commandDir,
                        &si,
                        &pi))
        {
          CloseHandle(pi.hProcess);
          CloseHandle(pi.hThread);
          return true;
        }
    }

  // Call ShellExecuteEx as an 'all-else-fails' mechanism since things like UAC escalation don't play nice
  // with CreateProcess
  sei.cbSize = sizeof(sei);
  sei.fMask = SEE_MASK_FLAG_NO_UI|SEE_MASK_ASYNCOK|SEE_MASK_UNICODE;
  sei.lpFile = program;
  sei.lpParameters = arguments;
  sei.lpDirectory = commandDir;
  sei.lpVerb = verb;
  sei.nShow = nShow;

  return (ShellExecuteEx(&sei) == TRUE);
}

bool ELFileTypeCommand(WCHAR *document, WCHAR *docArgs, WCHAR *commandLine)
{
  WCHAR *extension = PathFindExtension(document);
  WCHAR docExecutable[MAX_LINE_LENGTH], shortDoc[MAX_LINE_LENGTH], quotedDoc[MAX_LINE_LENGTH];
  DWORD size = MAX_LINE_LENGTH;
  BOOL isDirectory = PathIsDirectory(document);
  UINT substitutions = 0;

  if (isDirectory)
    extension = (WCHAR*)TEXT("Folder");

  // Don't attempt to determine URL handler
  if (PathIsURL(document))
    return false;

  GetShortPathName(document, shortDoc, MAX_LINE_LENGTH);
  swprintf(quotedDoc, TEXT("\"%s\""), document);

  if (ELIsExecutable(extension))
    return false;

  if (FAILED(AssocQueryString(ASSOCF_NOTRUNCATE, ASSOCSTR_COMMAND,
                              extension, NULL, docExecutable, &size)))
    return false;

  ELStringReplace(docExecutable, (WCHAR*)TEXT("/idlist,%I,"), (WCHAR*)TEXT(""), true);
  substitutions += ELStringReplace(docExecutable, (WCHAR*)TEXT("%*"), docArgs, false);
  substitutions += ELStringReplace(docExecutable, (WCHAR*)TEXT("%1"), shortDoc, false);
  if (isDirectory)
    substitutions += ELStringReplace(docExecutable, (WCHAR*)TEXT("%L"), quotedDoc, true);
  else
    substitutions += ELStringReplace(docExecutable, (WCHAR*)TEXT("%L"), document, true);
  if (substitutions == 0)
    swprintf(commandLine, TEXT("%s \"%s\""), docExecutable, document);
  else
    wcscpy(commandLine, docExecutable);

  return true;
}

bool ELIsExecutable(WCHAR *extension)
{
  WCHAR pathext[MAX_PATH], *lwrextension;
  GetEnvironmentVariable(TEXT("PATHEXT"), pathext, MAX_LINE_LENGTH);

  _wcslwr(pathext);
  lwrextension = _wcslwr(_wcsdup(extension));

  if (wcsstr(pathext, lwrextension) != NULL)
    {
      free(lwrextension);
      return true;
    }

  free(lwrextension);
  return false;
}

bool ELGetAppPath(const WCHAR *program, WCHAR *path)
{
  WCHAR appString[MAX_LINE_LENGTH];
  DWORD size;
  HKEY key;
  bool ret = false;

  swprintf(appString, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%s"),
           program);

  if (wcslen(PathFindExtension(program)) == 0)
    wcscat(appString, TEXT(".exe"));

  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, appString, 0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS)
    {
      ret = (RegQueryValueEx(key, NULL, NULL, NULL, (BYTE*)path, &size) == ERROR_SUCCESS);
      RegCloseKey(key);
    }

  return ret;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELRun
// Requires:	Nothing
// Returns:	bool
// Purpose:	Displays the MS Run dialog
//----  --------------------------------------------------------------------------------------------------------
bool ELRun()
{
  if (MSRun == NULL)
    MSRun = (lpfnMSRun)GetProcAddress(shell32DLL, (LPCSTR) 61);
  if (MSRun)
    {
      MSRun(NULL, NULL, NULL, NULL, NULL, 0);
      return true;
    }

  return false;
}

HMODULE ELGetSystemLibrary(const WCHAR *library)
{
  WCHAR libraryPath[MAX_PATH];

  if (GetSystemDirectory(libraryPath, MAX_PATH) == 0)
    return NULL;

  wcscat(libraryPath, TEXT("\\"));
  wcscat(libraryPath, library);

  return GetModuleHandle(libraryPath);
}

HMODULE ELGetEmergeLibrary(const WCHAR *library)
{
  WCHAR libraryPath[MAX_PATH];

  if (!ELGetCurrentPath(libraryPath))
    return NULL;

  wcscat(libraryPath, TEXT("\\"));
  wcscat(libraryPath, library);

  return GetModuleHandle(libraryPath);
}

HMODULE ELLoadSystemLibrary(const WCHAR *library)
{
  WCHAR libraryPath[MAX_PATH];

  if (GetSystemDirectory(libraryPath, MAX_PATH) == 0)
    return NULL;

  wcscat(libraryPath, TEXT("\\"));
  wcscat(libraryPath, library);

  return LoadLibrary(libraryPath);
}

bool ELRegisterShellHook(HWND hwnd, DWORD method)
{
  HWND shellWnd = NULL;
  MINIMIZEDMETRICS minMetrics;
  bool result = false;

  if (MSRegisterShellHookWindow == NULL)
    MSRegisterShellHookWindow = (lpfnMSRegisterShellHookWindow)GetProcAddress(shell32DLL, (LPSTR)((long)0xB5));
  if (MSRegisterShellHookWindow)
    {
      if (method == RSH_TASKMGR)
        {
          shellWnd = hwnd;

          // Hide minimized windows
          ZeroMemory(&minMetrics, sizeof(MINIMIZEDMETRICS));
          minMetrics.cbSize = sizeof(MINIMIZEDMETRICS);
          SystemParametersInfo(SPI_GETMINIMIZEDMETRICS, sizeof(MINIMIZEDMETRICS), &minMetrics,
                               0);
          minMetrics.iArrange |= ARW_HIDE;
          SystemParametersInfo(SPI_SETMINIMIZEDMETRICS, sizeof(MINIMIZEDMETRICS), &minMetrics,
                               SPIF_SENDCHANGE);
        }

      result = (MSRegisterShellHookWindow(shellWnd, method) == TRUE);
    }

  return result;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELShutdown
// Requires:	HWND wnd - calling applications window
// Returns:		bool
// Purpose:		Displays the MS Shutdown dialog
//----  --------------------------------------------------------------------------------------------------------
bool ELShutdown(HWND wnd)
{
  Shutdown shutdown(emergeLibInstance, wnd);
  shutdown.Show();

  return false;
}

int ELMessageBox(HWND hwnd, const WCHAR* messageText, const WCHAR* messageTitle, DWORD msgFlags)
{
  int ret;

  MsgBox *msgBox = new MsgBox(ELGetEmergeLibrary(TEXT("emergeLib.dll")), hwnd, messageText, messageTitle,
                              (msgFlags & 0x0000ff), (msgFlags & 0x00ff00));
  ret = msgBox->Show((msgFlags & ELMB_MODAL) == ELMB_MODAL);

  return ret;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELQuit
// Requires:	Nothing
// Returns:	bool
// Purpose:	Displays the Emerge Desktop Quit dialog
//----  --------------------------------------------------------------------------------------------------------
bool ELQuit(bool prompt)
{
  UINT response = IDYES;

  if (prompt)
    response = ELMessageBox(GetDesktopWindow(),
                            (WCHAR*)TEXT("Do you want to quit Emerge Desktop?"),
                            (WCHAR*)TEXT("Emerge Desktop"),
                            ELMB_YESNO|ELMB_ICONQUESTION|ELMB_MODAL);

  if (response == IDYES)
    {
      PostMessage(ELGetCoreWindow(), EMERGE_DISPATCH, EMERGE_CORE, CORE_QUIT);
      return true;
    }

  return false;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELCheckPathWithExtension
// Requires:	LPTSTR path - path to check
// Returns:	bool
// Purpose:	Checks to see if the path (file) specified exists in the system path
//----  --------------------------------------------------------------------------------------------------------
bool ELCheckPathWithExtension(LPTSTR path)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  WCHAR pathext[MAX_LINE_LENGTH], *token;

  stripQuotes(path);
  if (wcslen(PathFindExtension(path)) != 0)
    return PathTokenCheck(path);
  else
    {
      GetEnvironmentVariable(TEXT("PATHEXT"), pathext, MAX_LINE_LENGTH);
      wcscat(pathext, TEXT(";.LNK"));
      token = wcstok(pathext, TEXT(";"));
      while (token != NULL)
        {
          wcscpy(tmp, path);
          wcscat(tmp, token);

          if (PathTokenCheck(tmp))
            {
              wcscpy(path, tmp);
              return true;
            }

          token = wcstok(NULL, TEXT(";"));
        }
    }

  return false;
}

bool PathTokenCheck(WCHAR *path)
{
  WCHAR tmp[MAX_LINE_LENGTH], corePath[MAX_PATH], sysWOW64[MAX_PATH];
  const WCHAR *corePathPtr[MAX_PATH];
  std::wstring working = path;

  working = ELExpandVars(working);
  if (!ELGetAppPath(working.c_str(), tmp))
    wcscpy(tmp, working.c_str());

  // Strip quotes since the system 'Path...' functions don't seem to like them
  stripQuotes(tmp);

  if (ELPathFileExists(tmp) && !PathIsDirectory(tmp))
    {
      wcscpy(path, tmp);
      return true;
    }

  ELGetCurrentPath(corePath);

  corePathPtr[0] = corePath;
  corePathPtr[1] = NULL;

  ExpandEnvironmentStrings((LPCTSTR)TEXT("%systemroot%\\SysWOW64\\"),
                           (LPTSTR)sysWOW64,
                           MAX_PATH);

  if (PathFindOnPath(tmp, corePathPtr) && !PathIsDirectory(tmp))
    {
      wcscpy(path, tmp);
      return true;
    }
  else
    {
      wcscpy(tmp, sysWOW64);
      wcscat(tmp, path);

      if (ELPathFileExists(tmp) && !PathIsDirectory(tmp))
        {
          wcscpy(path, tmp);
          return true;
        }
    }

  return false;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	IsClose
// Required:	int side - side of window
//		int edge - desktop edge
// Returns:	bool
// Purpose:	Determines if a window size is near the desktop edge
//----  --------------------------------------------------------------------------------------------------------
bool IsClose(int side, int edge)
{
  return abs(edge - side) < 10;
}

BOOL CALLBACK FullscreenEnum(HWND hwnd, LPARAM lParam)
{
  RECT wndRect;
  HMONITOR hwndMonitor;
  POINT hwndPt;
  MONITORINFO hwndMonitorInfo;

  if (!IsWindowVisible(hwnd))
    return true;

  /*  if (IsZoomed(hwnd))
      return true;*/

  GetWindowRect(hwnd, &wndRect);
  hwndMonitorInfo.cbSize = sizeof(MONITORINFO);

  hwndPt.x = ELMid(wndRect.right, wndRect.left);
  hwndPt.y = ELMid(wndRect.bottom, wndRect.top);

  hwndMonitor = MonitorFromPoint(hwndPt, MONITOR_DEFAULTTONEAREST);

  if (hwndMonitor != (HMONITOR)lParam)
    return true;

  if (hwnd == FindWindow(TEXT("Progman"), NULL))
    return true;

  if (hwnd == FindWindow(TEXT("InstallShield_Win"), NULL))
    return true;

  GetMonitorInfo(hwndMonitor, &hwndMonitorInfo);
  if ((wndRect.left <= hwndMonitorInfo.rcMonitor.left) &&
      (wndRect.top <= hwndMonitorInfo.rcMonitor.top) &&
      (wndRect.right >= hwndMonitorInfo.rcMonitor.right) &&
      (wndRect.bottom >= hwndMonitorInfo.rcMonitor.bottom))
    {
      SetLastError(ERROR_SUCCESS);
      return false;
    }

  return true;
}

void ELThreadExecute(void *argument)
{
  ELExecute((WCHAR*)argument);
}

bool ELIsFullScreen(HWND appletWnd, HWND appWnd)
{
  HMONITOR appletMonitor = MonitorFromWindow(appletWnd, MONITOR_DEFAULTTONEAREST);
  DWORD threadID;

  threadID = GetWindowThreadProcessId(appWnd, NULL);

  if (EnumThreadWindows(threadID, FullscreenEnum, (LPARAM)appletMonitor) == ERROR_SUCCESS)
    return true;

  return false;
}

HMONITOR ELGetDesktopRect(RECT *appletRect, RECT *rect)
{
  MONITORINFO appletMonitorInfo;
  HMONITOR appletMonitor = MonitorFromRect(appletRect, MONITOR_DEFAULTTONEAREST);

  appletMonitorInfo.cbSize = sizeof(MONITORINFO);
  if (!GetMonitorInfo(appletMonitor, &appletMonitorInfo))
    return NULL;

  if (!CopyRect(rect, &appletMonitorInfo.rcMonitor))
    return NULL;

  return appletMonitor;
}

BOOL CALLBACK SnapMoveEnum(HWND hwnd, LPARAM lParam)
{
  WCHAR hwndClass[MAX_LINE_LENGTH];
  RECT hwndRect;

  if (hwnd == ((LPSNAPMOVEINFO)lParam)->AppletWindow)
    return true;

  if (GetClassName(hwnd, hwndClass, MAX_LINE_LENGTH) == 0)
    return true;

  if (_wcsicmp(hwndClass, TEXT("EmergeDesktopApplet")) != 0)
    return true;

  GetWindowRect(hwnd, &hwndRect);

  if (SnapMoveToEdge((LPSNAPMOVEINFO)lParam, hwndRect))
    {
      SetLastError(ERROR_SUCCESS);
      return false;
    }

  return true;
}

void ELDisplayChange(HWND hwnd)
{
  SNAPMOVEINFO snapMove;
  RECT rect;
  POINT pt = {0, 0};
  GetWindowRect(hwnd, &rect);

  snapMove.AppletWindow = hwnd;
  snapMove.AppletRect = &rect;
  snapMove.origin = pt;

  ELSnapMoveToDesk(&rect);

  SetWindowPos(hwnd, NULL, snapMove.AppletRect->left, snapMove.AppletRect->top, 0, 0,
               SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

bool SnapMoveToEdge(LPSNAPMOVEINFO snapMove, RECT rt)
{
  bool verticalSnap = false;
  bool horizontalSnap = false;
  int verticalOffset = 0;
  int horizontalOffset = 0;

  if (IsClose(snapMove->AppletRect->top, rt.top))
    {
      verticalSnap = true;
      verticalOffset = rt.top - snapMove->AppletRect->top;
    }
  else if (IsClose(snapMove->AppletRect->bottom, rt.top))
    {
      verticalSnap = true;
      verticalOffset = rt.top - snapMove->AppletRect->bottom;
    }
  else if (IsClose(snapMove->AppletRect->bottom, rt.bottom))
    {
      verticalSnap = true;
      verticalOffset = rt.bottom - snapMove->AppletRect->bottom;
    }
  else if (IsClose(snapMove->AppletRect->top, rt.bottom))
    {
      verticalSnap = true;
      verticalOffset = rt.bottom - snapMove->AppletRect->top;
    }
  else if (((snapMove->AppletRect->top < rt.bottom) && (snapMove->AppletRect->top > rt.top)) ||
           ((snapMove->AppletRect->bottom > rt.top) && (snapMove->AppletRect->bottom < rt.bottom)))
    {
      verticalSnap = true;
      verticalOffset = 0;
    }

  if (IsClose(snapMove->AppletRect->left, rt.left))
    {
      horizontalSnap = true;
      horizontalOffset = rt.left - snapMove->AppletRect->left;
    }
  else if (IsClose(snapMove->AppletRect->right, rt.left))
    {
      horizontalSnap = true;
      horizontalOffset = rt.left - snapMove->AppletRect->right;
    }
  else if (IsClose(snapMove->AppletRect->right, rt.right))
    {
      horizontalSnap = true;
      horizontalOffset = rt.right - snapMove->AppletRect->right;
    }
  else if (IsClose(snapMove->AppletRect->left, rt.right))
    {
      horizontalSnap = true;
      horizontalOffset = rt.right - snapMove->AppletRect->left;
    }
  else if (((snapMove->AppletRect->left < rt.right) && (snapMove->AppletRect->left > rt.left)) ||
           ((snapMove->AppletRect->right > rt.left) && (snapMove->AppletRect->right < rt.right)))
    {
      horizontalSnap = true;
      horizontalOffset = 0;
    }

  if (verticalSnap && horizontalSnap)
    {
      OffsetRect(snapMove->AppletRect, horizontalOffset, verticalOffset);
      return true;
    }

  return false;
}

bool ELSnapMove(LPSNAPMOVEINFO snapMove)
{
  bool moved = false;
  POINT pt;

  if ((snapMove->AppletRect->left == snapMove->AppletRect->right) ||
      (snapMove->AppletRect->top == snapMove->AppletRect->bottom))
    return false;

  GetCursorPos(&pt);
  OffsetRect(snapMove->AppletRect, pt.x - (snapMove->AppletRect->left + snapMove->origin.x),
             pt.y - (snapMove->AppletRect->top + snapMove->origin.y));

  if (EnumWindows(SnapMoveEnum, (LPARAM)snapMove) == ERROR_SUCCESS)
    moved = true;

  moved = ELSnapMoveToDesk(snapMove->AppletRect);

  return moved;
}

bool ELSnapMoveToDesk(RECT *AppletRect)
{
  bool verticalMove = false;
  bool horizontalMove = false;
  int verticalOffset = 0;
  int horizontalOffset = 0;
  bool moved = false;
  MONITORINFO appletMonitorInfo;
  appletMonitorInfo.cbSize = sizeof(MONITORINFO);
  HMONITOR appletMonitor = MonitorFromRect(AppletRect, MONITOR_DEFAULTTONULL);

  if (appletMonitor == NULL)
    return false;

  if (!GetMonitorInfo(appletMonitor, &appletMonitorInfo))
    return false;

  if (IsClose(AppletRect->top, appletMonitorInfo.rcMonitor.top))
    {
      verticalMove = true;
      verticalOffset = appletMonitorInfo.rcMonitor.top - AppletRect->top;
    }
  else if (IsClose(AppletRect->bottom, appletMonitorInfo.rcMonitor.bottom))
    {
      verticalMove = true;
      verticalOffset = appletMonitorInfo.rcMonitor.bottom - AppletRect->bottom;
    }

  if (IsClose(AppletRect->left, appletMonitorInfo.rcMonitor.left))
    {
      horizontalMove = true;
      horizontalOffset = appletMonitorInfo.rcMonitor.left - AppletRect->left;
    }
  else if (IsClose(AppletRect->right, appletMonitorInfo.rcMonitor.right))
    {
      horizontalMove = true;
      horizontalOffset = appletMonitorInfo.rcMonitor.right - AppletRect->right;
    }

  if (horizontalMove)
    {
      OffsetRect(AppletRect, horizontalOffset, 0);
      moved = true;
    }

  if (verticalMove)
    {
      OffsetRect(AppletRect, 0, verticalOffset);
      moved = true;
    }

  return moved;
}

BOOL CALLBACK SnapSizeEnum(HWND hwnd, LPARAM lParam)
{
  WCHAR hwndClass[MAX_LINE_LENGTH];
  RECT hwndRect;

  if (hwnd == ((LPSNAPSIZEINFO)lParam)->AppletWindow)
    return true;

  if (GetClassName(hwnd, hwndClass, MAX_LINE_LENGTH) == 0)
    return true;

  if (_wcsicmp(hwndClass, TEXT("EmergeDesktopApplet")) != 0)
    return true;

  GetWindowRect(hwnd, &hwndRect);

  if (SnapSizeToEdge((LPSNAPSIZEINFO)lParam, hwndRect))
    {
      SetLastError(ERROR_SUCCESS);
      return false;
    }

  return true;
}

bool SnapSizeToEdge(LPSNAPSIZEINFO snapSize, RECT rt)
{
  bool verticalSnap = false;
  bool horizontalSnap = false;
  bool moved = false;

  if (IsClose(snapSize->AppletRect->top, rt.bottom) || IsClose(snapSize->AppletRect->top, rt.top) ||
      IsClose(snapSize->AppletRect->bottom, rt.bottom) || IsClose(snapSize->AppletRect->bottom, rt.top))
    verticalSnap = true;
  if (IsClose(snapSize->AppletRect->left, rt.right) || IsClose(snapSize->AppletRect->left, rt.left) ||
      IsClose(snapSize->AppletRect->right, rt.right) || IsClose(snapSize->AppletRect->right, rt.left))
    horizontalSnap = true;

  if (!verticalSnap || !horizontalSnap)
    return false;

  switch (snapSize->AppletEdge)
    {
    case WMSZ_TOP:
      if (IsClose(snapSize->AppletRect->top, rt.top))
        {
          snapSize->AppletRect->top = rt.top;
          moved = true;
        }
      else if (IsClose(snapSize->AppletRect->top, rt.bottom))
        {
          snapSize->AppletRect->top = rt.bottom;
          moved = true;
        }
      break;
    case WMSZ_TOPLEFT:
      if (IsClose(snapSize->AppletRect->top, rt.top))
        {
          snapSize->AppletRect->top = rt.top;
          moved = true;
        }
      else if (IsClose(snapSize->AppletRect->top, rt.bottom))
        {
          snapSize->AppletRect->top = rt.bottom;
          moved = true;
        }

      if (IsClose(snapSize->AppletRect->left, rt.left))
        {
          moved = true;
          snapSize->AppletRect->left = rt.left;
        }
      else if (IsClose(snapSize->AppletRect->left, rt.right))
        {
          moved = true;
          snapSize->AppletRect->left = rt.right;
        }
      break;
    case WMSZ_TOPRIGHT:
      if (IsClose(snapSize->AppletRect->top, rt.top))
        {
          snapSize->AppletRect->top = rt.top;
          moved = true;
        }
      else if (IsClose(snapSize->AppletRect->top, rt.bottom))
        {
          snapSize->AppletRect->top = rt.bottom;
          moved = true;
        }

      if (IsClose(snapSize->AppletRect->right, rt.right))
        {
          moved = true;
          snapSize->AppletRect->right = rt.right;
        }
      else if (IsClose(snapSize->AppletRect->right, rt.left))
        {
          moved = true;
          snapSize->AppletRect->right = rt.left;
        }
      break;
    case WMSZ_LEFT:
      if (IsClose(snapSize->AppletRect->left, rt.left))
        {
          moved = true;
          snapSize->AppletRect->left = rt.left;
        }
      else if (IsClose(snapSize->AppletRect->left, rt.right))
        {
          moved = true;
          snapSize->AppletRect->left = rt.right;
        }
      break;
    case WMSZ_RIGHT:
      if (IsClose(snapSize->AppletRect->right, rt.right))
        {
          moved = true;
          snapSize->AppletRect->right = rt.right;
        }
      else if (IsClose(snapSize->AppletRect->right, rt.left))
        {
          moved = true;
          snapSize->AppletRect->right = rt.left;
        }
      break;
    case WMSZ_BOTTOM:
      if (IsClose(snapSize->AppletRect->bottom, rt.bottom))
        {
          moved = true;
          snapSize->AppletRect->bottom = rt.bottom;
        }
      else if (IsClose(snapSize->AppletRect->bottom, rt.top))
        {
          moved = true;
          snapSize->AppletRect->bottom = rt.top;
        }
      break;
    case WMSZ_BOTTOMLEFT:
      if (IsClose(snapSize->AppletRect->bottom, rt.bottom))
        {
          moved = true;
          snapSize->AppletRect->bottom = rt.bottom;
        }
      else if (IsClose(snapSize->AppletRect->bottom, rt.top))
        {
          moved = true;
          snapSize->AppletRect->bottom = rt.top;
        }

      if (IsClose(snapSize->AppletRect->left, rt.left))
        {
          moved = true;
          snapSize->AppletRect->left = rt.left;
        }
      else if (IsClose(snapSize->AppletRect->left, rt.right))
        {
          moved = true;
          snapSize->AppletRect->left = rt.right;
        }
      break;
    case WMSZ_BOTTOMRIGHT:
      if (IsClose(snapSize->AppletRect->bottom, rt.bottom))
        {
          moved = true;
          snapSize->AppletRect->bottom = rt.bottom;
        }
      else if (IsClose(snapSize->AppletRect->bottom, rt.top))
        {
          moved = true;
          snapSize->AppletRect->bottom = rt.top;
        }

      if (IsClose(snapSize->AppletRect->right, rt.right))
        {
          moved = true;
          snapSize->AppletRect->right = rt.right;
        }
      else if (IsClose(snapSize->AppletRect->right, rt.left))
        {
          moved = true;
          snapSize->AppletRect->right = rt.left;
        }
      break;
    }

  return moved;
}

bool ELSnapSize(LPSNAPSIZEINFO snapSize)
{
  RECT rt;
  bool moved = false;

  if ((snapSize->AppletRect->left == snapSize->AppletRect->right) ||
      (snapSize->AppletRect->top == snapSize->AppletRect->bottom))
    return false;

  if (EnumWindows(SnapSizeEnum, (LPARAM)snapSize) == ERROR_SUCCESS)
    moved = true;

  ELGetDesktopRect(snapSize->AppletRect, &rt);

  switch (snapSize->AppletEdge)
    {
    case WMSZ_TOP:
      if ((IsClose(snapSize->AppletRect->top, rt.top)) | (snapSize->AppletRect->top < rt.top))
        {
          moved = true;
          snapSize->AppletRect->top = rt.top;
        }
      break;
    case WMSZ_TOPLEFT:
      if ((IsClose(snapSize->AppletRect->top, rt.top)) | (snapSize->AppletRect->top < rt.top))
        {
          moved = true;
          snapSize->AppletRect->top = rt.top;
        }
      if ((IsClose(snapSize->AppletRect->left, rt.left)) | (snapSize->AppletRect->left < rt.left))
        {
          moved = true;
          snapSize->AppletRect->left = rt.left;
        }
      break;
    case WMSZ_TOPRIGHT:
      if ((IsClose(snapSize->AppletRect->top, rt.top)) | (snapSize->AppletRect->top < rt.top))
        {
          moved = true;
          snapSize->AppletRect->top = rt.top;
        }
      if ((IsClose(snapSize->AppletRect->right, rt.right)) | (snapSize->AppletRect->right > rt.right))
        {
          moved = true;
          snapSize->AppletRect->right = rt.right;
        }
      break;
    case WMSZ_LEFT:
      if ((IsClose(snapSize->AppletRect->left, rt.left)) | (snapSize->AppletRect->left < rt.left))
        {
          moved = true;
          snapSize->AppletRect->left = rt.left;
        }
      break;
    case WMSZ_RIGHT:
      if ((IsClose(snapSize->AppletRect->right, rt.right)) | (snapSize->AppletRect->right > rt.right))
        {
          moved = true;
          snapSize->AppletRect->right = rt.right;
        }
      break;
    case WMSZ_BOTTOM:
      if ((IsClose(snapSize->AppletRect->bottom, rt.bottom)) | (snapSize->AppletRect->bottom > rt.bottom))
        {
          moved = true;
          snapSize->AppletRect->bottom = rt.bottom;
        }
      break;
    case WMSZ_BOTTOMLEFT:
      if ((IsClose(snapSize->AppletRect->bottom, rt.bottom)) | (snapSize->AppletRect->bottom > rt.bottom))
        {
          moved = true;
          snapSize->AppletRect->bottom = rt.bottom;
        }
      if ((IsClose(snapSize->AppletRect->left, rt.left)) | (snapSize->AppletRect->left < rt.left))
        {
          moved = true;
          snapSize->AppletRect->left = rt.left;
        }
      break;
    case WMSZ_BOTTOMRIGHT:
      if ((IsClose(snapSize->AppletRect->bottom, rt.bottom)) | (snapSize->AppletRect->bottom > rt.bottom))
        {
          moved = true;
          snapSize->AppletRect->bottom = rt.bottom;
        }
      if ((IsClose(snapSize->AppletRect->right, rt.right)) | (snapSize->AppletRect->right > rt.right))
        {
          moved = true;
          snapSize->AppletRect->right = rt.right;
        }
      break;
    }

  return moved;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELSwitchTorhisWindow
// Required:	HWND wnd - window that will receive focus
// Returns:	bool
// Purpose:	Forces the window supplied to receive focus
//----  --------------------------------------------------------------------------------------------------------
bool ELSwitchToThisWindow(HWND wnd)
{
  if (MSSwitchToThisWindow == NULL)
    MSSwitchToThisWindow = (lpfnMSSwitchToThisWindow)GetProcAddress(user32DLL, "SwitchToThisWindow");
  if (MSSwitchToThisWindow)
    {
      MSSwitchToThisWindow(wnd, TRUE);
      return true;
    }

  return false;
}

POINT ELGetAnchorPoint(HWND hwnd)
{
  POINT pt;
  RECT wndRect;

  GetWindowRect(hwnd, &wndRect);
  GetCursorPos(&pt);

  pt.x -= wndRect.left;
  pt.y -= wndRect.top;

  return pt;
}

bool ELSetForeground(HWND wnd)
{
  WINDOWPLACEMENT winPlacement;
  winPlacement.length = sizeof(WINDOWPLACEMENT);

  if (!GetWindowPlacement(wnd, &winPlacement))
    return false;

  if ((winPlacement.showCmd == SW_SHOWMINIMIZED) &&
      (winPlacement.flags == WPF_RESTORETOMAXIMIZED))
    PostMessage(wnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
  else if (winPlacement.showCmd == SW_SHOWMINIMIZED)
    PostMessage(wnd, WM_SYSCOMMAND, SC_RESTORE, 0);

  return (SetForegroundWindow(wnd) == TRUE);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELParseShortcut
// Required:	LPTSTR shortcut - shortcut path
// 		LPSHORTCUTINFO - shortcut information
// Returns:	bool
// Purpose:	Extracted relevant information from a shortcut
//----  --------------------------------------------------------------------------------------------------------
bool ELParseShortcut(LPCTSTR shortcut, LPSHORTCUTINFO shortcutInfo)
{
  IShellLink *psl = NULL;
  IPersistFile* ppf = NULL;
  LPITEMIDLIST pidl = NULL;
  LPVOID lpVoid;
  bool ret = true;

  if (FAILED(CoInitialize(NULL)))
    return false;

  // Get a pointer to the IShellLink interface.
  if (FAILED(CoCreateInstance(CLSID_ShellLink, NULL,
                              CLSCTX_INPROC_SERVER, IID_IShellLink,
                              &lpVoid)))
    {
      CoUninitialize();
      return false;
    }
  psl = reinterpret_cast <IShellLink*> (lpVoid);

  // Get a pointer to the IPersistFile interface.
  if (FAILED(psl->QueryInterface(IID_IPersistFile,
                                 &lpVoid)))
    {
      psl->Release();
      CoUninitialize();
      return false;
    }
  ppf = reinterpret_cast <IPersistFile*> (lpVoid);

  // Load the shortcut.
  if (FAILED(ppf->Load(shortcut, STGM_READ)))
    {
      ppf->Release();
      psl->Release();
      CoUninitialize();
      return false;
    }

  if (FAILED(psl->Resolve(NULL, SLR_INVOKE_MSI | SLR_NOUPDATE | SLR_NO_UI)))
    {
      ppf->Release();
      psl->Release();
      CoUninitialize();
      return false;
    }


  // Get the path to the link target.
  if ((shortcutInfo->flags & SI_PATH) == SI_PATH)
    {
      if (SUCCEEDED(psl->GetIDList(&pidl)))
        {
          ret = (SHGetPathFromIDList(pidl, shortcutInfo->Path) == TRUE);
          ELILFree(pidl);
        }
    }

  // Get the arguments for the link target.
  if ((shortcutInfo->flags & SI_ARGUMENTS) == SI_ARGUMENTS)
    {
      if (FAILED(psl->GetArguments(shortcutInfo->Arguments, MAX_PATH)))
        ret = false;
    }

  // Get the working directory of the link target
  if ((shortcutInfo->flags & SI_WORKINGDIR) == SI_WORKINGDIR)
    {
      if (FAILED(psl->GetWorkingDirectory(shortcutInfo->WorkingDirectory, MAX_PATH)))
        ret = false;
    }

  // Get the icon info for the link target
  if ((shortcutInfo->flags & SI_ICONPATH) == SI_ICONPATH)
    {
      if (FAILED(psl->GetIconLocation(shortcutInfo->IconPath, MAX_PATH, &shortcutInfo->IconIndex)))
        ret = false;
    }

  // Get the show value of the link target
  if ((shortcutInfo->flags & SI_SHOW) == SI_SHOW)
    {
      if (FAILED(psl->GetShowCmd(&shortcutInfo->ShowCmd)))
        ret = false;
    }

  ppf->Release();
  psl->Release();
  CoUninitialize();
  return ret;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELGetCurrentPath
// Required:	LPTSTR path - receives current path
// Returns:	bool
// Purpose:	Retrieves current path
//----  --------------------------------------------------------------------------------------------------------
bool ELGetCurrentPath(LPTSTR path)
{
  if (GetModuleFileName(NULL, path, MAX_PATH) == 0)
    return false;
  PathRemoveFileSpec(path);
  return true;
}

std::wstring ELGetUserDataPath()
{
  std::wstring path = TEXT("%EmergeDir%");
  path = ELExpandVars(path);
  return path;
}

/*!
  @fn ELExecuteAlias(LPTSTR alias)
  @brief Take the supplied alias comparing it to the command file and executes the appropriate command based on the alias.
  @param alias String containing the alias
  @return true if successful
  */

bool ELExecuteAlias(LPTSTR alias)
{
  bool ret = false;
  std::wstring aliasFile = TEXT("%EmergeDir%\\files\\cmd.txt");
  aliasFile = ELExpandVars(aliasFile);

  if (wcslen(alias) < 1)
    return false;

  if (alias[0] == '.')
    {
      WCHAR line[MAX_LINE_LENGTH], *command=NULL, *value=NULL;

      FILE *fp = _wfopen(aliasFile.c_str(), TEXT("r"));

      if (!fp)
        return false;

      while (fgetws(line, MAX_LINE_LENGTH, fp))
        {
          value = wcstok(line, TEXT(" \t"));
          if (value != NULL)
            command = wcstok(NULL, TEXT("\n"));

          // execute the command
          if (wcscmp(value, alias) == 0)
            {
              ELExecute(command);
              ret = true;
              break;
            }
        }

      fclose(fp);
    }

  return ret;
}

bool ELExecuteAll(WCHAR *command, WCHAR *workingDir)
{
  bool result;

  result = ELExecuteInternal(command);
  if (!result)
    {
      result = ELExecuteAlias(command);
      if (!result)
        {
          result = ELExecuteSpecialFolder(command);
          if (!result)
            result = ELExecute(command, workingDir);
        }
    }

  return result;
}

/*!
  @fn ELExit(UINT uFlag, bool prompt)
  @brief Alternate shutdown method
  @param uFlag shutdown method.  Valid flags include: EWX_FORCE, EWX_FORCEIFHUNG, EMERGE_LOGOFF, EMERGE_HALT,
  EMERGE_REBOOT, EMERGE_SUSPEND, EMERGE_HIBERNATE
  @param prompt if true, display dialogue, if false, no dialogue
  @return true if successful
  */

bool ELExit(UINT uFlag, bool prompt)
{
  HANDLE hToken;
  TOKEN_PRIVILEGES tkp;
  WCHAR messageText[MAX_LINE_LENGTH], method[20];
  bool exitStatus = true;
  UINT response = IDYES;
  UINT mode = EWX_FORCE;
  bool elevate = false;
  UINT reason = 0;
#ifndef _W64
  LPTSTR pData = NULL;
  DWORD cbReturned = 0;
#endif

  switch (uFlag)
    {
    case EMERGE_LOGOFF:
      mode |= EWX_LOGOFF;
      wcscpy(method, TEXT("Logoff"));
      break;
    case EMERGE_REBOOT:
      mode |= EWX_REBOOT;
      elevate = true;
      wcscpy(method, TEXT("Reboot"));
      break;
    case EMERGE_HALT:
      mode |= EWX_POWEROFF;
      elevate = true;
      wcscpy(method, TEXT("Halt"));
      break;
    case EMERGE_SUSPEND:
      elevate = true;
      wcscpy(method, TEXT("Suspend"));
      break;
    case EMERGE_HIBERNATE:
      elevate = true;
      wcscpy(method, TEXT("Hibernate"));
      break;
    case EMERGE_DISCONNECT:
      wcscpy(method, TEXT("Disconnect"));
      break;
    default:
      return false;
    }

  if (elevate)
    {
      reason = SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED;

      /* Obtain the privileges necessary to shutdown the computer */
      if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return false;

      LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

      tkp.PrivilegeCount = 1;
      tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

      AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);

      if (GetLastError() != ERROR_SUCCESS)
        {
          swprintf(messageText,
                   TEXT("You do not have persission to perform a %s\n on this system."),
                   method);
          ELMessageBox(GetDesktopWindow(), messageText, (WCHAR*)TEXT("Emerge Desktop"),
                       ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
          return false;
        }
    }

  if (prompt)
    {
      swprintf(messageText, TEXT("Are you sure you want to %s?"), method);
      response = ELMessageBox(GetDesktopWindow(),
                              messageText,
                              (WCHAR*)TEXT("Emerge Desktop"),
                              ELMB_ICONQUESTION|ELMB_YESNO|ELMB_MODAL);
    }

  if (response == IDYES)
    {
      switch (uFlag)
        {
        case EMERGE_SUSPEND:
          if (!SetSuspendState(FALSE, FALSE, FALSE))
            exitStatus = false;
          break;
        case EMERGE_HIBERNATE:
          if (!SetSuspendState(TRUE, FALSE, FALSE))
            exitStatus = false;
          break;
#ifndef _W64
        case EMERGE_DISCONNECT:
          if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                                         WTS_CURRENT_SESSION, WTSConnectState,
                                         &pData, &cbReturned)
              && cbReturned == sizeof(int))
            {
              if (*((int *)pData) == WTSActive)

                if (!WTSDisconnectSession(WTS_CURRENT_SERVER_HANDLE,
                                          WTS_CURRENT_SESSION,
                                          FALSE))
                  exitStatus = false;
            }
          break;
#endif
        default:
          if (!ExitWindowsEx(mode, reason))
            exitStatus = false;
        }
    }

  if (elevate)
    {
      /* Release the privileges necessary to shutdown the computer */
      tkp.Privileges[0].Attributes = 0;
      AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);
      CloseHandle(hToken);
    }

  if (!exitStatus)
    {
      swprintf(messageText, TEXT("Failed to %s."), method);
      ELMessageBox(GetDesktopWindow(), messageText, (WCHAR*)TEXT("Emerge Desktop"),
                   ELMB_ICONERROR|ELMB_OK|ELMB_MODAL);
      return false;
    }

  return true;
}

/*!
  @fn ELCheckWindow(HWND hwnd)
  @brief Check to seee if the window is a valid 'task'
  @param hwnd window handle
  */

bool ELCheckWindow(HWND hwnd)
{
  // If the window is hidden, a toolwindow or has no title, ignore it
  if ((IsWindowVisible(hwnd)) &&
      !(GetWindowLongPtr(hwnd, GWL_STYLE) & WS_POPUP) &&
      !(GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW))
    return true;

  return false;
}

bool ELGetTempFileName(WCHAR *fileName)
{
  WCHAR tmpPath[MAX_PATH], tmpFile[MAX_PATH];

  if (GetTempPath(MAX_PATH, tmpPath) != 0)
    {
      if (GetTempFileName(tmpPath, TEXT("eds"), 0, tmpFile) != 0)
        {
          wcsncpy(fileName, tmpFile, MAX_PATH);
          return true;
        }
    }

  return false;
}

bool WriteValue(const WCHAR *fileName, WCHAR *keyword, WCHAR *value)
{
  WCHAR fileLine[MAX_LINE_LENGTH], *token, *tokenLine;
  WCHAR tmpFile[MAX_PATH];
  bool written = false;

  if (!ELGetTempFileName(tmpFile))
    return false;

  FILE *tmpFP = _wfopen(tmpFile, TEXT("w"));
  FILE *existingFP = _wfopen(fileName, TEXT("r"));

  if (!tmpFP)
    return written;

  if (existingFP != NULL)
    {
      while (fgetws(fileLine, MAX_LINE_LENGTH, existingFP))
        {
          tokenLine = _wcsdup(fileLine);
          token = wcstok(tokenLine, TEXT("\t ="));

          if (_wcsicmp(token, keyword) == 0)
            {
              swprintf(fileLine, TEXT("%s\t%s\n"), keyword, value);
              written = true;
            }

          free(tokenLine);
          fputws(fileLine, tmpFP);
        }
    }

  if (!written)
    {
      swprintf(fileLine, TEXT("%s\t%s\n"), keyword, value);
      fputws(fileLine, tmpFP);
      written = true;
    }


  fclose(tmpFP);
  if (existingFP != NULL)
    fclose(existingFP);

  CopyFile(tmpFile, fileName, FALSE);
  DeleteFile(tmpFile);

  return written;
}

bool ELWriteFileInt(const WCHAR *fileName, WCHAR *keyword, int value)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  swprintf(tmp, TEXT("%d"), value);

  return WriteValue(fileName, keyword, tmp);
}

bool ELWriteFileString(const WCHAR *fileName, WCHAR *keyword, WCHAR *value)
{
  return WriteValue(fileName, keyword, value);
}

bool ELWriteFileBool(const WCHAR *fileName, WCHAR *keyword, bool value)
{
  WCHAR tmp[MAX_LINE_LENGTH];

  if (value)
    wcscpy(tmp, TEXT("true"));
  else
    wcscpy(tmp, TEXT("false"));

  return WriteValue(fileName, keyword, tmp);
}

bool ELWriteFileColor(const WCHAR *fileName, WCHAR *keyword, COLORREF value)
{
  WCHAR tmp[MAX_LINE_LENGTH];

  swprintf(tmp, TEXT("%d,%d,%d"), GetRValue(value), GetGValue(value), GetBValue(value));

  return WriteValue(fileName, keyword, tmp);
}

/*!
  @fn ReadValue(WCHAR *fileName, WCHAR *keyword, WCHAR *value)
  @brief Read a variable from a file
  @param fileName Filename to search for the variable
  @param keyword Variable to search for
  @param value String that contains the keyword's value
  @return true if successful
  */

bool ReadValue(const WCHAR *fileName, WCHAR *keyword, WCHAR *value)
{
  WCHAR fileLine[MAX_LINE_LENGTH], *token, tmp[MAX_LINE_LENGTH];
  FILE *file = _wfopen(fileName, TEXT("r"));
  bool found = false;
  UINT i = 0, j = 0;

  ZeroMemory(value, MAX_LINE_LENGTH);

  if (!file)
    return found;

  while (fgetws(fileLine, MAX_LINE_LENGTH, file))
    {
      token = wcstok(fileLine, TEXT("\t ="));

      if (_wcsicmp(token, keyword) == 0)
        {
          token = wcstok(NULL, TEXT("\n\0"));

          wcscpy(tmp, token);

          while (i < wcslen(tmp))
            {
              if (found)
                {
                  value[j] = tmp[i];
                  j++;
                }

              if (!found && isalnum((int)tmp[i]))
                {
                  found = true;
                  value[j] = tmp[i];
                  j++;
                }

              i++;
            }

          break;
        }
    }

  fclose(file);

  return found;
}

/*!
  @fn ELReadFileInt(WCHAR *fileName, WCHAR *item, int *target, int notFound)
  @brief Read a variable to retrieve an integer, returning the default if not found
  @param fileName Filename to search for the variable
  @param item Variable to search for
  @param target Integer that is populated by the function
  @param notFound Integer to use if the variable is not found
  @return true if successful
  */

bool ELReadFileInt(const WCHAR *fileName, WCHAR *item, int *target, int notFound)
{
  WCHAR value[MAX_LINE_LENGTH];
  bool found = false;

  if (ReadValue(fileName, item, value))
    {
      found = true;

      for (UINT i = 0; i < wcslen(value); i++)
        if (isalpha(value[i]) != 0)
          {
            found = false;
            break;
          }
    }

  if (found)
    (*target) = _wtoi(value);
  else
    (*target) = notFound;

  return found;
}

bool ELReadFileByte(const WCHAR *fileName, WCHAR *item, BYTE *target, BYTE notFound)
{
  WCHAR value[MAX_LINE_LENGTH];
  bool found = false;

  if (ReadValue(fileName, item, value))
    {
      found = true;

      for (UINT i = 0; i < wcslen(value); i++)
        if (isalpha(value[i]) != 0)
          {
            found = false;
            break;
          }
    }

  if (found)
    target[0] = _wtoi(value);
  else
    (*target) = notFound;

  return found;
}

/*!
  @fn ELReadFileString(WCHAR *fileName, WCHAR *item, WCHAR *target, WCHAR *notFound)
  @brief Read a variable to retrieve a string, returning the default if not found
  @param fileName Filename to search for the variable
  @param item Variable to search for
  @param target String that is populated by the function
  @param notFound String to use if the variable is not found
  @return true if successful
  */

bool ELReadFileString(const WCHAR *fileName, WCHAR *item, WCHAR *target, WCHAR *notFound)
{
  bool found = false;
  WCHAR value[MAX_LINE_LENGTH];

  if (ReadValue(fileName, item, value))
    found = true;

  if (found)
    wcscpy(target, value);
  else
    wcscpy(target, notFound);

  return found;
}

/*!
  @fn ELReadFileBool(WCHAR *fileName, WCHAR *item, bool *target, bool notFound)
  @brief Read a variable to retrieve a bool, returning the default if not found
  @param fileName Filename to search for the variable
  @param item Variable to search for
  @param target bool that is populated by the function
  @param notFound bool to use if the variable is not found
  @return true if successful
  */

bool ELReadFileBool(const WCHAR *fileName, WCHAR *item, bool *target, bool notFound)
{
  WCHAR value[MAX_LINE_LENGTH];
  bool found = false;

  if (ReadValue(fileName, item, value))
    {
      if (_wcsicmp(value, TEXT("true")) == 0)
        {
          (*target) = true;
          found = true;
        }
      else if (_wcsicmp(value, TEXT("false")) == 0)
        {
          (*target) = false;
          found = true;
        }
    }

  if (!found)
    (*target) = notFound;

  return found;
}

/*!
  @fn ELReadFileColor(WCHAR *fileName, WCHAR *item, COLORREF *target, COLORREF notFound)
  @brief Read a variable to retrieve a colour, returning the default if not found
  @param fileName Filename to search for the variable
  @param item Variable to search for
  @param target COLORREF that is populated by the function
  @param notFound COLORREF to use if the variable is not found
  @return true if successful
  */

bool ELReadFileColor(const WCHAR *fileName, WCHAR *item, COLORREF *target, COLORREF notFound)
{
  WCHAR value[MAX_LINE_LENGTH];
  int red = 0, green = 0, blue = 0;
  bool found = false;

  if (ReadValue(fileName, item, value))
    {
      if (swscanf(value, TEXT("%d,%d,%d"), &red, &green, &blue) == 3)
        {
          if ((red >= 0 && red <= 255) &&
              (green >= 0 && green <= 255) &&
              (blue >= 0 && blue <= 255))
            {
              found = true;
              (*target) = RGB(red, green, blue);
            }
        }
    }

  if (!found)
    (*target) = notFound;

  return found;
}

/*!
  @fn ELVersionInfo()
  @brief Determines the version of the Operating System
  @return Version of Operating System

  @note Windows 7	              6.1
  Windows Server 2008 R2  6.1
  Windows Server 2008	    6.0
  Windows Vista	          6.0
  Windows Server 2003 R2	5.2
  Windows Server 2003	    5.2
  Windows XP	            5.1
  Windows 2000	          5.0
  */

float ELVersionInfo()
{
  OSVERSIONINFO osv;
  ZeroMemory(&osv, sizeof(osv));
  osv.dwOSVersionInfoSize = sizeof(osv);
  GetVersionEx(&osv);

  float fMajor = (float)osv.dwMajorVersion;
  float fMinor = (float)osv.dwMinorVersion / 10.0;

  return (fMajor + fMinor);
}

/*!
  @fn ELAppletVersionInfo(HWND appWnd, LPVERSIONINFO versionInfo)
  @brief Populates a VERSIONINFO structure based on the window handle supplied
  @param appWnd Window handle
  @param versionInfo VERSIONINFO structure to populate
  @return true if successful
  */

bool ELAppletVersionInfo(HWND appWnd, LPVERSIONINFO versionInfo)
{
  WCHAR applet[MAX_LINE_LENGTH];

  if (!ELGetWindowApp(appWnd, applet, false))
    return false;

  return ELAppletFileVersion(applet, versionInfo);
}

/*!
  @fn ELAppletFileVersion(WCHAR *applet, LPVERSIONINFO versionInfo)
  @brief Populates a VERSIONINFO structure based on the filename supplied
  @param applet File to open
  @param versionInfo VERSIONINFO structure to populate
  @return true if successful
  */

bool ELAppletFileVersion(WCHAR *applet, LPVERSIONINFO versionInfo)
{
  WCHAR tmp[MAX_LINE_LENGTH], var[MAX_LINE_LENGTH];
  void *buffer, *data;
  DWORD notUsed, dataSize;
  UINT bufferSize;
  VS_FIXEDFILEINFO *fixedFileInfo;
  std::wstring workingApplet = applet;
  workingApplet = ELExpandVars(workingApplet);

  dataSize = GetFileVersionInfoSize(workingApplet.c_str(), &notUsed);
  if (dataSize == 0)
    return false;

  data = (void *)malloc(dataSize + 1);
  if (!data)
    return false;

  if (GetFileVersionInfo(workingApplet.c_str(), notUsed, dataSize, data))
    {
      if (VerQueryValue(data, (WCHAR*)TEXT("\\VarFileInfo\\Translation"), &buffer, &bufferSize))
        swprintf(tmp, TEXT("\\StringFileInfo\\%04x%04x\\"), LOWORD(*((UINT*)buffer)),
                 HIWORD(*((UINT*)buffer)));
      else
        {
          free(data);
          return false;
        }

      wcscpy(var, tmp);
      wcscat(var, TEXT("FileDescription"));
      if (VerQueryValue(data, var, &buffer, &bufferSize))
        wcscpy(versionInfo->Description, (WCHAR *)buffer);
      else
        {
          free(data);
          return false;
        }

      if (VerQueryValue(data, (WCHAR*)TEXT("\\"), &buffer, &bufferSize))
        {
          fixedFileInfo = (VS_FIXEDFILEINFO*)buffer;
          swprintf(versionInfo->Version, TEXT("%d.%d.%d.%d"),
                   HIWORD(fixedFileInfo->dwFileVersionMS), LOWORD(fixedFileInfo->dwFileVersionMS),
                   HIWORD(fixedFileInfo->dwFileVersionLS), LOWORD(fixedFileInfo->dwFileVersionLS));
        }
      else
        {
          free(data);
          return false;
        }

      wcscpy(var, tmp);
      wcscat(var, TEXT("CompanyName"));
      if (VerQueryValue(data, var, &buffer, &bufferSize))
        wcscpy(versionInfo->Author, (WCHAR *)buffer);
      else
        {
          free(data);
          return false;
        }

      free(data);
      return true;
    }

  free(data);
  return false;
}

/*!
  @fn GetWindowApp(DWORD processID, WCHAR *processName, bool fullName)
  @brief Determines the process name based on the supplied process ID
  @param processID Process ID
  @param fullName if true return fully qualified name, if false return basename
  @return applet name if successful
  */

std::wstring ELGetProcessIDApp(DWORD processID, bool fullName)
{
  DWORD needed;
  HANDLE hProcess;
  HMODULE hMod;
  WCHAR tmp[MAX_PATH];
  std::wstring applet = TEXT("");

  ZeroMemory(&tmp, MAX_PATH);

  // Get a handle to the process.
  hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |  PROCESS_VM_READ,  FALSE, processID);

  // Get the process name.
  if (NULL != hProcess )
    {
      if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &needed))
        {
          if (fullName)
            GetModuleFileNameEx(hProcess, hMod, tmp, sizeof(tmp));
          else
            GetModuleBaseName(hProcess, hMod, tmp, sizeof(tmp));
        }

      // Print the process name and identifier.
      applet = tmp;

      CloseHandle(hProcess);
    }

  return applet;
}

/*!
  @fn ELGetWindowApp(HWND hWnd, WCHAR *processName, bool fullName)
  @brief Determines the process name based on the supplied window handle
  @param hWnd Window handle
  @param processName Populated with process owning the window handle
  @param fullName if true return fully qualified name, if false return basename
  @return true if successful
  */

bool ELGetWindowApp(HWND hWnd, WCHAR *processName, bool fullName)
{
  DWORD processID;
  std::wstring tmpName;

  GetWindowThreadProcessId(hWnd, &processID);
  tmpName = ELGetProcessIDApp(processID, fullName);

  if (tmpName.size() != 0)
    {
      wcscpy(processName, tmpName.c_str());
      return true;
    }

  return false;
}

/*!
  @fn ELIsInternalCommand(WCHAR *command)
  @brief Determines the internal command based on the string passed
  @param command string to perform check on
  */

UINT ELIsInternalCommand(WCHAR *command)
{
  if (_wcsicmp(command, TEXT("run")) == 0)
    return COMMAND_RUN;

  if (_wcsicmp(command, TEXT("quit")) == 0)
    return COMMAND_QUIT;

  if (_wcsicmp(command, TEXT("shutdown")) == 0)
    return COMMAND_SHUTDOWN;

  if (_wcsicmp(command, TEXT("logoff")) == 0)
    return COMMAND_LOGOFF;

  if (_wcsicmp(command, TEXT("lock")) == 0)
    return COMMAND_LOCK;

  return 0;
}

int ELSpecialFolderID(WCHAR *folder)
{
  int csidl = 0;

  if (_wcsicmp(folder, TEXT("CSIDL_PERSONAL")) == 0)
    csidl = CSIDL_PERSONAL;
  else if (_wcsicmp(folder, TEXT("CSIDL_DRIVES")) == 0)
    csidl = CSIDL_DRIVES;
  else if (_wcsicmp(folder, TEXT("CSIDL_CONTROLS")) == 0)
    csidl = CSIDL_CONTROLS;
  else if (_wcsicmp(folder, TEXT("CSIDL_BITBUCKET")) == 0)
    csidl = CSIDL_BITBUCKET;
  else if (_wcsicmp(folder, TEXT("CSIDL_NETWORK")) == 0)
    csidl = CSIDL_NETWORK;

  return csidl;
}

bool ELSpecialFolderValue(WCHAR *folder, WCHAR *value)
{
  LPITEMIDLIST pidl = NULL;
  SHFILEINFO fileInfo;
  bool ret = false;

  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl)))
    {
      SHGetFileInfo((LPCTSTR)pidl, 0, &fileInfo, sizeof(fileInfo), SHGFI_PIDL | SHGFI_DISPLAYNAME);
      if (_wcsicmp(folder, fileInfo.szDisplayName) == 0)
        {
          wcscpy(value, TEXT("CSIDL_PERSONAL"));
          ret = true;
        }
      ELILFree(pidl);
    }

  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidl)))
    {
      SHGetFileInfo((LPCTSTR)pidl, 0, &fileInfo, sizeof(fileInfo), SHGFI_PIDL | SHGFI_DISPLAYNAME);
      if (_wcsicmp(folder, fileInfo.szDisplayName) == 0)
        {
          wcscpy(value, TEXT("CSIDL_DRIVES"));
          ret = true;
        }
      ELILFree(pidl);
    }

  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_CONTROLS, &pidl)))
    {
      SHGetFileInfo((LPCTSTR)pidl, 0, &fileInfo, sizeof(fileInfo), SHGFI_PIDL | SHGFI_DISPLAYNAME);
      if (_wcsicmp(folder, fileInfo.szDisplayName) == 0)
        {
          wcscpy(value, TEXT("CSIDL_CONTROLS"));
          ret = true;
        }
      ELILFree(pidl);
    }

  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_BITBUCKET, &pidl)))
    {
      SHGetFileInfo((LPCTSTR)pidl, 0, &fileInfo, sizeof(fileInfo), SHGFI_PIDL | SHGFI_DISPLAYNAME);
      if (_wcsicmp(folder, fileInfo.szDisplayName) == 0)
        {
          wcscpy(value, TEXT("CSIDL_BITBUCKET"));
          ret = true;
        }
      ELILFree(pidl);
    }

  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_NETWORK, &pidl)))
    {
      SHGetFileInfo((LPCTSTR)pidl, 0, &fileInfo, sizeof(fileInfo), SHGFI_PIDL | SHGFI_DISPLAYNAME);
      if (_wcsicmp(folder, fileInfo.szDisplayName) == 0)
        {
          wcscpy(value, TEXT("CSIDL_NETWORK"));
          ret = true;
        }
      ELILFree(pidl);
    }

  return ret;
}

/*!
  @fn ELIsSpecialFolder(WCHAR *folder)
  @brief Determines the special folder based on the string passed
  @param folder string to perform check on
  */

int ELIsSpecialFolder(WCHAR *folder)
{
  LPITEMIDLIST pidl = NULL;
  SHFILEINFO fileInfo;
  int csidl = 0;

  csidl = ELSpecialFolderID(folder);

  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl)))
    {
      SHGetFileInfo((LPCTSTR)pidl, 0, &fileInfo, sizeof(fileInfo), SHGFI_PIDL | SHGFI_DISPLAYNAME);
      if (_wcsicmp(folder, fileInfo.szDisplayName) == 0)
        csidl = CSIDL_PERSONAL;
      ELILFree(pidl);
    }

  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidl)))
    {
      SHGetFileInfo((LPCTSTR)pidl, 0, &fileInfo, sizeof(fileInfo), SHGFI_PIDL | SHGFI_DISPLAYNAME);
      if (_wcsicmp(folder, fileInfo.szDisplayName) == 0)
        csidl = CSIDL_DRIVES;
      ELILFree(pidl);
    }

  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_CONTROLS, &pidl)))
    {
      SHGetFileInfo((LPCTSTR)pidl, 0, &fileInfo, sizeof(fileInfo), SHGFI_PIDL | SHGFI_DISPLAYNAME);
      if (_wcsicmp(folder, fileInfo.szDisplayName) == 0)
        csidl = CSIDL_CONTROLS;
      ELILFree(pidl);
    }

  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_BITBUCKET, &pidl)))
    {
      SHGetFileInfo((LPCTSTR)pidl, 0, &fileInfo, sizeof(fileInfo), SHGFI_PIDL | SHGFI_DISPLAYNAME);
      if (_wcsicmp(folder, fileInfo.szDisplayName) == 0)
        csidl = CSIDL_BITBUCKET;
      ELILFree(pidl);
    }

  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_NETWORK, &pidl)))
    {
      SHGetFileInfo((LPCTSTR)pidl, 0, &fileInfo, sizeof(fileInfo), SHGFI_PIDL | SHGFI_DISPLAYNAME);
      if (_wcsicmp(folder, fileInfo.szDisplayName) == 0)
        csidl = CSIDL_NETWORK;
      ELILFree(pidl);
    }

  return csidl;
}

/*!
  @fn ELGetSpecialFolder(UINT folder)
  @brief Returns the path to special Windows folders
  @param folder special folder to return path for
  */

bool ELGetSpecialFolder(int folder, WCHAR *folderPath)
{
  LPITEMIDLIST pidl;
  SHFILEINFO fileInfo;

  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, folder, &pidl)))
    {
      SHGetFileInfo((LPCTSTR)pidl, 0, &fileInfo, sizeof(fileInfo), SHGFI_PIDL | SHGFI_DISPLAYNAME);
      wcscpy(folderPath, fileInfo.szDisplayName);
      ELILFree(pidl);
      return true;
    }

  return false;
}

/*!
  @fn ELMid(int left, int right)
  @brief Returns the mid point between two sides.
  @param left value of one side
  @param right value of other side
  */

int ELMid(int left, int right)
{
  int middle;

  if (right > 0)
    middle = (left - right) / 2;
  else
    middle = (left + right) / 2;

  return middle;
}

bool ELStealFocus(HWND hwnd)
{
  DWORD threadID1, threadID2;
  bool result = false;

  threadID1 = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
  threadID2 = GetWindowThreadProcessId(hwnd, NULL);

  if (threadID1 != threadID2)
    {
      AttachThreadInput(threadID1, threadID2, TRUE);
      result = ((SetFocus(hwnd) != NULL) && SetForegroundWindow(hwnd));
      AttachThreadInput(threadID1, threadID2, FALSE);
    }
  else
    result = ((SetFocus(hwnd) != NULL) && SetForegroundWindow(hwnd));

  return result;
}

bool ELAdjustVolume(UINT command)
{
  bool ret = false;
  if (ELVersionInfo() >= 6.0)
    ret = VistaVolumeControl(command);
  else
    ret = VolumeControl(command);

  return ret;
}

bool VolumeControl(UINT command)
{
  HMIXER hMixer;
  MIXERLINE mxl;
  MIXERLINECONTROLS mxlc;
  MIXERCONTROL mxc;
  MIXERCONTROLDETAILS mxcd;
  MIXERCONTROLDETAILS_UNSIGNED vol;
  MIXERCONTROLDETAILS_BOOLEAN mute;
  bool ret = false;
  DWORD volDelta = (DWORD)(0xffff * 1.0 * 5.0 / 100.0);

  ZeroMemory(&mxl, sizeof(MIXERLINE));
  ZeroMemory(&mxlc, sizeof(MIXERLINECONTROLS));
  ZeroMemory(&mxc, sizeof(MIXERCONTROL));
  ZeroMemory(&mxcd, sizeof(MIXERCONTROLDETAILS));

  mxl.cbStruct = sizeof(MIXERLINE);
  mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

  if (mixerOpen(&hMixer, 0, 0, 0, 0) == MMSYSERR_NOERROR)
    {
      if (mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE) == MMSYSERR_NOERROR)
        {
          mxc.cbStruct = sizeof(mxc);

          mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
          mxlc.dwLineID = mxl.dwLineID;
          if (command == ELAV_MUTE)
            mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
          else
            mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
          mxlc.cControls = 1;
          mxlc.cbmxctrl = sizeof(MIXERCONTROL);
          mxlc.pamxctrl = &mxc;

          if (mixerGetLineControls((HMIXEROBJ)hMixer, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE) == MMSYSERR_NOERROR)
            {
              mxcd.dwControlID = mxc.dwControlID;
              mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
              if (command == ELAV_MUTE)
                {
                  mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
                  mxcd.paDetails = &mute;
                }
              else
                {
                  mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
                  mxcd.paDetails = &vol;
                }
              mxcd.cChannels = 1;

              if (mixerGetControlDetails((HMIXEROBJ)hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE) == MMSYSERR_NOERROR)
                {
                  if (command == ELAV_MUTE)
                    mute.fValue = !mute.fValue;
                  else if (command == ELAV_VOLUMEUP)
                    vol.dwValue += volDelta;
                  else
                    {
                      if (vol.dwValue - volDelta >= 0xffff)
                        vol.dwValue = 0;
                      else
                        vol.dwValue -= volDelta;
                    }

                  if (mixerSetControlDetails((HMIXEROBJ)hMixer, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE) == MMSYSERR_NOERROR)
                    ret = true;
                }
            }
        }
    }

  return ret;
}

bool VistaVolumeControl(UINT command)
{
  IMMDeviceEnumerator *deviceEnumerator = NULL;
  IMMDevice *defaultDevice = NULL;
  IAudioEndpointVolume *endpointVolume = NULL;
  LPVOID lpVoid = NULL;
  bool ret = false;
  BOOL mute;
  float volume;

  if (SUCCEEDED(CoInitialize(NULL)))
    {
      if (SUCCEEDED(CoCreateInstance(_uuidof(_MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, _uuidof(_IMMDeviceEnumerator),
                                     &lpVoid)))
        {
          deviceEnumerator = (IMMDeviceEnumerator*)lpVoid;
          if (SUCCEEDED(deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice)))
            {
              if (SUCCEEDED(defaultDevice->Activate(_uuidof(_IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL,
                                                    &lpVoid)))
                {
                  endpointVolume = (IAudioEndpointVolume*)lpVoid;
                  if (command == ELAV_MUTE)
                    {
                      endpointVolume->GetMute(&mute);
                      endpointVolume->SetMute(!mute, NULL);
                    }
                  else
                    {
                      endpointVolume->GetMasterVolumeLevelScalar(&volume);
                      double volumeDelta = 0.05;

                      if (command == ELAV_VOLUMEDOWN)
                        {
                          volume -= (float)volumeDelta;
                          if (volume < 0.0)
                            volume = 0.0;
                        }
                      else if (command == ELAV_VOLUMEUP)
                        {
                          volume += (float)volumeDelta;
                          if (volume > 1.0)
                            volume = 1.0;
                        }

                      endpointVolume->SetMasterVolumeLevelScalar(volume, NULL);
                    }

                  endpointVolume->Release();
                  ret = true;
                }

              defaultDevice->Release();
            }

          deviceEnumerator->Release();
        }

      CoUninitialize();
    }

  return ret;
}

//bool ELControlBrowser(UINT command)
//{
//  HWND browser = FindWindow(TEXT("Internet Explorer_Server"), NULL);

//  SendMessage(browser, WM_APPCOMMAND, (WPARAM)browser, MAKELPARAM(0, (FAPPCOMMAND_KEY | APPCOMMAND_BROWSER_BACKWARD)));

/*  INPUT inputs[1];

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_BROWSER_BACK;
    inputs[0].ki.dwFlags = 0;
    inputs[0].ki.dwExtraInfo = 0;
    inputs[0].ki.time = 0;
    inputs[0].ki.wScan = 0;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_BROWSER_BACK;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[1].ki.dwExtraInfo = 0;
    inputs[1].ki.time = 0;
    inputs[1].ki.wScan = 0;

    if (SendInput(2, inputs, sizeof(INPUT)) == 2)
    {
//      ELWriteDebug((WCHAR*)TEXT("Sent input"));
return true;
}*/

//  keybd_event(VK_BROWSER_BACK, 0, KEYEVENTF_EXTENDEDKEY, 0);
//  keybd_event(VK_BROWSER_BACK, 0, KEYEVENTF_KEYUP, 0);

//  return true;
//}

std::wstring ELToLower(std::wstring workingString)
{
  std::transform(workingString.begin(), workingString.end(), workingString.begin(), (int(*)(int)) std::tolower);
  return workingString;
}

std::wstring ELwstringReplace(std::wstring original, std::wstring pattern, std::wstring replacement, bool ignoreCase)
{
  std::wstring lowerOrig = original, lowerPat = pattern;
  size_t i = 0;

  if (ignoreCase)
    {
      std::transform(lowerOrig.begin(), lowerOrig.end(), lowerOrig.begin(), (int(*)(int)) std::tolower);
      std::transform(lowerPat.begin(), lowerPat.end(), lowerPat.begin(), (int(*)(int)) std::tolower);
    }

  i = lowerOrig.find(lowerPat, i);
  while (i != std::wstring::npos)
    {
      original.replace(i, pattern.length(), replacement);
      lowerOrig.replace(i, lowerPat.length(), replacement);
      i += replacement.length();
      i = lowerOrig.find(lowerPat, i);
    }

  return original;
}

UINT ELStringReplace(WCHAR *original, const WCHAR *pattern, const WCHAR *replacement, bool ignoreCase)
{
  std::wstring workingOrig = original, workingRepl = replacement;
  std::wstring lowerOrig = original, lowerPat = pattern;
  size_t i = 0;
  UINT substitutions = 0;

  if (ignoreCase)
    {
      std::transform(lowerOrig.begin(), lowerOrig.end(), lowerOrig.begin(), (int(*)(int)) std::tolower);
      std::transform(lowerPat.begin(), lowerPat.end(), lowerPat.begin(), (int(*)(int)) std::tolower);
    }

  i = lowerOrig.find(lowerPat, i);
  while (i != std::wstring::npos)
    {
      workingOrig.replace(i, lowerPat.length(), workingRepl);
      lowerOrig.replace(i, lowerPat.length(), workingRepl);
      i += workingRepl.length();
      i = lowerOrig.find(lowerPat, i);
      substitutions++;
    }

  wcscpy(original, workingOrig.c_str());

  return substitutions;
}

BOOL ELIsWow64()
{
  BOOL bIsWow64 = FALSE;

  if (MSIsWow64Process == NULL)
    MSIsWow64Process = (lpfnIsWow64Process)GetProcAddress(kernel32DLL, "IsWow64Process");
  if (MSIsWow64Process)
    MSIsWow64Process(GetCurrentProcess(), &bIsWow64);

  return bIsWow64;
}

void ELClearEmergeVars()
{
  SetEnvironmentVariable(TEXT("StartMenu"), NULL);
  SetEnvironmentVariable(TEXT("CommonStartMenu"), NULL);
  SetEnvironmentVariable(TEXT("Desktop"), NULL);
  SetEnvironmentVariable(TEXT("CommonDesktop"), NULL);
  SetEnvironmentVariable(TEXT("Documents"), NULL);
  SetEnvironmentVariable(TEXT("CommonDocuments"), NULL);
  SetEnvironmentVariable(TEXT("ThemeDir"), NULL);
  SetEnvironmentVariable(TEXT("EmergeDir"), NULL);
  SetEnvironmentVariable(TEXT("AppletDir"), NULL);
}

bool ELSetEmergeVars()
{
  WCHAR appletPath[MAX_PATH];

  THEMEINFO themeInfo;
  ELGetThemeInfo(&themeInfo);
  ELGetCurrentPath(appletPath);

  if (!SetEnvironmentVariable(TEXT("ThemeDir"), themeInfo.themePath))
    return false;

  if (!SetEnvironmentVariable(TEXT("EmergeDir"), themeInfo.path))
    return false;

  if (!SetEnvironmentVariable(TEXT("AppletDir"), appletPath))
    return false;

  return true;
}

void ELSetEnvironmentVars(bool showErrors)
{
  WCHAR tmp[MAX_LINE_LENGTH];

  bool localCheck = true;
  if (SHGetSpecialFolderPath(GetDesktopWindow(), tmp, CSIDL_STARTMENU, FALSE))
    {
      if (!SetEnvironmentVariable(TEXT("StartMenu"), tmp))
        localCheck = false;
    }
  else
    localCheck = false;
  if (showErrors && !localCheck)
    ELMessageBox(GetDesktopWindow(), TEXT("Failed to set %%StartMenu%%"),
                 TEXT("Emerge Desktop"), ELMB_OK|ELMB_ICONERROR);


  localCheck = true;
  if (SHGetSpecialFolderPath(GetDesktopWindow(), tmp, CSIDL_COMMON_STARTMENU, FALSE))
    {
      if (!SetEnvironmentVariable(TEXT("CommonStartMenu"), tmp))
        localCheck = false;
    }
  else
    localCheck = false;
  if (showErrors && !localCheck)
    ELMessageBox(GetDesktopWindow(), TEXT("Failed to set %%CommonStartMenu%%"),
                 TEXT("Emerge Desktop"), ELMB_OK|ELMB_ICONERROR);

  localCheck = true;
  if (SHGetSpecialFolderPath(GetDesktopWindow(), tmp, CSIDL_DESKTOPDIRECTORY, FALSE))
    {
      if (!SetEnvironmentVariable(TEXT("Desktop"), tmp))
        localCheck = false;
    }
  else
    localCheck = false;
  if (showErrors && !localCheck)
    ELMessageBox(GetDesktopWindow(), TEXT("Failed to set %%Desktop%%"),
                 TEXT("Emerge Desktop"), ELMB_OK|ELMB_ICONERROR);

  localCheck = true;
  if (SHGetSpecialFolderPath(GetDesktopWindow(), tmp, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE))
    {
      if (!SetEnvironmentVariable(TEXT("CommonDesktop"), tmp))
        localCheck = false;
    }
  else
    localCheck = false;
  if (showErrors && !localCheck)
    ELMessageBox(GetDesktopWindow(), TEXT("Failed to set %%CommonDesktop%%"),
                 TEXT("Emerge Desktop"), ELMB_OK|ELMB_ICONERROR);

  localCheck = true;
  if (SHGetSpecialFolderPath(GetDesktopWindow(), tmp, CSIDL_PERSONAL, FALSE))
    {
      if (!SetEnvironmentVariable(TEXT("Documents"), tmp))
        localCheck = false;
    }
  else
    localCheck = false;
  if (showErrors && !localCheck)
    ELMessageBox(GetDesktopWindow(), TEXT("Failed to set %%Documents%%"),
                 TEXT("Emerge Desktop"), ELMB_OK|ELMB_ICONERROR);

  localCheck = true;
  if (SHGetSpecialFolderPath(GetDesktopWindow(), tmp, CSIDL_COMMON_DOCUMENTS, FALSE))
    {
      if (!SetEnvironmentVariable(TEXT("CommonDocuments"), tmp))
        localCheck = false;
    }
  else
    localCheck = false;
  if (showErrors && !localCheck)
    ELMessageBox(GetDesktopWindow(), TEXT("Failed to set %%CommonDocuments%%"),
                 TEXT("Emerge Desktop"), ELMB_OK|ELMB_ICONERROR);
}

std::wstring ELExpandVars(std::wstring value)
{
  std::wstring expandedValue;
  WCHAR *tmpValue = NULL;
  size_t bufferSize;

  if (value.empty())
    return value;

  bufferSize = ExpandEnvironmentStrings(value.c_str(), tmpValue, 0);
  if (bufferSize == 0)
    return value;

  // Account for terminating NULL
  bufferSize++;

  tmpValue = (WCHAR*)GlobalAlloc(GPTR, bufferSize * sizeof(WCHAR));
  if (ExpandEnvironmentStrings(value.c_str(), tmpValue, bufferSize) != 0)
    expandedValue = tmpValue;
  GlobalFree(tmpValue);

  return expandedValue;
}

bool ELUnExpandVars(LPTSTR value)
{
  bool ret = false;
  WCHAR tmp[MAX_LINE_LENGTH];

  if (GetEnvironmentVariable(TEXT("StartMenu"), tmp, MAX_LINE_LENGTH) != 0)
    {
      if (ELStringReplace(value, tmp, (WCHAR*)TEXT("%StartMenu%"), true) > 0)
        ret = true;
    }

  if (GetEnvironmentVariable(TEXT("CommonStartMenu"), tmp, MAX_LINE_LENGTH) != 0)
    {
      if (ELStringReplace(value, tmp, (WCHAR*)TEXT("%CommonStartMenu%"), true) > 0)
        ret = true;
    }

  if (GetEnvironmentVariable(TEXT("Desktop"), tmp, MAX_LINE_LENGTH) != 0)
    {
      if (ELStringReplace(value, tmp, (WCHAR*)TEXT("%Desktop%"), true) > 0)
        ret = true;
    }

  if (GetEnvironmentVariable(TEXT("CommonDesktop"), tmp, MAX_LINE_LENGTH) != 0)
    {
      if (ELStringReplace(value, tmp, (WCHAR*)TEXT("%CommonDesktop%"), true) > 0)
        ret = true;
    }

  if (GetEnvironmentVariable(TEXT("Documents"), tmp, MAX_LINE_LENGTH) != 0)
    {
      if (ELStringReplace(value, tmp, (WCHAR*)TEXT("%Documents%"), true) > 0)
        ret = true;
    }

  if (GetEnvironmentVariable(TEXT("CommonDocuments"), tmp, MAX_LINE_LENGTH) != 0)
    {
      if (ELStringReplace(value, tmp, (WCHAR*)TEXT("%CommonDocuments%"), true) > 0)
        ret = true;
    }

  if (GetEnvironmentVariable(TEXT("ThemeDir"), tmp, MAX_LINE_LENGTH) != 0)
    {
      if (ELStringReplace(value, tmp, (WCHAR*)TEXT("%ThemeDir%"), true) > 0)
        ret = true;
    }

  // Resolve AppletDir before EmergeDir to handle the situation where both AppletDir
  // and EmergeDir refer to the same directory, as it can affect theme saving.
  if (GetEnvironmentVariable(TEXT("AppletDir"), tmp, MAX_LINE_LENGTH) != 0)
    {
      if (ELStringReplace(value, tmp, (WCHAR*)TEXT("%AppletDir%"), true) > 0)
        ret = true;
    }

  if (GetEnvironmentVariable(TEXT("EmergeDir"), tmp, MAX_LINE_LENGTH) != 0)
    {
      if (ELStringReplace(value, tmp, (WCHAR*)TEXT("%EmergeDir%"), true) > 0)
        ret = true;
    }

  if (!ret)
    {
      if (PathUnExpandEnvStrings(value, tmp, MAX_LINE_LENGTH))
        {
          wcscpy(value, tmp);
          ret = true;
        }
    }

  return ret;
}

bool ELIsModal(HWND window)
{
  HWND lastWnd = GetLastActivePopup(window);
  WCHAR windowClass[MAX_LINE_LENGTH];

  if (IsWindowVisible(lastWnd))
    {
      RealGetWindowClass(lastWnd, windowClass, MAX_LINE_LENGTH);
      if (_wcsicmp(windowClass, TEXT("#32770")) == 0)
        return true;
    }

  return false;
}

bool ELPlaySound(const WCHAR *sound)
{
  UINT soundFlags = SND_ALIAS | SND_ASYNC | SND_NODEFAULT;

  if (ELVersionInfo() >= 6.0)
    soundFlags |= SND_SYSTEM;

  return PlaySound(sound, NULL, soundFlags) == TRUE;
}

bool ELConvertUserPath(WCHAR *styleFile, DWORD flags)
{
  return ConvertPath(styleFile, flags, CP_USER);
}

bool ELConvertThemePath(WCHAR *styleFile, DWORD flags)
{
  return ConvertPath(styleFile, flags, CP_THEME);
}

bool ELConvertAppletPath(WCHAR *styleFile, DWORD flags)
{
  return ConvertPath(styleFile, flags, CP_APPLET);
}

bool ConvertPath(WCHAR *styleFile, DWORD flags, DWORD path)
{
  WCHAR tmpPath[MAX_PATH];
  UINT j = 0;
  bool converted = false;
  std::wstring themePath;

  if (wcslen(styleFile) == 0)
    return converted;

  if (path == CP_USER)
    themePath = TEXT("%EmergeDir%\\files\\");
  else if (path == CP_APPLET)
    themePath = TEXT("%AppletDir%\\");
  else
    themePath = TEXT("%ThemeDir%\\");

  switch (flags)
    {
    case CTP_FULL:
      if (ELPathIsRelative(styleFile))
        {
          themePath += styleFile;

          wcscpy(styleFile, themePath.c_str());
          converted = true;
        }
      break;
    case CTP_RELATIVE:
      if (!ELPathIsRelative(styleFile))
        {
          if (PathRelativePathTo(tmpPath, themePath.c_str(), FILE_ATTRIBUTE_DIRECTORY, styleFile, FILE_ATTRIBUTE_NORMAL))
            {
              // If the file is stored in the current directory, the PathRelativePathTo
              // prepends the string with '\' making windows think the file is in the root
              // directory, so I've implemented the change below to account for that.
              for (UINT i = 0; i < wcslen(tmpPath); i++)
                {
                  if ((i == 0) && (tmpPath[i] == '\\'))
                    continue;

                  styleFile[j] = tmpPath[i];
                  j++;
                }
              styleFile[j] = '\0';
              converted = true;
            }
        }
      break;
    }

  return converted;
}

RECT ELGetMonitorRect(int monitor)
{
  APPLETMONITORINFO appMonInfo;
  ZeroMemory(&appMonInfo, sizeof(appMonInfo));

  appMonInfo.appletMonitorNum = monitor;

  enumCount = 0;
  if (EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC)MonitorRectEnum, (LPARAM)&appMonInfo))
    {
      appMonInfo.appletMonitorRect.top = 0;
      appMonInfo.appletMonitorRect.left = 0;
      appMonInfo.appletMonitorRect.right = GetSystemMetrics(SM_CXFULLSCREEN);
      appMonInfo.appletMonitorRect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
    }

  return appMonInfo.appletMonitorRect;
}

BOOL CALLBACK MonitorRectEnum(HMONITOR hMonitor UNUSED, HDC hdcMonitor UNUSED, LPRECT lprcMonitor, LPARAM dwData)
{
  if (enumCount == ((LPAPPLETMONITORINFO)dwData)->appletMonitorNum)
    {
      CopyRect(&((LPAPPLETMONITORINFO)dwData)->appletMonitorRect, lprcMonitor);
      return FALSE;
    }

  enumCount++;

  return TRUE;
}

int ELGetAppletMonitor(HWND hwnd)
{
  APPLETMONITORINFO appMonInfo;
  ZeroMemory(&appMonInfo, sizeof(appMonInfo));

  appMonInfo.appletWnd = hwnd;
  appMonInfo.appletMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
  appMonInfo.appletMonitorNum = 0;

  EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC)AppletMonitorEnum, (LPARAM)&appMonInfo);

  return appMonInfo.appletMonitorNum;
}

BOOL CALLBACK AppletMonitorEnum(HMONITOR hMonitor, HDC hdcMonitor UNUSED, LPRECT lprcMonitor UNUSED, LPARAM dwData)
{
  if (((LPAPPLETMONITORINFO)dwData)->appletMonitor == hMonitor)
    return FALSE;

  ((LPAPPLETMONITORINFO)dwData)->appletMonitorNum++;

  return TRUE;
}

bool ELSetModifiedTheme(std::wstring theme)
{
  bool ret = false;
  std::wstring themePath = TEXT("%EmergeDir%\\themes\\");

  if (theme.empty())
    return ret;

  if (!ELIsModifiedTheme(theme))
    {
      // Generate the (Modified) theme name
      theme += TEXT(" (Modified)");
      themePath += theme;
      themePath = ELExpandVars(themePath);

      // Create the theme directory if it doesn't exist
      if (!PathIsDirectory(themePath.c_str()))
        ELCreateDirectory(themePath);

      // If the theme path is created, set it as the theme
      if (PathIsDirectory(themePath.c_str()))
        {
          ELSetTheme(themePath);
          ret = true;
        }
    }

  return ret;
}

/** \brief Augments wcsftime to support '%u' and '%V' as defined by glibc.
 *
 * \note ELwcsftime is based on the pseudo code found here: http://www.personal.ecu.edu/mccartyr/ISOwdALG.txt
 *
 * \param strDest - destination string
 * \param maxsize - the size of destination string
 * \param format - format string
 * \param timptr - pointer to the current time in a 'tm' struct
 * \return the new size of strDest
 *
 */
size_t ELwcsftime(WCHAR *strDest, size_t maxsize, WCHAR *format, const struct tm *timeptr)
{
  int year = timeptr->tm_year + 1900;
  int previousYear = year - 1;
  int month = timeptr->tm_mon + 1;
  int day = timeptr->tm_mday;
  int yy, c, g, jan1Weekday;
  int monthDay[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  int dayOfYear, weekday, h, yearNumber, weekNumber, i;
  bool isLeapYear = false, previousIsLeapYear = false;
  WCHAR stringDay[2], stringWeek[3];

  /**< Determine if the current year is a leap year */
  if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
    isLeapYear = true;

  /**< Determine if the previous year is a leap year */
  if (((previousYear % 4 == 0) && (previousYear % 100 != 0)) || (previousYear % 400 == 0))
    previousIsLeapYear = true;

  /**< Determine the dayOfYear for the current date */
  dayOfYear = day + monthDay[month - 1];
  if (isLeapYear && (month > 2))
    dayOfYear++;

  /**< Determine jan1Weekday for the current year (Monday = 1, Sunday = 7) */
  yy = previousYear % 100;
  c = previousYear - yy;
  g = yy + (yy / 4);
  jan1Weekday = 1 + (((((c / 100) % 4) * 5) + g) % 7);

  /**< Determine the current weekday (Monday = 1, Sunday = 7) */
  h = dayOfYear + (jan1Weekday - 1);
  weekday = 1 + ((h - 1) % 7);

  /**< Determine the current date falls in previous year, week numbers 52 or 53 */
  if ((dayOfYear <= (8 - jan1Weekday)) && (jan1Weekday > 4))
    {
      yearNumber = previousYear;
      if ((jan1Weekday == 5) || ((jan1Weekday == 6) && previousIsLeapYear))
        weekNumber = 53;
      else
        weekNumber = 52;
    }
  else
    yearNumber = year;

  /**< Determine if the current date falls in the previous year, week number 1 */
  if (yearNumber == year)
    {
      if (isLeapYear)
        i = 366;
      else
        i = 365;

      if ((i - dayOfYear) < (4 - weekday))
        {
          yearNumber = year + 1;
          weekNumber = 1;
        }
    }

  /**< Determine if the current date falls in the current year, week numbers 1 through 53 */
  if (yearNumber == year)
    {
      i = dayOfYear + (7 - weekday) + (jan1Weekday - 1);
      weekNumber = i / 7;
      if (jan1Weekday > 4)
        weekNumber--;
    }

  /**< Convert weekday and weekNumber into strings */
  wsprintf(stringDay, TEXT("%d"), weekday);
  wsprintf(stringWeek, TEXT("%d"), weekNumber);

  /**< Replace and occurances of '%u' and '%V' appropriately */
  ELStringReplace(format, (WCHAR*)TEXT("%u"), stringDay, false);
  ELStringReplace(format, (WCHAR*)TEXT("%V"), stringWeek, false);

  /**< Finally, pass the updated format string to the MSVC supplied wcsftime */
  return wcsftime(strDest, maxsize, format, timeptr);
}

void ELStripModified(WCHAR *theme)
{
  WCHAR *modified = (WCHAR*)TEXT(" (Modified)");
  UINT modifiedOffset = wcslen(theme) - wcslen(modified);

  if (!ELIsModifiedTheme(theme))
    return;

  theme[modifiedOffset] = '\0';
}

BOOL ELPathIsRelative(LPCTSTR lpszPath)
{
  std::wstring tmpPath = lpszPath;
  tmpPath = ELExpandVars(tmpPath);

  return PathIsRelative(tmpPath.c_str());
}

bool ELIsApplet(HWND hwnd)
{
  WCHAR windowClass[MAX_LINE_LENGTH];
  RealGetWindowClass(hwnd, windowClass, MAX_LINE_LENGTH);

  // emergeApplet Class
  if (_wcsicmp(windowClass, TEXT("EmergeDesktopApplet")) == 0)
    return true;

  // emergeCore Class
  if (_wcsicmp(windowClass, TEXT("EmergeDesktopCore")) == 0)
    return true;

  // emergeDesktop Class
  if (_wcsicmp(windowClass, TEXT("EmergeDesktopMenuBuilder")) == 0)
    return true;

  // Desktop Class
  if (_wcsicmp(windowClass, TEXT("progman")) == 0)
    return true;

  return false;
}
