#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "vc_volume.h"

namespace vc_core { class FileSystemFile; }

namespace vc_core {

class NativeSessionRegistry final {
public:
    static NativeSessionRegistry& Instance();
    std::uint64_t Insert(FdRandomAccess container, OpenedVolume opened, std::optional<EncryptedRange> protected_hidden_range);
    std::uint64_t CreateNormal(FdRandomAccess container, const CreateRequest& request, const std::vector<FdRandomAccess>& keyfiles, const CreateProgressCallback& progress);
    std::uint64_t CreateHidden(std::uint64_t outer_handle, const CreateRequest& request, const std::vector<FdRandomAccess>& keyfiles, const CreateProgressCallback& progress);
    void Close(std::uint64_t handle) noexcept;
    std::size_t Read(std::uint64_t handle, std::uint64_t offset, std::uint8_t* target, std::size_t length);
    std::size_t Write(std::uint64_t handle, std::uint64_t offset, const std::uint8_t* source, std::size_t length);
#if defined(VC_CORE_ENABLE_BENCHMARK_STATS)
    std::size_t ReadUncachedForBenchmark(std::uint64_t handle, std::uint64_t offset, std::uint8_t* target, std::size_t length);
#endif
#if defined(VC_CORE_ENABLE_BENCHMARK_STATS)
    std::size_t ReadLegacySector(std::uint64_t handle, std::uint64_t offset, std::uint8_t* target, std::size_t length);
    std::size_t WriteLegacySector(std::uint64_t handle, std::uint64_t offset, const std::uint8_t* source, std::size_t length);
#endif
    void Flush(std::uint64_t handle);
    FileSystemType MountFileSystem(std::uint64_t handle);
    std::vector<FileSystemEntry> ListDirectory(std::uint64_t handle, const std::string& relative_path);
    FileSystemEntry Stat(std::uint64_t handle, const std::string& relative_path);
    std::uint64_t OpenFile(std::uint64_t handle, const std::string& relative_path, bool writable, bool create, bool truncate);
    std::size_t ReadFile(std::uint64_t file_handle, std::uint64_t offset, std::uint8_t* target, std::size_t length);
    std::size_t WriteFile(std::uint64_t file_handle, std::uint64_t offset, const std::uint8_t* source, std::size_t length);
    void TruncateFile(std::uint64_t file_handle, std::uint64_t length);
    void PreallocateFile(std::uint64_t file_handle, std::uint64_t length);
    void FlushFile(std::uint64_t file_handle);
    void CloseFile(std::uint64_t file_handle) noexcept;
    void CreateDirectory(std::uint64_t handle, const std::string& relative_path);
    void Delete(std::uint64_t handle, const std::string& relative_path);
    void Rename(std::uint64_t handle, const std::string& from_relative_path, const std::string& to_relative_path);
    HiddenVolumeCapacity AnalyzeHiddenVolumeCapacity(std::uint64_t handle);
    VolumeInfo GetInfo(std::uint64_t handle);
    VolumePerformanceCounters GetPerformanceCounters(std::uint64_t handle);
    void BackupHeader(std::uint64_t handle, int output_fd, const OpenRequest& options, const std::vector<FdRandomAccess>& keyfiles);
    void ChangeCredentials(std::uint64_t handle, const OpenRequest& options, const std::vector<FdRandomAccess>& keyfiles);

private:
    NativeSessionRegistry() = default;
    struct FileHandle final {
        FileHandle(std::uint64_t session_handle, std::shared_ptr<NativeVolumeSession> session, std::unique_ptr<FileSystemFile> file);
        void Close() noexcept;

        const std::uint64_t session_handle;
        const std::shared_ptr<NativeVolumeSession> session;
        std::mutex mutex;
        std::unique_ptr<FileSystemFile> file;
    };

    std::shared_ptr<NativeVolumeSession> Find(std::uint64_t handle);
    std::shared_ptr<FileHandle> FindFile(std::uint64_t handle);
    void CloseFilesForSession(std::uint64_t session_handle) noexcept;
    std::mutex mutex_;
    std::uint64_t next_handle_ = 1;
    std::unordered_map<std::uint64_t, std::shared_ptr<NativeVolumeSession>> sessions_;
    std::unordered_map<std::uint64_t, std::uint64_t> parent_by_child_;
    std::unordered_map<std::uint64_t, std::shared_ptr<FileHandle>> files_;
};

}  // namespace vc_core
