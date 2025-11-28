#!/usr/bin/env pwsh
# Auto-Monitor für Paho MQTT Installation und automatischer Build

$GREEN = "Green"
$CYAN = "Cyan"
$YELLOW = "Yellow"

Write-Host "`n========================================" -ForegroundColor $CYAN
Write-Host "   ADS C++ - Auto Build Monitor" -ForegroundColor $CYAN
Write-Host "========================================`n" -ForegroundColor $CYAN

$checkCount = 0
$maxChecks = 60  # Max 30 Minuten (60 x 30 Sekunden)

while ($checkCount -lt $maxChecks) {
    $checkCount++
    
    Clear-Host
    Write-Host "`n========================================" -ForegroundColor $CYAN
    Write-Host "   Paho MQTT Installation Monitor" -ForegroundColor $CYAN
    Write-Host "========================================`n" -ForegroundColor $CYAN
    Write-Host "Check #$checkCount von $maxChecks" -ForegroundColor $YELLOW
    Write-Host "Zeit: $(Get-Date -Format 'HH:mm:ss')`n" -ForegroundColor $YELLOW
    
    # Prüfe ob paho-mqttpp3 installiert ist
    $installed = & C:\vcpkg\vcpkg.exe list 2>$null | Select-String "paho-mqttpp3:x64-windows"
    
    if ($installed) {
        Write-Host "`n✅ PAHO MQTT C++ INSTALLIERT!" -ForegroundColor $GREEN
        Write-Host "`n$installed`n" -ForegroundColor $GREEN
        Write-Host "Starte automatischen Build in 5 Sekunden...`n" -ForegroundColor $GREEN
        Start-Sleep -Seconds 5
        
        # Starte Build
        Set-Location C:\ADS-Realtime-CPP
        & .\build.bat
        
        break
    } else {
        Write-Host "⏳ Installation läuft noch...`n" -ForegroundColor $YELLOW
        
        # Zeige aktive Prozesse
        $processes = Get-Process | Where-Object { 
            $_.ProcessName -match "vcpkg|cmake|cl|perl|link" 
        } | Select-Object -First 5
        
        if ($processes) {
            Write-Host "Aktive Prozesse:" -ForegroundColor $CYAN
            $processes | Format-Table ProcessName, 
                @{N='CPU';E={[math]::Round($_.CPU,1)}}, 
                @{N='RAM(MB)';E={[math]::Round($_.WorkingSet64/1MB,0)}} -AutoSize
        }
        
        Write-Host "`nNächste Prüfung in 30 Sekunden..." -ForegroundColor $YELLOW
        Write-Host "(Drücke Strg+C zum Abbrechen)`n" -ForegroundColor $YELLOW
        
        Start-Sleep -Seconds 30
    }
}

if ($checkCount -ge $maxChecks) {
    Write-Host "`n⚠️ Timeout erreicht - Installation dauert zu lange" -ForegroundColor $YELLOW
    Write-Host "Manuell prüfen mit: C:\vcpkg\vcpkg.exe list | Select-String paho`n" -ForegroundColor $YELLOW
}

Read-Host "`nDrücke Enter zum Beenden"
