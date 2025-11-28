#include "../include/linux_rt_preempt.hpp"
#include <iostream>
#include <signal.h>
#include <atomic>

#ifdef __linux__

std::atomic<bool> running(true);

void signal_handler(int) {
    running = false;
}

int main() {
    std::cout << "=== Linux RT_PREEMPT Demo ===" << std::endl;

    // Check RT Kernel
    if (!linux_rt::has_rt_preempt()) {
        std::cout << "⚠️  Warning: RT_PREEMPT kernel not detected" << std::endl;
        std::cout << "   For hard real-time, use a PREEMPT_RT patched kernel" << std::endl;
        std::cout << "   See: https://wiki.linuxfoundation.org/realtime/start" << std::endl;
    } else {
        std::cout << "✅ RT_PREEMPT kernel detected" << std::endl;
    }

    // Check RT Priorities
    int max_fifo = linux_rt::get_max_rt_priority(linux_rt::RtPolicy::FIFO);
    std::cout << "Max FIFO Priority: " << max_fifo << std::endl;

    // RT Thread Configuration
    linux_rt::RtConfig config;
    config.policy = linux_rt::RtPolicy::FIFO;
    config.priority = 99;  // Highest
    config.cpu_affinity = 1;  // CPU Core 1
    config.lock_memory = true;

    // Periodic Task: 100µs cycle
    linux_rt::RtPeriodicTask periodic_task(100, config);

    std::cout << "\n🚀 Starting RT periodic task (100µs cycle)..." << std::endl;
    std::cout << "   Press Ctrl+C to stop" << std::endl;

    signal(SIGINT, signal_handler);

    uint32_t counter = 0;
    bool started = periodic_task.start([&counter]() {
        // Ultra-fast processing
        counter++;
        
        // Simuliere ADS variable processing
        volatile float temperature = 25.5f + (counter % 100) * 0.1f;
        (void)temperature;  // Suppress unused warning
    });

    if (!started) {
        std::cout << "❌ Failed to start RT thread" << std::endl;
        std::cout << "   Run as root or with CAP_SYS_NICE capability:" << std::endl;
        std::cout << "   sudo ./linux_rt_example" << std::endl;
        return 1;
    }

    std::cout << "✅ RT thread started with FIFO priority 99" << std::endl;

    // Laufe für 10 Sekunden
    for (int i = 0; i < 10 && running; ++i) {
        sleep(1);
        auto stats = periodic_task.latency_stats();
        
        std::cout << "\r[" << (i+1) << "s] "
                  << "Count: " << stats.count() << " | "
                  << "Min: " << stats.min_ns() / 1000 << "µs | "
                  << "Avg: " << stats.avg_ns() / 1000 << "µs | "
                  << "Max: " << stats.max_ns() / 1000 << "µs"
                  << std::flush;
    }

    periodic_task.stop();

    auto final_stats = periodic_task.latency_stats();
    
    std::cout << "\n\n📈 Final Latency Statistics:" << std::endl;
    std::cout << "   Total Cycles: " << final_stats.count() << std::endl;
    std::cout << "   Min Latency: " << final_stats.min_ns() / 1000 << "µs" << std::endl;
    std::cout << "   Avg Latency: " << final_stats.avg_ns() / 1000 << "µs" << std::endl;
    std::cout << "   Max Latency: " << final_stats.max_ns() / 1000 << "µs" << std::endl;

    if (final_stats.max_ns() < 1000000) {  // <1ms
        std::cout << "   ✅ Hard real-time constraint met (<1ms)" << std::endl;
    } else {
        std::cout << "   ⚠️  Deadline miss detected!" << std::endl;
    }

    // RT Timer Demo
    std::cout << "\n⏱️  RT Timer Precision Test:" << std::endl;
    linux_rt::RtTimer timer;
    
    for (uint32_t delay_us : {1, 10, 100, 1000}) {
        uint64_t start = timer.now_ns();
        timer.wait_us(delay_us);
        uint64_t end = timer.now_ns();
        uint64_t actual_us = (end - start) / 1000;
        
        std::cout << "   Target: " << delay_us << "µs -> "
                  << "Actual: " << actual_us << "µs "
                  << "(Error: " << ((int64_t)actual_us - delay_us) << "µs)"
                  << std::endl;
    }

    std::cout << "\n✅ RT_PREEMPT Demo completed" << std::endl;
    std::cout << "\n📝 For production RT_PREEMPT setup:" << std::endl;
    std::cout << "   1. Install RT kernel: apt install linux-image-rt-amd64" << std::endl;
    std::cout << "   2. Set RT limits: /etc/security/limits.conf" << std::endl;
    std::cout << "   3. Isolate CPUs: isolcpus=1,2,3 in grub" << std::endl;
    std::cout << "   4. Run with: sudo chrt -f 99 ./program" << std::endl;

    return 0;
}

#else
int main() {
    std::cout << "RT_PREEMPT is Linux-only. Use RTSS on Windows." << std::endl;
    return 1;
}
#endif
