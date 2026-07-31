# Pinned libyal dependency closure used exclusively by vc_core's NTFS reader.
# Source revisions are verified by tools/verify-ntfs-dependencies.ps1.
cmake_minimum_required(VERSION 3.22)

set(VC_LIBYAL_ROOT "${CMAKE_CURRENT_LIST_DIR}")
set(VC_LIBYAL_GENERATED "${CMAKE_CURRENT_BINARY_DIR}/generated")

set(VC_LIBYAL_LIBRARIES
    libcerror libcthreads libcdata libclocale libcnotify libcfile libcpath
    libcsplit libuna libbfio libfcache libfdata libfdatetime libfguid libfwnt libfsntfs)

# Upstream uses Autoconf substitutions in public headers. These are the
# API-29 Android NDK capabilities used by the pinned source closure. Keep this
# list explicit: silently treating a new Autoconf token as false can compile a
# broken filesystem reader.
set(VC_LIBYAL_TEMPLATE_SUBSTITUTIONS
    "@HAVE_INTTYPES_H@=1"
    "@HAVE_LIBBFIO@=1"
    "@HAVE_MULTI_THREAD_SUPPORT@=1"
    "@HAVE_OFF64_T@=1"
    "@HAVE_SIZE32_T@=0"
    "@HAVE_SIZE64_T@=0"
    "@HAVE_SSIZE32_T@=0"
    "@HAVE_SSIZE64_T@=0"
    "@HAVE_STDINT_H@=1"
    "@HAVE_SYS_TYPES_H@=1"
    "@HAVE_WCHAR_H@=1"
    # Android's 32-bit wchar_t is not supported by the pinned libyal wide
    # character conversion paths. NTFS names use libuna's explicit UTF-16
    # conversion instead.
    "@HAVE_WIDE_CHARACTER_TYPE@=0"
    "@VERSION@=0")

function(vc_libyal_configure_template contents library output_contents)
    string(REPLACE "@PACKAGE@" "${library}" configured "${contents}")
    foreach(substitution IN LISTS VC_LIBYAL_TEMPLATE_SUBSTITUTIONS)
        string(REPLACE "=" ";" substitution_parts "${substitution}")
        list(GET substitution_parts 0 token)
        list(GET substitution_parts 1 value)
        string(REPLACE "${token}" "${value}" configured "${configured}")
    endforeach()
    string(REGEX MATCH "@[A-Za-z0-9_]+@" unresolved_token "${configured}")
    if(unresolved_token)
        message(FATAL_ERROR "Unresolved libyal template token ${unresolved_token} in ${library}")
    endif()
    set(${output_contents} "${configured}" PARENT_SCOPE)
endfunction()

function(vc_libyal_generate_headers library)
    file(GLOB_RECURSE templates RELATIVE "${VC_LIBYAL_ROOT}/${library}/include"
         "${VC_LIBYAL_ROOT}/${library}/include/*.h.in")
    foreach(template IN LISTS templates)
        file(READ "${VC_LIBYAL_ROOT}/${library}/include/${template}" contents)
        vc_libyal_configure_template("${contents}" "${library}" contents)
        string(REGEX REPLACE "\\.in$" "" output "${template}")
        get_filename_component(output_directory "${VC_LIBYAL_GENERATED}/${library}/${output}" DIRECTORY)
        file(MAKE_DIRECTORY "${output_directory}")
        file(WRITE "${VC_LIBYAL_GENERATED}/${library}/${output}" "${contents}")
    endforeach()
    # libyal implementation files include the configured common types header
    # directly as <types.h>, while public headers use <libname/types.h>.
    file(GLOB common_templates RELATIVE "${VC_LIBYAL_ROOT}/${library}/common"
         "${VC_LIBYAL_ROOT}/${library}/common/*.h.in")
    foreach(template IN LISTS common_templates)
        file(READ "${VC_LIBYAL_ROOT}/${library}/common/${template}" contents)
        vc_libyal_configure_template("${contents}" "${library}" contents)
        string(REGEX REPLACE "\\.in$" "" output "${template}")
        file(WRITE "${VC_LIBYAL_GENERATED}/${library}/${output}" "${contents}")
    endforeach()
    # Internal implementation headers also carry autotools substitutions.
    file(GLOB implementation_templates RELATIVE "${VC_LIBYAL_ROOT}/${library}/${library}"
         "${VC_LIBYAL_ROOT}/${library}/${library}/*.h.in")
    foreach(template IN LISTS implementation_templates)
        file(READ "${VC_LIBYAL_ROOT}/${library}/${library}/${template}" contents)
        vc_libyal_configure_template("${contents}" "${library}" contents)
        string(REGEX REPLACE "\\.in$" "" output "${template}")
        file(WRITE "${VC_LIBYAL_GENERATED}/${library}/${output}" "${contents}")
    endforeach()
endfunction()

foreach(library IN LISTS VC_LIBYAL_LIBRARIES)
    vc_libyal_generate_headers("${library}")
endforeach()

