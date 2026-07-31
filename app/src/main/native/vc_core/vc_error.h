#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

namespace vc_core {

// Values intentionally match VcCoreFailure's Kotlin constants.
enum class CoreError : std::int32_t {
    kInvalidCredentialsOrFormat = 1,
    kCorruptHeader = 2,
    kUnsupportedAlgorithm = 3,
    kUnsupportedFileSystem = 4,
    kInsufficientMemory = 5,
    kSourceNotSeekable = 6,
    kReadOnlySource = 7,
    kHiddenVolumeRisk = 8,
    kCancelled = 9,
    kIoInterrupted = 10,
    kNotFound = 11,
};

class CoreException final : public std::runtime_error {
public:
    explicit CoreException(CoreError error, std::string detail = "vc_core failure")
        : std::runtime_error(std::move(detail)), error_(error) {}
    CoreError error() const noexcept { return error_; }

private:
    CoreError error_;
};

}  // namespace vc_core
