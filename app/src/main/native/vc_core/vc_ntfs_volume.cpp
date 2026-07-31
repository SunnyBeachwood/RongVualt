#include "vc_ntfs_volume.h"

#include <array>
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <libbfio.h>
#include <libcerror.h>
#include <libfsntfs.h>

#include "vc_error.h"
#include "vc_fatfs_volume.h"
#include "vc_volume.h"

namespace vc_core {
namespace {

struct NtfsIo final {
    NativeVolumeSession* session;
    std::uint64_t offset = 0;
};

bool IsReadableRange(std::uint64_t offset, std::uint64_t size, std::size_t length) {
    return offset <= size && length <= size - offset;
}

[[noreturn]] void RejectNtfsMutation() {
    throw CoreException(CoreError::kReadOnlySource);
}

void RejectNtfsMutationRequest(bool writable, bool create, bool truncate) {
    if (writable || create || truncate) RejectNtfsMutation();
}

bool TrySeekOffset(
        std::uint64_t current,
        std::uint64_t size,
        off64_t offset,
        int whence,
        std::uint64_t* result) {
    if (result == nullptr) return false;
    std::int64_t base = 0;
    if (whence == SEEK_CUR) {
        if (current > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) return false;
        base = static_cast<std::int64_t>(current);
    } else if (whence == SEEK_END) {
        if (size > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) return false;
        base = static_cast<std::int64_t>(size);
    } else if (whence != SEEK_SET) {
        return false;
    }
    if ((offset > 0 && base > std::numeric_limits<std::int64_t>::max() - offset) ||
        (offset < 0 && base < std::numeric_limits<std::int64_t>::min() - offset)) return false;
    const std::int64_t next = base + offset;
    if (next < 0) return false;
    *result = static_cast<std::uint64_t>(next);
    return true;
}

int FreeIo(intptr_t** value, libbfio_error_t**) {
    if (value != nullptr && *value != nullptr) {
        delete reinterpret_cast<NtfsIo*>(*value);
        *value = nullptr;
    }
    return 1;
}

int OpenIo(intptr_t* value, int access_flags, libbfio_error_t**) {
    if (value == nullptr || (access_flags & LIBFSNTFS_OPEN_WRITE) != 0) return -1;
    return 1;
}

int CloseIo(intptr_t*, libbfio_error_t**) { return 0; }

ssize_t ReadIo(intptr_t* value, std::uint8_t* buffer, size_t size, libbfio_error_t**) {
    if (value == nullptr || buffer == nullptr) return -1;
    auto* io = reinterpret_cast<NtfsIo*>(value);
    if (!IsReadableRange(io->offset, io->session->info().logical_size, size)) return -1;
    try {
        const std::size_t read = io->session->Read(io->offset, buffer, size);
        io->offset += read;
        return static_cast<ssize_t>(read);
    } catch (...) {
        return -1;
    }
}

off64_t SeekIo(intptr_t* value, off64_t offset, int whence, libbfio_error_t**) {
    if (value == nullptr) return -1;
    auto* io = reinterpret_cast<NtfsIo*>(value);
    std::uint64_t next = 0;
    if (!TrySeekOffset(io->offset, io->session->info().logical_size, offset, whence, &next)) return -1;
    io->offset = next;
    return static_cast<off64_t>(next);
}

int ExistsIo(intptr_t* value, libbfio_error_t**) { return value == nullptr ? -1 : 1; }
int IsOpenIo(intptr_t* value, libbfio_error_t**) { return value == nullptr ? -1 : 1; }

int GetSizeIo(intptr_t* value, size64_t* size, libbfio_error_t**) {
    if (value == nullptr || size == nullptr) return -1;
    *size = reinterpret_cast<NtfsIo*>(value)->session->info().logical_size;
    return 1;
}

void FreeNtfsError(libfsntfs_error_t** error) noexcept {
    if (error != nullptr && *error != nullptr) {
        libcerror_error_free(reinterpret_cast<libcerror_error_t**>(error));
    }
}

void FreeBfioError(libbfio_error_t** error) noexcept {
    if (error != nullptr && *error != nullptr) {
        libcerror_error_free(reinterpret_cast<libcerror_error_t**>(error));
    }
}

}  // namespace

struct NtfsVolume::State final {
    libbfio_handle_t* io = nullptr;
    libfsntfs_volume_t* volume = nullptr;
};

#if defined(VC_CORE_ENABLE_SELF_TESTS)
extern "C" void VcCoreNtfsCallbackSelfTest() {
    std::uint64_t result = 0;
    assert(IsReadableRange(0, 16, 16));
    assert(IsReadableRange(16, 16, 0));
    assert(!IsReadableRange(16, 16, 1));
    assert(!IsReadableRange(std::numeric_limits<std::uint64_t>::max(), 16, 1));

    assert(TrySeekOffset(8, 16, 4, SEEK_SET, &result) && result == 4);
    assert(TrySeekOffset(8, 16, -4, SEEK_CUR, &result) && result == 4);
    assert(TrySeekOffset(8, 16, -4, SEEK_END, &result) && result == 12);
    assert(!TrySeekOffset(0, 16, -1, SEEK_SET, &result));
    assert(!TrySeekOffset(8, 16, 0, 0x7fffffff, &result));
    assert(!TrySeekOffset(
            static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()),
            16,
            1,
            SEEK_CUR,
            &result));
    assert(!TrySeekOffset(0, 16, std::numeric_limits<off64_t>::min(), SEEK_SET, &result));

