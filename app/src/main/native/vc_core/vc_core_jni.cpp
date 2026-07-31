#include <jni.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

#include "fd_random_access.h"
#include "vc_botan.h"
#include "vc_error.h"
#include "vc_cpu_topology.h"
#include "vc_request.h"
#include "vc_session_registry.h"
#include "vc_volume.h"

#if defined(VC_CORE_ENABLE_SELF_TESTS)
extern "C" void VcCoreHeaderLayoutSelfTest();
extern "C" void VcCoreFatFsResultSelfTest();
extern "C" void VcCoreKdfParameterSelfTest();
extern "C" void VcCoreNtfsCallbackSelfTest();
extern "C" void VcCoreXtsDataUnitSelfTest();
extern "C" void VcCoreParallelXtsSelfTest();
#endif

namespace {

void ThrowIllegalArgument(JNIEnv* env, const char* message);

class ScopedByteWipe final {
public:
    explicit ScopedByteWipe(std::vector<std::uint8_t>& value) : value_(value) {}
    ~ScopedByteWipe() {
        volatile std::uint8_t* pointer = value_.empty() ? nullptr : value_.data();
        for (std::size_t index = 0; pointer != nullptr && index < value_.size(); ++index) pointer[index] = 0;
    }

private:
    std::vector<std::uint8_t>& value_;
};

void ThrowCoreFailure(JNIEnv* env, jint code, const char* message) {
    jclass exception = env->FindClass("org/eds/veracrypt/nativecore/VcCoreFailure");
    if (exception == nullptr) return;
    const jmethodID constructor = env->GetMethodID(exception, "<init>", "(ILjava/lang/String;)V");
    if (constructor == nullptr) return;
    jstring detail = env->NewStringUTF(message);
    if (detail == nullptr) return;
    jobject failure = env->NewObject(exception, constructor, code, detail);
    env->DeleteLocalRef(detail);
    if (failure != nullptr) {
        env->Throw(reinterpret_cast<jthrowable>(failure));
        env->DeleteLocalRef(failure);
    }
}

void ThrowNativeException(JNIEnv* env, const std::exception& error, jint fallback = 10) {
    if (const auto* core_error = dynamic_cast<const vc_core::CoreException*>(&error); core_error != nullptr) {
        ThrowCoreFailure(env, static_cast<jint>(core_error->error()), core_error->what());
        return;
    }
    ThrowCoreFailure(env, fallback, fallback == 1
            ? "Invalid VeraCrypt credentials or container format"
            : "VeraCrypt container I/O was interrupted");
}

vc_core::CreateProgressCallback CreateProgressCallback(JNIEnv* env, jobject progress) {
    if (progress == nullptr) throw std::invalid_argument("Create progress callback is required");
    jclass type = env->GetObjectClass(progress);
    if (type == nullptr) throw std::runtime_error("Could not inspect create progress callback");
    const jmethodID method = env->GetMethodID(type, "onProgress", "(IJJ)Z");
    env->DeleteLocalRef(type);
    if (method == nullptr) {
        env->ExceptionClear();
        throw std::invalid_argument("Create progress callback has an invalid signature");
    }
    return [env, progress, method](std::uint32_t stage, std::uint64_t completed, std::uint64_t total) {
        const jboolean keep_going = env->CallBooleanMethod(
                progress, method, static_cast<jint>(stage), static_cast<jlong>(completed), static_cast<jlong>(total));
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            return false;
        }
        return keep_going == JNI_TRUE;
    };
}

vc_core::OpenProgressCallback MakeOpenProgressCallback(JNIEnv* env, jobject progress) {
    if (progress == nullptr) throw std::invalid_argument("Open progress callback is required");
    jclass type = env->GetObjectClass(progress);
    if (type == nullptr) throw std::runtime_error("Could not inspect open progress callback");
    const jmethodID method = env->GetMethodID(type, "onProgress", "(II)Z");
    env->DeleteLocalRef(type);
    if (method == nullptr) {
        env->ExceptionClear();
        throw std::invalid_argument("Open progress callback has an invalid signature");
    }
    return [env, progress, method](std::uint32_t completed, std::uint32_t total) {
        const jboolean keep_going = env->CallBooleanMethod(
                progress, method, static_cast<jint>(completed), static_cast<jint>(total));
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            return false;
        }
        return keep_going == JNI_TRUE;
    };
}

bool ValidateArrayRange(JNIEnv* env, jbyteArray value, jint offset, jint length) {
    if (value == nullptr || offset < 0 || length < 0 || offset > env->GetArrayLength(value) - length) {
        ThrowIllegalArgument(env, "Invalid byte array range");
        return false;
    }
    return true;
}

