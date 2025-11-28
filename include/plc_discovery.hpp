#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <cstdint>

namespace ads {

struct PlcRoute {
    std::string name;
    std::string amsNetId;
    std::string ipAddress;
    uint16_t port;
    bool connected;
    uint64_t lastSeen;
};

struct SymbolInfo {
    std::string name;
    std::string type;
    uint32_t indexGroup;
    uint32_t indexOffset;
    uint32_t size;
    std::string comment;
};

class PlcDiscovery {
public:
    PlcDiscovery();
    ~PlcDiscovery();

    // Multi-PLC Management
    bool addRoute(const PlcRoute& route);
    bool removeRoute(const std::string& amsNetId);
    std::vector<PlcRoute> getRoutes() const;
    bool connectToPlc(const std::string& amsNetId);
    void disconnectFromPlc(const std::string& amsNetId);

    // Symbol Discovery
    std::vector<SymbolInfo> discoverSymbols(const std::string& amsNetId);
    bool subscribeToSymbol(const std::string& amsNetId, const SymbolInfo& symbol);
    void unsubscribeFromSymbol(const std::string& amsNetId, const std::string& symbolName);

    // Auto-Discovery
    std::vector<PlcRoute> scanNetwork(const std::string& subnet = "192.168.0.0/24");
    void startAutoDiscovery(uint32_t intervalSeconds = 30);
    void stopAutoDiscovery();

private:
    mutable std::mutex mutex_;
    std::map<std::string, std::shared_ptr<void>> connections_; // ADS connections per AmsNetId
    std::map<std::string, std::vector<SymbolInfo>> symbols_;
    bool autoDiscoveryRunning_;
    std::thread autoDiscoveryThread_;

    void autoDiscoveryLoop(uint32_t intervalSeconds);
    bool pingPlc(const std::string& ipAddress, uint16_t port);
};

} // namespace ads
