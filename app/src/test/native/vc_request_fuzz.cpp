#include <cstddef>
#include <cstdint>

#include "vc_request.h"

/**
 * libFuzzer entry point used by native CI with VC_CORE_ENABLE_SANITIZERS.
 * Parser failures are expected for arbitrary inputs and must never escape as
 * memory safety failures.
 */
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    try {
        (void) vc_core::ParseOpenRequest(data, size);
    } catch (...) {
        // Invalid request bytes are intentionally rejected.
    }
    try {
        (void) vc_core::ParseCreateRequest(data, size);
    } catch (...) {
        // Invalid request bytes are intentionally rejected.
    }
    return 0;
}
