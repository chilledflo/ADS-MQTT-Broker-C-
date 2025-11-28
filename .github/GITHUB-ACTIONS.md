# GitHub Actions Build

Dieses Repository kann automatisch auf GitHub kompiliert werden!

## 🚀 Automatische Builds

### Wann wird gebaut?

- ✅ **Bei jedem Push** auf `main` oder `develop`
- ✅ **Bei jedem Pull Request**
- ✅ **Manuell** über "Actions" Tab → "Run workflow"

### Was wird gebaut?

- **OpenSSL** (automatisch via vcpkg)
- **Paho MQTT C++** (automatisch via vcpkg)
- **ADS Realtime Bridge** (ohne TwinCAT ADS für CI/CD)

### Build-Artefakte

Nach jedem erfolgreichen Build:
- 📦 `ads-realtime-bridge.exe` als Download verfügbar
- 🕒 30 Tage Aufbewahrung
- 📥 Download unter "Actions" → Build → "Artifacts"

## 📋 Build-Status

![Build Status](https://github.com/chilledflo/ADS-MQTT-Broker-C-/actions/workflows/build.yml/badge.svg)

## 🔧 Lokaler Build vs. GitHub Build

| Feature | Lokal (Windows) | GitHub Actions |
|---------|----------------|----------------|
| **TwinCAT ADS** | ✅ Vollständig | ❌ Mock/Stub |
| **MQTT** | ✅ Paho C++ | ✅ Paho C++ |
| **Performance** | ✅ Hard Real-Time | ⚠️ Test-Build |
| **Compiler** | MSVC 2022 | MSVC 2022 |
| **Optimierung** | `/O2 /GL /LTCG` | `/O2 /GL /LTCG` |

## 🎯 GitHub Build aktivieren

### 1. Workflow-Datei pushen
```bash
git add .github/workflows/build.yml
git commit -m "Add GitHub Actions workflow"
git push origin main
```

### 2. Ersten Build starten
- Gehe zu: https://github.com/chilledflo/ADS-MQTT-Broker-C-/actions
- Klicke: "Build ADS Realtime C++" → "Run workflow"
- Wähle Branch: `main`
- Klicke: "Run workflow"

### 3. Build beobachten
- Build dauert ~15-20 Minuten
- Live-Log verfügbar
- Artefakte nach Fertigstellung downloadbar

## 📦 Releases erstellen

### Automatische Releases bei Tags:
```bash
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0
```

GitHub erstellt automatisch:
- ✅ Release-Seite
- ✅ Binary als Download
- ✅ Release Notes

## 🔍 Build-Details

### Dependencies (automatisch installiert):
- CMake 3.20+
- Visual Studio 2022 Build Tools
- vcpkg Package Manager
- OpenSSL 3.6.0
- Paho MQTT C++ 1.5.2

### Build-Schritte:
1. Code auschecken
2. CMake + vcpkg setup
3. Dependencies installieren (OpenSSL, Paho MQTT)
4. CMake konfigurieren
5. MSVC kompilieren
6. Artefakte hochladen

### Build-Optionen:
```cmake
-DBUILD_WITHOUT_ADS=ON   # Ohne TwinCAT (für GitHub)
-DCMAKE_BUILD_TYPE=Release
```

## 💾 Artefakte herunterladen

### Via Web:
1. https://github.com/chilledflo/ADS-MQTT-Broker-C-/actions
2. Letzten erfolgreichen Build anklicken
3. "Artifacts" → "ads-realtime-bridge-windows" downloaden

### Via GitHub CLI:
```bash
gh run download --repo chilledflo/ADS-MQTT-Broker-C-
```

## 🐛 Troubleshooting

### Build schlägt fehl?
- Prüfe Workflow-Log im Actions-Tab
- Häufige Probleme:
  - vcpkg Package nicht gefunden → `vcpkgGitCommitId` aktualisieren
  - CMake Konfiguration fehlt → `CMakeLists.txt` prüfen
  - MSVC Version fehlt → `setup-msbuild` Version prüfen

### Slow Build?
- OpenSSL kompilieren dauert ~10-15 Min
- Nutze vcpkg binary cache (TODO)
- Selbst-gehostete Runner für schnellere Builds

## 📊 Build-Zeit

| Phase | Dauer |
|-------|-------|
| Setup | ~2 Min |
| vcpkg install OpenSSL | ~10-12 Min |
| vcpkg install Paho MQTT | ~1-2 Min |
| CMake + Compile | ~2-3 Min |
| **Total** | **~15-20 Min** |

## 🚀 Nächste Schritte

- [ ] Binary cache für vcpkg aktivieren (schnellere Builds)
- [ ] Multi-Platform Builds (Linux, macOS)
- [ ] Unit Tests in CI/CD
- [ ] Code Coverage Reports
- [ ] Docker Images automatisch bauen
- [ ] Performance Benchmarks

---

**Vorteil**: Jeder kann den Code kompilieren, ohne lokales Setup! 🎉
