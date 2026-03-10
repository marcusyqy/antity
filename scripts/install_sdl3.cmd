@echo off
setlocal

pushd

rem to make sure that we cd into the right directory.
cd /d "%~dp0"

set PLATFORM=windows
set BUILD_DIR=..\.build

where cmake >nul 2>&1 if %ERRORLEVEL% neq 0 ( echo Error: CMake not found. Please install CMake and add it to PATH. exit /b 1)

echo Building SDL3 for %PLATFORM%...
cmake -S ..\third_party\SDL -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=Release -GNinja

pushd %BUILD_DIR%
ninja all & echo SDL3 built!
popd

cmd /c exit %ERRORLEVEL%