bool ReadRelativePath(JNIEnv* env, jstring value, std::string& result) {
    if (value == nullptr) {
        ThrowIllegalArgument(env, "Internal filesystem path is required");
        return false;
    }
    const jsize length = env->GetStringLength(value);
    const jchar* chars = env->GetStringChars(value, nullptr);
    if (chars == nullptr) return false;
    bool valid = true;
    std::string utf8;
    for (jsize index = 0; index < length; ++index) {
        const std::uint32_t unit = chars[index];
        if (unit == 0) {
            valid = false;
            break;
        }
        std::uint32_t code_point = unit;
        if (unit >= 0xD800 && unit <= 0xDBFF) {
            if (index + 1 >= length || chars[index + 1] < 0xDC00 || chars[index + 1] > 0xDFFF) {
                valid = false;
                break;
            }
            code_point = 0x10000 + ((unit - 0xD800) << 10) + (chars[++index] - 0xDC00);
        } else if (unit >= 0xDC00 && unit <= 0xDFFF) {
            valid = false;
            break;
        }
        if (code_point <= 0x7F) utf8.push_back(static_cast<char>(code_point));
        else if (code_point <= 0x7FF) {
            utf8.push_back(static_cast<char>(0xC0 | (code_point >> 6)));
            utf8.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
        } else if (code_point <= 0xFFFF) {
            utf8.push_back(static_cast<char>(0xE0 | (code_point >> 12)));
            utf8.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
            utf8.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
        } else {
            utf8.push_back(static_cast<char>(0xF0 | (code_point >> 18)));
            utf8.push_back(static_cast<char>(0x80 | ((code_point >> 12) & 0x3F)));
            utf8.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
            utf8.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
        }
    }
    env->ReleaseStringChars(value, chars);
    if (!valid) {
        ThrowIllegalArgument(env, "Internal filesystem path is not valid Unicode");
        return false;
    }
    result = std::move(utf8);
    return true;
}

jstring NewUtf8String(JNIEnv* env, const std::string& value) {
    std::u16string utf16;
    for (std::size_t index = 0; index < value.size();) {
        const std::uint8_t first = static_cast<std::uint8_t>(value[index++]);
        std::uint32_t code_point = 0;
        std::size_t continuation_count = 0;
        if (first <= 0x7F) code_point = first;
        else if (first >= 0xC2 && first <= 0xDF) {
            code_point = first & 0x1F;
            continuation_count = 1;
        } else if (first >= 0xE0 && first <= 0xEF) {
            code_point = first & 0x0F;
            continuation_count = 2;
        } else if (first >= 0xF0 && first <= 0xF4) {
            code_point = first & 0x07;
            continuation_count = 3;
        } else {
            ThrowIllegalArgument(env, "Native filesystem name is not valid UTF-8");
            return nullptr;
        }
        if (index + continuation_count > value.size()) {
            ThrowIllegalArgument(env, "Native filesystem name is not valid UTF-8");
            return nullptr;
        }
        for (std::size_t part = 0; part < continuation_count; ++part) {
            const std::uint8_t continuation = static_cast<std::uint8_t>(value[index++]);
            if ((continuation & 0xC0) != 0x80) {
                ThrowIllegalArgument(env, "Native filesystem name is not valid UTF-8");
                return nullptr;
            }
            code_point = (code_point << 6) | (continuation & 0x3F);
        }
        const std::uint32_t minimum = continuation_count == 1 ? 0x80 : continuation_count == 2 ? 0x800 : continuation_count == 3 ? 0x10000 : 0;
        if (code_point < minimum || code_point > 0x10FFFF || (code_point >= 0xD800 && code_point <= 0xDFFF)) {
            ThrowIllegalArgument(env, "Native filesystem name is not valid UTF-8");
            return nullptr;
        }
        if (code_point <= 0xFFFF) utf16.push_back(static_cast<char16_t>(code_point));
        else {
            const std::uint32_t adjusted = code_point - 0x10000;
            utf16.push_back(static_cast<char16_t>(0xD800 + (adjusted >> 10)));
            utf16.push_back(static_cast<char16_t>(0xDC00 + (adjusted & 0x3FF)));
        }
    }
    return env->NewString(reinterpret_cast<const jchar*>(utf16.data()), static_cast<jsize>(utf16.size()));
}

jobject NewFileEntry(JNIEnv* env, const vc_core::FileSystemEntry& entry) {
    jclass type = env->FindClass("org/eds/veracrypt/nativecore/NativeFileEntry");
    if (type == nullptr) return nullptr;
    const jmethodID constructor = env->GetMethodID(type, "<init>", "(Ljava/lang/String;ZJII)V");
    if (constructor == nullptr) return nullptr;
    jstring name = NewUtf8String(env, entry.name);
    if (name == nullptr) return nullptr;
    jobject result = env->NewObject(type, constructor, name, entry.directory ? JNI_TRUE : JNI_FALSE,
                                    static_cast<jlong>(entry.size), static_cast<jint>(entry.modified_date),
                                    static_cast<jint>(entry.modified_time));
    env->DeleteLocalRef(name);
    return result;
}

void ThrowIllegalArgument(JNIEnv* env, const char* message) {
    jclass exception = env->FindClass("java/lang/IllegalArgumentException");
    if (exception != nullptr) env->ThrowNew(exception, message);
}

