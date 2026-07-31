#include "vc_header.h"

#include <limits>
#include <stdexcept>

namespace vc_core {
namespace {

constexpr std::size_t kSignatureOffset = 64;
constexpr std::size_t kHeaderVersionOffset = 68;
constexpr std::size_t kRequiredVersionOffset = 70;
constexpr std::size_t kKeyAreaCrcOffset = 72;
constexpr std::size_t kHiddenVolumeSizeOffset = 92;
constexpr std::size_t kVolumeDataSizeOffset = 100;
constexpr std::size_t kEncryptedAreaOffset = 108;
constexpr std::size_t kEncryptedAreaSizeOffset = 116;
constexpr std::size_t kFlagsOffset = 124;
constexpr std::size_t kSectorSizeOffset = 128;
constexpr std::size_t kHeaderCrcOffset = 252;
constexpr std::size_t kKeyAreaOffset = 256;
constexpr std::size_t kKeyAreaSize = 256;
constexpr std::uint16_t kMinHeaderVersion = 3;
constexpr std::uint16_t kMaxHeaderVersion = 5;

std::uint16_t ReadU16(const std::uint8_t* value) {
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(value[0]) << 8) | value[1]);
}

std::uint32_t ReadU32(const std::uint8_t* value) {
    return (static_cast<std::uint32_t>(value[0]) << 24) |
           (static_cast<std::uint32_t>(value[1]) << 16) |
           (static_cast<std::uint32_t>(value[2]) << 8) |
           static_cast<std::uint32_t>(value[3]);
}

std::uint64_t ReadU64(const std::uint8_t* value) {
    std::uint64_t result = 0;
    for (std::size_t index = 0; index < 8; ++index) result = (result << 8) | value[index];
    return result;
}

std::uint32_t Crc32(const std::uint8_t* data, std::size_t size) {
    std::uint32_t crc = 0xffffffffU;
    for (std::size_t index = 0; index < size; ++index) {
        crc ^= data[index];
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc >> 1) ^ ((crc & 1U) == 0 ? 0U : 0xedb88320U);
        }
    }
    return ~crc;
}

void RequireRange(std::uint64_t offset, std::uint64_t length) {
    if (length == 0 || offset > std::numeric_limits<std::uint64_t>::max() - length) {
        throw std::invalid_argument("VeraCrypt header has an invalid encrypted area");
    }
}

}  // namespace

HeaderMetadata ParseDecryptedVeraCryptHeader(const std::uint8_t* header, std::size_t size) {
    if (header == nullptr || size != kVeraCryptHeaderSize) {
        throw std::invalid_argument("VeraCrypt header must be exactly 512 bytes");
    }
    if (header[kSignatureOffset] != 'V' || header[kSignatureOffset + 1] != 'E' ||
        header[kSignatureOffset + 2] != 'R' || header[kSignatureOffset + 3] != 'A') {
        throw std::invalid_argument("VeraCrypt header signature is missing");
    }

    HeaderMetadata metadata {
        ReadU16(header + kHeaderVersionOffset),
        ReadU16(header + kRequiredVersionOffset),
        ReadU64(header + kHiddenVolumeSizeOffset),
        ReadU64(header + kVolumeDataSizeOffset),
        ReadU64(header + kEncryptedAreaSizeOffset),
        ReadU64(header + kEncryptedAreaOffset),
        ReadU32(header + kFlagsOffset),
        ReadU32(header + kSectorSizeOffset),
    };
    if (metadata.header_version < kMinHeaderVersion || metadata.header_version > kMaxHeaderVersion) {
        throw std::invalid_argument("Unsupported VeraCrypt header version");
    }
    // VeraCrypt/TrueCrypt-compatible header v3 predates the header-field CRC;
    // v4 and v5 require it. The master-key-area CRC is mandatory for all
    // supported XTS headers.
    if (metadata.header_version >= 4 &&
        Crc32(header + kSignatureOffset, kHeaderCrcOffset - kSignatureOffset) != ReadU32(header + kHeaderCrcOffset)) {
        throw std::invalid_argument("VeraCrypt header CRC mismatch");
    }
    if (Crc32(header + kKeyAreaOffset, kKeyAreaSize) != ReadU32(header + kKeyAreaCrcOffset)) {
        throw std::invalid_argument("VeraCrypt master-key CRC mismatch");
    }
    // Header v3/v4 predate the explicit sector-size field. VeraCrypt treats
    // both as the legacy 512-byte data-unit size.
    if (metadata.header_version < 5) metadata.sector_size = 512;
    // Filesystems can use a larger logical sector. The XTS data unit remains
    // 512 bytes and is handled separately by the volume session.
    if (metadata.sector_size != 512 && metadata.sector_size != 1024 &&
        metadata.sector_size != 2048 && metadata.sector_size != 4096) {
        throw std::invalid_argument("Unsupported VeraCrypt sector size");
    }
    RequireRange(metadata.encrypted_area_offset, metadata.encrypted_area_size);
    return metadata;
}

}  // namespace vc_core
