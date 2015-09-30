@echo off
set _OPATH=%PATH%

WHERE git >nul 2>nul
if %ERRORLEVEL% EQU 0 goto UpdateRev

set GIT_ROOT=C:\Program Files (x86)\Git\bin
set PATH=%GIT_ROOT%;%_OPATH%
WHERE git >nul 2>nul
if %ERRORLEVEL% EQU 0 goto UpdateRev

:ErrNoGit
echo Error: git was not found.
goto TheEnd

:UpdateRev
for /f %%i in ('git rev-list --count HEAD') do set rev=%%i
echo #define BUILD_VERSION %rev% > %1hgversion.h
goto TheEnd

:TheEnd
set PATH=%_OPATH%