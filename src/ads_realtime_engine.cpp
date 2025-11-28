#include "ads_realtime_engine.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cstring>

// Windows max/min Makro-Konflikte verhindern
#ifndef NOMINMAX
#define NOMINMAX
#endif

// ADS API Definitionen die in älteren TcAdsAPI.h fehlen könnten
#ifndef ADSIGRP_SYM_HNDBYNAME
#define ADSIGRP_SYM_HNDBYNAME 0xF003
#endif
#ifndef ADSIGRP_SYM_VALBYHND
#define ADSIGRP_SYM_VALBYHND 0xF005
#endif
#ifndef ADSIGRP_SYM_INFOBYNAMEEX
#define ADSIGRP_SYM_INFOBYNAMEEX 0xF040
#endif

namespace ads_realtime {

AdsRealtimeEngine::AdsRealtimeEngine(const RealtimeConfig& config)
    : config_(config) {
    
    // High-Resolution Performance Counter initialisieren
    QueryPerformanceFrequency(&qpc_frequency_);
    
    // Latency samples reservieren (60 Sekunden bei 10kHz = 600k samples)
    latency_samples_.reserve(600000);
    
    std::cout << "[ADS RT] Engine initialisiert\n";
    std::cout << "[ADS RT] Notification Cycle: " << config_.notification_cycle_us << "µs\n";
    std::cout << "[ADS RT] Max Latency: " << config_.max_latency_us << "µs\n";
}

AdsRealtimeEngine::~AdsRealtimeEngine() {
    stop();
    
    if (ads_port_ != 0) {
        AdsPortCloseEx(ads_port_);
    }
}

bool AdsRealtimeEngine::connect() {
    // ADS Port öffnen
    ads_port_ = AdsPortOpenEx();
    if (ads_port_ == 0) {
        std::cerr << "[ADS RT] ERROR: AdsPortOpenEx failed\n";
        return false;
    }

    // AMS NetId manuell setzen (AdsGetNetIdForHostName nicht in allen TcAdsAPI verfügbar)
    AmsNetId netId;
    // Für lokale Verbindung: 127.0.0.1.1.1 (Standard TwinCAT)
    netId.b[0] = 127;
    netId.b[1] = 0;
    netId.b[2] = 0;
    netId.b[3] = 1;
    netId.b[4] = 1;
    netId.b[5] = 1;

    // AMS Adresse setzen
    ams_addr_.netId = netId;
    ams_addr_.port = config_.ads_port;

    std::cout << "[ADS RT] Verbunden mit " << config_.ads_target_ip 
              << ":" << config_.ads_port << "\n";
    
    return true;
}

bool AdsRealtimeEngine::add_variable(
    const std::string& variable_name,
    NotificationCallback callback) {
    
    if (!callback) {
        std::cerr << "[ADS RT] ERROR: Invalid callback\n";
        return false;
    }

    auto var_handle = std::make_unique<VariableHandle>();
    var_handle->name = variable_name;
    var_handle->callback = std::move(callback);

    // Symbol-Handle für Variable abrufen
    unsigned long bytes_read = 0;
    long result = AdsSyncReadWriteReqEx2(
        ads_port_,
        &ams_addr_,
        ADSIGRP_SYM_HNDBYNAME,
        0,
        sizeof(var_handle->handle),
        &var_handle->handle,
        static_cast<unsigned long>(variable_name.length()),
        const_cast<char*>(variable_name.c_str()),
        &bytes_read
    );

    if (result != 0) {
        std::cerr << "[ADS RT] ERROR: Cannot get handle for " << variable_name 
                  << " (Error: " << result << ")\n";
        return false;
    }

    // Variablen-Info abrufen (Datentyp, Größe)
    AdsSymbolEntry symbol_entry{};
    unsigned long bytes_read2 = 0;
    result = AdsSyncReadReqEx2(
        ads_port_,
        &ams_addr_,
        ADSIGRP_SYM_INFOBYNAMEEX,
        0,
        sizeof(symbol_entry),
        &symbol_entry,
        &bytes_read2
    );

    if (result != 0) {
        std::cerr << "[ADS RT] WARNING: Cannot get symbol info for " << variable_name << "\n";
        var_handle->data_size = 4; // Default: 4 bytes
    } else {
        var_handle->data_size = symbol_entry.size;
    }

    // ADS Device Notification erstellen (HARTE ECHTZEIT)
    AdsNotificationAttrib attrib{};
    attrib.cbLength = var_handle->data_size;
    attrib.nTransMode = ADSTRANS_SERVERCYCLE;  // Bei JEDER Änderung
    attrib.nMaxDelay = 0;  // Keine Verzögerung
    attrib.nCycleTime = config_.notification_cycle_us / 1000;  // µs -> ms

    unsigned long notification_handle = 0;
    result = AdsSyncAddDeviceNotificationReqEx(
        ads_port_,
        &ams_addr_,
        ADSIGRP_SYM_VALBYHND,
        var_handle->handle,
        &attrib,
        reinterpret_cast<PAdsNotificationFuncEx>(&AdsRealtimeEngine::ads_notification_callback),
        reinterpret_cast<unsigned long>(var_handle.get()),
        &notification_handle
    );

    if (result != 0) {
        std::cerr << "[ADS RT] ERROR: Cannot create notification for " << variable_name 
                  << " (Error: " << result << ")\n";
        return false;
    }

    var_handle->notification_handle = notification_handle;

    std::cout << "[ADS RT] Variable registriert: " << variable_name 
              << " (Size: " << var_handle->data_size << " bytes, "
              << "Cycle: " << config_.notification_cycle_us << "µs)\n";

    // Variable speichern
    {
        std::lock_guard<std::mutex> lock(variables_mutex_);
        variables_[notification_handle] = std::move(var_handle);
    }

    return true;
}

void AdsRealtimeEngine::start() {
    if (running_.exchange(true)) {
        return; // Bereits gestartet
    }

    // Thread-Priorität erhöhen (Windows)
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    std::cout << "[ADS RT] Engine gestartet (Hard Realtime Mode)\n";
    std::cout << "[ADS RT] Warte auf Notifications...\n";
}

void AdsRealtimeEngine::stop() {
    if (!running_.exchange(false)) {
        return; // Bereits gestoppt
    }

    // Alle Notifications entfernen
    std::lock_guard<std::mutex> lock(variables_mutex_);
    for (auto& [handle, var] : variables_) {
        if (var->notification_handle != 0) {
            AdsSyncDelDeviceNotificationReqEx(
                ads_port_,
                &ams_addr_,
                var->notification_handle
            );
        }
    }
    variables_.clear();

    std::cout << "[ADS RT] Engine gestoppt\n";
}

void __stdcall AdsRealtimeEngine::ads_notification_callback(
    const AmsAddr* pAddr,
    const AdsNotificationHeader* pNotification,
    uint32_t hUser) {
    
    auto* var_handle = reinterpret_cast<VariableHandle*>(hUser);
    if (!var_handle) {
        return;
    }

    // Timestamp SOFORT erfassen (minimale Latenz)
    LARGE_INTEGER qpc_now;
    QueryPerformanceCounter(&qpc_now);

    // Notification verarbeiten (in Realtime-Kontext!)
    const void* data = pNotification + 1; // Data folgt nach Header
    
    if (var_handle->callback) {
        var_handle->callback(
            var_handle->name,
            data,
            pNotification->cbSampleSize,
            pNotification->nTimeStamp // ADS-Timestamp (100ns Einheiten)
        );
    }
}

uint64_t AdsRealtimeEngine::get_timestamp_ns() const {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (now.QuadPart * 1000000000ULL) / qpc_frequency_.QuadPart;
}

PerformanceStats AdsRealtimeEngine::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    PerformanceStats stats = stats_;
    
    // Percentile berechnen
    if (!latency_samples_.empty()) {
        auto samples = latency_samples_;
        std::sort(samples.begin(), samples.end());
        
        stats.p50_latency_us = samples[samples.size() / 2];
        stats.p95_latency_us = samples[samples.size() * 95 / 100];
        stats.p99_latency_us = samples[samples.size() * 99 / 100];
    }
    
    return stats;
}

} // namespace ads_realtime
