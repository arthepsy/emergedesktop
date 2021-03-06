@echo off
rem ----------------------------------------
rem Setup C::B root folder of *binaries* (!)
rem ----------------------------------------
set CB_ROOT=C:\CodeBlocks\trunk\src\output
rem ------------------------------------------
rem Setup GCC root folder with "bin" subfolder
rem ------------------------------------------
set GCC_ROOT=C:\mingw32

rem -------------------------------------------
rem Usually below here no changes are required.
rem -------------------------------------------
if not exist "%CB_ROOT%"  goto ErrNoCB
if not exist "%GCC_ROOT%" goto ErrNoGCC
set PATH=%CB_ROOT%;%GCC_ROOT%;%PATH%

set START_CMD=start "Code::Blocks Build" /D%~dp0 /min /b
set CB_EXE="%CB_ROOT%\codeblocks.exe"
set CB_PARAMS=--batch-build-notify --no-batch-window-close
set CB_CMD=--rebuild "%~dp0Emerge Desktop.workspace"

set CB_TARGET=--target=Debug
%START_CMD% %CB_EXE% %CB_PARAMS% %CB_TARGET% %CB_CMD%
goto TheEnd

:ErrNoCB
echo Error: C::B root folder not found. Adjust batch file accordingly
goto TheEnd

:ErrNoGCC
echo Error: GCC root folder not found. Adjust batch file accordingly
goto TheEnd

:TheEnd
