//----  --------------------------------------------------------------------------------------------------------
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
//----  --------------------------------------------------------------------------------------------------------

#include "Environment.h"

std::wstring ELExpandVars(std::wstring value)
{
  std::wstring expandedValue;
  WCHAR* tmpValue = NULL;
  size_t bufferSize;

  if (value.empty())
  {
    return value;
  }

  bufferSize = ExpandEnvironmentStrings(value.c_str(), tmpValue, 0);
  if (bufferSize == 0)
  {
    return value;
  }

  // Account for terminating NULL
  bufferSize++;

  tmpValue = (WCHAR*)GlobalAlloc(GPTR, bufferSize * sizeof(WCHAR));
  if (ExpandEnvironmentStrings(value.c_str(), tmpValue, bufferSize) != 0)
  {
    expandedValue = tmpValue;
  }
  GlobalFree(tmpValue);

  return expandedValue;
}

std::wstring ELUnExpandVars(std::wstring value)
{
  std::wstring output = value;
  bool success = false;
  WCHAR tmp[MAX_LINE_LENGTH];
  std::wstring collapsedEnvVar, collapsedOutput;

  std::array<std::wstring, 9> envVars;
  size_t counter;

  envVars[0] = TEXT("StartMenu");
  envVars[1] = TEXT("CommonStartMenu");
  envVars[2] = TEXT("Desktop");
  envVars[3] = TEXT("CommonDesktop");
  envVars[4] = TEXT("Documents");
  envVars[5] = TEXT("CommonDocuments");
  envVars[6] = TEXT("ThemeDir");
  envVars[7] = TEXT("AppletDir");   /* Resolve AppletDir before EmergeDir to handle the situation where both AppletDir
  and EmergeDir refer to the same directory, as it can affect theme saving. */
  envVars[8] = TEXT("EmergeDir");

  for (counter = 0; counter < envVars.size(); counter++)
    if (GetEnvironmentVariable(envVars[counter].c_str(), tmp, MAX_LINE_LENGTH) != 0)
    {
      collapsedEnvVar = TEXT("%");
      collapsedEnvVar.append(envVars[counter]);
      collapsedEnvVar.append(TEXT("%"));
      collapsedOutput = ELwstringReplace(output, tmp, collapsedEnvVar, true);
      if (output != collapsedOutput)
      {
        output = collapsedOutput;
        success = true;
      }
    }

  if (!success)
  {
    if (PathUnExpandEnvStrings(output.c_str(), tmp, MAX_LINE_LENGTH))
    {
      output = tmp;
    }
  }

  return output;
}

std::wstring ELGetCurrentPath()
{
  WCHAR tempPath[MAX_PATH];
  if (GetModuleFileName(NULL, tempPath, MAX_PATH) == 0)
  {
    return TEXT("");
  }

  PathRemoveFileSpec(tempPath);
  return tempPath;
}

std::wstring ELGetUserDataPath()
{
  std::wstring path = TEXT("%EmergeDir%");
  path = ELExpandVars(path);
  return path;
}

PORTABLEMODE ELGetPortableMode()
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* section;
  std::wstring portablePath, customDataPath;
  PORTABLEMODE portableMode;

  portablePath = ELGetCurrentPath();
  portablePath = portablePath + TEXT("\\portable.xml");

  if (!ELFileExists(portablePath))
  {
    return PM_INSTALLED;
  }

  portableMode = PM_PORTABLE;

  configXML = ELOpenXMLConfig(portablePath, false);
  if (configXML)
  {
    section = ELGetXMLSection(configXML.get(), TEXT("Portable"), false);
    if (section)
    {
      customDataPath = ELReadXMLStringValue(section, TEXT("CustomDataPath"), TEXT(""));
      if (!customDataPath.empty())
      {
        portableMode = PM_CUSTOM;
      }
    }
  }

  return portableMode;
}

bool ELIsWow64()
{
  BOOL bIsWow64 = FALSE;

  if (MSIsWow64Process == NULL)
  {
    MSIsWow64Process = (lpfnIsWow64Process)GetProcAddress(emergeLibGlobals::getKernel32DLL(), "IsWow64Process");
  }
  if (MSIsWow64Process)
  {
    MSIsWow64Process(GetCurrentProcess(), &bIsWow64);
  }

  return (bIsWow64 == TRUE);
}

