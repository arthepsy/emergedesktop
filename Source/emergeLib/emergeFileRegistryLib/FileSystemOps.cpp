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

#include "FileSystemOps.h"

bool ELCreateDirectory(std::wstring directoryPath)
{
  size_t i = 0;
  std::wstring subdir;

  directoryPath = ELExpandVars(directoryPath);

  i = directoryPath.find_first_of(TEXT("\\"), i);
  while (i != std::wstring::npos)
  {
    subdir = directoryPath.substr(0, i);

    if (!subdir.empty())
    {
      if (!ELIsDirectory(subdir))
      {
        if (!CreateDirectory(subdir.c_str(), NULL))
        {
          return false;
        }
      }

    }
    i++;
    i = directoryPath.find_first_of(TEXT("\\"), i);
  }

  if (!ELIsDirectory(directoryPath))
  {
    if (!CreateDirectory(directoryPath.c_str(), NULL))
    {
      return false;
    }
  }

  return true;
}

std::wstring ELGetTempFileName()
{
  WCHAR tmpPath[MAX_PATH], tmpFile[MAX_PATH];

  if (GetTempPath(MAX_PATH, tmpPath) != 0)
  {
    if (GetTempFileName(tmpPath, TEXT("eds"), 0, tmpFile) != 0)
    {
      return tmpFile;
    }
  }

  return TEXT("");
}

bool ELParseCommand(std::wstring appToParse, WCHAR* program, WCHAR* arguments)
{
  std::wstring workingApp, workingArgs, tempPath, searchPath;
  int specialFlags;
  std::wstring pathArgsSeparator = TEXT(" ,\t"); //space, comma, tab
  size_t nextQuote, firstNonSpace, nextSeparator, validSeparator;

  wcscpy(program, TEXT("\0"));
  wcscpy(arguments, TEXT("\0"));

  workingApp = appToParse;
  if ((workingApp.empty()) || (workingApp == TEXT("@")))
  {
    return false;
  }

  if (workingApp.at(0) == '@')
  {
    workingApp = workingApp.substr(1);
  }

  // Parse application string of form: "Command" arguments
  // Check to see if the string starts with " and contains a second "
  nextQuote = workingApp.find_first_of('"', 2);
  if ((*workingApp.begin() == '"') && (nextQuote != std::wstring::npos))
  {
    // If so, first define the arguments
    if (nextQuote < (workingApp.length() - 1))
    {
      workingArgs = workingApp.substr(nextQuote + 1, workingApp.length() - nextQuote);
      // Strip any leading spaces from the arguments
      firstNonSpace = workingArgs.find_first_not_of(L" \t");
      if (firstNonSpace != std::wstring::npos)
        workingArgs = workingArgs.substr(firstNonSpace, workingArgs.length() - firstNonSpace);
    }
      // Finally define the application
      workingApp = workingApp.substr(1, nextQuote - 1);

      wcscpy(program, workingApp.c_str());
      wcscpy(arguments, workingArgs.c_str());
  }
  else
  {
    //Calculate the path/args manually:
    //Check workingApp for any separator
    //nextSeparator = workingApp.find_first_of(pathArgsSeparator);
    nextSeparator = 0;
    validSeparator = std::wstring::npos;
    do
    {
      nextSeparator = workingApp.find_first_of(pathArgsSeparator, nextSeparator + 1);

      // If a separator is found, check with extension to see if it's valid
      if (nextSeparator == std::wstring::npos)
      {
        tempPath = workingApp;
      }
      else
      {
        tempPath = workingApp.substr(0, nextSeparator);
      }

      specialFlags = ELGetFileSpecialFlags(tempPath);
      if (((specialFlags & SF_CLSID) == SF_CLSID) || ((specialFlags & SF_ALIAS) == SF_ALIAS) || ((specialFlags & SF_INTERNALCOMMAND) == SF_INTERNALCOMMAND) || ((specialFlags & SF_URL) == SF_URL))
      {
        wcscpy(program, tempPath.c_str());
        validSeparator = nextSeparator;
      }

      if (specialFlags == SF_NOTHING)
      {
        searchPath = ELExhaustivelyFindFilePath(tempPath);
        if (!searchPath.empty())
        {
          tempPath = searchPath;

          specialFlags = ELGetFileSpecialFlags(tempPath);
        }
        else
        {
          if (tempPath == workingApp)
          {
            tempPath = ELExpandVars(tempPath);
            tempPath = ELGetAbsolutePath(tempPath);

            specialFlags = ELGetFileSpecialFlags(tempPath);
          }
        }
      }

      // Bail if workingApp is a directory, UNC Path or it exists
      if (((specialFlags & SF_DIRECTORY) == SF_DIRECTORY) || ((specialFlags & SF_SPECIALFOLDER) == SF_SPECIALFOLDER) || ((specialFlags & SF_UNC) == SF_UNC) || ELFileExists(tempPath))
      {
        wcscpy(program, tempPath.c_str());
        validSeparator = nextSeparator;
      }

      // Loop to the next separator
      //nextSeparator = workingApp.find_first_of(pathArgsSeparator, nextSeparator + 1);
    } while (nextSeparator != std::wstring::npos);

    // If the validSeparator is set, copy the arguments based on its position
    if (validSeparator != std::wstring::npos)
    {
      workingArgs = workingApp.substr(validSeparator, workingApp.length() - validSeparator).c_str();
      // Strip any leading spaces from the arguments
      firstNonSpace = workingArgs.find_first_not_of(L" \t");
      if (firstNonSpace != std::wstring::npos)
        workingArgs = workingArgs.substr(firstNonSpace, workingArgs.length() - firstNonSpace);

      wcscpy(arguments, workingArgs.c_str());
    }
  }

  return (wcslen(program) > 0);
}

