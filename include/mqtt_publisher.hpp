#pragma once

// Windows max/min Makro deaktivieren BEVOR andere Headers
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "realtime_config.hpp"
#include <mqtt/async_client.h>
#include <string>
#include <atomic>
#include <memory>

namespace ads_realtime {

/**
 * High-Performance MQTT Publisher
 * 
 * Zero-Copy Publishing mit QoS 0 für minimale Latenz
 */
class MqttPublisher {
public:
    explicit MqttPublisher(const RealtimeConfig& config);
    ~MqttPublisher();

    // Disable copy/move
    MqttPublisher(const MqttPublisher&) = delete;
    MqttPublisher& operator=(const MqttPublisher&) = delete;

    /**
     * Verbindung zum MQTT Broker herstellen
     */
    bool connect();

    /**
     * Verbindung trennen
     */
    void disconnect();

    /**
     * Wert publizieren (Zero-Copy, inline)
     * @param topic MQTT Topic
     * @param payload Daten
     * @param length Datenlänge
     */
    inline void publish(const std::string& topic, const void* payload, size_t length) {
        if (!connected_.load(std::memory_order_acquire)) {
            return;
        }

        // Zero-Copy: Direkter Pointer auf Daten
        mqtt::message_ptr msg = mqtt::make_message(
            topic,
            payload,
            length,
            0,      // QoS 0 (Fire-and-Forget, minimale Latenz)
            false   // Nicht retained
        );

        // Async publish (blockiert nicht)
        try {
            client_->publish(msg);
        } catch (...) {
            // Fehler ignorieren für maximale Performance
        }
    }

    /**
     * String publizieren (für einfache Werte)
     */
    void publish_string(const std::string& topic, const std::string& value);

private:
    RealtimeConfig config_;
    std::unique_ptr<mqtt::async_client> client_;
    std::atomic<bool> connected_{false};
};

} // namespace ads_realtime
