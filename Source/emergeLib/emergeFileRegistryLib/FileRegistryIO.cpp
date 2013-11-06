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

#include "FileRegistryIO.h"

bool ELReadRegDWord(HKEY key, std::wstring value, DWORD* target, DWORD defaultValue)
{
  DWORD dwSize = sizeof(DWORD);

  if (RegQueryValueEx(key, value.c_str(), NULL, NULL, (LPBYTE)target, &dwSize) == ERROR_SUCCESS)
  {
    return true;
  }

  *target = defaultValue;
  return false;
}

bool ELWriteRegDWord(HKEY key, std::wstring value, DWORD* source)
{
  if (RegSetValueEx(key, value.c_str(), 0, REG_DWORD, (BYTE*)source, sizeof(DWORD)) == ERROR_SUCCESS)
  {
    return true;
  }

  return false;
}

bool ELReadRegRect(HKEY key, std::wstring value, RECT* target, RECT* defaultValue)
{
  DWORD dwSize = MAX_LINE_LENGTH;
  WCHAR tmp[MAX_LINE_LENGTH];

  if (RegQueryValueEx(key, value.c_str(), NULL, NULL, (LPBYTE)tmp, &dwSize) != ERROR_SUCCESS)
  {
    CopyRect(target, defaultValue);
    return false;
  }

  if (swscanf(tmp, TEXT("%d,%d,%d,%d"), &target->top, &target->left,
              &target->bottom, &target->right) != 4)
  {
    CopyRect(target, defaultValue);
    return false;
  }

  return true;
}

std::wstring ELReadRegString(HKEY key, std::wstring value, std::wstring defaultValue)
{
  WCHAR tempTarget[MAX_LINE_LENGTH];
  DWORD dwSize = MAX_LINE_LENGTH;

  if (RegQueryValueEx(key, value.c_str(), NULL, NULL, (LPBYTE)tempTarget, &dwSize) == ERROR_SUCCESS)
  {
    return tempTarget;
  }

  return defaultValue;
}

bool ELWriteRegRect(HKEY key, std::wstring value, RECT* source)
{
  WCHAR tmp[MAX_LINE_LENGTH];

  swprintf(tmp, TEXT("%d,%d,%d,%d"), source->top, source->left, source->bottom, source->right);

  return ELWriteRegString(key, value, tmp);
}

bool ELWriteRegString(HKEY key, std::wstring value, std::wstring source)
{
  if (RegSetValueEx(key, value.c_str(), 0, REG_SZ, (BYTE*)source.c_str(), (DWORD)wcslen(source.c_str()) * sizeof(source[0])) ==
      ERROR_SUCCESS)
  {
    return true;
  }

  return false;
}

bool ELOpenRegKey(std::wstring subkey, HKEY* key, bool createKey)
{
  DWORD result;
  std::wstring inputKey;

  inputKey = TEXT("Software\\Emerge Desktop\\") + subkey;

  if (createKey)
  {
    if (RegCreateKeyEx(HKEY_CURRENT_USER, inputKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                       NULL, key, &result) == ERROR_SUCCESS)
    {
      return true;
    }
  }
  else
  {
    if (RegOpenKeyEx(HKEY_CURRENT_USER, inputKey.c_str(), 0, KEY_ALL_ACCESS, key) == ERROR_SUCCESS)
    {
      return true;
    }
  }

  return false;
}

bool ELDeleteRegKey(std::wstring subkey)
{
  std::wstring inputKey;

  inputKey = TEXT("Software\\Emerge Desktop\\") + subkey;

  if (SHDeleteKey(HKEY_CURRENT_USER, inputKey.c_str()) == ERROR_SUCCESS)
  {
    return true;
  }

  return false;
}

bool ELCloseRegKey(HKEY key)
{
  if (RegCloseKey(key) == ERROR_SUCCESS)
  {
    return true;
  }

  return false;
}

bool ELWriteFileInt(std::wstring fileName, std::wstring keyword, int value)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  swprintf(tmp, TEXT("%d"), value);

  return WriteValue(fileName, keyword, tmp);
}

bool ELWriteFileString(std::wstring fileName, std::wstring keyword, std::wstring value)
{
  return WriteValue(fileName, keyword, value);
}