bool ELParseShortcut(std::wstring shortcut, LPSHORTCUTINFO shortcutInfo)
{
  IShellLink* psl = NULL;
  IShellLinkDataList* psdl = NULL;
  IPersistFile* ppf = NULL;
  LPITEMIDLIST pidl = NULL;
  LPVOID lpVoid;
  DWORD dwFlags;
  bool ret = true;

  if (FAILED(CoInitialize(NULL)))
  {
    return false;
  }

  // Get a pointer to the IShellLink interface.
  if (FAILED(CoCreateInstance(CLSID_ShellLink, NULL,
                              CLSCTX_INPROC_SERVER, IID_IShellLink,
                              &lpVoid)))
  {
    CoUninitialize();
    return false;
  }
  psl = reinterpret_cast <IShellLink*> (lpVoid);

  if (FAILED(psl->QueryInterface(IID_IShellLinkDataList, &lpVoid)))
  {
    psl->Release();
    CoUninitialize();
    return false;
  }
  psdl = reinterpret_cast <IShellLinkDataList*> (lpVoid);

  // Get a pointer to the IPersistFile interface.
  if (FAILED(psl->QueryInterface(IID_IPersistFile,
                                 &lpVoid)))
  {
    psl->Release();
    psdl->Release();
    CoUninitialize();
    return false;
  }
  ppf = reinterpret_cast <IPersistFile*> (lpVoid);

  // Load the shortcut.
  if (FAILED(ppf->Load(shortcut.c_str(), STGM_READ)))
  {
    ppf->Release();
    psl->Release();
    psdl->Release();
    CoUninitialize();
    return false;
  }

  if (FAILED(psl->Resolve(NULL, SLR_INVOKE_MSI | SLR_NOUPDATE | SLR_NO_UI)))
  {
    ppf->Release();
    psl->Release();
    psdl->Release();
    CoUninitialize();
    return false;
  }

  // Get the path to the link target.
  if ((shortcutInfo->flags & SI_RUNAS) == SI_RUNAS)
  {
    shortcutInfo->runAs = false;

    if (SUCCEEDED(psdl->GetFlags(&dwFlags)))
    {
      if ((dwFlags & SLDF_RUNAS_USER) == SLDF_RUNAS_USER)
      {
        shortcutInfo->runAs = true;
      }
    }
  }

  // Get the path to the link target.
  if ((shortcutInfo->flags & SI_PATH) == SI_PATH)
  {
    if (SUCCEEDED(psl->GetIDList(&pidl)))
    {
      ret = (SHGetPathFromIDList(pidl, shortcutInfo->Path) == TRUE);
      CoTaskMemFree(pidl);
    }
  }

  // Get the arguments for the link target.
  if ((shortcutInfo->flags & SI_ARGUMENTS) == SI_ARGUMENTS)
  {
    if (FAILED(psl->GetArguments(shortcutInfo->Arguments, MAX_PATH)))
    {
      ret = false;
    }
  }

  // Get the working directory of the link target
  if ((shortcutInfo->flags & SI_WORKINGDIR) == SI_WORKINGDIR)
  {
    if (FAILED(psl->GetWorkingDirectory(shortcutInfo->WorkingDirectory, MAX_PATH)))
    {
      ret = false;
    }
  }

  // Get the icon info for the link target
  if ((shortcutInfo->flags & SI_ICONPATH) == SI_ICONPATH)
  {
    if (FAILED(psl->GetIconLocation(shortcutInfo->IconPath, MAX_PATH, &shortcutInfo->IconIndex)))
    {
      ret = false;
    }
  }

  // Get the show value of the link target
  if ((shortcutInfo->flags & SI_SHOW) == SI_SHOW)
  {
    if (FAILED(psl->GetShowCmd(&shortcutInfo->ShowCmd)))
    {
      ret = false;
    }
  }

  ppf->Release();
  psl->Release();
  psdl->Release();
  CoUninitialize();
  return ret;
}

std::wstring ELGetFileName(std::wstring filePath)
{
  WCHAR tempFile[MAX_PATH];
  WCHAR* tempFilePtr;

  wcscpy(tempFile, filePath.c_str());
  tempFilePtr = tempFile;
  wcscpy(tempFile, PathFindFileName(tempFilePtr));

  return tempFile;
}

std::wstring ELGetFileExtension(std::wstring filePath)
{
  WCHAR tempFile[MAX_PATH];
  WCHAR* tempFilePtr;

  wcscpy(tempFile, filePath.c_str());
  tempFilePtr = tempFile;
  wcscpy(tempFile, PathFindExtension(tempFilePtr));

  return tempFile;
}

std::wstring ELStripFileExtension(std::wstring fileName)
{
  WCHAR tempBuffer[MAX_PATH];

  if (fileName.empty())
  {
    return TEXT("");
  }

  wcscpy(tempBuffer, fileName.c_str());
  PathRemoveExtension(tempBuffer);

  return tempBuffer;
}

