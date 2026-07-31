#pragma once

#include <cstdint>
#include <string>

namespace vc_core {

/** Filesystem values returned across the JNI boundary. Keep these stable. */
enum class FileSystemType : std::uint8_t {
    kFat = 1,
    kExFat = 2,
    kNtfs = 3,
};

// FatFs can create and mount only the two writable filesystem families.
using FatFsType = FileSystemType;

/** Metadata shared by all mounted native filesystem adapters. */
struct FileSystemEntry final {
    std::string name;
    bool directory;
    std::uint64_t size;
    std::uint16_t modified_date;
    std::uint16_t modified_time;
};

using FatFsEntry = FileSystemEntry;

}  // namespace vc_core
