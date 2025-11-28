#pragma once

#ifdef _WIN32
#include <windows.h>
#include <cstdint>
#include <functional>
#include <string>

// RTSS (Real-Time SubSystem) Integration für Kithara/INtime
// Benötigt: Kithara RealTime Suite oder INtime RTOS Installation

namespace rtss {

// RTSS Priority Levels (höher als Windows TIME_CRITICAL)
enum class RtssPriority {
    LOW = 1,
    NORMAL = 127,
    HIGH = 200,
    CRITICAL = 255  // Höchste Priorität
};

// RTSS Thread Configuration
struct RtssConfig {
    RtssPriority priority = RtssPriority::CRITICAL;
    uint32_t cpu_affinity = 1;  // CPU Core 1 für RTSS
    uint32_t stack_size_kb = 256;
    bool use_dedicated_core = true;
};

class RtssThread {
public:
    RtssThread(const RtssConfig& config = RtssConfig())
        : config_(config), rtss_handle_(nullptr), running_(false) {}

    ~RtssThread() {
        stop();
    }

    // Starte RTSS Thread
    bool start(std::function<void()> task) {
        if (running_) return false;

        task_ = task;

#ifdef HAS_RTSS_SDK
        // Kithara/INtime RTSS API
        // rtss_handle_ = RtCreateThread(rtss_thread_proc, this, 
        //                               config_.stack_size_kb * 1024,
        //                               static_cast<int>(config_.priority));
        
        // Placeholder für RTSS SDK
        return false;  // RTSS SDK nicht verfügbar
#else
        // Fallback: Windows High-Priority Thread
        thread_handle_ = CreateThread(
            nullptr,
            config_.stack_size_kb * 1024,
            windows_thread_proc,
            this,
            CREATE_SUSPENDED,
            nullptr
        );

        if (!thread_handle_) return false;

        // Setze höchste Windows Priorität
        SetThreadPriority(thread_handle_, THREAD_PRIORITY_TIME_CRITICAL);
        
        // CPU Affinity
        if (config_.use_dedicated_core) {
            DWORD_PTR mask = 1ULL << config_.cpu_affinity;
            SetThreadAffinityMask(thread_handle_, mask);
        }

        running_ = true;
        ResumeThread(thread_handle_);
        return true;
#endif
    }

    void stop() {
        if (!running_) return;
        
        running_ = false;

#ifdef HAS_RTSS_SDK
        // RtDeleteThread(rtss_handle_);
        rtss_handle_ = nullptr;
#else
        if (thread_handle_) {
            WaitForSingleObject(thread_handle_, 1000);
            CloseHandle(thread_handle_);
            thread_handle_ = nullptr;
        }
#endif
    }

    bool is_running() const { return running_; }

private:
    static DWORD WINAPI windows_thread_proc(LPVOID param) {
        auto* self = static_cast<RtssThread*>(param);
        if (self->task_) {
            self->task_();
        }
        return 0;
    }

    RtssConfig config_;
    void* rtss_handle_;
    HANDLE thread_handle_ = nullptr;
    bool running_;
    std::function<void()> task_;
};

// RTSS High-Resolution Timer (Sub-Microsecond)
class RtssTimer {
public:
    RtssTimer() {
        QueryPerformanceFrequency(&freq_);
    }

    // Warte mit RTSS-Präzision (Busy-Wait für <10µs)
    void wait_us(uint32_t microseconds) {
        if (microseconds < 10) {
            // Busy-Wait für ultra-kurze Delays
            LARGE_INTEGER start, now;
            QueryPerformanceCounter(&start);
            
            int64_t target_ticks = (microseconds * freq_.QuadPart) / 1000000;
            
            do {
                QueryPerformanceCounter(&now);
            } while ((now.QuadPart - start.QuadPart) < target_ticks);
        } else {
            // Sleep für längere Delays
            Sleep((microseconds + 999) / 1000);
        }
    }

    // Aktuelle Zeit in Nanosekunden
    uint64_t now_ns() {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (now.QuadPart * 1000000000ULL) / freq_.QuadPart;
    }

private:
    LARGE_INTEGER freq_;
};

// RTSS Memory Pool (Lock-Free, RTSS-kompatibel)
template<typename T, size_t PoolSize = 1024>
class RtssMemoryPool {
public:
    RtssMemoryPool() : next_index_(0) {
        // Pre-allocate all memory
        for (size_t i = 0; i < PoolSize; ++i) {
            pool_[i].in_use = false;
        }
    }

    T* allocate() {
        for (size_t i = 0; i < PoolSize; ++i) {
            size_t idx = (next_index_++) % PoolSize;
            
            bool expected = false;
            if (pool_[idx].in_use.compare_exchange_strong(expected, true)) {
                return &pool_[idx].data;
            }
        }
        return nullptr;  // Pool exhausted
    }

    void deallocate(T* ptr) {
        if (!ptr) return;

        // Find entry and mark as free
        for (size_t i = 0; i < PoolSize; ++i) {
            if (&pool_[i].data == ptr) {
                pool_[i].in_use.store(false);
                break;
            }
        }
    }

private:
    struct Entry {
        std::atomic<bool> in_use;
        T data;
    };

    Entry pool_[PoolSize];
    std::atomic<size_t> next_index_;
};

// RTSS-optimierte ADS Notification Handler
class RtssAdsHandler {
public:
    using NotificationCallback = std::function<void(
        const void* data, 
        size_t size, 
        uint64_t timestamp_ns
    )>;

    RtssAdsHandler(const RtssConfig& config = RtssConfig())
        : rtss_thread_(config), timer_() {}

    bool start(NotificationCallback callback) {
        callback_ = callback;
        
        return rtss_thread_.start([this]() {
            rtss_processing_loop();
        });
    }

    void stop() {
        rtss_thread_.stop();
    }

    // Simuliere ADS Notification (für Testing)
    void simulate_notification(const void* data, size_t size) {
        if (callback_) {
            uint64_t timestamp = timer_.now_ns();
            callback_(data, size, timestamp);
        }
    }

private:
    void rtss_processing_loop() {
        while (rtss_thread_.is_running()) {
            // RTSS Cycle: 100µs
            timer_.wait_us(100);
            
            // Hier würde ADS Notification Processing stattfinden
            // Im echten RTSS würde dies durch Hardware-Interrupts getriggert
        }
    }

    RtssThread rtss_thread_;
    RtssTimer timer_;
    NotificationCallback callback_;
};

} // namespace rtss

#endif // _WIN32