std::wstring ELGetFileArguments(std::wstring filePath)
{
  std::wstring workingPath = filePath;
  LPWSTR* argList;
  int argCount;
  std::wstring tempArgs;

  if (workingPath.empty()) //CommandLineToArgvW breaks if the lpCmdLine parameter is empty
    workingPath = ELGetCurrentPath();

  argList = CommandLineToArgvW(workingPath.c_str(), &argCount);

  for (int counter = 1; counter < argCount; counter++) //ignore the first element (counter = 0) because that's the base app, not args
  {
    tempArgs = tempArgs + TEXT(" ") + argList[counter];
  }

  LocalFree(argList);

  return tempArgs;
}

std::wstring ELStripFileArguments(std::wstring filePath)
{
  std::wstring workingPath = filePath;
  LPWSTR* argList;
  int argCount;
  std::wstring tempPath = TEXT("");

  if (workingPath.empty()) //CommandLineToArgvW breaks if the lpCmdLine parameter is empty
    workingPath = ELGetCurrentPath();

  argList = CommandLineToArgvW(workingPath.c_str(), &argCount);

  if (argCount > 0)
  {
    tempPath = argList[0]; //ignore any elements after the first because those are args, not the base app
  }

  LocalFree(argList);

  return tempPath;
}

std::wstring ELGetAbsolutePath(std::wstring relativeFilePath, std::wstring baseDirPath)
{
  std::wstring fullyExpandedRelativePath = ELExpandVars(stripSpecialChars(relativeFilePath));
  std::wstring fullyExpandedBasePath = ELExpandVars(stripSpecialChars(baseDirPath));
  WCHAR originalWorkingDir[MAX_PATH] = TEXT("");
  WCHAR tmpPath[MAX_PATH];

  if ((!fullyExpandedBasePath.empty()) && (GetCurrentDirectory(MAX_PATH, originalWorkingDir)))
  {
    SetCurrentDirectory(fullyExpandedBasePath.c_str());
  }

  GetFullPathName(fullyExpandedRelativePath.c_str(), MAX_PATH, tmpPath, NULL);

  if (_wcsicmp(originalWorkingDir, TEXT("")) != 0)
  {
    SetCurrentDirectory(originalWorkingDir);  //restore the original working directory if it's been saved
  }

  return tmpPath;
}

std::wstring ELGetRelativePath(std::wstring filePath, std::wstring baseDirPath)
{
  std::wstring fullyExpandedBasePath = ELExpandVars(baseDirPath);
  std::wstring workingFilePath = filePath;
  WCHAR tmpPath[MAX_PATH], program[MAX_PATH], arguments[MAX_LINE_LENGTH];
  std::wstring unc;
  DWORD flags;

  // Convert %AppletDir% to relative path
  if (workingFilePath.find(TEXT("%AppletDir%")) != std::wstring::npos)
  {
    workingFilePath = ELExpandVars(workingFilePath);
  }

  // If dstPath is not equal to dstPath after var expansion, then destPath is
  // already defined as a variable, so don't convert it.
  if (workingFilePath != ELExpandVars(workingFilePath))
  {
    return workingFilePath;
  }

  //PathIsRelative treats "\" as a fully qualified path, which isn't true (except for UNC paths); check for a drive number first.
  if ((PathGetDriveNumber(workingFilePath.c_str()) == -1) && !PathIsUNC(workingFilePath.c_str()))
  {
    return workingFilePath;
  }

  // Separate the program and the arguments for conversion
  if (!ELParseCommand(workingFilePath, program, arguments))
  {
    return workingFilePath;
  }

  if (ELIsDirectory(program))
  {
    flags = FILE_ATTRIBUTE_DIRECTORY;
  }
  else
  {
    flags = FILE_ATTRIBUTE_NORMAL;
  }

  // Resolve path and srcPath to their UNC equivalents if appropriate
  unc = ELGetUNCFromMap(program);
  if (!unc.empty())
  {
    wcscpy(program, unc.c_str());
  }
  unc = ELGetUNCFromMap(fullyExpandedBasePath.c_str());
  if (!unc.empty())
  {
    fullyExpandedBasePath = unc;
  }

  if (PathRelativePathTo(tmpPath, fullyExpandedBasePath.c_str(), FILE_ATTRIBUTE_DIRECTORY,
                         program, flags))
  {
    // If the file is stored in the current directory, the PathRelativePathTo
    // prepends the string with '\' making windows think the file is in the
    // root directory, so I've implemented the change below to account for
    // that.
    workingFilePath = tmpPath;
    if (workingFilePath.at(0) == '\\')
    {
      workingFilePath = workingFilePath.substr(1);
    }
  }
  else
  {
    workingFilePath = program;
  }

  if (wcslen(arguments))
  {
    workingFilePath = workingFilePath + TEXT(" ");
    workingFilePath = workingFilePath + arguments;
  }

  return workingFilePath;
}

bool ELPathIsRelative(std::wstring filePath)
{
  /*std::wstring relativePath;

  if (filePath.empty())
  {
    return true;
  }

  relativePath = ELGetRelativePath(filePath);
  return (ELToLower(relativePath) == ELToLower(filePath));*/
  std::wstring tmpPath = filePath;
  tmpPath = ELExpandVars(tmpPath);

  // PathIsRelative treats "\" as a fully qualified path, which isn't true
  // (except for UNC); therefore, check for a drive number first.
  return ((PathGetDriveNumber(tmpPath.c_str()) == -1) &&
    !PathIsUNC(tmpPath.c_str()));
}

