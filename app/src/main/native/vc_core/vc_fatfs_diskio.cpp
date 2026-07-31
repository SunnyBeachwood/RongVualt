#include "vc_fatfs_diskio.h"

#include <array>
#include <mutex>
#include <stdexcept>

extern "C" {
#include "diskio.h"
}

namespace vc_core {
namespace {

constexpr std::size_t kMaxFatFsDrives = 4;
std::mutex g_drives_mutex;
std::array<EncryptedBlockDevice*, kMaxFatFsDrives> g_drives {};

}  // namespace

EncryptedBlockDevice* FindDrive(BYTE drive) {
    std::lock_guard<std::mutex> lock(g_drives_mutex);
    return drive < g_drives.size() ? g_drives[drive] : nullptr;
}

FatFsDiskIo::FatFsDiskIo(EncryptedBlockDevice& device) : drive_number_(0) {
    std::lock_guard<std::mutex> lock(g_drives_mutex);
    for (std::size_t index = 0; index < g_drives.size(); ++index) {
        if (g_drives[index] == nullptr) {
            g_drives[index] = &device;
            drive_number_ = static_cast<std::uint8_t>(index);
            return;
        }
    }
    throw std::runtime_error("No FatFs drive slots are available");
}

FatFsDiskIo::~FatFsDiskIo() {
    std::lock_guard<std::mutex> lock(g_drives_mutex);
    if (drive_number_ < g_drives.size()) g_drives[drive_number_] = nullptr;
}

}  // namespace vc_core

extern "C" DSTATUS disk_initialize(BYTE pdrv) {
    return vc_core::FindDrive(pdrv) == nullptr ? STA_NOINIT : 0;
}

extern "C" DSTATUS disk_status(BYTE pdrv) {
    vc_core::EncryptedBlockDevice* device = vc_core::FindDrive(pdrv);
    if (device == nullptr) return STA_NOINIT;
    return device->writable() ? 0 : STA_PROTECT;
}

extern "C" DRESULT disk_read(BYTE pdrv, BYTE* buffer, DWORD sector, UINT count) {
    vc_core::EncryptedBlockDevice* device = vc_core::FindDrive(pdrv);
    if (device == nullptr) return RES_NOTRDY;
    try {
        device->ReadSectors(sector, count, buffer);
        return RES_OK;
    } catch (...) {
        return RES_ERROR;
    }
}

extern "C" DRESULT disk_write(BYTE pdrv, const BYTE* buffer, DWORD sector, UINT count) {
    vc_core::EncryptedBlockDevice* device = vc_core::FindDrive(pdrv);
    if (device == nullptr) return RES_NOTRDY;
    if (!device->writable()) return RES_WRPRT;
    try {
        device->WriteSectors(sector, count, buffer);
        return RES_OK;
    } catch (...) {
        return RES_ERROR;
    }
}

extern "C" DRESULT disk_ioctl(BYTE pdrv, BYTE command, void* buffer) {
    vc_core::EncryptedBlockDevice* device = vc_core::FindDrive(pdrv);
    if (device == nullptr) return RES_NOTRDY;
    try {
        switch (command) {
            case CTRL_SYNC:
                device->Flush();
                return RES_OK;
            case GET_SECTOR_COUNT:
                if (buffer == nullptr) return RES_PARERR;
                if (device->sector_count() > static_cast<std::uint64_t>(std::numeric_limits<DWORD>::max())) {
                    return RES_PARERR;
                }
                *static_cast<DWORD*>(buffer) = static_cast<DWORD>(device->sector_count());
                return RES_OK;
            case GET_SECTOR_SIZE:
                if (buffer == nullptr) return RES_PARERR;
                *static_cast<WORD*>(buffer) = static_cast<WORD>(device->sector_size());
                return RES_OK;
            case GET_BLOCK_SIZE:
                if (buffer == nullptr) return RES_PARERR;
                *static_cast<DWORD*>(buffer) = 1;
                return RES_OK;
            default:
                return RES_PARERR;
        }
    } catch (...) {
        return RES_ERROR;
    }
}
