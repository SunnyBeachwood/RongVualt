#include "fd_random_access.h"

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdint>
#include <fcntl.h>
#include <limits>
#include <stdexcept>
#include <system_error>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

namespace vc_core {
namespace {

[[noreturn]] void ThrowErrno(const char* operation) {
    throw std::system_error(errno, std::generic_category(), operation);
}

void CheckOffset(std::uint64_t value, const char* message) {
    if (value > static_cast<std::uint64_t>(std::numeric_limits<off_t>::max())) {
        throw std::out_of_range(message);
    }
}

}  // namespace

FdRandomAccess FdRandomAccess::Open(int fd, bool writable) {
    if (fd < 0) throw std::invalid_argument("Container descriptor is invalid");
    const int duplicate = fcntl(fd, F_DUPFD_CLOEXEC, 0);
    if (duplicate < 0) ThrowErrno("dup container descriptor");
    try {
        struct stat status {};
        if (fstat(duplicate, &status) != 0) ThrowErrno("stat container descriptor");
        if (status.st_size < 0) throw std::invalid_argument("Container length is unknown");
        if (lseek(duplicate, 0, SEEK_CUR) == static_cast<off_t>(-1)) ThrowErrno("seek container descriptor");
        return FdRandomAccess(
                duplicate, writable, static_cast<std::uint64_t>(status.st_size),
                status.st_atim, status.st_mtim);
    } catch (...) {
        ::close(duplicate);
        throw;
    }
}

FdRandomAccess FdRandomAccess::Duplicate() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fd_ < 0) throw std::runtime_error("Container descriptor is closed");
    const int duplicate = fcntl(fd_, F_DUPFD_CLOEXEC, 0);
    if (duplicate < 0) ThrowErrno("duplicate container descriptor");
    return FdRandomAccess(duplicate, writable_, size_, access_time_, modified_time_);
}

FdRandomAccess::FdRandomAccess(int fd, bool writable, std::uint64_t size, timespec access_time, timespec modified_time)
    : fd_(fd), writable_(writable), size_(size), access_time_(access_time), modified_time_(modified_time) {}

FdRandomAccess::FdRandomAccess(FdRandomAccess&& other) noexcept {
    std::lock_guard<std::mutex> lock(other.mutex_);
    fd_ = other.fd_;
    writable_ = other.writable_;
    size_ = other.size_;
    access_time_ = other.access_time_;
    modified_time_ = other.modified_time_;
    counters_ = other.counters_;
    other.fd_ = -1;
    other.size_ = 0;
    other.counters_ = {};
}

FdRandomAccess& FdRandomAccess::operator=(FdRandomAccess&& other) noexcept {
    if (this == &other) return *this;
    std::scoped_lock lock(mutex_, other.mutex_);
    close();
    fd_ = other.fd_;
    writable_ = other.writable_;
    size_ = other.size_;
    access_time_ = other.access_time_;
    modified_time_ = other.modified_time_;
    counters_ = other.counters_;
    other.fd_ = -1;
    other.size_ = 0;
    other.counters_ = {};
    return *this;
}

FdRandomAccess::~FdRandomAccess() { close(); }

std::uint64_t FdRandomAccess::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return size_;
}

bool FdRandomAccess::SameFile(int fd) const {
    if (fd < 0) throw std::invalid_argument("Descriptor is invalid");
    std::lock_guard<std::mutex> lock(mutex_);
    if (fd_ < 0) throw std::runtime_error("Container descriptor is closed");
    struct stat current {};
    struct stat other {};
    if (fstat(fd_, &current) != 0) ThrowErrno("stat container descriptor");
    if (fstat(fd, &other) != 0) ThrowErrno("stat destination descriptor");
    return current.st_dev == other.st_dev && current.st_ino == other.st_ino;
}

void FdRandomAccess::ReadAt(std::uint64_t offset, std::uint8_t* destination, std::size_t length) const {
    if (length != 0 && destination == nullptr) throw std::invalid_argument("Read destination is null");
    std::lock_guard<std::mutex> lock(mutex_);
    checkRange(offset, length, false);
    std::size_t done = 0;
    while (done < length) {
        ++counters_.read_syscalls;
        const ssize_t read = pread(fd_, destination + done, length - done, static_cast<off_t>(offset + done));
        if (read < 0) {
            if (errno == EINTR) continue;
            ThrowErrno("read container");
        }
        if (read == 0) throw std::runtime_error("Container was truncated during read");
        done += static_cast<std::size_t>(read);
        counters_.read_bytes += static_cast<std::uint64_t>(read);
    }
}

void FdRandomAccess::WriteAt(std::uint64_t offset, const std::uint8_t* source, std::size_t length) {
    if (length != 0 && source == nullptr) throw std::invalid_argument("Write source is null");
    std::lock_guard<std::mutex> lock(mutex_);
    if (!writable_) throw std::system_error(EROFS, std::generic_category(), "write read-only container");
    checkRange(offset, length, true);
    std::size_t done = 0;
    while (done < length) {
        ++counters_.write_syscalls;
        const ssize_t written = pwrite(fd_, source + done, length - done, static_cast<off_t>(offset + done));
        if (written < 0) {
            if (errno == EINTR) continue;
            ThrowErrno("write container");
        }
        if (written == 0) throw std::runtime_error("Container write made no progress");
        done += static_cast<std::size_t>(written);
        counters_.write_bytes += static_cast<std::uint64_t>(written);
    }
    size_ = std::max(size_, offset + length);
}

void FdRandomAccess::Resize(std::uint64_t length) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!writable_) throw std::system_error(EROFS, std::generic_category(), "resize read-only container");
    CheckOffset(length, "Container size is too large");
    if (ftruncate(fd_, static_cast<off_t>(length)) != 0) ThrowErrno("resize container");
    size_ = length;
}

void FdRandomAccess::Sync() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fsync(fd_) != 0) ThrowErrno("sync container");
}

RandomAccessCounters FdRandomAccess::counters() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return counters_;
}

void FdRandomAccess::close() noexcept {
    if (fd_ >= 0) {
        // Opening a writable SAF descriptor can update a provider's observed
        // access/modified time even before the encrypted volume is changed.
        // Restore the original POSIX timestamps on close. Filesystems that do
        // not expose timestamp mutation simply reject futimens; the encrypted
        // data close must never fail because metadata restoration is optional.
        const timespec timestamps[] {access_time_, modified_time_};
        (void) futimens(fd_, timestamps);
        ::close(fd_);
        fd_ = -1;
    }
}

void FdRandomAccess::checkRange(std::uint64_t offset, std::size_t length, bool allow_end) const {
    if (fd_ < 0) throw std::runtime_error("Container descriptor is closed");
    if (length > std::numeric_limits<std::uint64_t>::max() - offset) {
        throw std::out_of_range("Container byte range overflows");
    }
    const std::uint64_t end = offset + length;
    CheckOffset(offset, "Container offset is too large");
    CheckOffset(end, "Container range is too large");
    if ((!allow_end && end > size_) || (allow_end && offset > size_)) {
        throw std::out_of_range("Container range is outside the current volume");
    }
}

}  // namespace vc_core