std::wstring ELExhaustivelyFindFilePath(std::wstring filePath)
{
  std::wstring tempPath = filePath;
  WCHAR pathext[MAX_LINE_LENGTH], *token;

  tempPath = ELExpandVars(tempPath);
  tempPath = ELGetAbsolutePath(tempPath);
  if ((ELFileExists(tempPath)) && (!ELIsDirectory(tempPath)))
  {
    return tempPath;
  }

  tempPath = filePath;

  tempPath = ELGetFileExtension(tempPath);
  if (!tempPath.empty())
  {
    return FindFileOnPATH(filePath);
  }
  else
  {
    GetEnvironmentVariable(TEXT("PATHEXT"), pathext, MAX_LINE_LENGTH);
    wcscat(pathext, TEXT(";.LNK"));
    token = wcstok(pathext, TEXT(";"));
    while (token != NULL)
    {
      tempPath = filePath;
      tempPath = tempPath + token;

      tempPath = FindFileOnPATH(tempPath);
      if (!tempPath.empty())
      {
        return tempPath;
      }

      token = wcstok(NULL, TEXT(";"));
    }
  }

  return TEXT("");
}

std::vector<std::wstring> ELGetFilesInFolder(std::wstring folder, std::wstring fileMask, bool fullPath)
{
  std::vector<std::wstring> fileList;
  std::wstring fileName;
  WIN32_FIND_DATA fileSearchData;
  HANDLE searchHandle;
  std::wstring fullSearchQuery = folder + fileMask;

  searchHandle = FindFirstFile(fullSearchQuery.c_str(), &fileSearchData);
  if (searchHandle != INVALID_HANDLE_VALUE)
  {
    if (fullPath)
    {
      fileName = folder;
      fileName = fileName + fileSearchData.cFileName;
    }
    else
    {
      fileName = fileSearchData.cFileName;
    }
    fileList.push_back(fileName);

    while (FindNextFile(searchHandle, &fileSearchData) != 0)
    {
      if (fullPath)
      {
        fileName = folder;
        fileName = fileName + fileSearchData.cFileName;
      }
      else
      {
        fileName = fileSearchData.cFileName;
      }
      fileList.push_back(fileName);
    }
  }

  FindClose(searchHandle);

  return fileList;
}

bool ELFileExists(std::wstring filePath)
{
  std::wstring tempFile;

  if (filePath.empty())
  {
    return false;
  }

  tempFile = filePath;

  if ((tempFile.at(0) == '"') && (tempFile.at(tempFile.length() - 1) == '"'))
  {
    tempFile = tempFile.substr(1, tempFile.length() - 2);
  }

  return (PathFileExists(tempFile.c_str()) && !PathIsRelative(tempFile.c_str()));
}

int ELGetFileSpecialFlags(std::wstring filePath)
{
  std::wstring workingPath = filePath;
  int flags = SF_NOTHING;

  if (workingPath.empty())
  {
    return SF_NOTHING;
  }

  if (IsCLSID(workingPath))
  {
    flags = (flags | SF_CLSID);
  }

  if (ELIsDirectory(workingPath))
  {
    flags = (flags | SF_DIRECTORY);
  }

  if (IsAlias(workingPath))
  {
    flags = (flags | SF_ALIAS);
  }

  if (ELIsInternalCommand(workingPath))
  {
    flags = (flags | SF_INTERNALCOMMAND);
  }

  if (IsShortcut(workingPath))
  {
    flags = (flags | SF_SHORTCUT);
  }

  if ((ELGetSpecialFolderIDFromPath(workingPath) != 0) || (ELGetSpecialFolderIDFromName(workingPath) != 0))
  {
    flags = (flags | SF_SPECIALFOLDER);
  }

  if (IsURL(workingPath))
  {
    flags = (flags | SF_URL);
  }

  if ((flags == SF_NOTHING) && (ELFileExists(workingPath)))
  {
    flags = SF_FILE;
  }

  return flags;
}

DLL_EXPORT bool ELIsDirectory(std::wstring filePath)
{
  if (filePath.empty())
  {
    return false;
  }

  return (PathIsDirectory(filePath.c_str()) || PathIsUNCServerShare(filePath.c_str()) || PathIsUNCServer(filePath.c_str()));
}

std::wstring ELGetFileTypeCommand(std::wstring document, std::wstring docArgs)
{
  std::wstring extension = ELGetFileExtension(document);
  std::wstring quotedDoc, commandLine;
  WCHAR docExecutable[MAX_LINE_LENGTH], shortDoc[MAX_LINE_LENGTH];
  DWORD bufferSize = MAX_LINE_LENGTH;
  int specialFlags = ELGetFileSpecialFlags(document);

  if ((specialFlags & SF_DIRECTORY) == SF_DIRECTORY)
  {
    extension = TEXT("Folder");
  }

  // Don't attempt to determine URL handler
  if ((specialFlags & SF_URL) == SF_URL)
  {
    return TEXT("");
  }

  GetShortPathName(document.c_str(), shortDoc, MAX_LINE_LENGTH);
  quotedDoc = TEXT("\"") + document + TEXT("\"");

  if (ELIsFileTypeExecutable(extension))
  {
    return TEXT("");
  }

  if (FAILED(AssocQueryString(ASSOCF_NOTRUNCATE, ASSOCSTR_COMMAND,
                              extension.c_str(), NULL, docExecutable, &bufferSize)))
  {
    return TEXT("");
  }

  commandLine = docExecutable;
  commandLine = ELwstringReplace(commandLine, TEXT("/idlist,%I,"), TEXT(""), true);
  commandLine = ELwstringReplace(commandLine, TEXT("%*"), docArgs, false);
  commandLine = ELwstringReplace(commandLine, TEXT("%1"), shortDoc, false);
  if ((specialFlags & SF_DIRECTORY) == SF_DIRECTORY)
  {
    commandLine = ELwstringReplace(commandLine, TEXT("%L"), quotedDoc, true);
  }
  else
  {
    commandLine = ELwstringReplace(commandLine, TEXT("%L"), document, true);
  }

  if (ELToLower(commandLine) == ELToLower(docExecutable))
  {
    commandLine = docExecutable;
    commandLine = commandLine + TEXT("\"");
    commandLine = commandLine + document;
    commandLine = commandLine + TEXT("\"");
  }

  return commandLine;
}

