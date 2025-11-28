# ADS-Realtime-CPP Build Instructions

## Windows mit Visual Studio

### 1. Dependencies

**TwinCAT ADS**:
```powershell
# Überprüfen ob vorhanden
Test-Path "C:\TwinCAT\ADS Api\TcAdsDll\x64\lib\TcAdsDll.lib"
```

**vcpkg installieren**:
```powershell
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

**Paho MQTT**:
```powershell
cd C:\vcpkg
.\vcpkg install paho-mqttpp3:x64-windows
```

### 2. Build

```powershell
cd C:\ADS-Realtime-CPP

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build --config Release

# Executable
.\build\Release\ads-realtime-bridge.exe
```

### 3. Installation

```powershell
# System-weite Installation (optional)
cmake --install build --prefix "C:\Program Files\ADS-Realtime-CPP"
```

## Performance Tuning

### Windows Real-Time Optimierungen:

1. **Disable CPU Throttling**:
```powershell
powercfg -setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c  # High Performance
powercfg -change -standby-timeout-ac 0
```

2. **Disable Timer Coalescing**:
```powershell
# In Registry: HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\kernel
# DisableDynamicTick = 1
```

3. **Set Core Affinity**:
```cpp
// In src/main.cpp
DWORD_PTR mask = 1 << 2;  // Core 2
SetThreadAffinityMask(GetCurrentThread(), mask);
```

## Troubleshooting

### ADS Connection Failed
```
Fehler: Cannot resolve AMS NetId
Lösung: TwinCAT Router prüfen, AMS NetId hinzufügen
```

### MQTT Connection Failed
```
Fehler: Connection refused
Lösung: Mosquitto Broker starten: mosquitto -v
```

### High Latency
```
Problem: Latenz >1ms
Lösung:
- Windows Defender deaktivieren
- Antivirus ausschließen
- CPU Governor auf Performance
- Dedicated Core verwenden
```
