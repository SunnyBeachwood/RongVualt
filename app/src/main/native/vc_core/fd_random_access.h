#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <sys/types.h>
#include <time.h>

namespace vc_core {

/** Aggregate descriptor I/O facts; contains no path or data content. */
struct RandomAccessCounters final {
    std::uint64_t read_syscalls = 0;
    std::uint64_t write_syscalls = 0;
    std::uint64_t read_bytes = 0;
    std::uint64_t write_bytes = 0;
};

/**
 * Owns a duplicated, seekable SAF descriptor. All offsets are checked before
 * syscall dispatch; this class never translates a URI or filesystem path.
 */
class FdRandomAccess final {
public:
    static FdRandomAccess Open(int fd, bool writable);
    /** Returns an independently owned descriptor for a child native session. */
    FdRandomAccess Duplicate() const;

    FdRandomAccess(const FdRandomAccess&) = delete;
    FdRandomAccess& operator=(const FdRandomAccess&) = delete;
    FdRandomAccess(FdRandomAccess&& other) noexcept;
    FdRandomAccess& operator=(FdRandomAccess&& other) noexcept;
    ~FdRandomAccess();

    std::uint64_t size() const;
    bool writable() const noexcept { return writable_; }
    void ReadAt(std::uint64_t offset, std::uint8_t* destination, std::size_t length) const;
    void WriteAt(std::uint64_t offset, const std::uint8_t* source, std::size_t length);
    void Resize(std::uint64_t length);
    void Sync() const;
    RandomAccessCounters counters() const;

private:
    FdRandomAccess(int fd, bool writable, std::uint64_t size, timespec access_time, timespec modified_time);
    void close() noexcept;
    void checkRange(std::uint64_t offset, std::size_t length, bool allow_end) const;

    mutable std::mutex mutex_;
    int fd_ = -1;
    bool writable_ = false;
    std::uint64_t size_ = 0;
    timespec access_time_ {};
    timespec modified_time_ {};
    mutable RandomAccessCounters counters_ {};
};

}  // namespace vc_core
