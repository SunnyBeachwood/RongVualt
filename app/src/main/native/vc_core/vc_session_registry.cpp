#include "vc_session_registry.h"

#include "vc_fatfs_volume.h"

#include <limits>
#include <stdexcept>
#include <utility>

namespace vc_core {

NativeSessionRegistry::FileHandle::FileHandle(
        std::uint64_t session_handle_value,
        std::shared_ptr<NativeVolumeSession> session_value,
        std::unique_ptr<FileSystemFile> file_value)
    : session_handle(session_handle_value), session(std::move(session_value)), file(std::move(file_value)) {}

void NativeSessionRegistry::FileHandle::Close() noexcept {
    std::lock_guard<std::mutex> lock(mutex);
    file.reset();
}

NativeSessionRegistry& NativeSessionRegistry::Instance() {
    static NativeSessionRegistry registry;
    return registry;
}

std::uint64_t NativeSessionRegistry::Insert(
        FdRandomAccess container, OpenedVolume opened, std::optional<EncryptedRange> protected_hidden_range) {
    auto session = std::make_shared<NativeVolumeSession>(std::move(container), std::move(opened), protected_hidden_range);
    std::lock_guard<std::mutex> lock(mutex_);
    if (next_handle_ == std::numeric_limits<std::uint64_t>::max()) throw std::runtime_error("Native session handle space exhausted");
    const std::uint64_t handle = next_handle_++;
    sessions_.emplace(handle, std::move(session));
    return handle;
}

std::uint64_t NativeSessionRegistry::CreateNormal(
        FdRandomAccess container, const CreateRequest& request, const std::vector<FdRandomAccess>& keyfiles,
        const CreateProgressCallback& progress) {
    auto session = CreateNormalVeraCryptVolume(std::move(container), request, keyfiles, progress);
    std::lock_guard<std::mutex> lock(mutex_);
    if (next_handle_ == std::numeric_limits<std::uint64_t>::max()) {
        throw std::runtime_error("Native session handle space exhausted");
    }
    const std::uint64_t handle = next_handle_++;
    sessions_.emplace(handle, std::move(session));
    return handle;
}

std::uint64_t NativeSessionRegistry::CreateHidden(
        std::uint64_t outer_handle, const CreateRequest& request, const std::vector<FdRandomAccess>& keyfiles,
        const CreateProgressCallback& progress) {
    // Hold the registry lock while the child is registered so Close cannot
    // release the outer session's descriptor or writer lease mid-creation.
    std::lock_guard<std::mutex> lock(mutex_);
    const auto outer = sessions_.find(outer_handle);
    if (outer == sessions_.end()) throw std::invalid_argument("Outer volume session is closed");
    auto hidden = outer->second->CreateHiddenVolume(request, keyfiles, progress);
    if (next_handle_ == std::numeric_limits<std::uint64_t>::max()) {
        throw std::runtime_error("Native session handle space exhausted");
    }
    const std::uint64_t handle = next_handle_++;
    sessions_.emplace(handle, std::move(hidden));
    parent_by_child_.emplace(handle, outer_handle);
    return handle;
}

void NativeSessionRegistry::Close(std::uint64_t handle) noexcept {
    std::vector<std::shared_ptr<FileHandle>> files;
    std::vector<std::shared_ptr<NativeVolumeSession>> sessions;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::uint64_t> pending {handle};
        for (std::size_t index = 0; index < pending.size(); ++index) {
            const std::uint64_t current = pending[index];
            const auto session = sessions_.find(current);
            if (session == sessions_.end()) continue;
            sessions.push_back(std::move(session->second));
            sessions_.erase(session);
            parent_by_child_.erase(current);
            for (auto parent = parent_by_child_.begin(); parent != parent_by_child_.end();) {
                if (parent->second == current) {
                    pending.push_back(parent->first);
                    parent = parent_by_child_.erase(parent);
                } else {
                    ++parent;
                }
            }
            for (auto file = files_.begin(); file != files_.end();) {
                if (file->second->session_handle == current) {
                    files.push_back(std::move(file->second));
                    file = files_.erase(file);
                } else {
                    ++file;
                }
            }
        }
    }
    if (sessions.empty()) return;
    for (const auto& file : files) file->Close();
    for (const auto& session : sessions) {
        // f_close may write directory metadata after an earlier explicit flush.
        // Sync once more before the encrypted container descriptor is released.
        try { session->Flush(); } catch (...) {}
        session->UnmountFileSystem();
    }
}