std::wstring ELGetSpecialFolderNameFromPath(std::wstring folderPath)
{
  LPITEMIDLIST pidl = NULL;
  SHFILEINFO fileInfo;
  std::wstring returnValue;
  std::map<int, std::wstring> specialFolderMap = GetSpecialFolderMap();
  std::map<int, std::wstring>::iterator specialFolderMapIter;

  for (specialFolderMapIter = specialFolderMap.begin(); specialFolderMapIter != specialFolderMap.end(); ++specialFolderMapIter)
  {
    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, specialFolderMapIter->first, &pidl)))
    {
      SHGetFileInfo((LPCTSTR)pidl, 0, &fileInfo, sizeof(fileInfo), SHGFI_PIDL | SHGFI_DISPLAYNAME);
      if (_wcsicmp(folderPath.c_str(), fileInfo.szDisplayName) == 0)
      {
        returnValue = specialFolderMapIter->second;
      }
      CoTaskMemFree(pidl);
    }
  }

  return returnValue;
}

std::wstring ELGetSpecialFolderNameFromID(int specialFolderID)
{
  std::map<int, std::wstring> specialFolderMap = GetSpecialFolderMap();
  std::map<int, std::wstring>::iterator specialFolderMapIter;

  for (specialFolderMapIter = specialFolderMap.begin(); specialFolderMapIter != specialFolderMap.end(); ++specialFolderMapIter)
  {
    if (specialFolderID == specialFolderMapIter->first)
    {
      return specialFolderMapIter->second;
    }
  }

  return TEXT("");
}

int ELGetSpecialFolderIDFromName(std::wstring specialFolderName)
{
  int csidl = 0;
  std::map<int, std::wstring> specialFolderMap = GetSpecialFolderMap();
  std::map<int, std::wstring>::iterator specialFolderMapIter;

  for (specialFolderMapIter = specialFolderMap.begin(); specialFolderMapIter != specialFolderMap.end(); ++specialFolderMapIter)
  {
    if (ELToLower(specialFolderName) == ELToLower(specialFolderMapIter->second))
    {
      csidl = specialFolderMapIter->first;
    }
  }

  return csidl;
}

std::wstring ELGetSpecialFolderPathFromID(int specialFolderID)
{
  LPITEMIDLIST pidl;
  SHFILEINFO fileInfo;
  std::wstring folderPath;

  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, specialFolderID, &pidl)))
  {
    SHGetFileInfo((LPCTSTR)pidl, 0, &fileInfo, sizeof(fileInfo), SHGFI_PIDL | SHGFI_DISPLAYNAME);
    folderPath = fileInfo.szDisplayName;
    CoTaskMemFree(pidl);
    return folderPath;
  }

  return TEXT("");
}

std::wstring ELGetSpecialFolderPathFromName(std::wstring specialFolderName)
{
  int specialFolderID = ELGetSpecialFolderIDFromName(specialFolderName);

  return ELGetSpecialFolderPathFromID(specialFolderID);
}

int ELGetSpecialFolderIDFromPath(std::wstring folderPath)
{
  LPITEMIDLIST pidl = NULL;
  SHFILEINFO fileInfo;
  int csidl = 0;
  std::map<int, std::wstring> specialFolderMap = GetSpecialFolderMap();
  std::map<int, std::wstring>::iterator specialFolderMapIter;

  for (specialFolderMapIter = specialFolderMap.begin(); specialFolderMapIter != specialFolderMap.end(); ++specialFolderMapIter)
  {
    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, specialFolderMapIter->first, &pidl)))
    {
      SHGetFileInfo((LPCTSTR)pidl, 0, &fileInfo, sizeof(fileInfo), SHGFI_PIDL | SHGFI_DISPLAYNAME);
      if (_wcsicmp(folderPath.c_str(), fileInfo.szDisplayName) == 0)
      {
        csidl = specialFolderMapIter->first;
      }
      CoTaskMemFree(pidl);
    }
  }

  return csidl;
}

std::wstring ELGetUNCFromMap(std::wstring uncMap)
{
  if (emergeLibGlobals::getMprDLL())
  {
    if (MSWNetGetConnection == NULL)
    {
      MSWNetGetConnection = (fnWNetGetConnection)GetProcAddress(emergeLibGlobals::getMprDLL(), (LPCSTR)"WNetGetConnectionW");
    }
  }

  if (MSWNetGetConnection)
  {
    WCHAR tmp[MAX_PATH];
    DWORD tmpLength = MAX_PATH;
    std::wstring workingMap = uncMap, drive, unc;
    size_t colon = workingMap.find(':');
    if (colon != std::wstring::npos)
    {
      drive = workingMap.substr(0, colon + 1);
      if (MSWNetGetConnection(drive.c_str(), tmp, &tmpLength) == NO_ERROR)
      {
        unc = tmp;
        unc = unc + workingMap.substr(colon + 1, workingMap.length() - colon);
        return unc;
      }
    }
  }

  return TEXT("");
}