bool ELWriteFileBool(std::wstring fileName, std::wstring keyword, bool value)
{
  WCHAR tmp[MAX_LINE_LENGTH];

  if (value)
  {
    wcscpy(tmp, TEXT("true"));
  }
  else
  {
    wcscpy(tmp, TEXT("false"));
  }

  return WriteValue(fileName, keyword, tmp);
}

bool ELWriteFileColor(std::wstring fileName, std::wstring keyword, COLORREF value)
{
  WCHAR tmp[MAX_LINE_LENGTH];

  swprintf(tmp, TEXT("%d,%d,%d"), GetRValue(value), GetGValue(value), GetBValue(value));

  return WriteValue(fileName, keyword, tmp);
}

int ELReadFileInt(std::wstring fileName, std::wstring item, int defaultValue)
{
  std::wstring tempValue;
  bool found = false;
  int returnValue;

  tempValue = ReadValue(fileName, item);
  if (!tempValue.empty())
  {
    found = true;

    for (UINT i = 0; i < tempValue.size(); i++)
      if (isalpha(tempValue.at(i)) != 0)
      {
        found = false;
        break;
      }
  }

  if (found)
  {
    returnValue = _wtoi(tempValue.c_str());
  }
  else
  {
    returnValue = defaultValue;
  }

  return returnValue;
}

BYTE ELReadFileByte(std::wstring fileName, std::wstring item, BYTE defaultValue)
{
  std::wstring tempValue;
  bool found = false;
  BYTE returnValue;

  tempValue = ReadValue(fileName, item);
  if (!tempValue.empty())
  {
    found = true;

    for (UINT i = 0; i < tempValue.size(); i++)
      if (isalpha(tempValue.at(i)) != 0)
      {
        found = false;
        break;
      }
  }

  if (found)
  {
    returnValue = _wtoi(tempValue.c_str());
  }
  else
  {
    returnValue = defaultValue;
  }

  return returnValue;
}

std::wstring ELReadFileString(std::wstring fileName, std::wstring item, std::wstring defaultValue)
{
  std::wstring tempValue;

  tempValue = ReadValue(fileName, item);

  if (tempValue.empty())
  {
    tempValue = defaultValue;
  }

  return tempValue;
}

bool ELReadFileBool(std::wstring fileName, std::wstring item, bool defaultValue)
{
  std::wstring tempValue;

  tempValue = ReadValue(fileName, item);

  if (ELToLower(tempValue) == TEXT("true"))
  {
    return true;
  }
  else if (ELToLower(tempValue) == TEXT("false"))
  {
    return false;
  }
  else
  {
    return defaultValue;
  }
}

COLORREF ELReadFileColor(std::wstring fileName, std::wstring item, COLORREF defaultValue)
{
  std::wstring tempValue;
  int red = 0, green = 0, blue = 0;

  tempValue = ReadValue(fileName, item);

  if (!tempValue.empty())
  {
    if (swscanf(tempValue.c_str(), TEXT("%d,%d,%d"), &red, &green, &blue) == 3)
    {
      if ((red >= 0 && red <= 255) &&
          (green >= 0 && green <= 255) &&
          (blue >= 0 && blue <= 255))
      {
        return RGB(red, green, blue);
      }
    }
  }

  return defaultValue;
}

std::tr1::shared_ptr<TiXmlDocument> ELOpenXMLConfig(std::wstring file, bool create)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  file = ELExpandVars(file);
  std::string ansiFile = ELwstringTostring(file, CP_ACP);

  configXML = OpenXMLConfig(ansiFile, create);

  return configXML;
}

COLORREF ELReadXMLColorValue(TiXmlElement* section, std::wstring item, COLORREF defaultValue)
{
  WCHAR stringDefault[MAX_LINE_LENGTH];
  std::wstring stringValue;
  int red = 0, blue = 0, green = 0;

  swprintf(stringDefault, TEXT("%d,%d,%d"), GetRValue(defaultValue),
           GetGValue(defaultValue), GetBValue(defaultValue));

  stringValue = ELReadXMLStringValue(section, item, stringDefault);

  if (swscanf(stringValue.c_str(), TEXT("%d,%d,%d"), &red, &green, &blue) == 3)
  {
    if ((red >= 0 && red <= 255) &&
        (green >= 0 && green <= 255) &&
        (blue >= 0 && blue <= 255))
    {
      return RGB(red, green, blue);
    }
  }

  return defaultValue;
}