std::shared_ptr<NativeVolumeSession> NativeSessionRegistry::Find(std::uint64_t handle) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = sessions_.find(handle);
    if (it == sessions_.end()) throw std::invalid_argument("Native volume session is closed");
    return it->second;
}

std::size_t NativeSessionRegistry::Read(std::uint64_t handle, std::uint64_t offset, std::uint8_t* target, std::size_t length) {
    return Find(handle)->Read(offset, target, length);
}

std::size_t NativeSessionRegistry::Write(std::uint64_t handle, std::uint64_t offset, const std::uint8_t* source, std::size_t length) {
    return Find(handle)->Write(offset, source, length);
}

#if defined(VC_CORE_ENABLE_BENCHMARK_STATS)
std::size_t NativeSessionRegistry::ReadUncachedForBenchmark(
        std::uint64_t handle, std::uint64_t offset, std::uint8_t* target, std::size_t length) {
    return Find(handle)->ReadUncachedForBenchmark(offset, target, length);
}
#endif

#if defined(VC_CORE_ENABLE_BENCHMARK_STATS)
std::size_t NativeSessionRegistry::ReadLegacySector(
        std::uint64_t handle, std::uint64_t offset, std::uint8_t* target, std::size_t length) {
    return Find(handle)->ReadLegacySector(offset, target, length);
}

std::size_t NativeSessionRegistry::WriteLegacySector(
        std::uint64_t handle, std::uint64_t offset, const std::uint8_t* source, std::size_t length) {
    return Find(handle)->WriteLegacySector(offset, source, length);
}
#endif

void NativeSessionRegistry::Flush(std::uint64_t handle) {
    std::lock_guard<std::mutex> registry_lock(mutex_);
    const auto session = sessions_.find(handle);
    if (session == sessions_.end()) throw std::invalid_argument("Native volume session is closed");
    // Hold the registry lock until both file metadata and the container reach
    // stable storage so Close cannot introduce a later f_close write.
    for (const auto& entry : files_) {
        if (entry.second->session_handle != handle) continue;
        std::lock_guard<std::mutex> file_lock(entry.second->mutex);
        if (entry.second->file != nullptr) entry.second->file->Flush();
    }
    session->second->Flush();
}

FileSystemType NativeSessionRegistry::MountFileSystem(std::uint64_t handle) {
    return Find(handle)->MountFileSystem();
}

VolumePerformanceCounters NativeSessionRegistry::GetPerformanceCounters(std::uint64_t handle) {
    return Find(handle)->performance_counters();
}

std::vector<FileSystemEntry> NativeSessionRegistry::ListDirectory(std::uint64_t handle, const std::string& relative_path) {
    return Find(handle)->ListDirectory(relative_path);
}

FileSystemEntry NativeSessionRegistry::Stat(std::uint64_t handle, const std::string& relative_path) {
    return Find(handle)->StatFileSystemEntry(relative_path);
}

std::uint64_t NativeSessionRegistry::OpenFile(
        std::uint64_t handle, const std::string& relative_path, bool writable, bool create, bool truncate) {
    // Keep the registry lock through f_open so Close cannot erase and unmount
    // the session between lookup and file-handle registration.
    std::lock_guard<std::mutex> lock(mutex_);
    const auto session_it = sessions_.find(handle);
    if (session_it == sessions_.end()) throw std::invalid_argument("Native volume session is closed");
    const auto session = session_it->second;
    auto file = session->OpenFileSystemFile(relative_path, writable, create, truncate);
    if (next_handle_ == std::numeric_limits<std::uint64_t>::max()) throw std::runtime_error("Native file handle space exhausted");
    const std::uint64_t file_handle = next_handle_++;
    files_.emplace(file_handle, std::make_shared<FileHandle>(handle, session, std::move(file)));
    return file_handle;
}

