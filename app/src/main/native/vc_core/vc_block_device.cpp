#include "vc_block_device.h"

#include <limits>
#include <stdexcept>

#include "vc_error.h"

namespace vc_core {

EncryptedBlockDevice::EncryptedBlockDevice(NativeVolumeSession& session)
    : session_(session),
      sector_size_(session.metadata().sector_size),
      sector_count_(sector_size_ == 0 ? 0 : session.metadata().volume_data_size / sector_size_),
      writable_(session.writable()) {
    // Access mode is enforced by NativeVolumeSession::Write. Keep the block
    // device geometry strict so a filesystem cannot address partial sectors.
    if (sector_size_ == 0 || session.metadata().volume_data_size % sector_size_ != 0) {
        throw std::invalid_argument("VeraCrypt volume has invalid block-device geometry");
    }
}

void EncryptedBlockDevice::CheckRange(std::uint64_t first_sector, std::uint32_t count) const {
    if (count == 0 || first_sector > sector_count_ || count > sector_count_ - first_sector) {
        throw std::out_of_range("Filesystem block range is outside the unlocked volume");
    }
}

void EncryptedBlockDevice::ReadSectors(
        std::uint64_t first_sector, std::uint32_t count, std::uint8_t* destination) {
    if (destination == nullptr) throw std::invalid_argument("Filesystem read destination is null");
    CheckRange(first_sector, count);
    const std::uint64_t offset = first_sector * static_cast<std::uint64_t>(sector_size_);
    const std::uint64_t byte_count = static_cast<std::uint64_t>(count) * sector_size_;
    if (byte_count > std::numeric_limits<std::size_t>::max()) throw std::out_of_range("Filesystem read is too large");
    session_.Read(offset, destination, static_cast<std::size_t>(byte_count));
}

void EncryptedBlockDevice::WriteSectors(
        std::uint64_t first_sector, std::uint32_t count, const std::uint8_t* source) {
    if (source == nullptr) throw std::invalid_argument("Filesystem write source is null");
    CheckRange(first_sector, count);
    const std::uint64_t offset = first_sector * static_cast<std::uint64_t>(sector_size_);
    const std::uint64_t byte_count = static_cast<std::uint64_t>(count) * sector_size_;
    if (byte_count > std::numeric_limits<std::size_t>::max()) throw std::out_of_range("Filesystem write is too large");
    if (!writable_) throw CoreException(CoreError::kReadOnlySource);
    session_.Write(offset, source, static_cast<std::size_t>(byte_count));
}

void EncryptedBlockDevice::Flush() {
    session_.Flush();
}

}  // namespace vc_core
