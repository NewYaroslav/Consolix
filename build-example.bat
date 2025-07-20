@echo off
setlocal

REM === [ Settings ] ===
set BUILD_DIR=build-mingw
set GENERATOR="MinGW Makefiles"
REM For MSVC, you can use:
REM set BUILD_DIR=build-msvc
REM set GENERATOR="NMake Makefiles"

echo [1/3] Creating build directory: %BUILD_DIR%
if not exist %BUILD_DIR% (
    mkdir %BUILD_DIR%
)

echo [2/3] Configuring CMake
cmake -S . -B %BUILD_DIR% -G %GENERATOR% > %BUILD_DIR%\cmake_configure_log.txt 2>&1
if errorlevel 1 (
    echo CMake configuration failed. See cmake_configure_log.txt for details.
    exit /b 1
)

echo [3/3] Building project
cmake --build %BUILD_DIR% > %BUILD_DIR%\build_log.txt 2>&1
if errorlevel 1 (
    echo Build failed. See build_log.txt for details.
    exit /b 1
)

echo [OK] Build completed successfully.
endlocal
PAUSE