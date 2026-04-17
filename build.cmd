@echo off
setlocal enabledelayedexpansion

REM to make sure that we cd into the right directory.
cd /d "%~dp0"

for %%a in (%*) do set "%%a=1"

set CC=cl.exe
set COMPILE_FLAGS=/W4 /O2 /EHsc /Zi -external:W0
set DEFINE_FLAGS=-DVK_NO_PROTOTYPES -DVK_USE_PLATFORM_WIN32_KHR
set BUILD_DIR=.build

if not exist .build mkdir .build
if not exist .build\SDL3.lib call scripts\install_sdl3.cmd || exit /b 1
if not exist .build\vma.lib  call scripts\build_vma.cmd || exit /b 1

REM need to replace this awayyy
call python3 scripts\install_vulkan.py

set LINK_FLAGS=/link SDL3.lib vma.lib
set INCLUDE_FLAGS=-external:I..\third_party\SDL\include /I..\src -external:I..\third_party
REM -external:I%VULKAN_SDK%\Include

pushd .build
call %CC% %COMPILE_FLAGS% %DEFINES% %INCLUDE_FLAGS% ..\src\main.c /Fe:game.exe %LINK_FLAGS% || (
  del game.exe
  exit /b 1
)

popd

REM call python3 scripts\generate_compile_flags.py

cmd /c exit %ERRORLEVEL%

