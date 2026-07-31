#pragma once

#include <cstddef>
#include <cstdint>

#include "vc_volume.h"

namespace vc_core {

/**
 * Filesystem-facing logical block device backed exclusively by an unlocked
 * native volume session. It never exposes the container descriptor, XTS key,
 * encrypted offsets, or host paths to a filesystem library.
 */
class EncryptedBlockDevice final {
public:
    explicit EncryptedBlockDevice(NativeVolumeSession& session);

    std::uint32_t sector_size() const noexcept { return sector_size_; }
    std::uint64_t sector_count() const noexcept { return sector_count_; }
    bool writable() const noexcept { return writable_; }
    bool hidden_volume_protection_triggered() const noexcept {
        return session_.hidden_volume_protection_triggered();
    }

    void ReadSectors(std::uint64_t first_sector, std::uint32_t count, std::uint8_t* destination);
    void WriteSectors(std::uint64_t first_sector, std::uint32_t count, const std::uint8_t* source);
    void Flush();

private:
    void CheckRange(std::uint64_t first_sector, std::uint32_t count) const;

    NativeVolumeSession& session_;
    std::uint32_t sector_size_;
    std::uint64_t sector_count_;
    bool writable_;
};

}  // namespace vc_core
