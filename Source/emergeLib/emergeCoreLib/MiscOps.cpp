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

#include "MiscOps.h"

void ELDispatchCoreMessage(DWORD type, DWORD message, std::wstring instanceName)
{
  HWND coreWindowHwnd;
  NOTIFYINFO notifyInfo;
  COPYDATASTRUCT cds;

  coreWindowHwnd = ELGetCoreWindow();
  if (coreWindowHwnd == NULL)
  {
    return;
  }

  ZeroMemory(&notifyInfo, sizeof(notifyInfo));
  notifyInfo.Type = type;
  notifyInfo.Message = message;
  if (!instanceName.empty())
  {
    wcsncpy(notifyInfo.InstanceName, instanceName.c_str(), MAX_PATH - 1);
  }

  cds.dwData = EMERGE_DISPATCH;
  cds.cbData = sizeof(notifyInfo);
  cds.lpData = &notifyInfo;

  SendMessageTimeout(coreWindowHwnd, WM_COPYDATA, 0, (LPARAM)&cds,
                     SMTO_ABORTIFHUNG, 500, NULL);
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
  std::wstring applet = ELGetWindowApp(appWnd, false);

  return ELAppletFileVersion(applet.c_str(), versionInfo);
}

/*!
  @fn ELAppletFileVersion(WCHAR *applet, LPVERSIONINFO versionInfo)
  @brief Populates a VERSIONINFO structure based on the filename supplied
  @param applet File to open
  @param versionInfo VERSIONINFO structure to populate
  @return true if successful
  */

bool ELAppletFileVersion(std::wstring applet, LPVERSIONINFO versionInfo)
{
  WCHAR baseQueryData[MAX_LINE_LENGTH];
  void* buffer, *fileVersionInfoData;
  DWORD notUsed, dataSize;
  UINT bufferSize;
  VS_FIXEDFILEINFO* fixedFileInfo;
  std::wstring workingApplet, queryStringSpecifier;

  workingApplet = ELExpandVars(applet);

  dataSize = GetFileVersionInfoSize(workingApplet.c_str(), &notUsed);
  if (dataSize == 0)
  {
    return false;
  }

  fileVersionInfoData = (void*)malloc(dataSize + 1);
  if (!fileVersionInfoData)
  {
    return false;
  }

  if (GetFileVersionInfo(workingApplet.c_str(), notUsed, dataSize, fileVersionInfoData))
  {
    if (VerQueryValue(fileVersionInfoData, (WCHAR*)TEXT("\\VarFileInfo\\Translation"), &buffer, &bufferSize))
      swprintf(baseQueryData, TEXT("\\StringFileInfo\\%04x%04x\\"), LOWORD(*((UINT*)buffer)),
               HIWORD(*((UINT*)buffer)));
    else
    {
      free(fileVersionInfoData);
      return false;
    }

    queryStringSpecifier = baseQueryData;
    queryStringSpecifier = queryStringSpecifier + TEXT("FileDescription");
    if (VerQueryValue(fileVersionInfoData, queryStringSpecifier.c_str(), &buffer, &bufferSize))
    {
      wcscpy(versionInfo->Description, (WCHAR*)buffer);
    }
    else
    {
      free(fileVersionInfoData);
      return false;
    }

    if (VerQueryValue(fileVersionInfoData, (WCHAR*)TEXT("\\"), &buffer, &bufferSize))
    {
      fixedFileInfo = (VS_FIXEDFILEINFO*)buffer;
      swprintf(versionInfo->Version, TEXT("%d.%d.%d.%d"),
               HIWORD(fixedFileInfo->dwFileVersionMS), LOWORD(fixedFileInfo->dwFileVersionMS),
               HIWORD(fixedFileInfo->dwFileVersionLS), LOWORD(fixedFileInfo->dwFileVersionLS));
    }
    else
    {
      free(fileVersionInfoData);
      return false;
    }

    queryStringSpecifier = baseQueryData;
    queryStringSpecifier = queryStringSpecifier + TEXT("CompanyName");
    if (VerQueryValue(fileVersionInfoData, queryStringSpecifier.c_str(), &buffer, &bufferSize))
    {
      wcscpy(versionInfo->Author, (WCHAR*)buffer);
    }
    else
    {
      free(fileVersionInfoData);
      return false;
    }

    free(fileVersionInfoData);
    return true;
  }

  free(fileVersionInfoData);
  return false;
}

bool ELIsAppletRunning(std::wstring applet)
{
  DWORD processList[1024], cbNeeded, processCount, i;
  std::wstring appletPath = applet;

  if (ELPathIsRelative(appletPath))
  {
    appletPath = ELGetAbsolutePath(appletPath);
  }

  EnumProcesses(processList, sizeof(processList), &cbNeeded);
  processCount = cbNeeded / sizeof(DWORD);
  appletPath = ELToLower(ELExpandVars(appletPath));

  for (i = 0; i < processCount; i++)
  {
    if (ELToLower(ELGetProcessIDApp(processList[i], true)) == appletPath)
    {
      break;
    }
  }

  return (i != processCount);
}

std::wstring ELGetThemeName()
{
  size_t i;
  std::wstring working = TEXT("%ThemeDir%");
  working = ELExpandVars(working);
  i = working.find_last_of(TEXT("\\"));
  return working.substr(i + 1);
}