    for (const auto request : {std::array<bool, 3> {true, false, false},
                               std::array<bool, 3> {false, true, false},
                               std::array<bool, 3> {false, false, true}}) {
        bool rejected = false;
        try {
            RejectNtfsMutationRequest(request[0], request[1], request[2]);
        } catch (const CoreException& error) {
            rejected = error.error() == CoreError::kReadOnlySource;
        }
        assert(rejected);
    }
}
#endif

namespace {

class NtfsFile final : public FileSystemFile {
public:
    explicit NtfsFile(libfsntfs_file_entry_t* entry) : entry_(entry) {}
    ~NtfsFile() override {
        libfsntfs_error_t* error = nullptr;
        libfsntfs_file_entry_free(&entry_, &error);
        FreeNtfsError(&error);
    }
    std::size_t Read(std::uint64_t offset, std::uint8_t* target, std::size_t length) override {
        if (offset > static_cast<std::uint64_t>(std::numeric_limits<off64_t>::max())) throw CoreException(CoreError::kIoInterrupted);
        libfsntfs_error_t* error = nullptr;
        const ssize_t count = libfsntfs_file_entry_read_buffer_at_offset(entry_, target, length, static_cast<off64_t>(offset), &error);
        FreeNtfsError(&error);
        if (count < 0) throw CoreException(CoreError::kIoInterrupted);
        return static_cast<std::size_t>(count);
    }
    std::size_t Write(std::uint64_t, const std::uint8_t*, std::size_t) override { RejectNtfsMutation(); }
    void Truncate(std::uint64_t) override { RejectNtfsMutation(); }
    void Flush() override {}
private:
    libfsntfs_file_entry_t* entry_ = nullptr;
};

std::string NtfsPath(const std::string& relative_path) {
    if (relative_path.empty() || relative_path == "/") return "\\";
    std::string result("\\");
    result.reserve(relative_path.size() + 1);
    for (const char character : relative_path) {
        if (character == '\\') throw CoreException(CoreError::kNotFound);
        result.push_back(character == '/' ? '\\' : character);
    }
    return result;
}

libfsntfs_file_entry_t* FindEntry(libfsntfs_volume_t* volume, const std::string& relative_path) {
    const std::string path = NtfsPath(relative_path);
    libfsntfs_file_entry_t* entry = nullptr;
    libfsntfs_error_t* error = nullptr;
    const int result = libfsntfs_volume_get_file_entry_by_utf8_path(
            volume, reinterpret_cast<const std::uint8_t*>(path.data()), path.size(), &entry, &error);
    FreeNtfsError(&error);
    if (result == 0) throw CoreException(CoreError::kNotFound);
    if (result != 1 || entry == nullptr) throw CoreException(CoreError::kIoInterrupted);
    return entry;
}

FileSystemEntry ToEntry(libfsntfs_file_entry_t* entry) {
    libfsntfs_error_t* error = nullptr;
    size_t name_size = 0;
    std::uint32_t attributes = 0;
    size64_t size = 0;
    if (libfsntfs_file_entry_get_utf8_name_size(entry, &name_size, &error) != 1 || name_size == 0 ||
        libfsntfs_file_entry_get_file_attribute_flags(entry, &attributes, &error) < 0 ||
        libfsntfs_file_entry_get_size(entry, &size, &error) < 0) {
        FreeNtfsError(&error);
        throw CoreException(CoreError::kIoInterrupted);
    }
    std::vector<std::uint8_t> name(name_size);
    if (libfsntfs_file_entry_get_utf8_name(entry, name.data(), name.size(), &error) != 1) {
        FreeNtfsError(&error);
        throw CoreException(CoreError::kIoInterrupted);
    }
    FreeNtfsError(&error);
    return {std::string(reinterpret_cast<const char*>(name.data()), name.size() - 1),
            (attributes & LIBFSNTFS_FILE_ATTRIBUTE_FLAG_DIRECTORY) != 0, static_cast<std::uint64_t>(size), 0, 0};
}

void ValidateReadableEntry(libfsntfs_file_entry_t* entry) {
    libfsntfs_error_t* error = nullptr;
    std::uint32_t attributes = 0;
    if (libfsntfs_file_entry_get_file_attribute_flags(entry, &attributes, &error) < 0) {
        FreeNtfsError(&error);
        throw CoreException(CoreError::kIoInterrupted);
    }
    FreeNtfsError(&error);
    if ((attributes & (LIBFSNTFS_FILE_ATTRIBUTE_FLAG_ENCRYPTED | LIBFSNTFS_FILE_ATTRIBUTE_FLAG_REPARSE_POINT)) != 0) {
        throw CoreException(CoreError::kUnsupportedFileSystem);
    }
}

}  // namespace

