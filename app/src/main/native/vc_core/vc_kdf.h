#pragma once

#include <cstddef>
#include <cstdint>

#include "vc_request.h"

namespace vc_core {

constexpr std::size_t kVeraCryptHeaderKeyBytes = 192;

struct Argon2Parameters final {
    std::uint32_t iterations;
    std::uint32_t memory_kib;
};

/** VeraCrypt 1.26.29 non-system PBKDF2 iteration rule. */
std::uint32_t VeraCryptPbkdf2Iterations(KdfHint kdf, std::int32_t pim);

/** VeraCrypt 1.26.29 Argon2id rule (version 0x13, parallelism one). */
Argon2Parameters VeraCryptArgon2Parameters(std::int32_t pim);

/**
 * Derives header key material from a processed password and 64-byte header
 * salt. Argon2id always returns 192 bytes, even when a selected cipher uses a
 * shorter prefix.
 */
SecureBytes DeriveVeraCryptHeaderKey(
        const SecureBytes& password,
        const std::uint8_t* salt,
        std::size_t salt_size,
        KdfHint kdf,
        std::int32_t pim);

}  // namespace vc_core
