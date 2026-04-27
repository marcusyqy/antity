@echo off
setlocal enabledelayedexpansion

rem to make sure that we cd into the right directory.
cd /d "%~dp0"

set FT_ROOT=..\third_party\freetype
set OUT_DIR=..\.build
set OBJ_DIR=%OUT_DIR%\freetype_obj
set OUT_LIB=%OUT_DIR%\freetype.lib
set BUILD_TYPE=release
set REBUILD=0

for %%a in (%*) do (
  if /I "%%~a"=="debug" set BUILD_TYPE=debug
  if /I "%%~a"=="release" set BUILD_TYPE=release
  if /I "%%~a"=="rebuild" set REBUILD=1
)

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

where cl >nul 2>&1
if %ERRORLEVEL% neq 0 (
  echo Error: cl.exe not found. Open a Developer Command Prompt.
  exit /b 1
)
where lib >nul 2>&1
if %ERRORLEVEL% neq 0 (
  echo Error: lib.exe not found. Open a Developer Command Prompt.
  exit /b 1
)

if "%REBUILD%"=="1" (
  if exist "%OBJ_DIR%" rmdir /s /q "%OBJ_DIR%"
  if exist "%OUT_LIB%" del /q "%OUT_LIB%"
)

if not exist "%OBJ_DIR%" mkdir "%OBJ_DIR%"

set CFLAGS=/nologo /c /W4 /D_CRT_SECURE_NO_WARNINGS /DFT2_BUILD_LIBRARY /I"%FT_ROOT%\include" /I"%FT_ROOT%"
if /I "%BUILD_TYPE%"=="debug" (
  set CFLAGS=!CFLAGS! /Od /Zi /D_DEBUG
) else (
  set CFLAGS=!CFLAGS! /O2 /DNDEBUG
)

set SOURCES=^
 "%FT_ROOT%\src\autofit\autofit.c" ^
 "%FT_ROOT%\src\base\ftbase.c" ^
 "%FT_ROOT%\src\base\ftbbox.c" ^
 "%FT_ROOT%\src\base\ftbdf.c" ^
 "%FT_ROOT%\src\base\ftbitmap.c" ^
 "%FT_ROOT%\src\base\ftcid.c" ^
 "%FT_ROOT%\src\base\ftfstype.c" ^
 "%FT_ROOT%\src\base\ftgasp.c" ^
 "%FT_ROOT%\src\base\ftglyph.c" ^
 "%FT_ROOT%\src\base\ftgxval.c" ^
 "%FT_ROOT%\src\base\ftinit.c" ^
 "%FT_ROOT%\src\base\ftmm.c" ^
 "%FT_ROOT%\src\base\ftotval.c" ^
 "%FT_ROOT%\src\base\ftpatent.c" ^
 "%FT_ROOT%\src\base\ftpfr.c" ^
 "%FT_ROOT%\src\base\ftstroke.c" ^
 "%FT_ROOT%\src\base\ftsynth.c" ^
 "%FT_ROOT%\src\base\fttype1.c" ^
 "%FT_ROOT%\src\base\ftwinfnt.c" ^
 "%FT_ROOT%\src\bdf\bdf.c" ^
 "%FT_ROOT%\src\cache\ftcache.c" ^
 "%FT_ROOT%\src\cff\cff.c" ^
 "%FT_ROOT%\src\cid\type1cid.c" ^
 "%FT_ROOT%\src\dlg\dlgwrap.c" ^
 "%FT_ROOT%\src\gzip\ftgzip.c" ^
 "%FT_ROOT%\src\lzw\ftlzw.c" ^
 "%FT_ROOT%\src\pcf\pcf.c" ^
 "%FT_ROOT%\src\pfr\pfr.c" ^
 "%FT_ROOT%\src\psaux\psaux.c" ^
 "%FT_ROOT%\src\pshinter\pshinter.c" ^
 "%FT_ROOT%\src\psnames\psmodule.c" ^
 "%FT_ROOT%\src\raster\raster.c" ^
 "%FT_ROOT%\src\sfnt\sfnt.c" ^
 "%FT_ROOT%\src\smooth\smooth.c" ^
 "%FT_ROOT%\src\sdf\sdf.c" ^
 "%FT_ROOT%\src\svg\svg.c" ^
 "%FT_ROOT%\src\truetype\truetype.c" ^
 "%FT_ROOT%\src\type1\type1.c" ^
 "%FT_ROOT%\src\type42\type42.c" ^
 "%FT_ROOT%\src\winfonts\winfnt.c" ^
 "%FT_ROOT%\builds\windows\ftdebug.c" ^
 "%FT_ROOT%\builds\windows\ftsystem.c"

echo Building FreeType ^(%BUILD_TYPE%^)...
for %%f in (!SOURCES!) do (
  call cl !CFLAGS! /Fo"%OBJ_DIR%\\" "%%~f"
  if !ERRORLEVEL! neq 0 (
    echo Error: compile failed for %%~f
    exit /b 1
  )
)

call lib /nologo /OUT:"%OUT_LIB%" "%OBJ_DIR%\*.obj"
if %ERRORLEVEL% neq 0 (
  echo Error: failed to archive "%OUT_LIB%"
  exit /b 1
)

echo FreeType built: %OUT_LIB%
cmd /c exit %ERRORLEVEL%