// Don't use any EmergeVars in ELGetThemeInfo because it is used to derive the EmergeVars
void ELGetThemeInfo(LPTHEMEINFO themeInfo)
{
  std::wstring userPath, workingPath;
  std::wstring tempCustomDataPath;
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* section;
  const std::wstring currentThemeValue = TEXT("Current"), defaultThemeValue = TEXT("Default");

  workingPath = ELGetCurrentPath();
  userPath = workingPath + TEXT("\\theme.xml");

  tempCustomDataPath = GetCustomDataPath();
  if ((ELGetPortableMode() == PM_CUSTOM) && (!tempCustomDataPath.empty()))
  {
    workingPath = tempCustomDataPath;
    userPath = workingPath + TEXT("\\theme.xml");
  }

  if ((ELGetPortableMode() == PM_INSTALLED) && !ELFileExists(userPath))
  {
    workingPath = TEXT("%AppData%\\Emerge Desktop");
    workingPath = ELExpandVars(workingPath);
    userPath = workingPath + TEXT("\\theme.xml");
  }
  themeInfo->path = workingPath;

  configXML = ELOpenXMLConfig(userPath, true);
  if (configXML)
  {
    section = ELGetXMLSection(configXML.get(), TEXT("Theme"), true);
    if (section)
    {
      themeInfo->theme = ELReadXMLStringValue(section, currentThemeValue, defaultThemeValue);
      if (ELToLower(themeInfo->theme) != ELToLower(defaultThemeValue))
      {
        ELWriteXMLStringValue(section, currentThemeValue, themeInfo->theme);
        ELWriteXMLConfig(configXML.get());
      }
      else
      {
        themeInfo->themePath = themeInfo->path + TEXT("\\themes\\") + themeInfo->theme;
        if (!ELIsDirectory(themeInfo->themePath))
        {
          themeInfo->theme = defaultThemeValue;
          ELWriteXMLStringValue(section, currentThemeValue, themeInfo->theme);
          ELWriteXMLConfig(configXML.get());
        }
      }
    }
  }

  if (ELToLower(themeInfo->theme) != ELToLower(defaultThemeValue))
  {
    themeInfo->themePath = themeInfo->path + TEXT("\\themes\\") + themeInfo->theme;
    workingPath = themeInfo->themePath;
    ELCreateDirectory(workingPath);
  }
  themeInfo->userPath = themeInfo->path + TEXT("\\files");
  workingPath = themeInfo->userPath;
  ELCreateDirectory(workingPath);
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
  std::wstring appletPath;

  THEMEINFO themeInfo;
  ELGetThemeInfo(&themeInfo);
  appletPath = ELGetCurrentPath();

  if (!SetEnvironmentVariable(TEXT("ThemeDir"), themeInfo.themePath.c_str()))
  {
    return false;
  }

  if (!SetEnvironmentVariable(TEXT("EmergeDir"), themeInfo.path.c_str()))
  {
    return false;
  }

  if (!SetEnvironmentVariable(TEXT("AppletDir"), appletPath.c_str()))
  {
    return false;
  }

  // Clear the __COMPAT_LAYER variable as it seems to cause some applications
  // to misbehave when inherited as part of the Environment block.
  SetEnvironmentVariable(TEXT("__COMPAT_LAYER"), NULL);

  return true;
}

