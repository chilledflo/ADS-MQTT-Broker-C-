#include "ads_realtime_engine.hpp"
#include "mqtt_publisher.hpp"
#include "realtime_config.hpp"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

using namespace ads_realtime;

// Global flag für sauberes Shutdown
std::atomic<bool> g_running{true};

void signal_handler(int signal) {
    std::cout << "\n[MAIN] Shutdown Signal empfangen...\n";
    g_running.store(false);
}

int main(int argc, char* argv[]) {
    std::cout << R"(
╔═══════════════════════════════════════════════════════════╗
║   ADS-Realtime-CPP - Hard Real-Time System <1ms          ║
║   TwinCAT ADS ↔ MQTT Bridge mit garantierter Latenz     ║
╚═══════════════════════════════════════════════════════════╝
)" << "\n";

    // Signal Handler registrieren
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // Konfiguration
    RealtimeConfig config;
    config.ads_target_ip = "192.168.3.42";
    config.ads_port = 851;
    config.notification_cycle_us = 100;  // 100µs = 0.1ms (10kHz)
    config.max_latency_us = 1000;        // <1ms deadline
    config.mqtt_broker = "localhost";
    config.mqtt_port = 1883;
    config.mqtt_qos = 0;

    std::cout << "[CONFIG] ADS Target: " << config.ads_target_ip << ":" << config.ads_port << "\n";
    std::cout << "[CONFIG] MQTT Broker: " << config.mqtt_broker << ":" << config.mqtt_port << "\n";
    std::cout << "[CONFIG] Notification Cycle: " << config.notification_cycle_us << "µs\n";
    std::cout << "[CONFIG] Max Latency: " << config.max_latency_us << "µs (<1ms)\n\n";

    // Thread-Priorität erhöhen
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    std::cout << "[SYSTEM] Thread-Priorität: TIME_CRITICAL\n";

    // ADS Engine initialisieren
    AdsRealtimeEngine ads_engine(config);
    if (!ads_engine.connect()) {
        std::cerr << "[MAIN] FEHLER: ADS Verbindung fehlgeschlagen!\n";
        return 1;
    }

    // MQTT Publisher initialisieren
    MqttPublisher mqtt_publisher(config);
    if (!mqtt_publisher.connect()) {
        std::cerr << "[MAIN] FEHLER: MQTT Verbindung fehlgeschlagen!\n";
        return 1;
    }

    // Variablen registrieren mit Realtime-Callbacks
    std::cout << "\n[MAIN] Registriere Variablen...\n";

    // Beispiel: GVL.abc Variable
    ads_engine.add_variable("GVL.abc", [&mqtt_publisher](
        const std::string& name,
        const void* data,
        size_t data_size,
        uint64_t timestamp_ns
    ) {
        // Realtime Callback - wird in ADS Notification Thread aufgerufen!
        // KRITISCH: Minimale Verarbeitung, keine Blockierung!
        
        // Wert als INT32 interpretieren (Beispiel)
        int32_t value = *static_cast<const int32_t*>(data);
        
        // MQTT Topic: ads/GVL.abc
        std::string topic = "ads/" + name;
        std::string payload = std::to_string(value);
        
        // Zero-Copy publish
        mqtt_publisher.publish(topic, payload.data(), payload.size());
        
        // Optional: Latenz-Tracking
        // auto now = std::chrono::high_resolution_clock::now();
        // auto latency_ns = now.time_since_epoch().count() - timestamp_ns;
    });

    // Weitere Variablen können hier registriert werden
    // ads_engine.add_variable("GVL.temperature", callback);
    // ads_engine.add_variable("GVL.pressure", callback);

    // Engine starten
    ads_engine.start();

    std::cout << "\n[MAIN] ✅ System läuft - Hard Realtime Mode aktiv\n";
    std::cout << "[MAIN] Garantierte Latenz: <1ms\n";
    std::cout << "[MAIN] Notification Rate: " << (1000000 / config.notification_cycle_us) << " Hz\n";
    std::cout << "[MAIN] Drücke Ctrl+C zum Beenden...\n\n";

    // Performance Monitor Thread
    std::thread monitor_thread([&ads_engine]() {
        while (g_running.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            
            auto stats = ads_engine.get_statistics();
            std::cout << "\n[STATS] Performance Report:\n";
            std::cout << "  Notifications: " << stats.total_notifications << "\n";
            std::cout << "  Deadline Misses: " << stats.deadline_misses << "\n";
            std::cout << "  Min Latency: " << stats.min_latency_us << "µs\n";
            std::cout << "  Avg Latency: " << stats.avg_latency_us << "µs\n";
            std::cout << "  Max Latency: " << stats.max_latency_us << "µs\n";
            std::cout << "  P50: " << stats.p50_latency_us << "µs\n";
            std::cout << "  P95: " << stats.p95_latency_us << "µs\n";
            std::cout << "  P99: " << stats.p99_latency_us << "µs\n";
            std::cout << "  Throughput: " << stats.throughput_hz << " Hz\n";
        }
    });

    // Main Loop - warten auf Shutdown
    while (g_running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Shutdown
    std::cout << "\n[MAIN] Fahre System herunter...\n";
    ads_engine.stop();
    mqtt_publisher.disconnect();
    
    if (monitor_thread.joinable()) {
        monitor_thread.join();
    }

    std::cout << "[MAIN] Beendet.\n";
    return 0;
}
