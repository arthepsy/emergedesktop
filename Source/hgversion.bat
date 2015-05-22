@echo off
set GIT_ROOT=C:\Program Files (x86)\Git\bin

if not exist "%GIT_ROOT%"  goto ErrNoCB
set PATH=%GIT_ROOT%;%PATH%
for /f %%i in ('git rev-list --count HEAD') do set rev=%%i
echo #define BUILD_VERSION %rev% > %1hgversion.h
goto TheEnd

:ErrNoGit
echo Error: Git root folder not found. Adjust batch file accordingly
goto TheEnd

:TheEnd