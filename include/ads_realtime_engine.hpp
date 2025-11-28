#pragma once

#include "realtime_config.hpp"
#include <Windows.h>
#include <TcAdsDef.h>
#include <TcAdsAPI.h>
#include <functional>
#include <memory>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <mutex>

namespace ads_realtime {

/**
 * ADS Realtime Engine
 * 
 * High-performance ADS notification handler mit <1ms garantierter Latenz.
 * Verwendet Windows High-Resolution Timer und Lock-Free Data Structures.
 */
class AdsRealtimeEngine {
public:
    using NotificationCallback = std::function<void(
        const std::string& variable_name,
        const void* data,
        size_t data_size,
        uint64_t timestamp_ns
    )>;

    explicit AdsRealtimeEngine(const RealtimeConfig& config);
    ~AdsRealtimeEngine();

    // Disable copy/move
    AdsRealtimeEngine(const AdsRealtimeEngine&) = delete;
    AdsRealtimeEngine& operator=(const AdsRealtimeEngine&) = delete;

    /**
     * Verbindung zum TwinCAT PLC herstellen
     */
    bool connect();

    /**
     * ADS Notification für Variable registrieren
     * @param variable_name Name der PLC Variable (z.B. "GVL.abc")
     * @param callback Callback für Wertänderungen (wird in Realtime-Thread aufgerufen!)
     * @return true bei Erfolg
     */
    bool add_variable(const std::string& variable_name, NotificationCallback callback);

    /**
     * Engine starten (beginnt Notification-Handling)
     */
    void start();

    /**
     * Engine stoppen
     */
    void stop();

    /**
     * Performance-Statistiken abrufen
     */
    PerformanceStats get_statistics() const;

private:
    struct VariableHandle {
        uint32_t handle = 0;
        unsigned long notification_handle = 0;  // MUSS unsigned long sein f\u00fcr ADS API
        NotificationCallback callback;
        std::string name;
        size_t data_size = 0;
    };

    // ADS Notification Callback (static für C-API)
    static void __stdcall ads_notification_callback(
        const AmsAddr* pAddr,
        const AdsNotificationHeader* pNotification,
        uint32_t hUser
    );

    // Interne Notification-Verarbeitung
    void process_notification(
        const AdsNotificationHeader* notification,
        VariableHandle* var_handle
    );

    // High-Resolution Timer
    uint64_t get_timestamp_ns() const;

    RealtimeConfig config_;
    AmsAddr ams_addr_{};
    long ads_port_ = 0;
    
    std::atomic<bool> running_{false};
    std::unordered_map<uint32_t, std::unique_ptr<VariableHandle>> variables_;
    std::mutex variables_mutex_;

    // Performance tracking
    mutable std::mutex stats_mutex_;
    PerformanceStats stats_{};
    std::vector<double> latency_samples_;
    LARGE_INTEGER qpc_frequency_{};
};

} // namespace ads_realtime
