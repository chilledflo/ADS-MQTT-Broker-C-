#pragma once

#include <cstdint>
#include <string>

namespace ads_realtime {

/**
 * Hard Real-Time Configuration
 * Garantierte Latenz: <1ms
 */
struct RealtimeConfig {
    // ADS Configuration
    std::string ads_target_ip = "192.168.3.42";
    uint16_t ads_port = 851;
    
    // Real-Time Settings
    uint32_t notification_cycle_us = 100;  // 100µs = 0.1ms (10kHz)
    uint32_t max_latency_us = 1000;        // <1ms hard deadline
    
    // MQTT Settings
    std::string mqtt_broker = "localhost";
    uint16_t mqtt_port = 1883;
    uint8_t mqtt_qos = 0;  // QoS 0 für minimale Latenz
    
    // Performance Monitoring
    bool enable_latency_tracking = true;
    bool enable_deadline_monitoring = true;
    uint32_t stats_interval_ms = 1000;
    
    // Threading
    uint32_t worker_threads = 4;
    bool pin_to_cores = true;  // CPU affinity für deterministische Performance
    int8_t priority_boost = 2;  // Thread priority (Windows: THREAD_PRIORITY_HIGHEST)
};

/**
 * Performance Statistics
 */
struct PerformanceStats {
    uint64_t total_notifications = 0;
    uint64_t deadline_misses = 0;
    double min_latency_us = 0.0;
    double max_latency_us = 0.0;
    double avg_latency_us = 0.0;
    double p50_latency_us = 0.0;
    double p95_latency_us = 0.0;
    double p99_latency_us = 0.0;
    uint32_t throughput_hz = 0;
};

} // namespace ads_realtime
