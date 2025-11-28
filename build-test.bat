@echo off
REM Schnell-Build ohne MQTT (nur ADS Testing)

echo ========================================
echo ADS Test Build (ohne MQTT)
echo ========================================
echo.

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

echo [1/2] CMake konfigurieren...
cmake -B build-test -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DBUILD_MQTT=OFF

if %ERRORLEVEL% NEQ 0 (
    echo FEHLER: CMake fehlgeschlagen!
    pause
    exit /b 1
)

echo.
echo [2/2] Bauen...
cmake --build build-test --config Release

if %ERRORLEVEL% NEQ 0 (
    echo FEHLER: Build fehlgeschlagen!
    pause
    exit /b 1
)

echo.
echo ✓ Test-Build erfolgreich!
echo   Executable: build-test\Release\ads-test.exe
echo.
pause
