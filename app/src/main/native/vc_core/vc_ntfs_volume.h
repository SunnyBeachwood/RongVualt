#pragma once

#include <memory>
#include <string>
#include <vector>

#include "vc_filesystem_types.h"

namespace vc_core {

class NativeVolumeSession;
class FileSystemFile;

/**
 * Read-only NTFS adapter. libfsntfs receives a libbfio callback over the
 * decrypted volume session; it never sees a URI, host path, or raw container.
 */
class NtfsVolume final {
public:
    explicit NtfsVolume(NativeVolumeSession& session);
    NtfsVolume(const NtfsVolume&) = delete;
    NtfsVolume& operator=(const NtfsVolume&) = delete;
    ~NtfsVolume();

    FileSystemType Mount();
    void Unmount() noexcept;
    bool mounted() const noexcept { return mounted_; }
    std::vector<FileSystemEntry> ListDirectory(const std::string& relative_path);
    FileSystemEntry Stat(const std::string& relative_path);
    std::unique_ptr<FileSystemFile> OpenFile(const std::string& relative_path, bool writable, bool create, bool truncate);

private:
    struct State;
    NativeVolumeSession& session_;
    std::unique_ptr<State> state_;
    bool mounted_ = false;
};

}  // namespace vc_core
