cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_definitions(-DANDROID )
include_directories(src/main/native/crypto)

# Botan is vendored as a reproducible, minimized arm64-v8a amalgamation.
# Lite deliberately targets modern 64-bit Android devices only.
set(VC_BOTAN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/vc_core/botan")
if(ANDROID_ABI STREQUAL "arm64-v8a")
    set(VC_BOTAN_DIR "${VC_BOTAN_ROOT}/arm64-v8a")
else()
    message(FATAL_ERROR "vc_core Lite build supports arm64-v8a only; got ${ANDROID_ABI}")
endif()

# Refuse to build an arm64 binary if the reproducible Botan amalgamation was
# replaced by a software-only or incomplete file. These checks are source
# gates; the benchmark provider probe below verifies runtime CPUID selection.
file(READ "${VC_BOTAN_DIR}/vc_botan.h" VC_BOTAN_HEADER)
file(READ "${VC_BOTAN_DIR}/vc_botan.cpp" VC_BOTAN_SOURCE)
foreach(VC_REQUIRED_MARKER
        "BOTAN_HAS_CPUID"
        "BOTAN_HAS_CPUID_DETECTION"
        "BOTAN_HAS_AES_ARMV8"
        "BOTAN_HAS_AES"
        "BOTAN_HAS_SERPENT"
        "BOTAN_HAS_TWOFISH"
        "BOTAN_HAS_CAMELLIA"
        "BOTAN_HAS_KUZNYECHIK"
        "BOTAN_HAS_CASCADE"
        "BOTAN_HAS_MODE_XTS"
        "BOTAN_HAS_BLAKE2S"
        "BOTAN_HAS_BLAKE2B"
        "BOTAN_HAS_HMAC"
        "BOTAN_HAS_PBKDF2"
        "BOTAN_HAS_ARGON2"
        "BOTAN_HAS_WHIRLPOOL"
        "BOTAN_HAS_STREEBOG"
        "BOTAN_HAS_SHA2_32"
        "BOTAN_HAS_SHA2_64"
        "BOTAN_HAS_SHA2_32_ARMV8"
        "BOTAN_HAS_SHA2_64_ARMV8")
    string(FIND "${VC_BOTAN_HEADER}" "#define ${VC_REQUIRED_MARKER}" VC_MARKER_POSITION)
    if(VC_MARKER_POSITION EQUAL -1)
        message(FATAL_ERROR "Botan arm64 amalgamation is missing ${VC_REQUIRED_MARKER}")
    endif()
endforeach()
foreach(VC_REQUIRED_INTRINSIC "vaeseq_u8" "vsha256hq_u32" "vsha512hq_u64")
    string(FIND "${VC_BOTAN_SOURCE}" "${VC_REQUIRED_INTRINSIC}" VC_INTRINSIC_POSITION)
    if(VC_INTRINSIC_POSITION EQUAL -1)
        message(FATAL_ERROR "Botan arm64 amalgamation is missing ${VC_REQUIRED_INTRINSIC}")
    endif()
endforeach()

find_library( log-lib
              log )

# Material Files uses this ordinary JNI library for POSIX-compatible file
# operations. It is deliberately separate from its disabled hidden-API and
# root integration, and is required to browse the local/SAF backends. Keep
# the vendored upstream source in reviewable fragments, but combine it before
# compiling because C preprocessor conditional blocks may cross fragment
# boundaries.
set(SYSCALL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../materialfiles/src/main/jni")
set(SYSCALL_COMBINED_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/materialfiles-syscall.c")
file(WRITE "${SYSCALL_COMBINED_SOURCE}" "")
file(GLOB SYSCALL_SOURCE_PARTS "${SYSCALL_SOURCE_DIR}/syscall-part-*.inc")
foreach(SYSCALL_PART IN LISTS SYSCALL_SOURCE_PARTS)
    file(READ "${SYSCALL_PART}" SYSCALL_PART_SOURCE)
    file(APPEND "${SYSCALL_COMBINED_SOURCE}" "${SYSCALL_PART_SOURCE}\n")
endforeach()
add_library(syscall SHARED
            ${SYSCALL_COMBINED_SOURCE})
target_compile_options(syscall PRIVATE -Wall -Werror)
target_link_libraries(syscall PRIVATE ${log-lib})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libyal ${CMAKE_CURRENT_BINARY_DIR}/libyal)

add_library(vc_core SHARED
             src/main/native/vc_core/vc_core_jni.cpp
             ${VC_BOTAN_DIR}/vc_botan.cpp
             src/main/native/vc_core/vc_block_device.cpp
             src/main/native/vc_core/vc_cpu_topology.cpp
             src/main/native/vc_core/vc_fatfs_diskio.cpp
             src/main/native/vc_core/vc_fatfs_system.cpp
             src/main/native/vc_core/vc_fatfs_volume.cpp
             src/main/native/vc_core/vc_ntfs_volume.cpp
             src/main/native/vc_core/fd_random_access.cpp
             src/main/native/vc_core/vc_header.cpp
             src/main/native/vc_core/vc_kdf.cpp
             src/main/native/vc_core/vc_keyfiles.cpp
             src/main/native/vc_core/vc_random.cpp
             src/main/native/vc_core/vc_session_registry.cpp
             src/main/native/vc_core/vc_volume.cpp
             src/main/native/vc_core/vc_request.cpp)

target_sources(vc_core PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/fatfs/ff.c
               ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/fatfs/ffunicode.c)

target_include_directories(vc_core PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/vc_core
                           ${VC_BOTAN_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/fatfs)


target_link_libraries(vc_core
                      vc_libyal_ntfs
                      ${log-lib})

# These framework-free invariants are linked only into Debug builds. Android
# instrumentation invokes them through a no-argument JNI entry point; release
# builds contain neither the test code nor the entry point implementation.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_sources(vc_core PRIVATE
                   src/test/native/vc_header_layout_test.cpp
                   src/test/native/vc_fatfs_result_test.cpp
                   src/test/native/vc_kdf_parameters_test.cpp
                   src/test/native/vc_xts_data_unit_test.cpp)
    target_compile_definitions(vc_core PRIVATE VC_CORE_ENABLE_SELF_TESTS=1)
endif()

option(VC_CORE_ENABLE_SANITIZERS "Enable ASan and UBSan for vc_core test builds" OFF)
if(VC_CORE_ENABLE_SANITIZERS)
    target_compile_options(vc_core PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(vc_core PRIVATE -fsanitize=address,undefined)
endif()

# Benchmark is signed with the debug key at the Gradle layer but must exercise
# the same optimizer/linker path as Release. JNIEXPORT keeps JNI entry points
# visible despite the otherwise hidden native symbol surface.
option(VC_CORE_BENCHMARK "Enable release-style vc_core optimization for benchmarks" OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR VC_CORE_BENCHMARK)
    target_compile_options(vc_core PRIVATE
                           -O3
                           -flto=thin
                           -fvisibility=hidden
                           -ffunction-sections
                           -fdata-sections)
    target_link_options(vc_core PRIVATE
                        -flto=thin
                        -Wl,--gc-sections)
endif()

if(VC_CORE_BENCHMARK)
    target_compile_definitions(vc_core PRIVATE VC_CORE_ENABLE_BENCHMARK_STATS=1)
endif()

# The lite product exposes only vc_core. Legacy EDS JNI libraries remain in
# the source tree as reference code and are intentionally not packaged.