bool ELIsFileTypeExecutable(std::wstring fileExtension)
{
  WCHAR pathext[MAX_PATH];
  std::wstring extension, executableExtensions;

  GetEnvironmentVariable(TEXT("PATHEXT"), pathext, MAX_LINE_LENGTH);

  executableExtensions = pathext;

  extension = ELToLower(fileExtension);
  executableExtensions = ELToLower(executableExtensions);

  return (executableExtensions.find(extension) != std::wstring::npos);
}

bool ELExecuteFileOrCommand(std::wstring application, std::wstring workingDir, int nShow, std::wstring verb)
{
  bool result = false;
  WCHAR workingApp[MAX_PATH], workingArgs[MAX_PATH];
  int specialFlags;
  std::wstring fullApplicationPath;

  if (!ELParseCommand(application, workingApp, workingArgs))
  {
    return false;
  }

  specialFlags = ELGetFileSpecialFlags(workingApp);

  if ((specialFlags & SF_INTERNALCOMMAND) == SF_INTERNALCOMMAND)
  {
    result = ELExecuteInternalCommand(workingApp, workingArgs);
  }
  else if ((specialFlags & SF_ALIAS) == SF_ALIAS)
  {
    result = ExecuteAlias(workingApp);
  }
  else if ((specialFlags & SF_SPECIALFOLDER) == SF_SPECIALFOLDER)
  {
    result = ExecuteSpecialFolder(workingApp);
  }
  else
  {
    fullApplicationPath = application;
    result = Execute(fullApplicationPath, workingDir, nShow, verb);
  }

  return result;
}

bool ELFileOp(HWND appletWnd, bool feedback, UINT function, std::wstring source, std::wstring destination)
{
  SHFILEOPSTRUCT fileOpStruct;
  WCHAR* fromString = NULL, *toString = NULL;
  bool ret = false;

  source = ELExpandVars(source);
  destination = ELExpandVars(destination);
  ZeroMemory(&fileOpStruct, sizeof(fileOpStruct));

  fileOpStruct.hwnd = appletWnd;
  fileOpStruct.wFunc = function;
  if (!feedback)
  {
    fileOpStruct.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI;
  }

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
  {
    ret = true;
  }

  if (destination.length() != 0)
  {
    GlobalFree(toString);
  }
  GlobalFree(fromString);

  return ret;
}

bool IsAlias(std::wstring filePath)
{
  if (filePath.length() < 2)
  {
    return false;
  }

  return (filePath.at(0) == '.');
}

bool IsCLSID(std::wstring filePath)
{
  if (filePath.length() < 3)
  {
    return false;
  }

  return ((filePath.at(0) == ':') && (filePath.at(1) == ':') && (filePath.at(2) == '{'));
}

bool IsShortcut(std::wstring filePath)
{
  SHORTCUTINFO shortcutInfo;

  return (ELParseShortcut(filePath, &shortcutInfo));
}

bool IsURL(std::wstring filePath)
{
  if (filePath.empty())
  {
    return false;
  }

  return (PathIsURL(filePath.c_str()) == TRUE);
}

std::wstring FindFileOnPATH(std::wstring path)
{
  WCHAR tempBuffer[MAX_LINE_LENGTH], sysWOW64[MAX_PATH], currentWorkingDir[MAX_PATH];
  const WCHAR* corePathPtr[4];
  std::wstring tempPath, corePath;

  tempPath = ELExpandVars(path);

  if (FindFilePathFromRegistry(tempPath) != TEXT(""))
  {
    tempPath = FindFilePathFromRegistry(tempPath);
  }

  if ((ELFileExists(tempPath)) && (!ELIsDirectory(tempPath)))
  {
    return tempPath;
  }

  corePath = ELGetCurrentPath();

  GetCurrentDirectory(MAX_PATH, currentWorkingDir);
  swprintf(currentWorkingDir, TEXT("%ls\\%ls"), currentWorkingDir, tempPath.c_str());

  ExpandEnvironmentStrings((LPCTSTR)TEXT("%systemroot%\\SysWOW64"),
                           (LPTSTR)sysWOW64,
                           MAX_PATH);
  swprintf(sysWOW64, TEXT("%ls\\%ls"), sysWOW64, tempPath.c_str());

  corePathPtr[0] = corePath.c_str();
  corePathPtr[1] = currentWorkingDir;
  corePathPtr[2] = sysWOW64;
  corePathPtr[3] = NULL;

  wcscpy(tempBuffer, tempPath.c_str());
  if ((PathFindOnPath(tempBuffer, corePathPtr)) && (!ELIsDirectory(tempBuffer)))
  {
    return tempBuffer;
  }

  return TEXT("");
}

