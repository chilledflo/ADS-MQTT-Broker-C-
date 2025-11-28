#pragma once

#ifdef __linux__
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>
#include <sys/mlock.h>
#include <cstdint>
#include <functional>
#include <string>

// Linux RT_PREEMPT Support für Hard Real-Time
// Benötigt: Linux Kernel mit CONFIG_PREEMPT_RT Patch

namespace linux_rt {

// RT Scheduling Policies
enum class RtPolicy {
    FIFO = SCHED_FIFO,     // First-In-First-Out
    RR = SCHED_RR,         // Round-Robin
    DEADLINE = SCHED_DEADLINE  // Earliest Deadline First (Kernel 3.14+)
};

// RT Thread Configuration
struct RtConfig {
    RtPolicy policy = RtPolicy::FIFO;
    int priority = 99;  // 1-99, 99 = höchste Priorität
    uint32_t cpu_affinity = 1;  // CPU Core
    bool lock_memory = true;    // mlockall für no page faults
    size_t stack_size_kb = 8192;  // 8MB Stack
};

class RtThread {
public:
    RtThread(const RtConfig& config = RtConfig())
        : config_(config), running_(false) {}

    ~RtThread() {
        stop();
    }

    // Starte RT Thread
    bool start(std::function<void()> task) {
        if (running_) return false;

        task_ = task;

        // Memory locking (verhindert page faults)
        if (config_.lock_memory) {
            if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
                // Benötigt CAP_IPC_LOCK capability
                return false;
            }
        }

        // Thread Attributes
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        // Stack Size
        pthread_attr_setstacksize(&attr, config_.stack_size_kb * 1024);

        // Scheduling Policy & Priority
        struct sched_param param;
        param.sched_priority = config_.priority;
        pthread_attr_setschedpolicy(&attr, static_cast<int>(config_.policy));
        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

        // CPU Affinity
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(config_.cpu_affinity, &cpuset);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);

        running_ = true;
        int ret = pthread_create(&thread_, &attr, thread_proc, this);
        pthread_attr_destroy(&attr);

        if (ret != 0) {
            running_ = false;
            return false;
        }

        return true;
    }

    void stop() {
        if (!running_) return;
        
        running_ = false;
        pthread_join(thread_, nullptr);

        if (config_.lock_memory) {
            munlockall();
        }
    }

    bool is_running() const { return running_; }

private:
    static void* thread_proc(void* param) {
        auto* self = static_cast<RtThread*>(param);
        
        // Pre-fault stack (verhindert page faults während RT execution)
        unsigned char dummy[self->config_.stack_size_kb * 1024];
        memset(dummy, 0, sizeof(dummy));

        if (self->task_) {
            self->task_();
        }
        return nullptr;
    }

    RtConfig config_;
    pthread_t thread_;
    bool running_;
    std::function<void()> task_;
};

// High-Resolution Timer (clock_nanosleep)
class RtTimer {
public:
    RtTimer() {}

    // Präzises Warten mit clock_nanosleep (unterbrechbar)
    void wait_ns(uint64_t nanoseconds) {
        struct timespec ts;
        ts.tv_sec = nanoseconds / 1000000000ULL;
        ts.tv_nsec = nanoseconds % 1000000000ULL;
        
        clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, nullptr);
    }

    void wait_us(uint32_t microseconds) {
        wait_ns(microseconds * 1000ULL);
    }

    // Aktuelle monotone Zeit in Nanosekunden
    uint64_t now_ns() {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
    }

    // Absolute Deadline Wait (für periodische Tasks)
    void wait_until_ns(uint64_t target_ns) {
        struct timespec ts;
        ts.tv_sec = target_ns / 1000000000ULL;
        ts.tv_nsec = target_ns % 1000000000ULL;
        
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, nullptr);
    }
};

// Cyclictest-style Latency Measurement
class RtLatencyTracker {
public:
    RtLatencyTracker() : min_ns_(UINT64_MAX), max_ns_(0), count_(0), sum_ns_(0) {}

    void record(uint64_t latency_ns) {
        if (latency_ns < min_ns_) min_ns_ = latency_ns;
        if (latency_ns > max_ns_) max_ns_ = latency_ns;
        sum_ns_ += latency_ns;
        count_++;
    }

    uint64_t min_ns() const { return min_ns_; }
    uint64_t max_ns() const { return max_ns_; }
    uint64_t avg_ns() const { return count_ > 0 ? sum_ns_ / count_ : 0; }
    uint64_t count() const { return count_; }

    void reset() {
        min_ns_ = UINT64_MAX;
        max_ns_ = 0;
        sum_ns_ = 0;
        count_ = 0;
    }

private:
    uint64_t min_ns_;
    uint64_t max_ns_;
    uint64_t count_;
    uint64_t sum_ns_;
};

// RT-kompatible Periodic Task
class RtPeriodicTask {
public:
    RtPeriodicTask(uint32_t period_us, const RtConfig& config = RtConfig())
        : period_ns_(period_us * 1000ULL), rt_thread_(config), timer_() {}

    bool start(std::function<void()> task) {
        task_ = task;
        
        return rt_thread_.start([this]() {
            periodic_loop();
        });
    }

    void stop() {
        rt_thread_.stop();
    }

    const RtLatencyTracker& latency_stats() const { return latency_tracker_; }

private:
    void periodic_loop() {
        uint64_t next_wake = timer_.now_ns() + period_ns_;

        while (rt_thread_.is_running()) {
            uint64_t start = timer_.now_ns();
            
            // Führe Task aus
            if (task_) {
                task_();
            }

            uint64_t end = timer_.now_ns();
            uint64_t latency = end - next_wake;
            
            latency_tracker_.record(latency);

            // Warte bis nächste Period
            next_wake += period_ns_;
            timer_.wait_until_ns(next_wake);
        }
    }

    uint64_t period_ns_;
    RtThread rt_thread_;
    RtTimer timer_;
    RtLatencyTracker latency_tracker_;
    std::function<void()> task_;
};

// Check RT Kernel
inline bool has_rt_preempt() {
    // Prüfe /sys/kernel/realtime
    FILE* f = fopen("/sys/kernel/realtime", "r");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}

// Check verfügbare RT Priorities
inline int get_max_rt_priority(RtPolicy policy) {
    return sched_get_priority_max(static_cast<int>(policy));
}

} // namespace linux_rt

#endif // __linux__
