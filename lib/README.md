# TwinCAT ADS Library

Dieses Verzeichnis enthält die TwinCAT ADS Bibliothek für die Kompilierung.

## 📦 Enthaltene Dateien:

### Binaries:
- **TcAdsDll.dll** (446 KB) - x64 Runtime Library
- **TcAdsDll.lib** (10 KB) - x64 Import Library

### Headers:
- **Include/TcAdsAPI.h** - ADS API Funktionen
- **Include/TcAdsDef.h** - ADS Definitionen und Konstanten

## 🎯 Verwendung:

### Lokal (mit System-Installation):
CMake verwendet automatisch die System-Installation von:
```
C:\Program Files (x86)\Beckhoff\TwinCAT\AdsApi\TcAdsDll
```

### GitHub Actions / CI/CD:
CMake verwendet automatisch diese gebündelten Dateien:
```cmake
# Automatisch erkannt:
${CMAKE_CURRENT_SOURCE_DIR}/lib/TcAdsDll.lib
${CMAKE_CURRENT_SOURCE_DIR}/lib/Include/
```

## ⚖️ Lizenz:

Diese Dateien sind Eigentum von **Beckhoff Automation GmbH & Co. KG**.

**Quelle**: TwinCAT ADS SDK
**Version**: x64
**Redistribution**: Für die Ausführung von mit TwinCAT kompilierten Programmen

### Lizenzhinweise:
- Die TwinCAT ADS Library ist erforderlich für die Kommunikation mit Beckhoff PLCs
- Diese Dateien werden gemäß Beckhoff Lizenzbestimmungen vertrieben
- Für kommerzielle Nutzung prüfen Sie bitte die Beckhoff Lizenzbedingungen

**Mehr Informationen**: https://www.beckhoff.com

## 🔧 Build-Konfiguration:

CMakeLists.txt prüft automatisch:

```cmake
1. Existiert lib/TcAdsDll.lib?
   ✅ Ja → Verwende gebündelte Version
   ❌ Nein → Suche System-Installation

2. Set TWINCAT_ADS_ROOT automatisch
3. Link gegen TcAdsDll.lib
4. Include lib/Include/ Pfad
```

## 📋 Versionsinfo:

Diese DLL unterstützt:
- ADS Routing
- ADS Device Notifications (für Hard Real-Time)
- ADS Read/Write Operationen
- Symbol-basierter Zugriff
- Index-basierter Zugriff

## 🚀 GitHub Actions:

Der automatische Build auf GitHub verwendet diese Dateien:
```yaml
# .github/workflows/build.yml
- name: Configure CMake
  run: |
    cmake -B build \
      -DTWINCAT_ADS_ROOT="${{ github.workspace }}/lib"
```

Dadurch ist **keine TwinCAT-Installation auf GitHub nötig**!

## ⚠️ Wichtig:

- Diese DLL ist **nur für x64** (64-bit Windows)
- Für x86 (32-bit) separate Version erforderlich
- Runtime erfordert Windows 7+ oder Windows Server 2008 R2+

## 🔄 Update:

Um die Bibliothek zu aktualisieren:

```powershell
# Von System-Installation kopieren:
Copy-Item "C:\Program Files (x86)\Beckhoff\TwinCAT\Common64\TcAdsDll.dll" lib\
Copy-Item "C:\Program Files (x86)\Beckhoff\TwinCAT\AdsApi\TcAdsDll\Lib\x64\TcAdsDll.lib" lib\
Copy-Item "C:\Program Files (x86)\Beckhoff\TwinCAT\AdsApi\TcAdsDll\Include\*" lib\Include\
```

---

**Hinweis**: Diese Dateien ermöglichen GitHub Actions, das Projekt ohne TwinCAT-Installation zu kompilieren!