RECT ELReadXMLRectValue(TiXmlElement* section, std::wstring item, RECT defaultValue)
{
  WCHAR stringDefault[MAX_LINE_LENGTH];
  std::wstring stringValue;
  RECT returnValue;

  swprintf(stringDefault, TEXT("%d,%d,%d,%d"), defaultValue.top, defaultValue.left,
           defaultValue.bottom, defaultValue.right);

  stringValue = ELReadXMLStringValue(section, item, stringDefault);

  if (swscanf(stringValue.c_str(), TEXT("%d,%d,%d,%d"), returnValue.top, returnValue.left,
              returnValue.bottom, returnValue.right) == 4)
  {
    return returnValue;
  }

  return defaultValue;
}

bool ELReadXMLBoolValue(TiXmlElement* section, std::wstring item, bool defaultValue)
{
  std::wstring stringDefault, stringValue;

  if (defaultValue)
  {
    stringDefault = TEXT("true");
  }
  else
  {
    stringDefault = TEXT("false");
  }

  stringValue = ELReadXMLStringValue(section, item, stringDefault);

  return (ELToLower(stringValue) == TEXT("true"));
}

int ELReadXMLIntValue(TiXmlElement* section, std::wstring item, int defaultValue)
{
  WCHAR stringDefault[MAX_LINE_LENGTH];
  std::wstring stringValue;
  int returnValue = 0;

  swprintf(stringDefault, TEXT("%d"), defaultValue);

  stringValue = ELReadXMLStringValue(section, item, stringDefault);

  swscanf(stringValue.c_str(), TEXT("%d"), returnValue);

  return returnValue;
}

float ELReadXMLFloatValue(TiXmlElement* section, std::wstring item, float defaultValue)
{
  WCHAR stringDefault[MAX_LINE_LENGTH];
  std::wstring stringValue;
  float returnValue = 0;

  swprintf(stringDefault, TEXT("%f"), defaultValue);

  stringValue = ELReadXMLStringValue(section, item, stringDefault);

  swscanf(stringValue.c_str(), TEXT("%f"), returnValue);

  return returnValue;
}

std::wstring ELReadXMLStringValue(TiXmlElement* section, std::wstring item, std::wstring defaultValue)
{
  std::string narrowItem = ELwstringTostring(item);
  std::wstring wideString;
  std::wstring returnValue;

  if (!section)
  {
    return defaultValue;
  }

  TiXmlElement* xmlItem = section->FirstChildElement(narrowItem.c_str());
  if (!xmlItem)
  {
    return defaultValue;
  }

  if (xmlItem->GetText() != NULL)
  {
    return ELstringTowstring(xmlItem->GetText());
  }

  return defaultValue;
}

bool ELWriteXMLColorValue(TiXmlElement* section, std::wstring item, COLORREF value)
{
  WCHAR stringValue[MAX_LINE_LENGTH];

  swprintf(stringValue, TEXT("%d,%d,%d"), GetRValue(value), GetGValue(value),
           GetBValue(value));

  return ELWriteXMLStringValue(section, item, stringValue);
}

bool ELWriteXMLRectValue(TiXmlElement* section, std::wstring item, RECT value)
{
  WCHAR stringValue[MAX_LINE_LENGTH];

  swprintf(stringValue, TEXT("%d,%d,%d,%d"), value.top, value.left,
           value.bottom, value.right);

  return ELWriteXMLStringValue(section, item, stringValue);
}

bool ELWriteXMLBoolValue(TiXmlElement* section, std::wstring item, bool value)
{
  WCHAR stringValue[MAX_LINE_LENGTH];

  swprintf(stringValue, TEXT("false"));
  if (value)
  {
    swprintf(stringValue, TEXT("true"));
  }

  return ELWriteXMLStringValue(section, item, stringValue);
}

bool ELWriteXMLIntValue(TiXmlElement* section, std::wstring item, int value)
{
  WCHAR stringValue[MAX_LINE_LENGTH];

  swprintf(stringValue, TEXT("%d"), value);

  return ELWriteXMLStringValue(section, item, stringValue);
}

bool ELWriteXMLFloatValue(TiXmlElement* section, std::wstring item, float value)
{
  WCHAR stringValue[MAX_LINE_LENGTH];

  swprintf(stringValue, TEXT("%f"), value);

  return ELWriteXMLStringValue(section, item, stringValue);
}

