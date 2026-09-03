#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "vc_botan.h"
#include "vc_kdf.h"

// Intentionally framework-free: the native test runner can call this after
// linking vc_core. It documents the source-pinned 1.26.29 KDF invariants.
extern "C" void VcCoreKdfParameterSelfTest() {
    using vc_core::KdfHint;

    assert(vc_core::VeraCryptPbkdf2Iterations(KdfHint::kPbkdf2HmacSha512, 0) == 500000U);
    assert(vc_core::VeraCryptPbkdf2Iterations(KdfHint::kPbkdf2HmacSha512, 1) == 16000U);
    assert(vc_core::VeraCryptPbkdf2Iterations(KdfHint::kPbkdf2HmacStreebog, 42) == 57000U);

    const auto default_argon2 = vc_core::VeraCryptArgon2Parameters(0);
    assert(default_argon2.iterations == 6U);
    assert(default_argon2.memory_kib == 425984U);
    const auto capped_argon2 = vc_core::VeraCryptArgon2Parameters(32);
    assert(capped_argon2.iterations == 14U);
    assert(capped_argon2.memory_kib == 1048576U);

    assert(vc_core::VeraCryptPbkdf2Iterations(KdfHint::kPbkdf2HmacSha512, vc_core::kVeraCryptMaximumPim) == 2147483000U);
    const auto maximum_argon2 = vc_core::VeraCryptArgon2Parameters(vc_core::kVeraCryptMaximumPim);
    assert(maximum_argon2.iterations == 2147450U);
    assert(maximum_argon2.memory_kib == 1048576U);

    bool rejected_negative = false;
    try {
        (void) vc_core::VeraCryptPbkdf2Iterations(KdfHint::kPbkdf2HmacSha512, -1);
    } catch (const std::invalid_argument&) {
        rejected_negative = true;
    }
    assert(rejected_negative);

    bool rejected_too_large = false;
    try {
        (void) vc_core::VeraCryptArgon2Parameters(vc_core::kVeraCryptMaximumPim + 1);
    } catch (const std::invalid_argument&) {
        rejected_too_large = true;
    }
    assert(rejected_too_large);

    // RFC 7693 Appendix B guards the arm64 amalgamation's BLAKE2s module and
    // the HMAC factory used by PBKDF2-HMAC-BLAKE2s.
    auto blake2s = Botan::HashFunction::create_or_throw("BLAKE2s(256)");
    const auto digest = blake2s->process<std::vector<std::uint8_t>>("abc");
    assert(Botan::hex_encode(digest.data(), digest.size(), false) ==
           "508c5e8c327c14e2e1a72ba34eeb452f37458b209ed63a294d999b4c86675982");
    auto blake2s_hmac = Botan::MessageAuthenticationCode::create_or_throw("HMAC(BLAKE2s(256))");
    assert(blake2s_hmac->output_length() == 32U);
}