bool ValidateOpenRequest(JNIEnv* env, jint container_fd, jboolean writable, jbyteArray request, jintArray keyfile_fds) {
    if (request == nullptr || keyfile_fds == nullptr) {
        ThrowIllegalArgument(env, "Native request and keyfile descriptor list are required");
        return false;
    }
    if (container_fd < 0) {
        ThrowIllegalArgument(env, "Container descriptor is invalid");
        return false;
    }
    try {
        // The duplicate is closed at the end of validation. A future volume
        // session takes its own duplicate after successful header opening.
        const auto container = vc_core::FdRandomAccess::Open(container_fd, writable == JNI_TRUE);
        (void) container.size();
    } catch (const std::exception& error) {
        ThrowIllegalArgument(env, error.what());
        return false;
    }
    const jsize request_size = env->GetArrayLength(request);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(request_size));
    const ScopedByteWipe wipe_bytes(bytes);
    if (request_size > 0) env->GetByteArrayRegion(request, 0, request_size, reinterpret_cast<jbyte*>(bytes.data()));
    try {
        const vc_core::OpenRequest parsed = vc_core::ParseOpenRequest(bytes.data(), bytes.size());
        if ((parsed.writable ? JNI_TRUE : JNI_FALSE) != writable) {
            ThrowIllegalArgument(env, "Container access mode does not match request");
            return false;
        }
        const jsize descriptor_count = env->GetArrayLength(keyfile_fds);
        const std::size_t expected_count = static_cast<std::size_t>(parsed.keyfile_count) +
                                           static_cast<std::size_t>(parsed.protection_keyfile_count);
        if (expected_count != static_cast<std::size_t>(descriptor_count)) {
            ThrowIllegalArgument(env, "Keyfile descriptor count does not match request");
            return false;
        }
        std::vector<jint> descriptors(static_cast<std::size_t>(descriptor_count));
        if (descriptor_count > 0) env->GetIntArrayRegion(keyfile_fds, 0, descriptor_count, descriptors.data());
        for (const jint descriptor : descriptors) {
            if (descriptor < 0) {
                ThrowIllegalArgument(env, "A keyfile descriptor is invalid");
                return false;
            }
        }
    } catch (const std::exception& error) {
        ThrowIllegalArgument(env, error.what());
        return false;
    }
    return true;
}

}  // namespace

extern "C" JNIEXPORT void JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeRunSelfTests(JNIEnv* env, jobject) {
#if defined(VC_CORE_ENABLE_SELF_TESTS)
    try {
        VcCoreHeaderLayoutSelfTest();
        VcCoreFatFsResultSelfTest();
        VcCoreKdfParameterSelfTest();
        VcCoreNtfsCallbackSelfTest();
        VcCoreXtsDataUnitSelfTest();
        VcCoreParallelXtsSelfTest();
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
    }
#else
    ThrowCoreFailure(env, 10, "Native self-tests are available only in Debug builds");
#endif
}

extern "C" JNIEXPORT jint JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeRecommendedWorkerCount(JNIEnv*, jobject) {
    return static_cast<jint>(std::max<std::size_t>(1, vc_core::DetectCpuTopology().worker_count));
}

extern "C" JNIEXPORT jstring JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeCpuTopologySummary(JNIEnv* env, jobject) {
    const auto topology = vc_core::DetectCpuTopology();
    std::ostringstream summary;
    summary << "allowed=";
    for (std::size_t index = 0; index < topology.allowed_cpus.size(); ++index) {
        if (index != 0) summary << ',';
        summary << topology.allowed_cpus[index];
    }
    summary << " performance=";
    for (std::size_t index = 0; index < topology.performance_cpus.size(); ++index) {
        if (index != 0) summary << ',';
        summary << topology.performance_cpus[index];
    }
    summary << " workers=" << topology.worker_count
            << " frequency_data=" << (topology.frequency_data_available ? "yes" : "no");
    if (!topology.degradation_reason.empty()) summary << " degradation=" << topology.degradation_reason;
    return env->NewStringUTF(summary.str().c_str());
}

// This entry point is deliberately omitted, not merely disabled, from release
// binaries. Kotlin only calls it from debug instrumentation or benchmark code.
#if defined(VC_CORE_ENABLE_SELF_TESTS) || defined(VC_CORE_ENABLE_BENCHMARK_STATS)
extern "C" JNIEXPORT jlongArray JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeGetPerformanceCounters(JNIEnv* env, jobject, jlong handle) {
    try {
        const auto counters = vc_core::NativeSessionRegistry::Instance().GetPerformanceCounters(static_cast<std::uint64_t>(handle));
        const jlong values[] = {
                static_cast<jlong>(counters.read_syscalls), static_cast<jlong>(counters.write_syscalls),
                static_cast<jlong>(counters.read_bytes), static_cast<jlong>(counters.write_bytes),
                static_cast<jlong>(counters.batched_reads), static_cast<jlong>(counters.batched_writes),
                static_cast<jlong>(counters.rmw_sectors), static_cast<jlong>(counters.xts_data_units),
                static_cast<jlong>(counters.cache_hits), static_cast<jlong>(counters.cache_misses),
        };
        jlongArray result = env->NewLongArray(static_cast<jsize>(std::size(values)));
        if (result != nullptr) env->SetLongArrayRegion(result, 0, static_cast<jsize>(std::size(values)), values);
        return result;
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
        return nullptr;
    }
}
#endif

#if defined(VC_CORE_ENABLE_BENCHMARK_STATS)
extern "C" JNIEXPORT jstring JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeGetBotanHardwareProviders(JNIEnv* env, jobject) {
    try {
        auto aes = Botan::BlockCipher::create("AES-256");
        auto sha512 = Botan::HashFunction::create("SHA-512");
        if (!aes || !sha512) throw std::runtime_error("Required Botan provider is unavailable");
        const std::string result = "aes=" + aes->provider() + ";sha512=" + sha512->provider();
        return env->NewStringUTF(result.c_str());
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
        return nullptr;
    }
}
#endif