std::wstring FindFilePathFromRegistry(std::wstring program)
{
  std::wstring appString;
  DWORD bufferSize = MAX_LINE_LENGTH;
  WCHAR tempPath[MAX_LINE_LENGTH];
  HKEY key;
  bool success = false;

  appString = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\");
  appString = appString + program;

  if ((ELGetFileExtension(program)).empty())
  {
    appString = appString + TEXT(".exe");
  }

  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, appString.c_str(), 0, KEY_READ, &key) == ERROR_SUCCESS)
  {
    success = (RegQueryValueEx(key, NULL, NULL, NULL, (BYTE*)tempPath, &bufferSize) == ERROR_SUCCESS);
    RegCloseKey(key);
  }

  return (success ? tempPath : TEXT(""));
}

std::wstring GetSpecialFolderGUID(int folderID)
{
  IShellFolder* pDesktop, *pFolder;
  LPITEMIDLIST pidl;
  IPersistFolder* pPersist;
  CLSID clsID;
  LPVOID lpVoid;
  WCHAR* GUIDString;
  WCHAR returnValue[MAX_PATH] = TEXT("\0");

  if (SUCCEEDED(CoInitialize(NULL)))
  {
    if (SUCCEEDED(SHGetFolderLocation(NULL, folderID, NULL, 0, &pidl)))
    {
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
                wcscpy(returnValue, GUIDString);
                CoTaskMemFree(GUIDString);
              }
            }

            pPersist->Release();
          }

          pFolder->Release();
        }

        pDesktop->Release();
      }

      ILFree(pidl);
    }

    CoUninitialize();
  }

  return returnValue;
}

std::map<int, std::wstring> GetSpecialFolderMap()
{
  std::map<int, std::wstring> specialFolderMap;

  specialFolderMap.insert(std::pair<int, std::wstring>(CSIDL_PERSONAL, TEXT("CSIDL_PERSONAL")));
  specialFolderMap.insert(std::pair<int, std::wstring>(CSIDL_DRIVES, TEXT("CSIDL_DRIVES")));
  specialFolderMap.insert(std::pair<int, std::wstring>(CSIDL_CONTROLS, TEXT("CSIDL_CONTROLS")));
  specialFolderMap.insert(std::pair<int, std::wstring>(CSIDL_BITBUCKET, TEXT("CSIDL_BITBUCKET")));
  specialFolderMap.insert(std::pair<int, std::wstring>(CSIDL_NETWORK, TEXT("CSIDL_NETWORK")));

  return specialFolderMap;
}

bool Execute(std::wstring application, std::wstring workingDir, int nShow, std::wstring verb)
{
  WCHAR program[MAX_PATH], workingDirectory[MAX_PATH], arguments[MAX_LINE_LENGTH];
  WCHAR commandLine[MAX_LINE_LENGTH];
  std::wstring workingString = application;
  std::wstring workingVerb = verb;
  int specialFlags;
  SHORTCUTINFO shortcutInfo;
  SHELLEXECUTEINFO sei;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&sei, sizeof(sei));
  ZeroMemory(&si, sizeof(si));
  ZeroMemory(&pi, sizeof(pi));
  ZeroMemory(program, MAX_PATH);
  ZeroMemory(workingDirectory, MAX_PATH);
  ZeroMemory(arguments, MAX_LINE_LENGTH);

  if (!workingString.empty())
  {
    if (workingString.at(0) == '@')
    {
      workingVerb = TEXT("runas");
      workingString = workingString.substr(1);
    }
  }
  workingString = ELExpandVars(workingString);

  if (ELParseCommand(workingString, program, arguments))
  {
    workingString = program;
  }
  else
  {
    return false;
  }

  shortcutInfo.flags = SI_PATH | SI_ARGUMENTS | SI_WORKINGDIR | SI_SHOW | SI_RUNAS;
  if (ELParseShortcut(workingString.c_str(), &shortcutInfo))
  {
    if (shortcutInfo.runAs)
    {
      workingVerb = TEXT("runas");
    }

    wcscpy(program, shortcutInfo.Path);
    wcscpy(arguments, shortcutInfo.Arguments);
    wcscpy(workingDirectory, shortcutInfo.WorkingDirectory);
    nShow = shortcutInfo.ShowCmd;

    // 0x800 is an undocumented flag that tells the new process that lpTitle
    // is actually the path of a shortcut file. The new console process can
    // then extract the icon and title from the shortcut it was launched from.
    //
    // For console shortcuts, the title is key because that's what used to
    // determine window dimensions and buffers (similar to when you change the
    // default options you are given the choice to 'Save properties for future
    // windows with the same title'.
    si.dwFlags = 0x800;
    si.lpTitle = (WCHAR*)workingString.c_str();
  }
  else
  {
    if (workingDir.empty())
    {
      wcscpy(workingDirectory, workingString.c_str());
      PathRemoveFileSpec(workingDirectory);
    }
    else
    {
      wcscpy(workingDirectory, workingDir.c_str());
    }
  }

  si.cb = sizeof(si);
  // Be sure to use the nShow value irregardless of shortcut.
  si.dwFlags |= STARTF_USESHOWWINDOW;
  si.wShowWindow = nShow;

  // Expand variables again so as to not confuse CreateProcess and ShellExecuteEx
  wcscpy(program, ELExpandVars(program).c_str());

  if (wcslen(workingDirectory) > 0)
  {
    wcscpy(workingDirectory, ELExpandVars(workingDirectory).c_str());
  }

  // If the program doesn't exist or is not a URL return false so as to not
  // generate stray DDE calls
  specialFlags = ELGetFileSpecialFlags(program);
  if ((!ELFileExists(program)) && ((specialFlags & SF_URL) != SF_URL) && ((specialFlags & SF_CLSID) != SF_CLSID))
  {
    return false;
  }

  wcscpy(commandLine, ELGetFileTypeCommand(program, arguments).c_str());
  if (wcslen(commandLine) == 0)
  {
    swprintf(commandLine, TEXT("\"%ls\" %ls"), program, arguments);
  }

  if (wcslen(commandLine) <= 2) //At the very least, the swprintf statement should leave command with two chars - the double quote pair
  {
    return false;
  }

  if (workingVerb.empty())
  {
    if (CreateProcess(NULL,
                      commandLine,
                      NULL,
                      NULL,
                      FALSE,
                      NORMAL_PRIORITY_CLASS,
                      NULL,
                      workingDirectory,
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
  sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC | SEE_MASK_UNICODE;
  sei.lpFile = program;
  sei.lpParameters = arguments;
  sei.lpDirectory = workingDirectory;
  sei.lpVerb = workingVerb.c_str();
  sei.nShow = nShow;

  return (ShellExecuteEx(&sei) == TRUE);
}

bool ExecuteAlias(std::wstring alias)
{
  bool ret = false;
  std::wstring aliasFile = TEXT("%EmergeDir%\\files\\alias.txt");
  aliasFile = ELExpandVars(aliasFile);

  if (!ELFileExists(aliasFile))
  {
    ELFileOp(NULL, false, FO_COPY, TEXT("%EmergeDir%\\files\\cmd.txt"), aliasFile);
  }

  if (alias.length() < 2)
    return false;

  if (alias.at(0) == '.')
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

          if (command != NULL)
            {
              ELStripLeadingSpaces(command);

              // execute the command
              if (ELToLower(value) == ELToLower(alias))
                {
                  ret = ELExecuteFileOrCommand(command);
                  break;
                }
            }
        }

      fclose(fp);
    }

  return ret;
}

