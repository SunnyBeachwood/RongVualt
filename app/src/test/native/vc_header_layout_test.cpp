#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "vc_header.h"

namespace {

void Put16(std::uint8_t* destination, std::uint16_t value) {
    destination[0] = static_cast<std::uint8_t>(value >> 8);
    destination[1] = static_cast<std::uint8_t>(value);
}

void Put32(std::uint8_t* destination, std::uint32_t value) {
    for (int index = 3; index >= 0; --index) destination[3 - index] = static_cast<std::uint8_t>(value >> (index * 8));
}

void Put64(std::uint8_t* destination, std::uint64_t value) {
    for (int index = 7; index >= 0; --index) destination[7 - index] = static_cast<std::uint8_t>(value >> (index * 8));
}

std::uint32_t Crc32(const std::uint8_t* data, std::size_t size) {
    std::uint32_t crc = 0xffffffffU;
    for (std::size_t index = 0; index < size; ++index) {
        crc ^= data[index];
        for (int bit = 0; bit < 8; ++bit) crc = (crc >> 1) ^ ((crc & 1U) == 0 ? 0U : 0xedb88320U);
    }
    return ~crc;
}

}  // namespace

extern "C" void VcCoreHeaderLayoutSelfTest() {
    std::array<std::uint8_t, vc_core::kVeraCryptHeaderSize> header {};
    header[64] = 'V'; header[65] = 'E'; header[66] = 'R'; header[67] = 'A';
    Put16(header.data() + 68, 5);
    Put16(header.data() + 70, 0x630);
    Put64(header.data() + 92, 0);
    Put64(header.data() + 100, 4096);     // Volume data size, not encrypted-area size.
    Put64(header.data() + 108, 131072);
    Put64(header.data() + 116, 8192);     // Encrypted-area size.
    Put32(header.data() + 124, 0);
    Put32(header.data() + 128, 512);
    Put32(header.data() + 72, Crc32(header.data() + 256, 256));
    Put32(header.data() + 252, Crc32(header.data() + 64, 188));

    const auto metadata = vc_core::ParseDecryptedVeraCryptHeader(header.data(), header.size());
    assert(metadata.volume_data_size == 4096);
    assert(metadata.encrypted_area_offset == 131072);
    assert(metadata.encrypted_area_size == 8192);

    // v4 reserves the sector-size location; compatibility is the legacy 512
    // byte data-unit size rather than the literal zero in the header.
    Put16(header.data() + 68, 4);
    Put32(header.data() + 128, 0);
    Put32(header.data() + 252, Crc32(header.data() + 64, 188));
    const auto legacy = vc_core::ParseDecryptedVeraCryptHeader(header.data(), header.size());
    assert(legacy.sector_size == 512);

    // v5 accepts VeraCrypt's larger logical sector sizes; they do not alter
    // the 512-byte XTS data-unit rule.
    Put16(header.data() + 68, 5);
    for (const std::uint32_t sector_size : {512U, 1024U, 2048U, 4096U}) {
        Put32(header.data() + 128, sector_size);
        Put32(header.data() + 252, Crc32(header.data() + 64, 188));
        assert(vc_core::ParseDecryptedVeraCryptHeader(header.data(), header.size()).sector_size == sector_size);
    }

    // A v5 header must reject both integrity checks independently. This keeps
    // a plausible signature from being treated as a successful unlock.
    Put16(header.data() + 68, 5);
    Put32(header.data() + 128, 512);
    Put32(header.data() + 72, Crc32(header.data() + 256, 256));
    Put32(header.data() + 252, Crc32(header.data() + 64, 188));
    header[256] ^= 0x01U;
    bool rejected_bad_key_crc = false;
    try {
        (void) vc_core::ParseDecryptedVeraCryptHeader(header.data(), header.size());
    } catch (const std::invalid_argument&) {
        rejected_bad_key_crc = true;
    }
    assert(rejected_bad_key_crc);
    header[256] ^= 0x01U;
    Put32(header.data() + 72, Crc32(header.data() + 256, 256));
    header[124] ^= 0x01U;
    bool rejected_bad_header_crc = false;
    try {
        (void) vc_core::ParseDecryptedVeraCryptHeader(header.data(), header.size());
    } catch (const std::invalid_argument&) {
        rejected_bad_header_crc = true;
    }
    assert(rejected_bad_header_crc);
}
