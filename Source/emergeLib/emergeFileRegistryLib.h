/*!
  @file emergeFileRegistryLib.h
  @brief export header for emergeLib
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2012  The Emerge Desktop Development Team

  @attention Emerge Desktop is free software; you can redistribute it and/or
  modify  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  @attention Emerge Desktop is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  @attention You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */

#ifndef __GUARD_2108849e_eff4_422c_b473_7f960e2e56a5
#define __GUARD_2108849e_eff4_422c_b473_7f960e2e56a5

#define UNICODE 1

#ifdef EMERGELIB_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#ifndef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif
#endif

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include <windows.h>
#include <string>
#include <vector>
#include "../tinyxml/tinyxml.h"

/*!
@struct _SHORTCUTINFO
@brief structure that contains shortcut information
*/
typedef struct _SHORTCUTINFO
{
  DWORD flags;
  WCHAR Path[MAX_PATH]; /*!< Path to the executable defined by the shortcut */
  WCHAR WorkingDirectory[MAX_PATH]; /*!< Working directory of the shortcut */
  WCHAR Arguments[MAX_PATH]; /*!< Arguments of the shortcut */
  int ShowCmd; /*!< Initial visibility state of the shortcut */
  WCHAR IconPath[MAX_PATH]; /*!< Path to the icon for the shortcut */
  int IconIndex; /*!< Index of the icon to use for the shortcut */
  bool runAs; /*!< Run As flag for the shortcut */
}
SHORTCUTINFO, *LPSHORTCUTINFO;

typedef enum _SPECIALFILEFLAGS
{
  SF_NOTHING = 0,
  SF_FILE,
  SF_SHORTCUT,
  SF_DIRECTORY,
  SF_SPECIALFOLDER,
  SF_URL,
  SF_CLSID,
  SF_INTERNALCOMMAND,
  SF_UNC
}
SPECIALFILEFLAGS;

typedef bool (*COMMANDHANDLERPROC)(std::vector<std::wstring>); //args

const int UNDEFINED_INTERNALCOMMAND_VALUE = -1;

//FileRegistryIO.h
DLL_EXPORT DWORD ELReadRegDWord(HKEY key, std::wstring value, DWORD defaultValue);
DLL_EXPORT std::wstring ELReadRegString(HKEY key, std::wstring value, std::wstring defaultValue);
DLL_EXPORT RECT ELReadRegRect(HKEY key, std::wstring value, RECT* defaultValue);
DLL_EXPORT bool ELWriteRegDWord(HKEY key, std::wstring value, DWORD* source);
DLL_EXPORT bool ELWriteRegString(HKEY key, std::wstring value, std::wstring source);
DLL_EXPORT bool ELWriteRegRect(HKEY key, std::wstring value, RECT* source);
DLL_EXPORT bool ELOpenRegKey(std::wstring subkey, HKEY* key, bool createKey);
DLL_EXPORT bool ELDeleteRegKey(std::wstring subkey);
DLL_EXPORT bool ELCloseRegKey(HKEY key);

DLL_EXPORT int ELReadFileInt(std::wstring fileName, std::wstring item, int defaultValue);
DLL_EXPORT BYTE ELReadFileByte(std::wstring fileName, std::wstring item, BYTE defaultValue);
DLL_EXPORT std::wstring ELReadFileString(std::wstring fileName, std::wstring item, std::wstring defaultValue);
DLL_EXPORT bool ELReadFileBool(std::wstring fileName, std::wstring item, bool defaultValue);
DLL_EXPORT COLORREF ELReadFileColor(std::wstring fileName, std::wstring item, COLORREF defaultValue);
DLL_EXPORT bool ELWriteFileInt(std::wstring fileName, std::wstring keyword, int value);
DLL_EXPORT bool ELWriteFileString(std::wstring fileName, std::wstring keyword, std::wstring value);
DLL_EXPORT bool ELWriteFileBool(std::wstring fileName, std::wstring keyword, bool value);
DLL_EXPORT bool ELWriteFileColor(std::wstring fileName, std::wstring keyword, COLORREF value);

