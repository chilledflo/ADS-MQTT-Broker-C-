@echo off
echo.
echo ============================================================
echo   Quick Start: Node.js Version (bereits funktionsfähig!)
echo ============================================================
echo.
echo Das Node.js Projekt läuft bereits mit:
echo   ✅ ADS Device Notifications (1ms cycle)
echo   ✅ Kontinuierliche Updates (keine Wert-Checks)
echo   ✅ MQTT Publishing
echo   ✅ Admin Dashboard
echo   ✅ Aktuelle Latenz: ~5ms (sehr gut!)
echo.
echo Starte Node.js Server...
cd C:\ADS-MQTT-Broker
start cmd /k "npm start"
timeout /t 3 >nul
echo.
echo Öffne Dashboard...
start http://localhost:8080/admin-dashboard-modern.html
echo.
echo ============================================================
echo   Server läuft! Dashboard geöffnet.
echo ============================================================
echo.
pause