extern "C" JNIEXPORT jlong JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeOpenWithProgress(
        JNIEnv* env, jobject, jint container_fd, jboolean writable, jbyteArray request, jintArray keyfile_fds, jobject progress) {
    // Parse before selecting the cryptographic backend so malformed credentials
    // or a mismatched descriptor list never reach a volume implementation.
    if (progress == nullptr || !ValidateOpenRequest(env, container_fd, writable, request, keyfile_fds)) return 0;
    const jsize request_size = env->GetArrayLength(request);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(request_size));
    const ScopedByteWipe wipe_bytes(bytes);
    if (request_size > 0) env->GetByteArrayRegion(request, 0, request_size, reinterpret_cast<jbyte*>(bytes.data()));
    try {
        vc_core::OpenRequest parsed = vc_core::ParseOpenRequest(bytes.data(), bytes.size());
        const jsize descriptor_count = env->GetArrayLength(keyfile_fds);
        std::vector<jint> descriptors(static_cast<std::size_t>(descriptor_count));
        if (descriptor_count > 0) env->GetIntArrayRegion(keyfile_fds, 0, descriptor_count, descriptors.data());
        std::vector<vc_core::FdRandomAccess> keyfiles;
        std::vector<vc_core::FdRandomAccess> protection_keyfiles;
        keyfiles.reserve(parsed.keyfile_count);
        protection_keyfiles.reserve(parsed.protection_keyfile_count);
        for (std::size_t index = 0; index < descriptors.size(); ++index) {
            auto opened_keyfile = vc_core::FdRandomAccess::Open(descriptors[index], false);
            if (index < parsed.keyfile_count) keyfiles.push_back(std::move(opened_keyfile));
            else protection_keyfiles.push_back(std::move(opened_keyfile));
        }
        auto container = vc_core::FdRandomAccess::Open(container_fd, writable == JNI_TRUE);
        auto opened = vc_core::OpenVeraCryptVolume(container, parsed, keyfiles, MakeOpenProgressCallback(env, progress));
        std::optional<vc_core::EncryptedRange> protected_hidden_range;
        if (parsed.protect_hidden_volume && !opened.hidden_volume) {
            std::vector<std::uint8_t> protection_password(parsed.protection_password.data(),
                                                           parsed.protection_password.data() + parsed.protection_password.size());
            vc_core::OpenRequest protection_request;
            protection_request.open_hidden_volume = true;
            protection_request.pim = parsed.protection_pim;
            protection_request.password = vc_core::SecureBytes(std::move(protection_password));
            const auto protected_hidden = vc_core::OpenVeraCryptVolume(container, protection_request, protection_keyfiles);
            protected_hidden_range = {protected_hidden.metadata.encrypted_area_offset, protected_hidden.metadata.volume_data_size};
        }
        return static_cast<jlong>(vc_core::NativeSessionRegistry::Instance().Insert(
                std::move(container), std::move(opened), protected_hidden_range));
    } catch (const std::bad_alloc&) {
        ThrowCoreFailure(env, 5, "Insufficient memory for VeraCrypt operation");
    } catch (const std::exception& error) {
        ThrowNativeException(env, error, 1);
    }
    return 0;
}

extern "C" JNIEXPORT jlong JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeCreateNormal(
        JNIEnv* env, jobject, jint container_fd, jbyteArray request, jintArray keyfile_fds, jobject progress) {
    if (container_fd < 0 || request == nullptr || keyfile_fds == nullptr || progress == nullptr) {
        ThrowIllegalArgument(env, "A writable container, create request, and keyfile descriptor list are required");
        return 0;
    }
    const jsize request_size = env->GetArrayLength(request);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(request_size));
    const ScopedByteWipe wipe_bytes(bytes);
    if (request_size > 0) env->GetByteArrayRegion(request, 0, request_size, reinterpret_cast<jbyte*>(bytes.data()));
    try {
        vc_core::CreateRequest parsed = vc_core::ParseCreateRequest(bytes.data(), bytes.size());
        if (parsed.hidden_volume) throw std::invalid_argument("Hidden volumes require two-stage creation");
        const jsize descriptor_count = env->GetArrayLength(keyfile_fds);
        if (static_cast<std::size_t>(descriptor_count) != parsed.keyfile_count) {
            throw std::invalid_argument("Create keyfile descriptor count does not match request");
        }
        std::vector<jint> descriptors(static_cast<std::size_t>(descriptor_count));
        if (descriptor_count > 0) env->GetIntArrayRegion(keyfile_fds, 0, descriptor_count, descriptors.data());
        std::vector<vc_core::FdRandomAccess> keyfiles;
        keyfiles.reserve(descriptors.size());
        for (const jint descriptor : descriptors) keyfiles.push_back(vc_core::FdRandomAccess::Open(descriptor, false));
        auto container = vc_core::FdRandomAccess::Open(container_fd, true);
        const auto progress_callback = CreateProgressCallback(env, progress);
        return static_cast<jlong>(vc_core::NativeSessionRegistry::Instance().CreateNormal(
                std::move(container), parsed, keyfiles, progress_callback));
    } catch (const std::bad_alloc&) {
        ThrowCoreFailure(env, 5, "Insufficient memory for VeraCrypt operation");
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
    }
    return 0;
}

