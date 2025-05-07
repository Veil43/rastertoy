@echo off
setlocal

if "%1"=="" (
    echo Please provide a mode: "debug" or "release".
    exit /b
)

set MODE=%1

set INCLUDE_DIR=..\include
set SRC_DIR=..\src
set LIB_DIR=..\lib\win32

set COMMON_FLAGS=/I"%INCLUDE_DIR%" /EHsc

:: Check the mode (debug or release)
:: /i to ignore case sensitiveness
:: /D_DEBUG defines DEBUG symbol
if /i "%MODE%"=="debug" (
    echo Building in Debug mode...
    set BUILD_FLAGS=/Zi
    set LIBS=%LIB_DIR%\SDL2main.lib %LIB_DIR%\SDL2.lib
    set TARGET_DIR=".\debug"
) else if /i "%MODE%"=="release" (
    
    echo Building in Release mode...
    set BUILD_FLAGS=/O2
    set LIBS=%LIB_DIR%\SDL2main.lib %LIB_DIR%\SDL2.lib
    set TARGET_DIR=".\release"
) else (
    echo Invalid mode! Use "debug" or "release".
    exit /b
)

mkdir %TARGET_DIR%
pushd %TARGET_DIR%
cl %COMMON_FLAGS% %BUILD_FLAGS% %SRC_DIR%\sdl2_rastertoy.cpp %SRC_DIR%\rastertoy.cpp %LIBS%
copy "%LIB_DIR%\SDL2.dll" .
popd

endlocal