void ELSetEnvironmentVars(bool showErrors)
{
  WCHAR tmp[MAX_PATH];

  bool localCheck = true;
  if (SHGetFolderPath(GetDesktopWindow(), CSIDL_STARTMENU, NULL, SHGFP_TYPE_CURRENT, tmp) == S_OK)
  {
    if (!SetEnvironmentVariable(TEXT("StartMenu"), tmp))
    {
      localCheck = false;
    }
  }
  else
  {
    localCheck = false;
  }
  if (showErrors && !localCheck)
    MessageBox(GetDesktopWindow(), TEXT("Failed to set %%StartMenu%%"),
                 TEXT("Emerge Desktop"), MB_OK | MB_ICONERROR);


  localCheck = true;
  if (SHGetFolderPath(GetDesktopWindow(), CSIDL_COMMON_STARTMENU, NULL, SHGFP_TYPE_CURRENT, tmp) == S_OK)
  {
    if (!SetEnvironmentVariable(TEXT("CommonStartMenu"), tmp))
    {
      localCheck = false;
    }
  }
  else
  {
    localCheck = false;
  }
  if (showErrors && !localCheck)
    MessageBox(GetDesktopWindow(), TEXT("Failed to set %%CommonStartMenu%%"),
                 TEXT("Emerge Desktop"), MB_OK | MB_ICONERROR);

  localCheck = true;
  if (SHGetFolderPath(GetDesktopWindow(), CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, tmp) == S_OK)
  {
    if (!SetEnvironmentVariable(TEXT("Desktop"), tmp))
    {
      localCheck = false;
    }
  }
  else
  {
    localCheck = false;
  }
  if (showErrors && !localCheck)
    MessageBox(GetDesktopWindow(), TEXT("Failed to set %%Desktop%%"),
                 TEXT("Emerge Desktop"), MB_OK | MB_ICONERROR);

  localCheck = true;
  if (SHGetFolderPath(GetDesktopWindow(), CSIDL_COMMON_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, tmp) == S_OK)
  {
    if (!SetEnvironmentVariable(TEXT("CommonDesktop"), tmp))
    {
      localCheck = false;
    }
  }
  else
  {
    localCheck = false;
  }
  if (showErrors && !localCheck)
    MessageBox(GetDesktopWindow(), TEXT("Failed to set %%CommonDesktop%%"),
                 TEXT("Emerge Desktop"), MB_OK | MB_ICONERROR);

  localCheck = true;
  if (SHGetFolderPath(GetDesktopWindow(), CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, tmp) == S_OK)
  {
    if (!SetEnvironmentVariable(TEXT("Documents"), tmp))
    {
      localCheck = false;
    }
  }
  else
  {
    localCheck = false;
  }
  if (showErrors && !localCheck)
    MessageBox(GetDesktopWindow(), TEXT("Failed to set %%Documents%%"),
                 TEXT("Emerge Desktop"), MB_OK | MB_ICONERROR);

  localCheck = true;
  if (SHGetFolderPath(GetDesktopWindow(), CSIDL_COMMON_DOCUMENTS, NULL, SHGFP_TYPE_CURRENT, tmp) == S_OK)
  {
    if (!SetEnvironmentVariable(TEXT("CommonDocuments"), tmp))
    {
      localCheck = false;
    }
  }
  else
  {
    localCheck = false;
  }
  if (showErrors && !localCheck)
    MessageBox(GetDesktopWindow(), TEXT("Failed to set %%CommonDocuments%%"),
                 TEXT("Emerge Desktop"), MB_OK | MB_ICONERROR);
}

bool ELIsExplorerShell()
{
  WCHAR explorerPath[MAX_PATH];

  if (GetWindowsDirectory(explorerPath, MAX_PATH) == 0)
  {
    return false;
  }

  wcscat(explorerPath, TEXT("\\explorer.exe"));
  _wcslwr(explorerPath);

  HWND progmanWnd = FindWindow(TEXT("progman"), NULL);
  if (progmanWnd)
  {
    std::wstring progmanExec = ELGetWindowApp(progmanWnd, true);
    if (ELToLower(progmanExec) == explorerPath)
    {
      return true;
    }
  }

  return false;
}

bool ELIsEmergeShell()
{
  HWND trayWnd = FindWindow(TEXT("Shell_TrayWnd"), NULL);
  if (trayWnd)
  {
    std::wstring trayExec = ELGetWindowApp(trayWnd, false);
    if (ELToLower(trayExec) == TEXT("emergetray.exe"))
    {
      return true;
    }
  }

  return false;
}

std::wstring GetCustomDataPath()
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* section;
  std::wstring portablePath, customDataPath;

  portablePath = ELGetCurrentPath();
  portablePath = portablePath + TEXT("\\portable.xml");

  if (!ELFileExists(portablePath))
  {
    return TEXT("");
  }

  configXML = ELOpenXMLConfig(portablePath, false);
  if (configXML)
  {
    section = ELGetXMLSection(configXML.get(), TEXT("Portable"), false);
    if (section)
    {
      customDataPath = ELReadXMLStringValue(section, TEXT("CustomDataPath"), TEXT(""));
      if (!customDataPath.empty())
      {
        return ELGetAbsolutePath(customDataPath);
      }
    }
  }

  return TEXT("");
}
