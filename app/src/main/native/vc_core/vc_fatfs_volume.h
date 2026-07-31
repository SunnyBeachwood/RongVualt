#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "vc_block_device.h"
#include "vc_fatfs_diskio.h"
#include "vc_filesystem_types.h"
#include "vc_error.h"

extern "C" {
#include "ff.h"
}

namespace vc_core {

class FatFsVolume;

/**
 * A mounted filesystem-owned file handle.  Registry/JNI code deliberately
 * knows nothing about FatFs or libfsntfs, so read-only filesystems cannot
 * accidentally inherit write support from the writable FAT implementation.
 */
class FileSystemFile {
public:
    virtual ~FileSystemFile() = default;
    virtual std::size_t Read(std::uint64_t offset, std::uint8_t* destination, std::size_t length) = 0;
    virtual std::size_t Write(std::uint64_t offset, const std::uint8_t* source, std::size_t length) = 0;
    virtual void Truncate(std::uint64_t length) = 0;
    virtual void Preallocate(std::uint64_t length) {
        throw CoreException(CoreError::kUnsupportedFileSystem);
    }
    virtual void Flush() = 0;
};

/** A contiguous unallocated tail of the mounted filesystem's data area. */
struct FatFsTailFreeRange final {
    std::uint64_t logical_offset;
    std::uint64_t size;
};

class FatFsFile final : public FileSystemFile {
public:
    FatFsFile(FatFsVolume& volume, FIL file);
    FatFsFile(const FatFsFile&) = delete;
    FatFsFile& operator=(const FatFsFile&) = delete;
    ~FatFsFile();

    std::size_t Read(std::uint64_t offset, std::uint8_t* destination, std::size_t length) override;
    std::size_t Write(std::uint64_t offset, const std::uint8_t* source, std::size_t length) override;
    void Truncate(std::uint64_t length) override;
    void Preallocate(std::uint64_t length) override;
    void Flush() override;

private:
    FatFsVolume& volume_;
    FIL file_ {};
    bool closed_ = false;
};

/** Mounts one FAT12/16/32 or exFAT filesystem over an encrypted block device. */
class FatFsVolume final {
public:
    explicit FatFsVolume(NativeVolumeSession& session);
    FatFsVolume(const FatFsVolume&) = delete;
    FatFsVolume& operator=(const FatFsVolume&) = delete;
    ~FatFsVolume();

    FatFsType Mount();
    FatFsType FormatAndMount(FatFsType requested_type);
    void Flush();
    void Unmount() noexcept;
    bool mounted() const noexcept { return mounted_; }
    bool writable() const noexcept { return device_.writable(); }
    FatFsType type() const;
    FATFS& native_filesystem() noexcept { return filesystem_; }
    const char* drive_path() const noexcept { return drive_path_.data(); }
    std::vector<FatFsEntry> ListDirectory(const std::string& relative_path);
    FatFsEntry Stat(const std::string& relative_path);
    std::unique_ptr<FatFsFile> OpenFile(const std::string& relative_path, bool writable, bool create, bool truncate);
    void CreateDirectory(const std::string& relative_path);
    void Delete(const std::string& relative_path);
    void Rename(const std::string& from_relative_path, const std::string& to_relative_path);
    /** Converts a non-success FatFs result into the matching core error. */
    static void ThrowFatFsFailure(FRESULT result);
    void ThrowFatFsFailureWithDevice(FRESULT result);
    /**
     * Finds the free cluster run ending at the final allocatable cluster.
     * Hidden volumes may only occupy this range; aggregate free-space values
     * are unsafe because earlier allocated clusters make them non-contiguous.
     */
    FatFsTailFreeRange FindTailFreeRange();

private:
    friend class FatFsFile;
    std::string ToFatFsPath(const std::string& relative_path) const;

    EncryptedBlockDevice device_;
    FatFsDiskIo disk_io_;
    FATFS filesystem_ {};
    std::array<char, 3> drive_path_ {};
    bool mounted_ = false;
    std::mutex api_mutex_;
};

}  // namespace vc_core
