#include "vc_keyfiles.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace vc_core {
namespace {

constexpr std::size_t kLegacyPasswordBytes = 64;
constexpr std::size_t kMaximumPasswordBytes = 128;
constexpr std::uint64_t kKeyfileMaximumBytes = 1024 * 1024;
constexpr std::size_t kReadChunkBytes = 64 * 1024;

std::uint32_t UpdateCrc32(std::uint32_t crc, std::uint8_t byte) {
    crc ^= byte;
    for (int bit = 0; bit < 8; ++bit) crc = (crc >> 1) ^ ((crc & 1U) == 0 ? 0U : 0xedb88320U);
    return crc;
}

}  // namespace

SecureBytes ApplyVeraCryptKeyfiles(const SecureBytes& password, const std::vector<FdRandomAccess>& keyfiles) {
    if (keyfiles.empty()) {
        std::vector<std::uint8_t> copy(password.data(), password.data() + password.size());
        return SecureBytes(std::move(copy));
    }
    const std::size_t pool_size = password.size() <= kLegacyPasswordBytes ? kLegacyPasswordBytes : kMaximumPasswordBytes;
    std::vector<std::uint8_t> pool(pool_size, 0);
    std::copy(password.data(), password.data() + password.size(), pool.begin());

    try {
        for (const FdRandomAccess& keyfile : keyfiles) {
            const std::uint64_t available = std::min(keyfile.size(), kKeyfileMaximumBytes);
            if (available == 0) throw std::invalid_argument("Keyfile is empty");
            std::array<std::uint8_t, kReadChunkBytes> buffer {};
            std::uint64_t offset = 0;
            std::uint32_t crc = 0xffffffffU;
            std::size_t pool_position = 0;
            while (offset < available) {
                const std::size_t length = static_cast<std::size_t>(std::min<std::uint64_t>(buffer.size(), available - offset));
                keyfile.ReadAt(offset, buffer.data(), length);
                for (std::size_t index = 0; index < length; ++index) {
                    crc = UpdateCrc32(crc, buffer[index]);
                    // Crc32::Process() in VeraCrypt returns its internal,
                    // non-finalized CRC state. Do not apply the customary
                    // final XOR here.
                    const auto mix = [&](std::uint8_t value) {
                        pool[pool_position] = static_cast<std::uint8_t>(pool[pool_position] + value);
                        pool_position = (pool_position + 1) % pool.size();
                    };
                    mix(static_cast<std::uint8_t>(crc >> 24));
                    mix(static_cast<std::uint8_t>(crc >> 16));
                    mix(static_cast<std::uint8_t>(crc >> 8));
                    mix(static_cast<std::uint8_t>(crc));
                }
                std::fill(buffer.begin(), buffer.end(), 0);
                offset += length;
            }
        }
    } catch (...) {
        std::fill(pool.begin(), pool.end(), 0);
        throw;
    }
    return SecureBytes(std::move(pool));
}

}  // namespace vc_core
