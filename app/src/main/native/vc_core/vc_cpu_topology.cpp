#include "vc_cpu_topology.h"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <sched.h>
#include <string>
#include <unistd.h>

namespace vc_core {
namespace {

// Include the full high-performance cluster when a prime core advertises a
// higher boost ceiling (for example 2.85 GHz + 3.4 GHz on the same cluster),
// while excluding the 2.0 GHz efficiency cluster on the validation device.
constexpr double kPerformanceFrequencyRatio = 0.80;

bool ReadFrequency(int cpu, long* frequency) {
    if (frequency == nullptr) return false;
    char path[256] {};
    std::snprintf(path, sizeof(path),
                  "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", cpu);
    FILE* file = std::fopen(path, "r");
    if (file == nullptr) {
        std::snprintf(path, sizeof(path),
                      "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", cpu);
        file = std::fopen(path, "r");
    }
    if (file == nullptr) return false;
    long value = 0;
    const bool ok = std::fscanf(file, "%ld", &value) == 1 && value > 0;
    std::fclose(file);
    if (ok) *frequency = value;
    return ok;
}

std::string ErrnoText(const char* operation, int error) {
    return std::string(operation) + ": " + std::strerror(error);
}

}  // namespace

CpuTopology DetectCpuTopology() {
    CpuTopology result;
    cpu_set_t mask;
    CPU_ZERO(&mask);
    if (sched_getaffinity(0, sizeof(mask), &mask) != 0) {
        result.worker_count = 1;
        result.degradation_reason = ErrnoText("sched_getaffinity", errno);
        return result;
    }

    const long configured = sysconf(_SC_NPROCESSORS_CONF);
    const int cpu_limit = static_cast<int>(configured > 0
                                                   ? std::min<long>(configured, CPU_SETSIZE)
                                                   : CPU_SETSIZE);
    for (int cpu = 0; cpu < cpu_limit; ++cpu) {
        if (CPU_ISSET(cpu, &mask)) result.allowed_cpus.push_back(cpu);
    }
    if (result.allowed_cpus.empty()) {
        result.worker_count = 1;
        result.degradation_reason = "process affinity mask contains no CPUs";
        return result;
    }

    long maximum_frequency = 0;
    std::vector<long> frequencies;
    frequencies.reserve(result.allowed_cpus.size());
    for (int cpu : result.allowed_cpus) {
        long frequency = 0;
        if (ReadFrequency(cpu, &frequency)) {
            result.frequency_data_available = true;
            maximum_frequency = std::max(maximum_frequency, frequency);
        }
        frequencies.push_back(frequency);
    }
    if (result.frequency_data_available && maximum_frequency > 0) {
        const long threshold = static_cast<long>(std::ceil(
                static_cast<double>(maximum_frequency) * kPerformanceFrequencyRatio));
        for (std::size_t index = 0; index < result.allowed_cpus.size(); ++index) {
            if (frequencies[index] >= threshold) result.performance_cpus.push_back(result.allowed_cpus[index]);
        }
    }
    if (result.performance_cpus.empty()) {
        // A device may hide cpufreq sysfs from an app. All allowed CPUs are a
        // safe fallback; the scheduler still handles online/offline changes.
        result.performance_cpus = result.allowed_cpus;
        if (!result.frequency_data_available) result.degradation_reason = "CPU frequency topology unavailable";
    }
    // Keep one native context per selected performance CPU. The transfer
    // layer independently bounds queued I/O buffers, so crypto placement is
    // not silently reduced on high-core-count devices.
    result.worker_count = std::max<std::size_t>(1, result.performance_cpus.size());
    return result;
}

bool BindCurrentThreadToCpu(int cpu, std::string* failure_reason) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);
    if (sched_setaffinity(0, sizeof(mask), &mask) == 0) return true;
    if (failure_reason != nullptr) *failure_reason = ErrnoText("sched_setaffinity", errno);
    return false;
}

CpuAffinityLease::CpuAffinityLease(int cpu) {
    bound_ = BindCurrentThreadToCpu(cpu, &failure_reason_);
}

}  // namespace vc_core