NtfsVolume::NtfsVolume(NativeVolumeSession& session) : session_(session), state_(std::make_unique<State>()) {}

NtfsVolume::~NtfsVolume() { Unmount(); }

FileSystemType NtfsVolume::Mount() {
    if (mounted_) return FileSystemType::kNtfs;
    libbfio_error_t* bfio_error = nullptr;
    libfsntfs_error_t* ntfs_error = nullptr;
    auto* io = new NtfsIo {&session_};
    if (libbfio_handle_initialize(&state_->io, reinterpret_cast<intptr_t*>(io), FreeIo, nullptr, OpenIo, CloseIo,
            ReadIo, nullptr, SeekIo, ExistsIo, IsOpenIo, GetSizeIo, 0, &bfio_error) != 1 ||
        libfsntfs_volume_initialize(&state_->volume, &ntfs_error) != 1 ||
        libfsntfs_volume_open_file_io_handle(state_->volume, state_->io, LIBFSNTFS_OPEN_READ, &ntfs_error) != 1) {
        FreeBfioError(&bfio_error);
        FreeNtfsError(&ntfs_error);
        Unmount();
        throw CoreException(CoreError::kUnsupportedFileSystem);
    }
    mounted_ = true;
    return FileSystemType::kNtfs;
}

void NtfsVolume::Unmount() noexcept {
    libfsntfs_error_t* ntfs_error = nullptr;
    libbfio_error_t* bfio_error = nullptr;
    if (state_ != nullptr && state_->volume != nullptr) {
        libfsntfs_volume_close(state_->volume, &ntfs_error);
        FreeNtfsError(&ntfs_error);
        libfsntfs_volume_free(&state_->volume, &ntfs_error);
        FreeNtfsError(&ntfs_error);
    }
    if (state_ != nullptr && state_->io != nullptr) {
        libbfio_handle_free(&state_->io, &bfio_error);
        FreeBfioError(&bfio_error);
    }
    mounted_ = false;
}

std::vector<FileSystemEntry> NtfsVolume::ListDirectory(const std::string& relative_path) {
    if (!mounted_) throw CoreException(CoreError::kUnsupportedFileSystem);
    std::unique_ptr<libfsntfs_file_entry_t, void(*)(libfsntfs_file_entry_t*)> directory(
            FindEntry(state_->volume, relative_path), [](libfsntfs_file_entry_t* value) {
                libfsntfs_error_t* error = nullptr; libfsntfs_file_entry_free(&value, &error); FreeNtfsError(&error); });
    ValidateReadableEntry(directory.get());
    int count = 0;
    libfsntfs_error_t* error = nullptr;
    if (libfsntfs_file_entry_get_number_of_sub_file_entries(directory.get(), &count, &error) != 1 || count < 0) {
        FreeNtfsError(&error); throw CoreException(CoreError::kNotFound);
    }
    std::vector<FileSystemEntry> result;
    result.reserve(static_cast<std::size_t>(count));
    for (int index = 0; index < count; ++index) {
        libfsntfs_file_entry_t* child = nullptr;
        if (libfsntfs_file_entry_get_sub_file_entry_by_index(directory.get(), index, &child, &error) != 1 || child == nullptr) {
            FreeNtfsError(&error); throw CoreException(CoreError::kIoInterrupted);
        }
        try { ValidateReadableEntry(child); result.push_back(ToEntry(child)); }
        catch (...) { libfsntfs_file_entry_free(&child, &error); FreeNtfsError(&error); throw; }
        libfsntfs_file_entry_free(&child, &error); FreeNtfsError(&error);
    }
    return result;
}

FileSystemEntry NtfsVolume::Stat(const std::string& relative_path) {
    if (!mounted_) throw CoreException(CoreError::kUnsupportedFileSystem);
    libfsntfs_file_entry_t* entry = FindEntry(state_->volume, relative_path);
    try { ValidateReadableEntry(entry); const FileSystemEntry result = ToEntry(entry); libfsntfs_error_t* error = nullptr; libfsntfs_file_entry_free(&entry, &error); FreeNtfsError(&error); return result; }
    catch (...) { libfsntfs_error_t* error = nullptr; libfsntfs_file_entry_free(&entry, &error); FreeNtfsError(&error); throw; }
}

std::unique_ptr<FileSystemFile> NtfsVolume::OpenFile(const std::string& relative_path, bool writable, bool create, bool truncate) {
    RejectNtfsMutationRequest(writable, create, truncate);
    if (!mounted_) throw CoreException(CoreError::kUnsupportedFileSystem);
    libfsntfs_file_entry_t* entry = FindEntry(state_->volume, relative_path);
    try { ValidateReadableEntry(entry); return std::make_unique<NtfsFile>(entry); }
    catch (...) { libfsntfs_error_t* error = nullptr; libfsntfs_file_entry_free(&entry, &error); FreeNtfsError(&error); throw; }
}

}  // namespace vc_core
