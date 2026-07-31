#pragma once

#include <cstddef>
#include <cstdint>

namespace vc_core {

/** Reads cryptographically secure random bytes from Android's kernel RNG. */
void FillSecureRandom(std::uint8_t* destination, std::size_t length);

}  // namespace vc_core
