#include <cassert>
#include <cstdint>
#include <stdexcept>

#include "vc_volume.h"

// Physical offsets are intentional: a hidden volume and a normal volume must
// never restart the XTS tweak sequence at their logical offset zero.
extern "C" void VcCoreXtsDataUnitSelfTest() {
    assert(vc_core::VeraCryptDataUnitNumberForPhysicalOffset(0) == 0);
    assert(vc_core::VeraCryptDataUnitNumberForPhysicalOffset(512) == 1);
    assert(vc_core::VeraCryptDataUnitNumberForPhysicalOffset(131072) == 256);
    assert(vc_core::VeraCryptDataUnitNumberForPhysicalOffset(196608) == 384);

    bool rejected_unaligned = false;
    try {
        (void) vc_core::VeraCryptDataUnitNumberForPhysicalOffset(1);
    } catch (const std::invalid_argument&) {
        rejected_unaligned = true;
    }
    assert(rejected_unaligned);
}
