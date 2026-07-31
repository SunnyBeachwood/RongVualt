#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include "fd_random_access.h"
#include "vc_filesystem_types.h"
#include "vc_header.h"
#include "vc_request.h"

namespace vc_core {

class FatFsVolume;
class FileSystemFile;
class NtfsVolume;
class XtsTransformContext;
class XtsParallelExecutor;
class SecureBlockCache;

struct OpenedVolume final {
    HeaderMetadata metadata;
    CipherHint cipher;
    KdfHint kdf;
    bool used_backup_header;
    bool hidden_volume;
    std::uint64_t primary_header_offset;
    std::uint64_t backup_header_offset;
    SecureBytes master_key;
    SecureBytes decrypted_header;
};

struct EncryptedRange final {
    std::uint64_t offset;
    std::uint64_t size;
};

/** Native-only placement result for the two-stage hidden-volume flow. */
struct HiddenVolumeCapacity final {
    std::uint64_t maximum_size;
    std::uint64_t encrypted_area_offset;
};

/** Returns false when the Java owner has requested cancellation. */
using CreateProgressCallback = std::function<bool(std::uint32_t stage, std::uint64_t completed, std::uint64_t total)>;

/** VeraCrypt XTS data-unit numbering is based on absolute container bytes. */
std::uint64_t VeraCryptDataUnitNumberForPhysicalOffset(std::uint64_t physical_offset);

/**
 * Metadata that is safe to expose across JNI.  This deliberately excludes
 * passwords, header salts, derived keys, and the master-key material.
 * Filesystem identification belongs to the filesystem adapter because it is
 * not encoded in a VeraCrypt volume header.
 */
struct VolumeInfo final {
    std::uint64_t logical_size;
    std::uint64_t encrypted_area_offset;
    std::uint64_t encrypted_area_size;
    std::uint32_t sector_size;
    CipherHint cipher;
    KdfHint kdf;
    bool hidden_volume;
    bool used_backup_header;
};

/** Debug/benchmark-only aggregate facts, deliberately excluding user data. */
struct VolumePerformanceCounters final {
    std::uint64_t read_syscalls;
    std::uint64_t write_syscalls;
    std::uint64_t read_bytes;
    std::uint64_t write_bytes;
    std::uint64_t batched_reads;
    std::uint64_t batched_writes;
    std::uint64_t rmw_sectors;
    std::uint64_t xts_data_units;
    std::uint64_t cache_hits;
    std::uint64_t cache_misses;
};

/** Called between header-candidate batches. Return false to cancel opening. */
using OpenProgressCallback = std::function<bool(std::uint32_t completed, std::uint32_t total)>;

/** Opens a non-system VeraCrypt file container. The returned key remains native. */
OpenedVolume OpenVeraCryptVolume(
        const FdRandomAccess& container,
        const OpenRequest& request,
        const std::vector<FdRandomAccess>& keyfiles,
        const OpenProgressCallback& progress = {});

/** Creates a normal container without exposing its master key outside native code. */
std::shared_ptr<class NativeVolumeSession> CreateNormalVeraCryptVolume(
        FdRandomAccess container,
        const CreateRequest& request,
        const std::vector<FdRandomAccess>& keyfiles,
        const CreateProgressCallback& progress);

/** Decrypts a 128 KiB header-backup group without treating it as volume data. */
OpenedVolume OpenVeraCryptHeaderBackup(
        const FdRandomAccess& header_backup,
        const OpenRequest& request,
        const std::vector<FdRandomAccess>& keyfiles);

/** Restores a 128 KiB VeraCrypt header-group backup after decrypting it first. */
void RestoreVeraCryptHeader(
        FdRandomAccess& container,
        const FdRandomAccess& header_backup,
        const OpenRequest& request,
        const std::vector<FdRandomAccess>& keyfiles);

/** Owns an unlocked AES-XTS volume and exposes logical plaintext offsets only. */
class NativeVolumeSession final {
public:
    NativeVolumeSession(FdRandomAccess container, OpenedVolume opened, std::optional<EncryptedRange> protected_hidden_range);
    NativeVolumeSession(const NativeVolumeSession&) = delete;
    NativeVolumeSession& operator=(const NativeVolumeSession&) = delete;
    ~NativeVolumeSession();

