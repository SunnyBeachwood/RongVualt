#pragma once

#include <cstdint>

#include "vc_block_device.h"

namespace vc_core {

/** Registers an encrypted block device as one process-local FatFs drive. */
class FatFsDiskIo final {
public:
    explicit FatFsDiskIo(EncryptedBlockDevice& device);
    FatFsDiskIo(const FatFsDiskIo&) = delete;
    FatFsDiskIo& operator=(const FatFsDiskIo&) = delete;
    ~FatFsDiskIo();

    std::uint8_t drive_number() const noexcept { return drive_number_; }

private:
    std::uint8_t drive_number_;
};

}  // namespace vc_core