extern "C" JNIEXPORT jlong JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeCreateHidden(
        JNIEnv* env, jobject, jlong outer_handle, jbyteArray request, jintArray keyfile_fds, jobject progress) {
    if (outer_handle <= 0 || request == nullptr || keyfile_fds == nullptr || progress == nullptr) {
        ThrowIllegalArgument(env, "A live outer session, create request, and keyfile descriptor list are required");
        return 0;
    }
    const jsize request_size = env->GetArrayLength(request);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(request_size));
    const ScopedByteWipe wipe_bytes(bytes);
    if (request_size > 0) env->GetByteArrayRegion(request, 0, request_size, reinterpret_cast<jbyte*>(bytes.data()));
    try {
        vc_core::CreateRequest parsed = vc_core::ParseCreateRequest(bytes.data(), bytes.size());
        if (!parsed.hidden_volume) throw std::invalid_argument("Hidden create request is required");
        const jsize descriptor_count = env->GetArrayLength(keyfile_fds);
        if (static_cast<std::size_t>(descriptor_count) != parsed.keyfile_count) {
            throw std::invalid_argument("Create keyfile descriptor count does not match request");
        }
        std::vector<jint> descriptors(static_cast<std::size_t>(descriptor_count));
        if (descriptor_count > 0) env->GetIntArrayRegion(keyfile_fds, 0, descriptor_count, descriptors.data());
        std::vector<vc_core::FdRandomAccess> keyfiles;
        keyfiles.reserve(descriptors.size());
        for (const jint descriptor : descriptors) keyfiles.push_back(vc_core::FdRandomAccess::Open(descriptor, false));
        const auto progress_callback = CreateProgressCallback(env, progress);
        return static_cast<jlong>(vc_core::NativeSessionRegistry::Instance().CreateHidden(
                static_cast<std::uint64_t>(outer_handle), parsed, keyfiles, progress_callback));
    } catch (const std::bad_alloc&) {
        ThrowCoreFailure(env, 5, "Insufficient memory for VeraCrypt operation");
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
    }
    return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeClose(JNIEnv*, jobject, jlong handle) {
    if (handle > 0) vc_core::NativeSessionRegistry::Instance().Close(static_cast<std::uint64_t>(handle));
}