    std::size_t Read(std::uint64_t logical_offset, std::uint8_t* target, std::size_t length);
    std::size_t Write(std::uint64_t logical_offset, const std::uint8_t* source, std::size_t length);
#if defined(VC_CORE_ENABLE_BENCHMARK_STATS)
    std::size_t ReadUncachedForBenchmark(std::uint64_t logical_offset, std::uint8_t* target, std::size_t length);
    std::size_t ReadLegacySector(std::uint64_t logical_offset, std::uint8_t* target, std::size_t length);
    std::size_t WriteLegacySector(std::uint64_t logical_offset, const std::uint8_t* source, std::size_t length);
#endif
    void Flush();
    FileSystemType MountFileSystem();
    FatFsType FormatFatFs(FatFsType requested_type);
    void UnmountFileSystem() noexcept;
    std::vector<FileSystemEntry> ListDirectory(const std::string& relative_path);
    FileSystemEntry StatFileSystemEntry(const std::string& relative_path);
    std::unique_ptr<FileSystemFile> OpenFileSystemFile(
            const std::string& relative_path, bool writable, bool create, bool truncate);
    void CreateFatFsDirectory(const std::string& relative_path);
    void DeleteFatFsEntry(const std::string& relative_path);
    void RenameFatFsEntry(const std::string& from_relative_path, const std::string& to_relative_path);
    HiddenVolumeCapacity AnalyzeHiddenVolumeCapacity();
    /** Creates a child hidden session from the still-valid outer capacity scan. */
    std::shared_ptr<NativeVolumeSession> CreateHiddenVolume(
            const CreateRequest& request, const std::vector<FdRandomAccess>& keyfiles, const CreateProgressCallback& progress);
    /** Commits backup then primary header after creation formatting succeeds. */
    void FinalizeCreatedHeaders(const SecureBytes& processed_password, std::int32_t pim);
    void BackupHeader(int output_fd, const OpenRequest& options, const std::vector<FdRandomAccess>& keyfiles);
    void ChangeCredentials(const OpenRequest& options, const std::vector<FdRandomAccess>& keyfiles);
    const HeaderMetadata& metadata() const noexcept { return metadata_; }
    bool writable() const noexcept { return container_.writable(); }
    VolumeInfo info() const;
    VolumePerformanceCounters performance_counters() const;
    bool hidden_volume_protection_triggered() const noexcept;

private:
    void TransformSectorAtPhysicalOffset(std::uint64_t physical_offset, std::uint8_t* data, bool encrypt);
    void TransformAlignedBlockAtPhysicalOffset(
            std::uint64_t physical_offset, std::uint8_t* data, std::size_t length, bool encrypt);
    std::size_t ReadUncachedLocked(std::uint64_t logical_offset, std::uint8_t* target, std::size_t length);
    void CheckHiddenVolumeProtectionRange(std::uint64_t logical_offset, std::size_t length);
    void CheckRange(std::uint64_t logical_offset, std::size_t length) const;

    FdRandomAccess container_;
    RandomAccessCounters io_counters_at_open_ {};
    HeaderMetadata metadata_;
    CipherHint cipher_;
    KdfHint kdf_;
    SecureBytes master_key_;
    std::unique_ptr<XtsTransformContext> xts_;
    std::unique_ptr<XtsParallelExecutor> parallel_xts_;
    std::unique_ptr<SecureBlockCache> read_cache_;
    std::vector<std::uint8_t> write_scratch_;
    SecureBytes decrypted_header_;
    std::uint64_t primary_header_offset_;
    std::uint64_t backup_header_offset_;
    bool hidden_volume_;
    bool used_backup_header_;
    std::optional<EncryptedRange> protected_hidden_range_;
    std::uint64_t batched_reads_ = 0;
    std::uint64_t batched_writes_ = 0;
    std::uint64_t rmw_sectors_ = 0;
    std::uint64_t xts_data_units_ = 0;
    // Set only by a successful tail-free-space scan. Any outer-volume write
    // clears it, so hidden creation cannot consume a stale filesystem view.
    std::optional<HiddenVolumeCapacity> analyzed_hidden_capacity_;
    bool hidden_volume_protection_triggered_ = false;
    mutable std::mutex mutex_;
    std::mutex filesystem_mutex_;
    std::unique_ptr<FatFsVolume> fatfs_;
    std::unique_ptr<NtfsVolume> ntfs_;
};

}  // namespace vc_core
