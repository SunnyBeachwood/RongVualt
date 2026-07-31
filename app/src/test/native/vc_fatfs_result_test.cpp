#include <cassert>

#include "vc_error.h"
#include "vc_fatfs_volume.h"

extern "C" void VcCoreFatFsResultSelfTest() {
    vc_core::FatFsVolume::ThrowFatFsFailure(FR_OK);

    bool rejected_disk_error = false;
    try {
        vc_core::FatFsVolume::ThrowFatFsFailure(FR_DISK_ERR);
    } catch (const vc_core::CoreException& error) {
        rejected_disk_error = error.error() == vc_core::CoreError::kIoInterrupted;
    }
    assert(rejected_disk_error);
}
