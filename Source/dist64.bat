@echo off & setlocal

@if /i "%2"=="" (
  echo "Usage: dist64 [bin/src/all] [VERSION]"
  goto exit
)

if /i "%1" == "bin" (
  @if exist EmergeDesktop64-%2.zip del EmergeDesktop64-%2.zip
	7z a EmergeDesktop64-%2.7z .\bin64\*.exe .\bin64\*.dll
	cd ..
	7z a Source\EmergeDesktop64-%2.7z .\Documentation\*.chm
	cd Source
)

if /i "%1" == "src" (
  @if exist EmergeDesktop-%2-src.zip del EmergeDesktop-%2-src.zip
	7z a EmergeDesktop-%2-src.7z * -xr@excludes.lst
)

if /i "%1" == "all" (
  @if exist EmergeDesktop64-%2.zip del EmergeDesktop64-%2.zip
  @if exist EmergeDesktop-%2-src.zip del EmergeDesktop-%2-src.zip
	7z a EmergeDesktop64-%2.7z .\bin64\*.exe .\bin64\*.dll
	cd ..
	7z a Source\EmergeDesktop64-%2.7z .\Documentation\*.chm
	cd Source
	7z a EmergeDesktop-%2-src.7z * -xr@excludes.lst
)

@if /i "%1"=="" (
  echo "Usage: dist64 [bin/src/all] [VERSION]"
)

:exit
endlocal & echo on