set(VC_LIBYAL_PUBLIC_INCLUDE_DIRECTORIES)
foreach(library IN LISTS VC_LIBYAL_LIBRARIES)
    list(APPEND VC_LIBYAL_PUBLIC_INCLUDE_DIRECTORIES
        "${VC_LIBYAL_GENERATED}/${library}"
        "${VC_LIBYAL_ROOT}/${library}/include"
        "${VC_LIBYAL_ROOT}/${library}/${library}")
endforeach()

set(VC_LIBYAL_LOCAL_DEFINITIONS
    HAVE_STDINT_H=1 HAVE_INTTYPES_H=1 HAVE_SYS_TYPES_H=1
    HAVE_STDLIB_H=1 HAVE_STRING_H=1 HAVE_STDARG_H=1 HAVE_ERRNO_H=1
    HAVE_LIMITS_H=1 HAVE_UNISTD_H=1 HAVE_FCNTL_H=1 HAVE_SYS_STAT_H=1
    HAVE_SYS_TIME_H=1 HAVE_SIGNAL_H=1 HAVE_LOCALE_H=1 HAVE_WCHAR_H=1
    HAVE_WCTYPE_H=1 HAVE_PTHREAD_H=1 HAVE_MALLOC=1 HAVE_REALLOC=1
    HAVE_FREE=1 HAVE_MEMCHR=1 HAVE_MEMCMP=1 HAVE_MEMCPY=1 HAVE_MEMSET=1
    HAVE_STRLEN=1 HAVE_STRNCMP=1 HAVE_STRNCASECMP=1 HAVE_STRCHR=1
    HAVE_STRRCHR=1 HAVE_STRSTR=1 HAVE_SNPRINTF=1 HAVE_SSCANF=1
    HAVE_VSNPRINTF=1 HAVE_STRERROR=1 HAVE_GETENV=1 HAVE_LOCALECONV=1
    HAVE_SETLOCALE=1 HAVE_CLOCK_GETTIME=1 HAVE_CLOSE=1 HAVE_FSTAT=1
    HAVE_FTRUNCATE=1 HAVE_LSEEK=1 HAVE_OPEN=1 HAVE_READ=1 HAVE_WRITE=1
    HAVE_STAT=1 HAVE_UNLINK=1 HAVE_CHDIR=1 HAVE_GETCWD=1 HAVE_MKDIR=1
    HAVE_FOPEN=1 HAVE_FCLOSE=1 HAVE_FEOF=1 HAVE_FREAD=1 HAVE_FWRITE=1
    HAVE_FGETS=1 HAVE_FGETWS=1 HAVE_VFPRINTF=1 HAVE_MULTI_THREAD_SUPPORT=1)
foreach(library IN LISTS VC_LIBYAL_LIBRARIES)
    string(TOUPPER "${library}" upper)
    list(APPEND VC_LIBYAL_LOCAL_DEFINITIONS "HAVE_LOCAL_${upper}=1")
endforeach()
list(APPEND VC_LIBYAL_LOCAL_DEFINITIONS HAVE_LIBBFIO=1 HAVE_MULTI_THREAD_SUPPORT=1)

function(vc_libyal_add_library library)
    file(GLOB sources CONFIGURE_DEPENDS "${VC_LIBYAL_ROOT}/${library}/${library}/*.c")
    if(NOT sources)
        message(FATAL_ERROR "Pinned ${library} sources are missing")
    endif()
    add_library("vc_${library}" STATIC ${sources})
    # Several libyal projects have same-named common.h/types.h files. The
    # owning project's generated headers must be searched first; dependencies
    # are public headers and are deliberately appended afterwards.
    target_include_directories("vc_${library}" PUBLIC
        "${VC_LIBYAL_GENERATED}/${library}"
        "${VC_LIBYAL_ROOT}/${library}/common"
        "${VC_LIBYAL_ROOT}/${library}/${library}"
        ${VC_LIBYAL_PUBLIC_INCLUDE_DIRECTORIES})
    target_compile_definitions("vc_${library}" PRIVATE ${VC_LIBYAL_LOCAL_DEFINITIONS})
    target_compile_options("vc_${library}" PRIVATE -Wno-unused-function -Wno-unused-parameter)
endfunction()

foreach(library IN LISTS VC_LIBYAL_LIBRARIES)
    vc_libyal_add_library("${library}")
endforeach()

# Preserve CMake's transitive static-library ordering for the complete closure.
set(VC_LIBYAL_DEPENDENCIES)
foreach(library IN LISTS VC_LIBYAL_LIBRARIES)
    if(NOT library STREQUAL "libfsntfs")
        list(APPEND VC_LIBYAL_DEPENDENCIES "vc_${library}")
    endif()
endforeach()
target_link_libraries(vc_libfsntfs PUBLIC ${VC_LIBYAL_DEPENDENCIES})

add_library(vc_libyal_ntfs INTERFACE)
target_link_libraries(vc_libyal_ntfs INTERFACE vc_libfsntfs)
target_include_directories(vc_libyal_ntfs INTERFACE
    "${VC_LIBYAL_GENERATED}/libfsntfs"
    "${VC_LIBYAL_GENERATED}/libbfio")
target_compile_definitions(vc_libyal_ntfs INTERFACE VC_CORE_HAS_LIBFSNTFS=1 ${VC_LIBYAL_LOCAL_DEFINITIONS})
