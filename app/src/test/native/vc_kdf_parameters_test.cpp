#include <cassert>

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
}
