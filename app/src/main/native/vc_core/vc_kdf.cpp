#include "vc_kdf.h"

#include <algorithm>
#include <cstdint>
#include <new>
#include <stdexcept>
#include <string_view>
#include <unistd.h>
#include <vector>

#include "vc_botan.h"
#include "vc_error.h"

namespace vc_core {
namespace {

const char* MacName(KdfHint kdf) {
    switch (kdf) {
        case KdfHint::kPbkdf2HmacSha512: return "HMAC(SHA-512)";
        case KdfHint::kPbkdf2HmacSha256: return "HMAC(SHA-256)";
        case KdfHint::kPbkdf2HmacBlake2s: return "HMAC(BLAKE2s(256))";
        case KdfHint::kPbkdf2HmacWhirlpool: return "HMAC(Whirlpool)";
        case KdfHint::kPbkdf2HmacStreebog: return "HMAC(Streebog-512)";
        default: throw std::invalid_argument("KDF is not PBKDF2");
    }
}

std::string_view PasswordView(const SecureBytes& password) {
    return std::string_view(reinterpret_cast<const char*>(password.data()), password.size());
}

bool HasEnoughAvailableMemory(std::uint32_t required_kib) {
    const long available_pages = sysconf(_SC_AVPHYS_PAGES);
    const long page_bytes = sysconf(_SC_PAGESIZE);
    // Some Android kernels/providers do not expose this value. In that case
    // Botan remains the authority and allocation failure maps to the same
    // stable error rather than changing Argon2 parameters.
    if (available_pages <= 0 || page_bytes <= 0) return true;
    const std::uint64_t available_bytes = static_cast<std::uint64_t>(available_pages) *
                                          static_cast<std::uint64_t>(page_bytes);
    const std::uint64_t required_bytes = static_cast<std::uint64_t>(required_kib) * 1024U;
    return available_bytes >= required_bytes;
}

}  // namespace

std::uint32_t VeraCryptPbkdf2Iterations(KdfHint kdf, std::int32_t pim) {
    if (pim < 0) throw std::invalid_argument("PIM cannot be negative");
    switch (kdf) {
        case KdfHint::kPbkdf2HmacSha512:
        case KdfHint::kPbkdf2HmacSha256:
        case KdfHint::kPbkdf2HmacBlake2s:
        case KdfHint::kPbkdf2HmacWhirlpool:
        case KdfHint::kPbkdf2HmacStreebog:
            return pim == 0 ? 500000U : static_cast<std::uint32_t>(15000U + static_cast<std::uint64_t>(pim) * 1000U);
        default:
            throw std::invalid_argument("KDF is not PBKDF2");
    }
}

Argon2Parameters VeraCryptArgon2Parameters(std::int32_t pim) {
    if (pim < 0) throw std::invalid_argument("PIM cannot be negative");
    const std::int32_t effective_pim = pim == 0 ? 12 : pim;
    const std::int64_t requested_mib = 64LL + (static_cast<std::int64_t>(effective_pim) - 1LL) * 32LL;
    const std::uint32_t memory_mib = static_cast<std::uint32_t>(std::min<std::int64_t>(requested_mib, 1024));
    const std::uint32_t iterations = effective_pim <= 31
            ? static_cast<std::uint32_t>(3 + (effective_pim - 1) / 3)
            : static_cast<std::uint32_t>(13 + (effective_pim - 31));
    return {iterations, memory_mib * 1024U};
}

SecureBytes DeriveVeraCryptHeaderKey(
        const SecureBytes& password,
        const std::uint8_t* salt,
        std::size_t salt_size,
        KdfHint kdf,
        std::int32_t pim) {
    if (salt == nullptr || salt_size != 64) throw std::invalid_argument("VeraCrypt header salt must be 64 bytes");
    std::vector<std::uint8_t> output(kVeraCryptHeaderKeyBytes);
    try {
        if (kdf == KdfHint::kArgon2id) {
            const Argon2Parameters parameters = VeraCryptArgon2Parameters(pim);
            if (!HasEnoughAvailableMemory(parameters.memory_kib)) {
                throw CoreException(CoreError::kInsufficientMemory);
            }
            Botan::argon2(output.data(), output.size(), PasswordView(password).data(), password.size(), salt, salt_size,
                          nullptr, 0, nullptr, 0, 2, 1, parameters.memory_kib, parameters.iterations);
        } else {
            const auto mac = Botan::MessageAuthenticationCode::create_or_throw(MacName(kdf));
            mac->set_key(password.data(), password.size());
            Botan::pbkdf2(*mac, output.data(), output.size(), salt, salt_size, VeraCryptPbkdf2Iterations(kdf, pim));
        }
    } catch (const std::bad_alloc&) {
        std::fill(output.begin(), output.end(), 0);
        throw CoreException(CoreError::kInsufficientMemory);
    } catch (...) {
        std::fill(output.begin(), output.end(), 0);
        throw;
    }
    return SecureBytes(std::move(output));
}

}  // namespace vc_core
