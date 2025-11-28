# ✅ GitHub Actions - AKTIVIERT!

## 🎉 Automatische Builds auf GitHub

**Repository**: https://github.com/chilledflo/ADS-MQTT-Broker-C-
**Actions**: https://github.com/chilledflo/ADS-MQTT-Broker-C-/actions

---

## 📊 Was wurde eingerichtet:

### 1. Workflow-Datei ✅
- **.github/workflows/build.yml**
- Automatischer Build bei jedem Push
- Manuell startbar über Actions-Tab

### 2. CMake-Anpassungen ✅
- **BUILD_WITHOUT_ADS=ON** Option
- Kompiliert ohne TwinCAT ADS (für GitHub)
- Lokaler Build weiterhin mit TwinCAT möglich

### 3. Dokumentation ✅
- **.github/GITHUB-ACTIONS.md**
- Vollständige Anleitung

---

## 🚀 Wie es funktioniert:

### Automatisch:
```
Push zu GitHub
    ↓
GitHub Actions startet
    ↓
Windows Server (GitHub)
    ↓
CMake + MSVC Setup
    ↓
vcpkg installiert OpenSSL (~10 Min)
    ↓
vcpkg installiert Paho MQTT (~2 Min)
    ↓
Projekt kompilieren (~3 Min)
    ↓
✅ ads-realtime-bridge.exe als Artefakt
```

### Manuell:
1. Gehe zu: https://github.com/chilledflo/ADS-MQTT-Broker-C-/actions
2. Klicke "Build ADS Realtime C++" → "Run workflow"
3. Wähle Branch: `main`
4. Klicke "Run workflow"
5. Warte ~15-20 Minuten
6. Download unter "Artifacts"

---

## 📦 Build-Artefakte

Nach jedem erfolgreichen Build:
- **ads-realtime-bridge.exe** (ohne TwinCAT ADS)
- **Alle DLLs** (Paho MQTT, etc.)
- **30 Tage Aufbewahrung**
- **Kostenloser Download**

### Download:
1. https://github.com/chilledflo/ADS-MQTT-Broker-C-/actions
2. Neuesten erfolgreichen Build anklicken
3. Runter scrollen zu "Artifacts"
4. "ads-realtime-bridge-windows" downloaden

---

## 🎯 Vorteile:

✅ **Kein lokales Setup nötig** - GitHub kompiliert alles
✅ **Immer aktuell** - Build bei jedem Push
✅ **Kostenlos** - GitHub Actions Free Tier (2000 Min/Monat)
✅ **Reproduzierbar** - Gleiche Build-Umgebung
✅ **Artefakte** - Fertige EXE zum Download
✅ **Multi-Platform** - Könnte auch Linux/macOS bauen

---

## ⏱️ Build-Zeit auf GitHub:

| Phase | Dauer |
|-------|-------|
| Setup (CMake, MSVC, vcpkg) | ~2 Min |
| OpenSSL kompilieren | ~10-12 Min |
| Paho MQTT kompilieren | ~1-2 Min |
| C++ Projekt kompilieren | ~2-3 Min |
| **GESAMT** | **~15-20 Min** |

---

## 🔧 Lokaler Build vs. GitHub Build:

| Feature | Lokal (dein PC) | GitHub Actions |
|---------|----------------|----------------|
| **TwinCAT ADS** | ✅ Vollständig | ❌ Deaktiviert (BUILD_WITHOUT_ADS) |
| **MQTT** | ✅ Paho C++ | ✅ Paho C++ |
| **Compiler** | MSVC 2022 | MSVC 2022 |
| **Optimierung** | `/O2 /GL /LTCG` | `/O2 /GL /LTCG` |
| **Hard Real-Time** | ✅ Ja | ❌ Nein (Test-Build) |
| **Einsatzfähig** | ✅ Produktiv | ⚠️ Test/Demo |

---

## 🔄 Nächster Build:

Der nächste Build startet automatisch bei:
- Push zu `main` Branch
- Push zu `develop` Branch  
- Pull Request erstellen
- Manuell über Actions-Tab

**Aktueller Status**: Push gerade erfolgt → Build sollte jetzt starten!

Prüfen: https://github.com/chilledflo/ADS-MQTT-Broker-C-/actions

---

## 🏷️ Releases erstellen:

### Automatisch bei Git-Tags:
```bash
git tag -a v1.0.0 -m "Release version 1.0.0"
git push origin v1.0.0
```

GitHub erstellt automatisch:
- Release-Seite auf GitHub
- Download-Link für Binary
- Release Notes

---

## 💡 Zusammenfassung:

**JA, GitHub kompiliert das Projekt automatisch!**

- ✅ Workflow aktiviert
- ✅ CMake angepasst
- ✅ Code gepusht
- ⏳ Erster Build läuft wahrscheinlich schon

**Prüfe jetzt**: https://github.com/chilledflo/ADS-MQTT-Broker-C-/actions

Wenn dort ein gelbes "🟡" Symbol ist → Build läuft
Wenn dort ein grünes "✅" Symbol ist → Build fertig!
