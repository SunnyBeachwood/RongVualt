#pragma once

#include <cstddef>
#include <cstdint>

namespace vc_core {

constexpr std::size_t kVeraCryptHeaderSize = 512;
constexpr std::size_t kVeraCryptHeaderSaltSize = 64;

struct HeaderMetadata final {
    std::uint16_t header_version;
    std::uint16_t required_program_version;
    std::uint64_t hidden_volume_size;
    std::uint64_t volume_data_size;
    std::uint64_t encrypted_area_size;
    std::uint64_t encrypted_area_offset;
    std::uint32_t flags;
    std::uint32_t sector_size;
};

/**
 * Validates a decrypted 512-byte VeraCrypt header. The input includes the
 * unencrypted 64-byte salt followed by the decrypted header area. No key area
 * is returned or retained after CRC verification.
 */
HeaderMetadata ParseDecryptedVeraCryptHeader(const std::uint8_t* header, std::size_t size);

}  // namespace vc_core
