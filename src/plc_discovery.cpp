#include "plc_discovery.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace ads {

PlcDiscovery::PlcDiscovery() : autoDiscoveryRunning_(false) {
    spdlog::info("PlcDiscovery initialized");
}

PlcDiscovery::~PlcDiscovery() {
    stopAutoDiscovery();
    spdlog::info("PlcDiscovery destroyed");
}

bool PlcDiscovery::addRoute(const PlcRoute& route) {
    std::lock_guard<std::mutex> lock(mutex_);

    spdlog::info("Adding route: {} ({}) - {}:{}", 
        route.name, route.amsNetId, route.ipAddress, route.port);
    
    // Route zur Liste hinzufügen
    // TODO: Implementierung mit ADS-Library
    
    return true;
}

bool PlcDiscovery::removeRoute(const std::string& amsNetId) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = connections_.find(amsNetId);
    if (it != connections_.end()) {
        connections_.erase(it);
        symbols_.erase(amsNetId);
        spdlog::info("Route removed: {}", amsNetId);
        return true;
    }

    return false;
}

std::vector<PlcRoute> PlcDiscovery::getRoutes() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<PlcRoute> routes;
    // TODO: Implementierung - Alle Routes zurückgeben

    return routes;
}

bool PlcDiscovery::connectToPlc(const std::string& amsNetId) {
    std::lock_guard<std::mutex> lock(mutex_);

    spdlog::info("Connecting to PLC: {}", amsNetId);

    // TODO: ADS Connection erstellen
    // connections_[amsNetId] = std::make_shared<AdsConnection>();
    
    return true;
}

void PlcDiscovery::disconnectFromPlc(const std::string& amsNetId) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = connections_.find(amsNetId);
    if (it != connections_.end()) {
        connections_.erase(it);
        spdlog::info("Disconnected from PLC: {}", amsNetId);
    }
}

std::vector<SymbolInfo> PlcDiscovery::discoverSymbols(const std::string& amsNetId) {
    std::lock_guard<std::mutex> lock(mutex_);

    spdlog::info("Discovering symbols on PLC: {}", amsNetId);
    
    std::vector<SymbolInfo> symbols;

    // TODO: ADS Symbol Upload durchführen
    // 1. ReadSymbolInfo
    // 2. ReadSymbolByName für alle Symbole
    // 3. Parse Symbol-Tabelle

    symbols_[amsNetId] = symbols;

    spdlog::info("Discovered {} symbols on {}", symbols.size(), amsNetId);
    return symbols;
}

bool PlcDiscovery::subscribeToSymbol(const std::string& amsNetId, const SymbolInfo& symbol) {
    std::lock_guard<std::mutex> lock(mutex_);

    spdlog::info("Subscribing to symbol: {} on PLC {}", symbol.name, amsNetId);
    
    // TODO: ADS Notification erstellen
    // AddDeviceNotification für zyklisches Lesen

    return true;
}

void PlcDiscovery::unsubscribeFromSymbol(const std::string& amsNetId, const std::string& symbolName) {
    std::lock_guard<std::mutex> lock(mutex_);

    spdlog::info("Unsubscribing from symbol: {} on PLC {}", symbolName, amsNetId);
    
    // TODO: ADS Notification löschen
    // DeleteDeviceNotification
}

std::vector<PlcRoute> PlcDiscovery::scanNetwork(const std::string& subnet) {
    spdlog::info("Scanning network: {}", subnet);

    std::vector<PlcRoute> foundPlcs;

    // Parse Subnet (z.B. 192.168.0.0/24)
    // Für jeden IP-Bereich:
    //   - Ping auf Port 48898 (ADS)
    //   - Wenn erfolgreich: Route erstellen

    std::vector<std::string> testIps = {
        "192.168.3.42",  // Dein PLC
        "192.168.3.151",
        "192.168.0.100"
    };

    for (const auto& ip : testIps) {
        if (pingPlc(ip, 48898)) {
            PlcRoute route;
            route.name = "PLC_" + ip;
            route.amsNetId = ip + ".1.1";
            route.ipAddress = ip;
            route.port = 48898;
            route.connected = false;
            route.lastSeen = std::chrono::system_clock::now().time_since_epoch().count();
            
            foundPlcs.push_back(route);
            spdlog::info("Found PLC at {}", ip);
        }
    }

    spdlog::info("Network scan complete. Found {} PLCs", foundPlcs.size());
    return foundPlcs;
}

void PlcDiscovery::startAutoDiscovery(uint32_t intervalSeconds) {
    if (autoDiscoveryRunning_) {
        spdlog::warn("Auto-discovery already running");
        return;
    }

    autoDiscoveryRunning_ = true;
    autoDiscoveryThread_ = std::thread(&PlcDiscovery::autoDiscoveryLoop, this, intervalSeconds);
    
    spdlog::info("Auto-discovery started (interval: {}s)", intervalSeconds);
}

void PlcDiscovery::stopAutoDiscovery() {
    if (!autoDiscoveryRunning_) {
        return;
    }

    autoDiscoveryRunning_ = false;

    if (autoDiscoveryThread_.joinable()) {
        autoDiscoveryThread_.join();
    }

    spdlog::info("Auto-discovery stopped");
}

void PlcDiscovery::autoDiscoveryLoop(uint32_t intervalSeconds) {
    while (autoDiscoveryRunning_) {
        try {
            // Call with default parameter
            auto foundPlcs = scanNetwork();

            for (const auto& plc : foundPlcs) {
                addRoute(plc);
            }

        } catch (const std::exception& e) {
            spdlog::error("Error in auto-discovery: {}", e.what());
        }

        // Warte bis zum nächsten Scan
        for (uint32_t i = 0; i < intervalSeconds && autoDiscoveryRunning_; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

bool PlcDiscovery::pingPlc(const std::string& ipAddress, uint16_t port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }
#endif

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    // Timeout setzen
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
#ifdef _WIN32
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
#else
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
#endif

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &addr.sin_addr);

    bool result = (connect(sock, (sockaddr*)&addr, sizeof(addr)) == 0);

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return result;
}

} // namespace ads
