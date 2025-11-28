#include "../include/rtss_integration.hpp"
#include <iostream>
#include <chrono>
#include <vector>

#ifdef _WIN32

int main() {
    std::cout << "=== RTSS Integration Demo ===" << std::endl;
    
    // RTSS Configuration
    rtss::RtssConfig config;
    config.priority = rtss::RtssPriority::CRITICAL;
    config.cpu_affinity = 1;  // CPU Core 1
    config.use_dedicated_core = true;

    // RTSS ADS Handler
    rtss::RtssAdsHandler handler(config);
    
    std::vector<uint64_t> latencies;
    rtss::RtssTimer timer;

    // Starte RTSS Thread
    bool started = handler.start([&](const void* data, size_t size, uint64_t timestamp) {
        // Ultra-fast notification processing
        uint64_t now = timer.now_ns();
        uint64_t latency = now - timestamp;
        
        if (latencies.size() < 10000) {
            latencies.push_back(latency);
        }
    });

    if (!started) {
        std::cout << "❌ RTSS SDK not available - using Windows fallback" << std::endl;
        std::cout << "   For true RTSS: Install Kithara RealTime Suite or INtime" << std::endl;
    } else {
        std::cout << "✅ RTSS Thread started with CRITICAL priority" << std::endl;
    }

    // Simuliere ADS Notifications
    std::cout << "\n📊 Simulating 10000 ADS notifications..." << std::endl;
    
    uint32_t test_value = 0;
    for (int i = 0; i < 10000; ++i) {
        handler.simulate_notification(&test_value, sizeof(test_value));
        timer.wait_us(100);  // 100µs cycle
        test_value++;
    }

    handler.stop();

    // Statistiken
    if (!latencies.empty()) {
        std::sort(latencies.begin(), latencies.end());
        
        uint64_t min = latencies.front();
        uint64_t max = latencies.back();
        uint64_t p50 = latencies[latencies.size() / 2];
        uint64_t p95 = latencies[latencies.size() * 95 / 100];
        uint64_t p99 = latencies[latencies.size() * 99 / 100];

        std::cout << "\n📈 Latency Statistics:" << std::endl;
        std::cout << "   Min: " << min / 1000 << "µs" << std::endl;
        std::cout << "   P50: " << p50 / 1000 << "µs" << std::endl;
        std::cout << "   P95: " << p95 / 1000 << "µs" << std::endl;
        std::cout << "   P99: " << p99 / 1000 << "µs" << std::endl;
        std::cout << "   Max: " << max / 1000 << "µs" << std::endl;
    }

    // RTSS Memory Pool Demo
    std::cout << "\n💾 Testing RTSS Memory Pool..." << std::endl;
    rtss::RtssMemoryPool<uint64_t, 1024> pool;
    
    uint64_t* ptr1 = pool.allocate();
    uint64_t* ptr2 = pool.allocate();
    
    if (ptr1 && ptr2) {
        *ptr1 = 12345;
        *ptr2 = 67890;
        std::cout << "   ✅ Allocated: " << *ptr1 << ", " << *ptr2 << std::endl;
        
        pool.deallocate(ptr1);
        pool.deallocate(ptr2);
        std::cout << "   ✅ Deallocated" << std::endl;
    }

    std::cout << "\n✅ RTSS Demo completed" << std::endl;
    std::cout << "\n📝 Note: For production RTSS, install:" << std::endl;
    std::cout << "   - Kithara RealTime Suite (https://www.kithara.com/)" << std::endl;
    std::cout << "   - INtime RTOS (https://www.tenasys.com/)" << std::endl;

    return 0;
}

#else
int main() {
    std::cout << "RTSS is Windows-only. Use linux_rt_preempt on Linux." << std::endl;
    return 1;
}
#endif
