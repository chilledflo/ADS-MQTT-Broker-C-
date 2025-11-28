// Windows max/min Makro deaktivieren BEVOR andere Headers
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "mqtt_publisher.hpp"
#include <iostream>

namespace ads_realtime {

MqttPublisher::MqttPublisher(const RealtimeConfig& config)
    : config_(config) {
    
    std::string server_address = "tcp://" + config_.mqtt_broker 
                                + ":" + std::to_string(config_.mqtt_port);
    
    client_ = std::make_unique<mqtt::async_client>(
        server_address,
        "ADS-Realtime-Bridge"
    );

    std::cout << "[MQTT] Publisher initialisiert: " << server_address << "\n";
}

MqttPublisher::~MqttPublisher() {
    disconnect();
}

bool MqttPublisher::connect() {
    try {
        mqtt::connect_options opts;
        opts.set_clean_session(true);
        opts.set_keep_alive_interval(20);
        opts.set_automatic_reconnect(true);

        auto tok = client_->connect(opts);
        tok->wait();

        connected_.store(true, std::memory_order_release);
        
        std::cout << "[MQTT] Verbunden mit " << config_.mqtt_broker 
                  << ":" << config_.mqtt_port << "\n";
        return true;

    } catch (const mqtt::exception& e) {
        std::cerr << "[MQTT] ERROR: " << e.what() << "\n";
        return false;
    }
}

void MqttPublisher::disconnect() {
    if (!connected_.exchange(false)) {
        return;
    }

    try {
        auto tok = client_->disconnect();
        tok->wait_for(std::chrono::seconds(1));
    } catch (...) {
        // Ignore errors during disconnect
    }

    std::cout << "[MQTT] Getrennt\n";
}

void MqttPublisher::publish_string(const std::string& topic, const std::string& value) {
    publish(topic, value.data(), value.size());
}

} // namespace ads_realtime
