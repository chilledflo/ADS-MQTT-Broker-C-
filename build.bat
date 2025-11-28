@echo off
REM Build Script für ADS-Realtime-CPP
REM Kompiliert das C++ Hard Real-Time System

echo ========================================
echo ADS Realtime C++ - Build Script
echo ========================================
echo.

REM Visual Studio Build Tools Environment laden
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

echo [1/4] CMake konfigurieren...
cmake -B build -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
  -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo FEHLER: CMake Konfiguration fehlgeschlagen!
    pause
    exit /b 1
)

echo.
echo [2/4] Projekt bauen...
cmake --build build --config Release -j %NUMBER_OF_PROCESSORS%

if %ERRORLEVEL% NEQ 0 (
    echo FEHLER: Build fehlgeschlagen!
    pause
    exit /b 1
)

echo.
echo [3/4] Binaries pruefen...
if exist "build\Release\ads-realtime-bridge.exe" (
    echo ✓ Erfolgreich gebaut: build\Release\ads-realtime-bridge.exe
) else (
    echo X ads-realtime-bridge.exe nicht gefunden!
    pause
    exit /b 1
)

echo.
echo [4/4] TcAdsDll.dll kopieren...
copy "C:\Program Files (x86)\Beckhoff\TwinCAT\AdsApi\TcAdsDll\Lib\x64\TcAdsDll.dll" "build\Release\" /Y

echo.
echo ========================================
echo Build erfolgreich!
echo ========================================
echo.
echo Starten mit: cd build\Release ^&^& ads-realtime-bridge.exe
echo.
pause