bool ELWriteXMLStringValue(TiXmlElement* section, std::wstring item, std::wstring value)
{
  std::string narrowItem = ELwstringTostring(item), narrowValue = ELwstringTostring(value);

  if (!section)
  {
    return false;
  }

  TiXmlElement* xmlItem = section->FirstChildElement(narrowItem.c_str());
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

TiXmlElement* ELGetXMLElementParent(TiXmlElement* xmlElement)
{
  return xmlElement->Parent()->ToElement();
}

TiXmlElement* ELGetXMLSection(TiXmlDocument* configXML, std::wstring section, bool createSection)
{
  std::string narrowSection = ELwstringTostring(section);

  TiXmlElement* xmlSection = configXML->FirstChildElement(narrowSection.c_str());
  if (!xmlSection && createSection)
  {
    xmlSection = new TiXmlElement(narrowSection.c_str());
    configXML->LinkEndChild(xmlSection);
    configXML->SaveFile();
  }

  return xmlSection;
}

TiXmlElement* ELGetFirstXMLElement(TiXmlElement* xmlSection)
{
  return xmlSection->FirstChildElement();
}

TiXmlElement* ELGetFirstXMLElementByName(TiXmlElement* xmlSection, std::wstring elementName, bool createElement)
{
  std::string narrowElement = ELwstringTostring(elementName);
  TiXmlElement* child;

  child = xmlSection->FirstChildElement(narrowElement.c_str());
  if (!child && createElement)
  {
    child = ELSetFirstXMLElementByName(xmlSection, elementName);
  }

  return child;
}

TiXmlElement* ELSetFirstXMLElementByName(TiXmlElement* xmlSection, std::wstring elementName)
{
  std::string narrowElement = ELwstringTostring(elementName);
  TiXmlElement* child;

  child = new TiXmlElement(narrowElement.c_str());
  xmlSection->LinkEndChild(child);

  return child;
}

void ELSetFirstXMLElement(TiXmlElement* xmlSection, TiXmlElement* element)
{
  xmlSection->LinkEndChild(element);
}

TiXmlElement* ELGetSiblingXMLElement(TiXmlElement* xmlElement)
{
  return xmlElement->NextSiblingElement();
}

TiXmlElement* ELSetSiblingXMLElement(TiXmlElement* targetElement, TiXmlElement* sourceElement, bool insertAfter)
{
  TiXmlElement* sibling;

  if (insertAfter)
  {
    sibling = targetElement->Parent()->InsertAfterChild(targetElement, *sourceElement)->ToElement();
  }
  else
  {
    sibling = targetElement->Parent()->InsertBeforeChild(targetElement, *sourceElement)->ToElement();
  }

  return sibling;
}

TiXmlElement* ELSetSiblingXMLElementByName(TiXmlElement* xmlElement, const WCHAR* elementName, bool insertAfter)
{
  std::string narrowElement = ELwstringTostring(elementName);
  TiXmlElement* sibling, newSibling(narrowElement.c_str());

  if (insertAfter)
  {
    sibling = xmlElement->Parent()->InsertAfterChild(xmlElement, newSibling)->ToElement();
  }
  else
  {
    sibling = xmlElement->Parent()->InsertBeforeChild(xmlElement, newSibling)->ToElement();
  }

  return sibling;
}

TiXmlElement* ELCloneXMLElement(TiXmlElement* sourceElement)
{
  return sourceElement->Clone()->ToElement();
}

TiXmlElement* ELCloneXMLElementAsSibling(TiXmlElement* sourceElement, TiXmlElement* targetElement)
{
  TiXmlElement* sibling = NULL, *newElement = NULL;
  if (sourceElement)
  {
    newElement = sourceElement->Clone()->ToElement();
  }

  if (targetElement && newElement)
  {
    sibling = targetElement->Parent()->InsertBeforeChild(targetElement, *newElement)->ToElement();
  }

  return sibling;
}

TiXmlElement* ELCloneXMLElementAsChild(TiXmlElement* sourceElement, TiXmlElement* targetElement)
{
  TiXmlElement* newElement = NULL;
  if (sourceElement)
  {
    newElement = sourceElement->Clone()->ToElement();
  }

  if (targetElement && newElement)
  {
    targetElement->LinkEndChild(newElement);
  }

  return newElement;
}

bool ELGetXMLElementText(TiXmlElement* xmlElement, std::wstring xmlString)
{
  if (!xmlElement)
  {
    return false;
  }

  if (xmlElement->GetText() != NULL)
  {
    xmlString = ELstringTowstring(xmlElement->GetText());
    return true;
  }

  return false;
}

bool ELGetXMLElementLabel(TiXmlElement* xmlElement, std::wstring xmlString)
{
  if (xmlElement->Value() != NULL)
  {
    xmlString = ELstringTowstring(xmlElement->Value());
    return true;
  }

  return false;
}

bool ELRemoveXMLElement(TiXmlElement* xmlElement)
{
  TiXmlNode* xmlSection = xmlElement->Parent();

  return xmlSection->RemoveChild(xmlElement);
}

TiXmlDocument* ELGetXMLConfig(TiXmlElement* element)
{
  return element->GetDocument();
}

bool ELWriteXMLConfig(TiXmlDocument* configXML)
{
  return configXML->SaveFile();
}

bool WriteValue(std::wstring fileName, std::wstring keyword, std::wstring value)
{
  WCHAR fileLine[MAX_LINE_LENGTH], *token, *tokenLine;
  WCHAR tmpFile[MAX_PATH];
  bool written = false;

  wcscpy(tmpFile, ELGetTempFileName().c_str());
  if (wcslen(tmpFile) == 0)
  {
    return false;
  }

  FILE* tmpFP = _wfopen(tmpFile, TEXT("w"));
  FILE* existingFP = _wfopen(fileName.c_str(), TEXT("r"));

  if (!tmpFP)
  {
    return written;
  }

  if (existingFP != NULL)
  {
    while (fgetws(fileLine, MAX_LINE_LENGTH, existingFP))
    {
      tokenLine = _wcsdup(fileLine);
      token = wcstok(tokenLine, TEXT("\t ="));

      if (_wcsicmp(token, keyword.c_str()) == 0)
      {
        swprintf(fileLine, TEXT("%ls\t%ls\n"), keyword.c_str(), value.c_str());
        written = true;
      }

      free(tokenLine);
      fputws(fileLine, tmpFP);
    }
  }

  if (!written)
  {
    swprintf(fileLine, TEXT("%ls\t%ls\n"), keyword.c_str(), value.c_str());
    fputws(fileLine, tmpFP);
    written = true;
  }


  fclose(tmpFP);
  if (existingFP != NULL)
  {
    fclose(existingFP);
  }

  CopyFile(tmpFile, fileName.c_str(), FALSE);
  DeleteFile(tmpFile);

  return written;
}

std::wstring ReadValue(std::wstring fileName, std::wstring keyword)
{
  WCHAR fileLine[MAX_LINE_LENGTH], *token, tmp[MAX_LINE_LENGTH], tempValue[MAX_LINE_LENGTH];
  FILE* file = _wfopen(fileName.c_str(), TEXT("r"));
  bool found = false;
  UINT i = 0, j = 0;

  ZeroMemory(tempValue, MAX_LINE_LENGTH);

  if (!file)
  {
    return TEXT("");
  }

  while (fgetws(fileLine, MAX_LINE_LENGTH, file))
  {
    token = wcstok(fileLine, TEXT("\t ="));

    if (_wcsicmp(token, keyword.c_str()) == 0)
    {
      token = wcstok(NULL, TEXT("\n\0"));

      wcscpy(tmp, token);

      while (i < wcslen(tmp))
      {
        if (found)
        {
          tempValue[j] = tmp[i];
          j++;
        }

        if (!found && isalnum((int)tmp[i]))
        {
          found = true;
          tempValue[j] = tmp[i];
          j++;
        }

        i++;
      }

      break;
    }
  }

  fclose(file);

  return tempValue;
}

std::tr1::shared_ptr<TiXmlDocument> OpenXMLConfig(std::string filename, bool create)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML(new TiXmlDocument(filename.c_str()));

  if (!configXML->LoadFile())
  {
    if (create)
    {
      TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "UTF-8", "yes");
      configXML->LinkEndChild(decl);
      if (!configXML->SaveFile())
      {
        configXML.reset();
      }
    }
    else
    {
      configXML.reset();
    }
  }

  return configXML;
}