bool ELSetTheme(std::wstring theme)
{
  bool ret = false;
  std::wstring emergeXML = TEXT("%EmergeDir%\\theme.xml");
  std::tr1::shared_ptr<TiXmlDocument> configXML = ELOpenXMLConfig(emergeXML, true);
  TiXmlElement* section;

  theme = ELExpandVars(theme);
  EnumWindows(ThemeEnum, (LPARAM)theme.c_str());
  if (configXML)
  {
    section = ELGetXMLSection(configXML.get(), TEXT("Theme"), true);

    if (section)
    {
      ELWriteXMLStringValue(section, TEXT("Current"), ELGetThemeName());
      ELWriteXMLConfig(configXML.get());

      ret = true;
    }
  }

  return ret;
}

bool ELIsModifiedTheme(std::wstring theme)
{
  return (theme.find(TEXT("(Modified)")) != std::wstring::npos);
}

bool ELSetModifiedTheme(std::wstring theme)
{
  bool ret = false;
  std::wstring themePath = TEXT("%EmergeDir%\\themes\\");

  if (theme.empty())
  {
    return ret;
  }

  if (!ELIsModifiedTheme(theme))
  {
    // Generate the (Modified) theme name
    theme += TEXT(" (Modified)");
    themePath += theme;
    themePath = ELExpandVars(themePath);

    // Create the theme directory if it doesn't exist
    if ((ELGetFileSpecialFlags(themePath) & SF_DIRECTORY) != SF_DIRECTORY)
    {
      ELCreateDirectory(themePath);
    }

    // If the theme path is created, set it as the theme
    if ((ELGetFileSpecialFlags(themePath) & SF_DIRECTORY) == SF_DIRECTORY)
    {
      ELSetTheme(themePath);
      ret = true;
    }
  }

  return ret;
}

std::wstring ELStripModifiedTheme(std::wstring theme)
{
  std::wstring modified = TEXT(" (Modified)");
  UINT modifiedOffset = theme.size() - modified.size();

  if (!ELIsModifiedTheme(theme))
  {
    return TEXT("");
  }

  return theme.substr(0, modifiedOffset);
}

bool ELAdjustVolume(VOLUMEFLAGS command)
{
  bool ret = false;
  //if (ELOSVersionInfo() >= 6.0)
  if (IsWindowsVistaOrGreater())
  {
    ret = VistaVolumeControl(command);
  }
  else
  {
    ret = VolumeControl(command);
  }

  return ret;
}

bool ELPlaySound(std::wstring sound)
{
  UINT soundFlags = SND_ALIAS | SND_ASYNC | SND_NODEFAULT;

  //if (ELOSVersionInfo() >= 6.0)
  if (IsWindowsVistaOrGreater()) //SND_SYSTEM is only available on Vista or later
  {
    soundFlags |= SND_SYSTEM;
  }

  return (PlaySound(sound.c_str(), NULL, soundFlags) == TRUE);
}

BOOL CALLBACK ThemeEnum(HWND hwnd, LPARAM lParam)
{
  WCHAR hwndClass[MAX_LINE_LENGTH], *test = reinterpret_cast<WCHAR*>(lParam);
  COPYDATASTRUCT cds;

  if (GetClassName(hwnd, hwndClass, MAX_LINE_LENGTH) == 0)
  {
    return TRUE;
  }

  if ((_wcsicmp(hwndClass, TEXT("EmergeDesktopApplet")) != 0) &&
      (_wcsicmp(hwndClass, TEXT("EmergeDesktopMenuBuilder")) != 0) &&
      (_wcsicmp(hwndClass, TEXT("EmergeDesktopCore")) != 0))
  {
    return TRUE;
  }

  cds.dwData = EMERGE_MESSAGE;
  cds.cbData = sizeof(WCHAR) * (wcslen(test) + 1);
  cds.lpData = test;

  SendMessage(hwnd, WM_COPYDATA, (WPARAM)0, (LPARAM)&cds);

  return TRUE;
}

bool VolumeControl(VOLUMEFLAGS command)
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
      {
        mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
      }
      else
      {
        mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
      }
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
          {
            mute.fValue = !mute.fValue;
          }
          else if (command == ELAV_VOLUMEUP)
          {
            vol.dwValue += volDelta;
          }
          else
          {
            if (vol.dwValue - volDelta >= 0xffff)
            {
              vol.dwValue = 0;
            }
            else
            {
              vol.dwValue -= volDelta;
            }
          }

          if (mixerSetControlDetails((HMIXEROBJ)hMixer, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE) == MMSYSERR_NOERROR)
          {
            ret = true;
          }
        }
      }
    }
  }

  return ret;
}

bool VistaVolumeControl(VOLUMEFLAGS command)
{
  IMMDeviceEnumerator* deviceEnumerator = NULL;
  IMMDevice* defaultDevice = NULL;
  IAudioEndpointVolume* endpointVolume = NULL;
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
              {
                volume = 0.0;
              }
            }
            else if (command == ELAV_VOLUMEUP)
            {
              volume += (float)volumeDelta;
              if (volume > 1.0)
              {
                volume = 1.0;
              }
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
