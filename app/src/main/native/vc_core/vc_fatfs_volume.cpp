#include "vc_fatfs_volume.h"

#include <array>
#include <algorithm>
#include <limits>
#include <stdexcept>

#include "vc_error.h"

namespace vc_core {
namespace {

constexpr std::uint32_t kFat32EndOfChain = 0x0ffffff8U;
constexpr std::uint32_t kExFatEndOfChain = 0xfffffff8U;
constexpr std::size_t kExFatDirectoryEntryBytes = 32;

std::uint32_t ReadLittleEndianU32(const std::uint8_t* bytes) {
    return static_cast<std::uint32_t>(bytes[0]) |
           (static_cast<std::uint32_t>(bytes[1]) << 8) |
           (static_cast<std::uint32_t>(bytes[2]) << 16) |
           (static_cast<std::uint32_t>(bytes[3]) << 24);
}

std::uint64_t ReadLittleEndianU64(const std::uint8_t* bytes) {
    return static_cast<std::uint64_t>(ReadLittleEndianU32(bytes)) |
           (static_cast<std::uint64_t>(ReadLittleEndianU32(bytes + 4)) << 32);
}

FatFsEntry EntryFromInfo(const FILINFO& info) {
    return {info.fname, (info.fattrib & AM_DIR) != 0, static_cast<std::uint64_t>(info.fsize), info.fdate, info.ftime};
}

}  // namespace

FatFsFile::FatFsFile(FatFsVolume& volume, FIL file) : volume_(volume), file_(file) {}

FatFsFile::~FatFsFile() {
    if (!closed_) {
        std::lock_guard<std::mutex> lock(volume_.api_mutex_);
        (void) f_close(&file_);
    }
}

std::size_t FatFsFile::Read(std::uint64_t offset, std::uint8_t* destination, std::size_t length) {
    if (destination == nullptr && length != 0) throw std::invalid_argument("FatFs read destination is null");
    std::lock_guard<std::mutex> lock(volume_.api_mutex_);
    if (offset > std::numeric_limits<FSIZE_t>::max()) throw std::out_of_range("FatFs read offset is too large");
    volume_.ThrowFatFsFailureWithDevice(f_lseek(&file_, static_cast<FSIZE_t>(offset)));
    std::size_t completed = 0;
    while (completed < length) {
        const UINT requested = static_cast<UINT>(std::min<std::size_t>(length - completed, std::numeric_limits<UINT>::max()));
        UINT received = 0;
        volume_.ThrowFatFsFailureWithDevice(f_read(&file_, destination + completed, requested, &received));
        completed += received;
        if (received < requested) break;
    }
    return completed;
}

std::size_t FatFsFile::Write(std::uint64_t offset, const std::uint8_t* source, std::size_t length) {
    if (source == nullptr && length != 0) throw std::invalid_argument("FatFs write source is null");
    std::lock_guard<std::mutex> lock(volume_.api_mutex_);
    if (!volume_.writable()) throw CoreException(CoreError::kReadOnlySource);
    if (offset > std::numeric_limits<FSIZE_t>::max()) throw std::out_of_range("FatFs write offset is too large");
    volume_.ThrowFatFsFailureWithDevice(f_lseek(&file_, static_cast<FSIZE_t>(offset)));
    std::size_t completed = 0;
    while (completed < length) {
        const UINT requested = static_cast<UINT>(std::min<std::size_t>(length - completed, std::numeric_limits<UINT>::max()));
        UINT written = 0;
        volume_.ThrowFatFsFailureWithDevice(f_write(&file_, source + completed, requested, &written));
        completed += written;
        if (written != requested) throw CoreException(CoreError::kIoInterrupted);
    }
    return completed;
}

void FatFsFile::Truncate(std::uint64_t length) {
    std::lock_guard<std::mutex> lock(volume_.api_mutex_);
    if (!volume_.writable()) throw CoreException(CoreError::kReadOnlySource);
    if (length > std::numeric_limits<FSIZE_t>::max()) throw std::out_of_range("FatFs truncate length is too large");
    volume_.ThrowFatFsFailureWithDevice(f_lseek(&file_, static_cast<FSIZE_t>(length)));
    volume_.ThrowFatFsFailureWithDevice(f_truncate(&file_));
}

void FatFsFile::Preallocate(std::uint64_t length) {
    std::lock_guard<std::mutex> lock(volume_.api_mutex_);
    if (!volume_.writable()) throw CoreException(CoreError::kReadOnlySource);
    if (length > std::numeric_limits<FSIZE_t>::max()) throw std::out_of_range("FatFs preallocation length is too large");
    volume_.ThrowFatFsFailureWithDevice(f_expand(&file_, static_cast<FSIZE_t>(length), 1));
}

void FatFsFile::Flush() {
    std::lock_guard<std::mutex> lock(volume_.api_mutex_);
    volume_.ThrowFatFsFailureWithDevice(f_sync(&file_));
}

FatFsVolume::FatFsVolume(NativeVolumeSession& session)
    : device_(session), disk_io_(device_) {
    const std::uint8_t drive = disk_io_.drive_number();
    drive_path_ = {static_cast<char>('0' + drive), ':', '\0'};
}

FatFsVolume::~FatFsVolume() {
    Unmount();
}

FatFsType FatFsVolume::Mount() {
    if (mounted_) return type();
    const FRESULT result = f_mount(&filesystem_, drive_path(), 1);
    if (result != FR_OK) ThrowFatFsFailureWithDevice(result);
    mounted_ = true;
    return type();
}

FatFsType FatFsVolume::FormatAndMount(FatFsType requested_type) {
    if (requested_type != FatFsType::kFat && requested_type != FatFsType::kExFat) {
        throw CoreException(CoreError::kUnsupportedFileSystem);
    }
    if (!writable()) throw CoreException(CoreError::kReadOnlySource);
    if (mounted_) Unmount();
    std::array<BYTE, 4096> work {};
    // VeraCrypt data areas contain a filesystem directly, not an MBR/GPT.
    // FM_FAT lets FatFs select FAT12/16/32 from the requested data size.
    const BYTE format_type = requested_type == FatFsType::kExFat ? FM_EXFAT : FM_FAT;
    const FRESULT format = f_mkfs(drive_path(), static_cast<BYTE>(format_type | FM_SFD), 0, work.data(), work.size());
    if (format != FR_OK) ThrowFatFsFailureWithDevice(format);
    device_.Flush();
    return Mount();
}

FatFsType FatFsVolume::type() const {
    if (!mounted_) throw CoreException(CoreError::kUnsupportedFileSystem);
    return filesystem_.fs_type == FS_EXFAT ? FatFsType::kExFat : FatFsType::kFat;
}

std::string FatFsVolume::ToFatFsPath(const std::string& relative_path) const {
    if (!mounted_) throw CoreException(CoreError::kUnsupportedFileSystem);
    if (relative_path.find('\0') != std::string::npos || relative_path.find('\\') != std::string::npos) {
        throw std::invalid_argument("Invalid internal filesystem path");
    }
    std::string result(drive_path());
    result.push_back('/');
    std::size_t start = 0;
    while (start < relative_path.size() && relative_path[start] == '/') ++start;
    while (start < relative_path.size()) {
        const std::size_t end = relative_path.find('/', start);
        const std::size_t length = (end == std::string::npos ? relative_path.size() : end) - start;
        const std::string part = relative_path.substr(start, length);
        if (part.empty() || part == "." || part == "..") throw std::invalid_argument("Invalid internal filesystem path");
        if (result.back() != '/') result.push_back('/');
        result.append(part);
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return result;
}

std::vector<FatFsEntry> FatFsVolume::ListDirectory(const std::string& relative_path) {
    std::lock_guard<std::mutex> lock(api_mutex_);
    DIR directory {};
    ThrowFatFsFailureWithDevice(f_opendir(&directory, ToFatFsPath(relative_path).c_str()));
    std::vector<FatFsEntry> result;
    try {
        FILINFO info {};
        while (true) {
            ThrowFatFsFailureWithDevice(f_readdir(&directory, &info));
            if (info.fname[0] == '\0') break;
            result.push_back(EntryFromInfo(info));
        }
    } catch (...) {
        (void) f_closedir(&directory);
        throw;
    }
    (void) f_closedir(&directory);
    return result;
}

FatFsEntry FatFsVolume::Stat(const std::string& relative_path) {
    std::lock_guard<std::mutex> lock(api_mutex_);
    FILINFO info {};
    ThrowFatFsFailureWithDevice(f_stat(ToFatFsPath(relative_path).c_str(), &info));
    return EntryFromInfo(info);
}

std::unique_ptr<FatFsFile> FatFsVolume::OpenFile(
        const std::string& relative_path, bool writable, bool create, bool truncate) {
    std::lock_guard<std::mutex> lock(api_mutex_);
    if (writable && !this->writable()) throw CoreException(CoreError::kReadOnlySource);
    BYTE mode = writable ? static_cast<BYTE>(FA_READ | FA_WRITE) : FA_READ;
    if (truncate) mode = static_cast<BYTE>(mode | FA_CREATE_ALWAYS);
    else if (create) mode = static_cast<BYTE>(mode | FA_OPEN_ALWAYS);
    else mode = static_cast<BYTE>(mode | FA_OPEN_EXISTING);
    FIL file {};
    ThrowFatFsFailureWithDevice(f_open(&file, ToFatFsPath(relative_path).c_str(), mode));
    return std::make_unique<FatFsFile>(*this, file);
}

void FatFsVolume::CreateDirectory(const std::string& relative_path) {
    std::lock_guard<std::mutex> lock(api_mutex_);
    if (!writable()) throw CoreException(CoreError::kReadOnlySource);
    ThrowFatFsFailureWithDevice(f_mkdir(ToFatFsPath(relative_path).c_str()));
}

void FatFsVolume::Delete(const std::string& relative_path) {
    std::lock_guard<std::mutex> lock(api_mutex_);
    if (!writable()) throw CoreException(CoreError::kReadOnlySource);
    ThrowFatFsFailureWithDevice(f_unlink(ToFatFsPath(relative_path).c_str()));
}

void FatFsVolume::Rename(const std::string& from_relative_path, const std::string& to_relative_path) {
    std::lock_guard<std::mutex> lock(api_mutex_);
    if (!writable()) throw CoreException(CoreError::kReadOnlySource);
    ThrowFatFsFailureWithDevice(f_rename(ToFatFsPath(from_relative_path).c_str(), ToFatFsPath(to_relative_path).c_str()));
}

FatFsTailFreeRange FatFsVolume::FindTailFreeRange() {
    std::lock_guard<std::mutex> lock(api_mutex_);
    if (!mounted_) throw CoreException(CoreError::kUnsupportedFileSystem);
    if (filesystem_.n_fatent < 3 || filesystem_.csize == 0) {
        throw CoreException(CoreError::kUnsupportedFileSystem);
    }

    const std::uint64_t cluster_bytes =
            static_cast<std::uint64_t>(filesystem_.csize) * device_.sector_size();
    // FatFs cluster numbers are relative to the data heap.  The filesystem
    // logical address exposed by EncryptedBlockDevice is relative to the
    // volume data area, so include the reserved/FAT/root-directory prefix.
    const std::uint64_t data_base_bytes =
            static_cast<std::uint64_t>(filesystem_.database) * device_.sector_size();
    const std::uint32_t last_cluster = filesystem_.n_fatent - 1;
    if (cluster_bytes == 0 || last_cluster < 2) throw CoreException(CoreError::kUnsupportedFileSystem);

    std::array<std::uint8_t, FF_MAX_SS> sector {};
    std::array<std::uint8_t, FF_MAX_SS> fat_sector {};
    std::uint64_t cached_sector = std::numeric_limits<std::uint64_t>::max();
    const auto read_byte = [&](std::uint64_t sector_number, std::uint32_t byte_offset) -> std::uint8_t {
        if (byte_offset >= device_.sector_size()) throw CoreException(CoreError::kCorruptHeader);
        if (cached_sector != sector_number) {
            device_.ReadSectors(sector_number, 1, fat_sector.data());
            cached_sector = sector_number;
        }
        return fat_sector[byte_offset];
    };
    const auto read_fat_entry = [&](std::uint32_t cluster) -> std::uint32_t {
        std::uint64_t offset = 0;
        switch (filesystem_.fs_type) {
            case FS_FAT12: offset = cluster + cluster / 2; break;
            case FS_FAT16: offset = static_cast<std::uint64_t>(cluster) * 2; break;
            case FS_FAT32: offset = static_cast<std::uint64_t>(cluster) * 4; break;
            default: throw CoreException(CoreError::kUnsupportedFileSystem);
        }
        const std::uint64_t first = filesystem_.fatbase + offset / device_.sector_size();
        const std::uint32_t inside = static_cast<std::uint32_t>(offset % device_.sector_size());
        const std::uint16_t value = static_cast<std::uint16_t>(read_byte(first, inside)) |
                (static_cast<std::uint16_t>(read_byte(first + (inside + 1) / device_.sector_size(),
                                                       (inside + 1) % device_.sector_size())) << 8);
        if (filesystem_.fs_type == FS_FAT12) {
            return (cluster & 1U) == 0 ? value & 0x0fffU : value >> 4;
        }
        if (filesystem_.fs_type == FS_FAT16) return value;
        const std::uint16_t high = static_cast<std::uint16_t>(read_byte(
                first + (inside + 2) / device_.sector_size(), (inside + 2) % device_.sector_size())) |
                (static_cast<std::uint16_t>(read_byte(
                        first + (inside + 3) / device_.sector_size(), (inside + 3) % device_.sector_size())) << 8);
        return (static_cast<std::uint32_t>(value) | (static_cast<std::uint32_t>(high) << 16)) & 0x0fffffffU;
    };

    std::uint32_t first_free_cluster = last_cluster + 1;
    if (filesystem_.fs_type != FS_EXFAT) {
        while (first_free_cluster > 2 && read_fat_entry(first_free_cluster - 1) == 0) --first_free_cluster;
    } else {
        // FatFs validates the allocation-bitmap entry at mount time, but its
        // public FATFS state does not retain the stream location. Re-read the
        // root directory and follow its FAT chain instead of assuming that the
        // bitmap starts at data cluster 2.
        const auto read_exfat_fat = [&](std::uint32_t cluster) -> std::uint32_t {
            const std::uint64_t offset = static_cast<std::uint64_t>(cluster) * 4;
            const std::uint64_t first = filesystem_.fatbase + offset / device_.sector_size();
            const std::uint32_t inside = static_cast<std::uint32_t>(offset % device_.sector_size());
            std::uint32_t value = 0;
            for (std::uint32_t index = 0; index < 4; ++index) {
                value |= static_cast<std::uint32_t>(read_byte(
                        first + (inside + index) / device_.sector_size(),
                        (inside + index) % device_.sector_size())) << (index * 8);
            }
            return value;
        };
        const auto cluster_sector = [&](std::uint32_t cluster) -> std::uint64_t {
            if (cluster < 2 || cluster > last_cluster) throw CoreException(CoreError::kCorruptHeader);
            return filesystem_.database + static_cast<std::uint64_t>(cluster - 2) * filesystem_.csize;
        };
        std::uint32_t bitmap_cluster = 0;
        std::uint64_t bitmap_size = 0;
        std::uint32_t directory_cluster = filesystem_.dirbase;
        bool directory_ended = false;
        for (std::uint32_t walked = 0; directory_cluster >= 2 && directory_cluster <= last_cluster && walked <= last_cluster; ++walked) {
            for (std::uint32_t sector_index = 0; sector_index < filesystem_.csize; ++sector_index) {
                device_.ReadSectors(cluster_sector(directory_cluster) + sector_index, 1, sector.data());
                for (std::uint32_t offset = 0; offset < device_.sector_size(); offset += kExFatDirectoryEntryBytes) {
                    const std::uint8_t type = sector[offset];
                    if (type == 0x00) {
                        directory_ended = true;
                        break;
                    }
                    if (type == 0x81 && (sector[offset + 1] & 1U) == 0) {
                        bitmap_cluster = ReadLittleEndianU32(sector.data() + offset + 20);
                        bitmap_size = ReadLittleEndianU64(sector.data() + offset + 24);
                        break;
                    }
                }
                if (bitmap_cluster != 0 || directory_ended) break;
            }
            if (bitmap_cluster != 0 || directory_ended) break;
            const std::uint32_t next = read_exfat_fat(directory_cluster);
            if (next >= kExFatEndOfChain) break;
            directory_cluster = next;
        }
        const std::uint64_t required_bitmap_bytes = (static_cast<std::uint64_t>(last_cluster - 1) + 7) / 8;
        if (bitmap_cluster < 2 || bitmap_cluster > last_cluster || bitmap_size < required_bitmap_bytes) {
            throw CoreException(CoreError::kCorruptHeader);
        }
        std::vector<std::uint32_t> bitmap_clusters;
        bitmap_clusters.reserve(static_cast<std::size_t>((bitmap_size + cluster_bytes - 1) / cluster_bytes));
        for (std::uint32_t cluster = bitmap_cluster; bitmap_clusters.size() * cluster_bytes < bitmap_size;) {
            if (cluster < 2 || cluster > last_cluster || bitmap_clusters.size() > last_cluster) {
                throw CoreException(CoreError::kCorruptHeader);
            }
            bitmap_clusters.push_back(cluster);
            const std::uint32_t next = read_exfat_fat(cluster);
            if (bitmap_clusters.size() * cluster_bytes >= bitmap_size) break;
            if (next >= kExFatEndOfChain) throw CoreException(CoreError::kCorruptHeader);
            cluster = next;
        }
        std::uint64_t bitmap_cached_sector = std::numeric_limits<std::uint64_t>::max();
        const auto bitmap_byte = [&](std::uint64_t offset) -> std::uint8_t {
            const std::uint64_t cluster_index = offset / cluster_bytes;
            if (cluster_index >= bitmap_clusters.size()) throw CoreException(CoreError::kCorruptHeader);
            const std::uint64_t in_cluster = offset % cluster_bytes;
            const std::uint64_t physical_sector = cluster_sector(bitmap_clusters[cluster_index]) + in_cluster / device_.sector_size();
            if (bitmap_cached_sector != physical_sector) {
                device_.ReadSectors(physical_sector, 1, sector.data());
                bitmap_cached_sector = physical_sector;
            }
            return sector[in_cluster % device_.sector_size()];
        };
        while (first_free_cluster > 2) {
            const std::uint32_t cluster = first_free_cluster - 1;
            const std::uint64_t bit = cluster - 2;
            if ((bitmap_byte(bit / 8) & (1U << (bit % 8))) != 0) break;
            --first_free_cluster;
        }
    }

    if (first_free_cluster > last_cluster) {
        return {
                data_base_bytes + static_cast<std::uint64_t>(last_cluster - 1) * cluster_bytes,
                0,
        };
    }
    const std::uint64_t cluster_count = static_cast<std::uint64_t>(last_cluster) - first_free_cluster + 1;
    return {
            data_base_bytes + static_cast<std::uint64_t>(first_free_cluster - 2) * cluster_bytes,
            cluster_count * cluster_bytes,
    };
}

void FatFsVolume::Flush() {
    if (!mounted_) return;
    // Individual FIL handles call f_sync before closure. At volume scope,
    // FatFs has no global sync API, so commit the encrypted block device.
    device_.Flush();
}

void FatFsVolume::Unmount() noexcept {
    if (!mounted_) return;
    // The disk bridge owns no dirty cache beyond FatFs. Ignore an unmount
    // failure in a destructor; the session FD closure still locks the volume.
    (void) f_mount(nullptr, drive_path(), 0);
    mounted_ = false;
}

void FatFsVolume::ThrowFatFsFailure(FRESULT result) {
    if (result == FR_OK) return;
    const std::string detail = "FatFs operation failed (FRESULT=" + std::to_string(static_cast<int>(result)) + ")";
    switch (result) {
        case FR_NO_FILESYSTEM:
        case FR_INVALID_DRIVE:
            throw CoreException(CoreError::kUnsupportedFileSystem, detail);
        case FR_WRITE_PROTECTED:
            throw CoreException(CoreError::kReadOnlySource, detail);
        case FR_NOT_ENOUGH_CORE:
            throw CoreException(CoreError::kInsufficientMemory, detail);
        case FR_NO_FILE:
        case FR_NO_PATH:
            throw CoreException(CoreError::kNotFound, detail);
        default:
            throw CoreException(CoreError::kIoInterrupted, detail);
    }
}

void FatFsVolume::ThrowFatFsFailureWithDevice(FRESULT result) {
    if (result == FR_OK) return;
    const std::string detail = "FatFs operation failed (FRESULT=" + std::to_string(static_cast<int>(result)) + ")";
    // FatFs collapses exceptions from disk_write into FR_DISK_ERR. Preserve
    // the hidden-volume protection signal so the managed session can latch
    // read-only state and refresh its DocumentsProvider roots.
    if (result == FR_DISK_ERR && device_.hidden_volume_protection_triggered()) {
        throw CoreException(CoreError::kHiddenVolumeRisk, detail);
    }
    ThrowFatFsFailure(result);
}

}  // namespace vc_core
