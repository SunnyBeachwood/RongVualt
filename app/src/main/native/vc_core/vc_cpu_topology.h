#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace vc_core {

/** Runtime CPU facts used only to size and place native crypto workers. */
struct CpuTopology final {
    std::vector<int> allowed_cpus;
    std::vector<int> performance_cpus;
    std::size_t worker_count = 1;
    bool frequency_data_available = false;
    std::string degradation_reason;
};

/**
 * Returns the intersection of the process affinity mask and the highest
 * frequency CPUs visible in sysfs. This function never throws: an empty or
 * unreadable topology falls back to one normally scheduled worker.
 */
CpuTopology DetectCpuTopology();

/** Binds the calling native thread to one CPU, returning false on failure. */
bool BindCurrentThreadToCpu(int cpu, std::string* failure_reason = nullptr);

/** RAII diagnostic for a worker's one-time affinity assignment. */
class CpuAffinityLease final {
public:
    explicit CpuAffinityLease(int cpu);
    CpuAffinityLease(const CpuAffinityLease&) = delete;
    CpuAffinityLease& operator=(const CpuAffinityLease&) = delete;
    bool bound() const noexcept { return bound_; }
    const std::string& failure_reason() const noexcept { return failure_reason_; }

private:
    bool bound_ = false;
    std::string failure_reason_;
};

}  // namespace vc_core