DLL_EXPORT std::tr1::shared_ptr<TiXmlDocument> ELOpenXMLConfig(std::wstring filename, bool create);
DLL_EXPORT TiXmlDocument* ELGetXMLConfig(TiXmlElement* element);
DLL_EXPORT bool ELWriteXMLConfig(TiXmlDocument* configXML);
DLL_EXPORT COLORREF ELReadXMLColorValue(TiXmlElement* section, std::wstring item, COLORREF defaultValue);
DLL_EXPORT RECT ELReadXMLRectValue(TiXmlElement* section, std::wstring item, RECT defaultValue);
DLL_EXPORT bool ELReadXMLBoolValue(TiXmlElement* section, std::wstring item, bool defaultValue);
DLL_EXPORT int ELReadXMLIntValue(TiXmlElement* section, std::wstring item, int defaultValue);
DLL_EXPORT float ELReadXMLFloatValue(TiXmlElement* section, std::wstring item, float defaultValue);
DLL_EXPORT std::wstring ELReadXMLStringValue(TiXmlElement* section, std::wstring item, std::wstring defaultValue);
DLL_EXPORT bool ELWriteXMLColorValue(TiXmlElement* section, std::wstring item, COLORREF value);
DLL_EXPORT bool ELWriteXMLRectValue(TiXmlElement* section, std::wstring item, RECT value);
DLL_EXPORT bool ELWriteXMLBoolValue(TiXmlElement* section, std::wstring item, bool value);
DLL_EXPORT bool ELWriteXMLIntValue(TiXmlElement* section, std::wstring item, int value);
DLL_EXPORT bool ELWriteXMLFloatValue(TiXmlElement* section, std::wstring item, float value);
DLL_EXPORT bool ELWriteXMLStringValue(TiXmlElement* section, std::wstring item, std::wstring value);
DLL_EXPORT TiXmlElement* ELGetXMLElementParent(TiXmlElement* xmlElement);
DLL_EXPORT TiXmlElement* ELGetXMLSection(TiXmlDocument* configXML, std::wstring section, bool createSection);
DLL_EXPORT TiXmlElement* ELGetFirstXMLElement(TiXmlElement* xmlSection);
DLL_EXPORT TiXmlElement* ELGetFirstXMLElementByName(TiXmlElement* xmlSection, std::wstring elementName, bool createElement);
DLL_EXPORT TiXmlElement* ELSetFirstXMLElementByName(TiXmlElement* xmlSection, std::wstring elementName);
DLL_EXPORT void ELSetFirstXMLElement(TiXmlElement* xmlSection, TiXmlElement* element);
DLL_EXPORT TiXmlElement* ELGetSiblingXMLElement(TiXmlElement* xmlElement);
DLL_EXPORT TiXmlElement* ELSetSiblingXMLElement(TiXmlElement* targetElement, TiXmlElement* sourceElement, bool insertAfter);
DLL_EXPORT TiXmlElement* ELSetSiblingXMLElementByName(TiXmlElement* xmlElement, const WCHAR* elementName, bool insertAfter = true);
DLL_EXPORT TiXmlElement* ELCloneXMLElement(TiXmlElement* sourceElement);
DLL_EXPORT TiXmlElement* ELCloneXMLElementAsSibling(TiXmlElement* sourceElement, TiXmlElement* targetElement);
DLL_EXPORT TiXmlElement* ELCloneXMLElementAsChild(TiXmlElement* sourceElement, TiXmlElement* targetElement);
DLL_EXPORT bool ELGetXMLElementText(TiXmlElement* xmlElement, std::wstring xmlString);
DLL_EXPORT bool ELGetXMLElementLabel(TiXmlElement* xmlElement, std::wstring xmlString);
DLL_EXPORT bool ELRemoveXMLElement(TiXmlElement* xmlElement);

//FileSystemOps.h
DLL_EXPORT bool ELCreateDirectory(std::wstring directory);
DLL_EXPORT std::wstring ELGetTempFileName();

DLL_EXPORT bool ELParseCommand(std::wstring appToParse, WCHAR* program, WCHAR* arguments);
DLL_EXPORT bool ELParseShortcut(std::wstring shortcut, LPSHORTCUTINFO shortcutInfo);

DLL_EXPORT std::wstring ELGetFileName(std::wstring filePath);
DLL_EXPORT std::wstring ELGetFileExtension(std::wstring filePath);
DLL_EXPORT std::wstring ELGetFileArguments(std::wstring filePath);
DLL_EXPORT std::wstring ELStripFileArguments(std::wstring filePath);

DLL_EXPORT std::wstring ELGetAbsolutePath(std::wstring relativeFilePath, std::wstring baseDirPath = TEXT("%AppletDir%\\"));
DLL_EXPORT std::wstring ELGetRelativePath(std::wstring filePath, std::wstring baseDirPath = TEXT("%AppletDir%\\"));
DLL_EXPORT bool ELPathIsRelative(std::wstring filePath);
DLL_EXPORT std::wstring ELExhaustivelyFindFilePath(std::wstring filePath);

DLL_EXPORT bool ELFileExists(std::wstring filePath);

DLL_EXPORT int ELGetFileSpecialFlags(std::wstring filePath);
DLL_EXPORT std::wstring ELGetFileTypeCommand(std::wstring document, std::wstring docArgs);

DLL_EXPORT std::wstring ELGetSpecialFolderNameFromPath(std::wstring folderPath);
DLL_EXPORT std::wstring ELGetSpecialFolderNameFromID(int specialFolderID);
DLL_EXPORT std::wstring ELGetSpecialFolderPathFromID(int specialFolderID);
DLL_EXPORT std::wstring ELGetSpecialFolderPathFromName(std::wstring specialFolderName);
DLL_EXPORT int ELGetSpecialFolderIDFromPath(std::wstring folderPath);
DLL_EXPORT int ELGetSpecialFolderIDFromName(std::wstring specialFolderName);

DLL_EXPORT std::wstring ELGetUNCFromMap(std::wstring uncMap);

DLL_EXPORT bool ELIsFileTypeExecutable(std::wstring extension);
DLL_EXPORT bool ELExecuteFileOrCommand(std::wstring application, std::wstring arguments = TEXT(""), std::wstring workingDir = TEXT(""), int nShow = SW_SHOW);
DLL_EXPORT bool ELFileOp(HWND appletWnd, bool feedback, UINT function, std::wstring source, std::wstring destination = TEXT(""));

//InternalCommandEngine.h
DLL_EXPORT bool ELRegisterInternalCommand(std::wstring commandName, int commandValue, COMMANDHANDLERPROC commandHandler);
DLL_EXPORT bool ELIsInternalCommand(std::wstring commandName);
DLL_EXPORT std::wstring ELGetInternalCommandName(int commandValue);
DLL_EXPORT int ELGetInternalCommandValue(std::wstring commandName);
DLL_EXPORT bool ELExecuteInternalCommand(std::wstring commandName, std::wstring arguments);
DLL_EXPORT bool ELPopulateInternalCommandList(HWND hwnd);

#endif
