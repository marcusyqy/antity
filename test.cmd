@echo off
setlocal enabledelayedexpansion

REM add debug configurations and other stuff as well.
REM to make sure that we cd into the right directory.
cd /d "%~dp0"

for %%a in (%*) do set "%%a=1"

set CC=cl.exe
set CXX=cl.exe
set COMPILE_FLAGS=/W4 /O2 /std:c++17 /EHsc /Zi
set BUILD_DIR=.build
set PLATFORM=windows

set CORE_INCLUDES=/I..\core /I..\third_party\xxHash /I..\third_party\glm /I..\third_party\stb
set CORE_SRC=..\core\core_build.cpp
set CORE_OBJ=core_build.obj
set CORE_OUT=core.lib

set TYRANT_INCLUDES=/I..\third_party\SDL\include /I..\tyrant /I..\third_party\imgui /I..\third_party\glad /I..\third_party\imgui_backends /I..\third_party\glm  /I..\embed %CORE_INCLUDES%
set TYRANT_LINK_FLAGS=/link SDL3.lib imgui.lib imgui_backends.lib glad.lib %CORE_OUT%
set TYRANT_SRC=..\tyrant\lib.cpp ..\tyrant\main.cpp
set TYRANT_OUT=tyrant.exe

set TEST_INCLUDES=%CORE_INCLUDES% /I..\test
set TEST_LINK_FLAGS=/link %CORE_OUT%
set TEST_SRC=..\test\main.cpp
set TEST_OUT=test.exe

if not exist %BUILD_DIR% mkdir %BUILD_DIR%
pushd %BUILD_DIR%
@echo off
setlocal enabledelayedexpansion

REM add debug configurations and other stuff as well.
REM to make sure that we cd into the right directory.
cd /d "%~dp0"

for %%a in (%*) do set "%%a=1"

set CC=cl.exe
set CXX=cl.exe
set COMPILE_FLAGS=
REM /W4 /O2 /std:c++20 /EHsc /Zi
set BUILD_DIR=.build
set PLATFORM=windows
set OUT_FILE=test.exe
set SRC_FILES=..\test\test.cpp ..\antity\build.cpp


if not exist %BUILD_DIR% mkdir %BUILD_DIR%

pushd %BUILD_DIR%
%CXX% %SRC_FILES% %COMPILE_FLAGS% /Fe:%OUT_FILE% && call %OUT_FILE%
popd

cmd /c exit %ERRORLEVEL%