bool ExecuteSpecialFolder(std::wstring folder)
{
  int specialFolderID = ELGetSpecialFolderIDFromName(folder);
  LPITEMIDLIST pidl = NULL;
  SHELLEXECUTEINFO sei;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  bool returnValue = false;
  std::wstring explorer = TEXT("%WINDIR%\\explorer.exe");
  WCHAR command[MAX_LINE_LENGTH];
  std::wstring guid, classID;
  DWORD bufferSize = MAX_LINE_LENGTH;

  if (specialFolderID == 0)
  {
    specialFolderID = ELGetSpecialFolderIDFromPath(folder);
  }

  if (specialFolderID == 0)
  {
    return false;
  }

  if (SUCCEEDED(AssocQueryString(ASSOCF_NOTRUNCATE, ASSOCSTR_COMMAND,
                                 TEXT("Folder"), NULL, command, &bufferSize)) && (ELOSVersionInfo() < 6.0))
  {
    explorer = ELToLower(ELExpandVars(explorer));
    _wcslwr(command);
    classID = TEXT("::");
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    if (wcsstr(command, explorer.c_str()) != NULL)
    {
      if (specialFolderID == CSIDL_CONTROLS)
      {
        guid = GetSpecialFolderGUID(CSIDL_DRIVES);
        if (!guid.empty())
        {
          classID = classID + guid;
          classID = classID + TEXT("\\::");
        }
      }

      guid = GetSpecialFolderGUID(specialFolderID);
        if (!guid.empty())
        {
          classID = classID + guid;
        }

      ELStringReplace(command, (WCHAR*)TEXT("/idlist,%I,"), (WCHAR*)TEXT(""), true);
      ELStringReplace(command, (WCHAR*)TEXT("%L"), classID.c_str(), true);

      si.cb = sizeof(si);
      si.dwFlags |= STARTF_USESHOWWINDOW;
      si.wShowWindow = SW_SHOW;

      if (CreateProcess(NULL,
                        command,
                        NULL,
                        NULL,
                        FALSE,
                        NORMAL_PRIORITY_CLASS,
                        NULL,
                        NULL,
                        &si,
                        &pi))
      {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
      }
    }
  }

  if (SUCCEEDED(SHGetFolderLocation(NULL, specialFolderID, NULL, 0, &pidl)))
  {
    ZeroMemory(&sei, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_IDLIST | SEE_MASK_FLAG_NO_UI | SEE_MASK_UNICODE;
    sei.nShow = SW_SHOW;
    sei.lpIDList = pidl;
    returnValue = (ShellExecuteEx(&sei) == TRUE);

    ILFree(pidl);
  }

  return returnValue;
}

std::wstring stripSpecialChars(std::wstring filePath)
{
  std::wstring workingPath = filePath;
  bool successfulMatch = true;

  while (successfulMatch)
  {
    successfulMatch = false;

    if ((workingPath.size() >= 1) && (workingPath.at(0) == ' '))
    {
      workingPath = workingPath.substr(1);
      successfulMatch = true;
    }

    if ((workingPath.size() >= 1) && (workingPath.at(workingPath.size() - 1) == ' '))
    {
      workingPath = workingPath.substr(0, workingPath.size() - 1);
      successfulMatch = true;
    }

    if ((workingPath.size() >= 2) && (workingPath.at(0) == '\"') && (workingPath.at(workingPath.size() - 1) == '\"'))
    {
      workingPath = workingPath.substr(1, workingPath.size() - 2);
      successfulMatch = true;
    }
  }

  return workingPath;
}
