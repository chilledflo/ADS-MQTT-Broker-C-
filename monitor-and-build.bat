@echo off
REM Monitor Script - Überwacht Paho MQTT Installation und startet Build

echo ========================================
echo ADS Realtime C++ - Build Monitor
echo ========================================
echo.
echo Warte auf Paho MQTT Installation...
echo.

:CHECK_LOOP
cls
echo ========================================
echo ADS Realtime C++ - Build Monitor
echo ========================================
echo.
echo [%TIME%] Prüfe vcpkg Pakete...
echo.

C:\vcpkg\vcpkg.exe list | findstr "paho-mqttpp3" > nul
if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo ✓ Paho MQTT C++ installiert!
    echo ========================================
    echo.
    C:\vcpkg\vcpkg.exe list | findstr "paho"
    echo.
    echo Starte automatischen Build in 5 Sekunden...
    timeout /t 5 /nobreak
    
    echo.
    echo ========================================
    echo Starte Build...
    echo ========================================
    call build.bat
    
    goto END
) else (
    echo ⏳ Paho MQTT wird noch installiert...
    echo.
    echo Aktive Prozesse:
    powershell -Command "Get-Process | Where-Object { $_.ProcessName -like '*cmake*' -or $_.ProcessName -like '*cl*' -or $_.ProcessName -like '*vcpkg*' } | Select-Object -First 5 | Format-Table ProcessName, CPU -AutoSize"
    echo.
    echo Nächste Prüfung in 30 Sekunden...
    echo (Drücke Strg+C zum Abbrechen)
    timeout /t 30 /nobreak > nul
    goto CHECK_LOOP
)

:END
echo.
echo ========================================
echo Fertig!
echo ========================================
pause
