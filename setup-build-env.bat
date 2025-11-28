@echo off
echo.
echo ============================================================
echo   ADS-Realtime-CPP - Build Environment Setup
echo ============================================================
echo.

echo [1/4] Checking CMake...
where cmake >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo ✅ CMake already installed
    cmake --version
) else (
    echo ⚠️ Installing CMake...
    winget install Kitware.CMake -e --silent
    if %ERRORLEVEL% NEQ 0 (
        echo ❌ CMake installation failed
        echo    Download manually: https://cmake.org/download/
    )
)

echo.
echo [2/4] Checking Visual Studio Build Tools...
where cl.exe >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo ✅ MSVC compiler found
) else (
    echo ⚠️ Visual Studio Build Tools not found
    echo    Installing... (this may take 10-20 minutes)
    winget install Microsoft.VisualStudio.2022.BuildTools -e --silent --override "--quiet --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
    if %ERRORLEVEL% NEQ 0 (
        echo ❌ Installation failed
        echo    Download manually: https://visualstudio.microsoft.com/downloads/
    )
)

echo.
echo [3/4] Checking TwinCAT ADS Library...
if exist "C:\TwinCAT\ADS Api\TcAdsDll\x64\lib\TcAdsDll.lib" (
    echo ✅ TcAdsDll.lib found
) else (
    echo ❌ TcAdsDll.lib NOT FOUND
    echo    Required: TwinCAT installation with ADS library
    echo    Path: C:\TwinCAT\ADS Api\TcAdsDll
    pause
    exit /b 1
)

echo.
echo [4/4] Checking vcpkg (for Paho MQTT)...
if exist "C:\vcpkg\vcpkg.exe" (
    echo ✅ vcpkg found
) else (
    echo ⚠️ Installing vcpkg...
    cd C:\
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    call bootstrap-vcpkg.bat
    vcpkg integrate install
)

echo.
echo [5/5] Installing Paho MQTT C++...
C:\vcpkg\vcpkg install paho-mqttpp3:x64-windows

echo.
echo ============================================================
echo   ✅ Setup Complete!
echo ============================================================
echo.
echo Next steps:
echo   1. Restart terminal to refresh PATH
echo   2. cd C:\ADS-Realtime-CPP
echo   3. cmake -B build -DCMAKE_BUILD_TYPE=Release
echo   4. cmake --build build --config Release
echo.
pause