extern "C" JNIEXPORT void JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeFlush(JNIEnv* env, jobject, jlong handle) {
    if (handle <= 0) {
        ThrowIllegalArgument(env, "A live session is required");
        return;
    }
    try {
        vc_core::NativeSessionRegistry::Instance().Flush(static_cast<std::uint64_t>(handle));
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeMountFileSystem(JNIEnv* env, jobject, jlong handle) {
    if (handle <= 0) {
        ThrowIllegalArgument(env, "A live session is required");
        return 0;
    }
    try {
        return static_cast<jint>(vc_core::NativeSessionRegistry::Instance().MountFileSystem(
                static_cast<std::uint64_t>(handle)));
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
        return 0;
    }
}

extern "C" JNIEXPORT jobjectArray JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeListDirectory(
        JNIEnv* env, jobject, jlong handle, jstring relative_path) {
    std::string path;
    if (handle <= 0 || !ReadRelativePath(env, relative_path, path)) return nullptr;
    try {
        const auto entries = vc_core::NativeSessionRegistry::Instance().ListDirectory(
                static_cast<std::uint64_t>(handle), path);
        jclass type = env->FindClass("org/eds/veracrypt/nativecore/NativeFileEntry");
        if (type == nullptr) return nullptr;
        jobjectArray result = env->NewObjectArray(static_cast<jsize>(entries.size()), type, nullptr);
        if (result == nullptr) return nullptr;
        for (std::size_t index = 0; index < entries.size(); ++index) {
            jobject entry = NewFileEntry(env, entries[index]);
            if (entry == nullptr) return nullptr;
            env->SetObjectArrayElement(result, static_cast<jsize>(index), entry);
            env->DeleteLocalRef(entry);
            if (env->ExceptionCheck()) return nullptr;
        }
        return result;
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
        return nullptr;
    }
}

extern "C" JNIEXPORT jobject JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeStat(
        JNIEnv* env, jobject, jlong handle, jstring relative_path) {
    std::string path;
    if (handle <= 0 || !ReadRelativePath(env, relative_path, path)) return nullptr;
    try {
        return NewFileEntry(env, vc_core::NativeSessionRegistry::Instance().Stat(
                static_cast<std::uint64_t>(handle), path));
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
        return nullptr;
    }
}

extern "C" JNIEXPORT jlong JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeOpenFile(
        JNIEnv* env, jobject, jlong handle, jstring relative_path, jboolean writable, jboolean create, jboolean truncate) {
    std::string path;
    if (handle <= 0 || !ReadRelativePath(env, relative_path, path)) return 0;
    if (truncate == JNI_TRUE && writable != JNI_TRUE) {
        ThrowIllegalArgument(env, "Truncating a file requires writable access");
        return 0;
    }
    try {
        return static_cast<jlong>(vc_core::NativeSessionRegistry::Instance().OpenFile(
                static_cast<std::uint64_t>(handle), path, writable == JNI_TRUE, create == JNI_TRUE, truncate == JNI_TRUE));
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
        return 0;
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeReadFile(
        JNIEnv* env, jobject, jlong handle, jlong offset, jbyteArray target, jint target_offset, jint length) {
    if (handle <= 0 || offset < 0 || !ValidateArrayRange(env, target, target_offset, length)) return -1;
    jbyte* bytes = env->GetByteArrayElements(target, nullptr);
    if (bytes == nullptr) return -1;
    try {
        const std::size_t read = vc_core::NativeSessionRegistry::Instance().ReadFile(
                static_cast<std::uint64_t>(handle), static_cast<std::uint64_t>(offset),
                reinterpret_cast<std::uint8_t*>(bytes + target_offset), static_cast<std::size_t>(length));
        env->ReleaseByteArrayElements(target, bytes, 0);
        return static_cast<jint>(read);
    } catch (const std::exception& error) {
        env->ReleaseByteArrayElements(target, bytes, JNI_ABORT);
        ThrowNativeException(env, error);
        return -1;
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeWriteFile(
        JNIEnv* env, jobject, jlong handle, jlong offset, jbyteArray source, jint source_offset, jint length) {
    if (handle <= 0 || offset < 0 || !ValidateArrayRange(env, source, source_offset, length)) return -1;
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
    const ScopedByteWipe wipe_bytes(bytes);
    if (length > 0) env->GetByteArrayRegion(source, source_offset, length, reinterpret_cast<jbyte*>(bytes.data()));
    try {
        const std::size_t written = vc_core::NativeSessionRegistry::Instance().WriteFile(
                static_cast<std::uint64_t>(handle), static_cast<std::uint64_t>(offset), bytes.data(), bytes.size());
        return static_cast<jint>(written);
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
        return -1;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeTruncateFile(JNIEnv* env, jobject, jlong handle, jlong length) {
    if (handle <= 0 || length < 0) {
        ThrowIllegalArgument(env, "A live file handle and non-negative length are required");
        return;
    }
    try {
        vc_core::NativeSessionRegistry::Instance().TruncateFile(static_cast<std::uint64_t>(handle), static_cast<std::uint64_t>(length));
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativePreallocateFile(JNIEnv* env, jobject, jlong handle, jlong length) {
    if (handle <= 0 || length < 0) {
        ThrowIllegalArgument(env, "A live file handle and non-negative length are required");
        return;
    }
    try {
        vc_core::NativeSessionRegistry::Instance().PreallocateFile(
                static_cast<std::uint64_t>(handle), static_cast<std::uint64_t>(length));
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeFlushFile(JNIEnv* env, jobject, jlong handle) {
    if (handle <= 0) {
        ThrowIllegalArgument(env, "A live file handle is required");
        return;
    }
    try {
        vc_core::NativeSessionRegistry::Instance().FlushFile(static_cast<std::uint64_t>(handle));
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeCloseFile(JNIEnv*, jobject, jlong handle) {
    if (handle > 0) vc_core::NativeSessionRegistry::Instance().CloseFile(static_cast<std::uint64_t>(handle));
}

extern "C" JNIEXPORT void JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeCreateDirectory(
        JNIEnv* env, jobject, jlong handle, jstring relative_path) {
    std::string path;
    if (handle <= 0 || !ReadRelativePath(env, relative_path, path)) return;
    try {
        vc_core::NativeSessionRegistry::Instance().CreateDirectory(static_cast<std::uint64_t>(handle), path);
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeDelete(
        JNIEnv* env, jobject, jlong handle, jstring relative_path) {
    std::string path;
    if (handle <= 0 || !ReadRelativePath(env, relative_path, path)) return;
    try {
        vc_core::NativeSessionRegistry::Instance().Delete(static_cast<std::uint64_t>(handle), path);
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeRename(
        JNIEnv* env, jobject, jlong handle, jstring from_relative_path, jstring to_relative_path) {
    std::string from;
    std::string to;
    if (handle <= 0 || !ReadRelativePath(env, from_relative_path, from) || !ReadRelativePath(env, to_relative_path, to)) return;
    try {
        vc_core::NativeSessionRegistry::Instance().Rename(static_cast<std::uint64_t>(handle), from, to);
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
    }
}

extern "C" JNIEXPORT jlongArray JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeGetVolumeInfo(JNIEnv* env, jobject, jlong handle) {
    if (handle <= 0) {
        ThrowIllegalArgument(env, "A live session is required");
        return nullptr;
    }
    try {
        const vc_core::VolumeInfo info = vc_core::NativeSessionRegistry::Instance().GetInfo(
                static_cast<std::uint64_t>(handle));
        // Keep this fixed-width format versionless by returning only primitive
        // values. Kotlin validates its exact shape before exposing it.
        const jlong values[] = {
                static_cast<jlong>(info.logical_size),
                static_cast<jlong>(info.encrypted_area_offset),
                static_cast<jlong>(info.encrypted_area_size),
                static_cast<jlong>(info.sector_size),
                static_cast<jlong>(info.cipher),
                static_cast<jlong>(info.kdf),
                info.hidden_volume ? 1 : 0,
                info.used_backup_header ? 1 : 0,
        };
        jlongArray result = env->NewLongArray(static_cast<jsize>(std::size(values)));
        if (result != nullptr) env->SetLongArrayRegion(result, 0, static_cast<jsize>(std::size(values)), values);
        return result;
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
        return nullptr;
    }
}

extern "C" JNIEXPORT jlongArray JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeAnalyzeHiddenCapacity(JNIEnv* env, jobject, jlong handle) {
    if (handle <= 0) {
        ThrowIllegalArgument(env, "A live outer-volume session is required");
        return nullptr;
    }
    try {
        const vc_core::HiddenVolumeCapacity capacity =
                vc_core::NativeSessionRegistry::Instance().AnalyzeHiddenVolumeCapacity(static_cast<std::uint64_t>(handle));
        const jlong values[] = {
                static_cast<jlong>(capacity.maximum_size),
                static_cast<jlong>(capacity.encrypted_area_offset),
        };
        jlongArray result = env->NewLongArray(static_cast<jsize>(std::size(values)));
        if (result != nullptr) env->SetLongArrayRegion(result, 0, static_cast<jsize>(std::size(values)), values);
        return result;
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
        return nullptr;
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeRead(
        JNIEnv* env, jobject, jlong handle, jlong offset, jbyteArray target, jint target_offset, jint length) {
    if (handle <= 0 || offset < 0 || !ValidateArrayRange(env, target, target_offset, length)) return -1;
    jbyte* bytes = env->GetByteArrayElements(target, nullptr);
    if (bytes == nullptr) return -1;
    try {
        const std::size_t read = vc_core::NativeSessionRegistry::Instance().Read(
                static_cast<std::uint64_t>(handle), static_cast<std::uint64_t>(offset),
                reinterpret_cast<std::uint8_t*>(bytes + target_offset), static_cast<std::size_t>(length));
        env->ReleaseByteArrayElements(target, bytes, 0);
        return static_cast<jint>(read);
    } catch (const std::exception& error) {
        env->ReleaseByteArrayElements(target, bytes, JNI_ABORT);
        ThrowNativeException(env, error);
    }
    return -1;
}

extern "C" JNIEXPORT jint JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeWrite(
        JNIEnv* env, jobject, jlong handle, jlong offset, jbyteArray source, jint source_offset, jint length) {
    if (handle <= 0 || offset < 0 || !ValidateArrayRange(env, source, source_offset, length)) return -1;
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
    const ScopedByteWipe wipe_bytes(bytes);
    if (length > 0) env->GetByteArrayRegion(source, source_offset, length, reinterpret_cast<jbyte*>(bytes.data()));
    try {
        const std::size_t written = vc_core::NativeSessionRegistry::Instance().Write(
                static_cast<std::uint64_t>(handle), static_cast<std::uint64_t>(offset), bytes.data(), bytes.size());
        return static_cast<jint>(written);
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
    }
    return -1;
}

#if defined(VC_CORE_ENABLE_BENCHMARK_STATS)
extern "C" JNIEXPORT jint JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeReadUncached(
        JNIEnv* env, jobject, jlong handle, jlong offset, jbyteArray target, jint target_offset, jint length) {
    if (handle <= 0 || offset < 0 || !ValidateArrayRange(env, target, target_offset, length)) return -1;
    std::vector<std::uint8_t> buffer(static_cast<std::size_t>(length));
    try {
        const auto read = vc_core::NativeSessionRegistry::Instance().ReadUncachedForBenchmark(
                static_cast<std::uint64_t>(handle), static_cast<std::uint64_t>(offset), buffer.data(), buffer.size());
        env->SetByteArrayRegion(target, target_offset, static_cast<jsize>(read), reinterpret_cast<const jbyte*>(buffer.data()));
        return static_cast<jint>(read);
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
        return 0;
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeReadLegacy(
        JNIEnv* env, jobject, jlong handle, jlong offset, jbyteArray target, jint target_offset, jint length) {
    if (handle <= 0 || offset < 0 || !ValidateArrayRange(env, target, target_offset, length)) return -1;
    std::vector<std::uint8_t> buffer(static_cast<std::size_t>(length));
    try {
        const auto read = vc_core::NativeSessionRegistry::Instance().ReadLegacySector(
                static_cast<std::uint64_t>(handle), static_cast<std::uint64_t>(offset), buffer.data(), buffer.size());
        env->SetByteArrayRegion(target, target_offset, static_cast<jsize>(read), reinterpret_cast<const jbyte*>(buffer.data()));
        return static_cast<jint>(read);
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
        return 0;
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeWriteLegacy(
        JNIEnv* env, jobject, jlong handle, jlong offset, jbyteArray source, jint source_offset, jint length) {
    if (handle <= 0 || offset < 0 || !ValidateArrayRange(env, source, source_offset, length)) return -1;
    std::vector<std::uint8_t> buffer(static_cast<std::size_t>(length));
    env->GetByteArrayRegion(source, source_offset, length, reinterpret_cast<jbyte*>(buffer.data()));
    try {
        return static_cast<jint>(vc_core::NativeSessionRegistry::Instance().WriteLegacySector(
                static_cast<std::uint64_t>(handle), static_cast<std::uint64_t>(offset), buffer.data(), buffer.size()));
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
        return 0;
    }
}
#endif

extern "C" JNIEXPORT void JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeChangeCredentials(
        JNIEnv* env, jobject, jlong handle, jbyteArray request, jintArray keyfile_fds) {
    if (handle <= 0 || request == nullptr || keyfile_fds == nullptr) {
        ThrowIllegalArgument(env, "A live session, credential request, and keyfile descriptor list are required");
        return;
    }
    const jsize request_size = env->GetArrayLength(request);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(request_size));
    const ScopedByteWipe wipe_bytes(bytes);
    if (request_size > 0) env->GetByteArrayRegion(request, 0, request_size, reinterpret_cast<jbyte*>(bytes.data()));
    try {
        vc_core::OpenRequest parsed = vc_core::ParseOpenRequest(bytes.data(), bytes.size());
        if (!parsed.writable || parsed.protect_hidden_volume) {
            throw std::invalid_argument("Credential updates require a writable non-protection request");
        }
        const jsize descriptor_count = env->GetArrayLength(keyfile_fds);
        if (static_cast<std::size_t>(descriptor_count) != parsed.keyfile_count) {
            throw std::invalid_argument("Credential keyfile descriptor count does not match request");
        }
        std::vector<jint> descriptors(static_cast<std::size_t>(descriptor_count));
        if (descriptor_count > 0) env->GetIntArrayRegion(keyfile_fds, 0, descriptor_count, descriptors.data());
        std::vector<vc_core::FdRandomAccess> keyfiles;
        keyfiles.reserve(descriptors.size());
        for (const jint descriptor : descriptors) keyfiles.push_back(vc_core::FdRandomAccess::Open(descriptor, false));
        vc_core::NativeSessionRegistry::Instance().ChangeCredentials(static_cast<std::uint64_t>(handle), parsed, keyfiles);
    } catch (const std::bad_alloc&) {
        ThrowCoreFailure(env, 5, "Insufficient memory for VeraCrypt operation");
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeBackupHeader(
        JNIEnv* env, jobject, jlong handle, jint output_fd, jbyteArray request, jintArray keyfile_fds) {
    if (handle <= 0 || !ValidateOpenRequest(env, output_fd, JNI_TRUE, request, keyfile_fds)) {
        if (handle <= 0) ThrowIllegalArgument(env, "A live session is required for header backup");
        return;
    }
    const jsize request_size = env->GetArrayLength(request);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(request_size));
    const ScopedByteWipe wipe_bytes(bytes);
    if (request_size > 0) env->GetByteArrayRegion(request, 0, request_size, reinterpret_cast<jbyte*>(bytes.data()));
    try {
        vc_core::OpenRequest parsed = vc_core::ParseOpenRequest(bytes.data(), bytes.size());
        if (!parsed.writable || parsed.protect_hidden_volume) {
            throw std::invalid_argument("Header backup requires a writable non-protection request");
        }
        const jsize descriptor_count = env->GetArrayLength(keyfile_fds);
        if (static_cast<std::size_t>(descriptor_count) != parsed.keyfile_count) {
            throw std::invalid_argument("Header backup keyfile descriptor count does not match request");
        }
        std::vector<jint> descriptors(static_cast<std::size_t>(descriptor_count));
        if (descriptor_count > 0) env->GetIntArrayRegion(keyfile_fds, 0, descriptor_count, descriptors.data());
        std::vector<vc_core::FdRandomAccess> keyfiles;
        keyfiles.reserve(descriptors.size());
        for (const jint descriptor : descriptors) keyfiles.push_back(vc_core::FdRandomAccess::Open(descriptor, false));
        vc_core::NativeSessionRegistry::Instance().BackupHeader(
                static_cast<std::uint64_t>(handle), output_fd, parsed, keyfiles);
    } catch (const std::bad_alloc&) {
        ThrowCoreFailure(env, 5, "Insufficient memory for VeraCrypt operation");
    } catch (const std::exception& error) {
        ThrowNativeException(env, error);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_org_eds_veracrypt_nativecore_VcCore_nativeRestoreHeader(
        JNIEnv* env, jobject, jint container_fd, jint input_fd, jbyteArray request, jintArray keyfile_fds) {
    if (!ValidateOpenRequest(env, container_fd, JNI_TRUE, request, keyfile_fds)) return;
    if (input_fd < 0) {
        ThrowIllegalArgument(env, "Header backup descriptor is invalid");
        return;
    }
    const jsize request_size = env->GetArrayLength(request);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(request_size));
    const ScopedByteWipe wipe_bytes(bytes);
    if (request_size > 0) env->GetByteArrayRegion(request, 0, request_size, reinterpret_cast<jbyte*>(bytes.data()));
    try {
        vc_core::OpenRequest parsed = vc_core::ParseOpenRequest(bytes.data(), bytes.size());
        if (parsed.protect_hidden_volume) throw std::invalid_argument("Hidden-volume protection is not applicable to header restoration");
        const jsize descriptor_count = env->GetArrayLength(keyfile_fds);
        std::vector<jint> descriptors(static_cast<std::size_t>(descriptor_count));
        if (descriptor_count > 0) env->GetIntArrayRegion(keyfile_fds, 0, descriptor_count, descriptors.data());
        std::vector<vc_core::FdRandomAccess> keyfiles;
        keyfiles.reserve(descriptors.size());
        for (const jint descriptor : descriptors) keyfiles.push_back(vc_core::FdRandomAccess::Open(descriptor, false));
        auto container = vc_core::FdRandomAccess::Open(container_fd, true);
        const auto backup = vc_core::FdRandomAccess::Open(input_fd, false);
        vc_core::RestoreVeraCryptHeader(container, backup, parsed, keyfiles);
    } catch (const std::bad_alloc&) {
        ThrowCoreFailure(env, 5, "Insufficient memory for VeraCrypt operation");
    } catch (const std::exception& error) {
        ThrowNativeException(env, error, 1);
    }
}