std::size_t NativeSessionRegistry::ReadFile(
        std::uint64_t file_handle, std::uint64_t offset, std::uint8_t* target, std::size_t length) {
    const auto file = FindFile(file_handle);
    std::lock_guard<std::mutex> lock(file->mutex);
    if (file->file == nullptr) throw std::invalid_argument("Native file handle is closed");
    return file->file->Read(offset, target, length);
}

std::size_t NativeSessionRegistry::WriteFile(
        std::uint64_t file_handle, std::uint64_t offset, const std::uint8_t* source, std::size_t length) {
    const auto file = FindFile(file_handle);
    std::lock_guard<std::mutex> lock(file->mutex);
    if (file->file == nullptr) throw std::invalid_argument("Native file handle is closed");
    return file->file->Write(offset, source, length);
}

void NativeSessionRegistry::TruncateFile(std::uint64_t file_handle, std::uint64_t length) {
    const auto file = FindFile(file_handle);
    std::lock_guard<std::mutex> lock(file->mutex);
    if (file->file == nullptr) throw std::invalid_argument("Native file handle is closed");
    file->file->Truncate(length);
}

void NativeSessionRegistry::PreallocateFile(std::uint64_t file_handle, std::uint64_t length) {
    const auto file = FindFile(file_handle);
    std::lock_guard<std::mutex> lock(file->mutex);
    if (file->file == nullptr) throw std::invalid_argument("Native file handle is closed");
    file->file->Preallocate(length);
}

void NativeSessionRegistry::FlushFile(std::uint64_t file_handle) {
    const auto file = FindFile(file_handle);
    std::lock_guard<std::mutex> lock(file->mutex);
    if (file->file == nullptr) throw std::invalid_argument("Native file handle is closed");
    file->file->Flush();
}

void NativeSessionRegistry::CloseFile(std::uint64_t file_handle) noexcept {
    std::shared_ptr<FileHandle> file;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto it = files_.find(file_handle);
        if (it == files_.end()) return;
        file = std::move(it->second);
        files_.erase(it);
    }
    file->Close();
}

void NativeSessionRegistry::CreateDirectory(std::uint64_t handle, const std::string& relative_path) {
    Find(handle)->CreateFatFsDirectory(relative_path);
}

void NativeSessionRegistry::Delete(std::uint64_t handle, const std::string& relative_path) {
    Find(handle)->DeleteFatFsEntry(relative_path);
}

void NativeSessionRegistry::Rename(
        std::uint64_t handle, const std::string& from_relative_path, const std::string& to_relative_path) {
    Find(handle)->RenameFatFsEntry(from_relative_path, to_relative_path);
}

HiddenVolumeCapacity NativeSessionRegistry::AnalyzeHiddenVolumeCapacity(std::uint64_t handle) {
    return Find(handle)->AnalyzeHiddenVolumeCapacity();
}

VolumeInfo NativeSessionRegistry::GetInfo(std::uint64_t handle) {
    return Find(handle)->info();
}

void NativeSessionRegistry::BackupHeader(
        std::uint64_t handle, int output_fd, const OpenRequest& options, const std::vector<FdRandomAccess>& keyfiles) {
    Find(handle)->BackupHeader(output_fd, options, keyfiles);
}

void NativeSessionRegistry::ChangeCredentials(
        std::uint64_t handle, const OpenRequest& options, const std::vector<FdRandomAccess>& keyfiles) {
    Find(handle)->ChangeCredentials(options, keyfiles);
}

std::shared_ptr<NativeSessionRegistry::FileHandle> NativeSessionRegistry::FindFile(std::uint64_t handle) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = files_.find(handle);
    if (it == files_.end()) throw std::invalid_argument("Native file handle is closed");
    return it->second;
}

void NativeSessionRegistry::CloseFilesForSession(std::uint64_t session_handle) noexcept {
    std::vector<std::shared_ptr<FileHandle>> files;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it = files_.begin(); it != files_.end();) {
            if (it->second->session_handle == session_handle) {
                files.push_back(std::move(it->second));
                it = files_.erase(it);
            } else {
                ++it;
            }
        }
    }
    for (const auto& file : files) file->Close();
}

}  // namespace vc_core
