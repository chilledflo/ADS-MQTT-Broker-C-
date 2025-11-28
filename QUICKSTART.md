# 🚀 Quick Start - ADS Realtime C++

## Build & Run in 3 Schritten

### 1️⃣ Abhängigkeiten installiert?
```powershell
# Prüfen:
cmake --version                    # CMake 4.2.0
C:\vcpkg\vcpkg.exe list | grep paho  # Paho MQTT
```

### 2️⃣ Projekt bauen
```powershell
cd C:\ADS-Realtime-CPP
.\build.bat
```

### 3️⃣ Konfiguration & Start
```powershell
# config.ini anpassen (PLC IP, MQTT Broker, Variablen)
notepad config.ini

# Programm starten
cd build\Release
.\ads-realtime-bridge.exe
```

---

## 📋 config.ini Beispiel

```ini
[plc]
ip = 192.168.3.42
ams_net_id = 192.168.3.42.1.1

[mqtt]
broker = localhost
port = 1883

[realtime]
notification_cycle_us = 100    # 100µs = 10kHz
max_latency_us = 1000         # <1ms guaranteed

[variables]
var_1 = MAIN.Temperature, REAL, 4
var_2 = MAIN.Pressure, REAL, 4
```

---

## ⚡ Performance

- **100µs Notification Cycle** (10kHz Update Rate)
- **<1ms Maximum Latency** (Hard Real-Time Guarantee)
- **TIME_CRITICAL Thread Priority**
- **Zero-Copy MQTT Publishing**

---

## 🔍 Monitoring

Das Programm gibt automatisch Performance-Statistiken aus:

```
=== ADS Realtime Bridge ===
PLC: 192.168.3.42 | MQTT: localhost:1883
Notification Cycle: 100µs (10kHz)

[12:34:56] Connected to PLC
[12:34:56] Subscribed: MAIN.Temperature
[12:34:56] Subscribed: MAIN.Pressure
[12:34:57] Started monitoring

--- Performance Stats (1s) ---
Updates: 10000
Latency P50: 45µs
Latency P95: 78µs
Latency P99: 120µs
Max Latency: 245µs ✅ <1ms
```

---

## 🛠️ Troubleshooting

### Fehler: "TwinCAT connection failed"
- TwinCAT Runtime läuft?
- PLC IP korrekt in config.ini?
- Firewall-Regel für ADS Port 851?

### Fehler: "MQTT connection failed"
- MQTT Broker läuft? (z.B. Mosquitto)
- Broker-Adresse korrekt?

### Fehler: "TcAdsDll.dll not found"
```powershell
# DLL ins Build-Verzeichnis kopieren:
copy "C:\Program Files (x86)\Beckhoff\TwinCAT\AdsApi\TcAdsDll\Lib\x64\TcAdsDll.dll" build\Release\
```

---

## 📚 Weitere Dokumentation

- **README.md** - Vollständige Dokumentation
- **BUILD-STATUS.md** - Aktueller Build-Status
- **docs/BUILD.md** - Detaillierte Build-Anleitung

---

**GitHub**: https://github.com/chilledflo/ADS-MQTT-Broker-C-
