@echo off
setlocal

pushd

rem to make sure that we cd into the right directory.
cd /d "%~dp0"

set PLATFORM=windows
set BUILD_DIR=..\.build

set COMPILE_FLAGS=/W4 /O2 /EHsc /Zi /std:c++20 -external:W0
set DEFINE_FLAGS=-DVK_NO_PROTOTYPES -DVK_USE_PLATFORM_WIN32_KHR
set INCLUDE_FLAGS=-external:I..\third_party
pushd %BUILD_DIR%
call cl %COMPILE_FLAGS% %DEFINE_FLAGS% %INCLUDE_FLAGS% -c ..\third_party\vma\vma.cpp
call lib vma.obj /OUT:vma.lib
popd

cmd /c exit %ERRORLEVEL%

