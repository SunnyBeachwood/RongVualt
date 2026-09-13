#include "vc_volume.h"

#include <algorithm>
#include <android/log.h>
#include <array>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <condition_variable>
#include <exception>
#include <functional>
#include <mutex>
#include <sched.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "vc_botan.h"
#include "vc_error.h"
#include "vc_cpu_topology.h"
#include "vc_fatfs_volume.h"
#include "vc_ntfs_volume.h"
#include "vc_kdf.h"
#include "vc_keyfiles.h"
#include "vc_random.h"

namespace vc_core {
namespace {

constexpr std::uint64_t kHeaderGroupBytes = 131072;
constexpr std::uint64_t kHiddenHeaderOffset = 65536;
constexpr std::uint64_t kMinimumCreatedDataBytes = 1024 * 1024;
constexpr std::uint64_t kMaximumCreatedDataBytes = 2ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL;
constexpr std::size_t kCipherKeyBytes = 32;
constexpr std::size_t kXtsKeyBytesPerCipher = 64;
constexpr std::size_t kSectorBytes = 512;
constexpr std::size_t kMaximumIoBlockBytes = 256 * 1024;
constexpr std::size_t kParallelXtsMinimumBytes = 128 * 1024;
constexpr std::size_t kCachedReadMaximumBytes = 64 * 1024;
constexpr std::size_t kCachePageBytes = 16 * 1024;
constexpr std::size_t kCachePageCount = 64;
constexpr std::size_t kKeyAreaCrcOffset = 72;
constexpr std::size_t kPaddingOffset = 132;
constexpr std::size_t kHeaderCrcOffset = 252;
constexpr std::size_t kKeyAreaOffset = 256;
constexpr std::size_t kKeyAreaSize = 256;
constexpr std::uint16_t kCreatedHeaderVersion = 5;
// VeraCrypt 1.26.29 advertises VERSION_NUM 0x0126.  A volume header's
// required-program field is a packed VeraCrypt version, not a decimal-looking
// release label; 0x0630 makes current desktop VeraCrypt reject our new volume
// as requiring a future program before it attempts to mount it.
constexpr std::uint16_t kCreatedRequiredProgramVersion = 0x0126;

struct CipherSuite final {
    CipherHint hint;
    std::initializer_list<const char*> ciphers;
};

const CipherSuite& Suite(CipherHint hint) {
    static const std::array<CipherSuite, 15> suites {{
        {CipherHint::kAes, {"AES-256"}},
        {CipherHint::kSerpent, {"Serpent"}},
        {CipherHint::kTwofish, {"Twofish"}},
        {CipherHint::kCamellia, {"Camellia-256"}},
        {CipherHint::kKuznyechik, {"Kuznyechik"}},
        {CipherHint::kTwofishAes, {"Twofish", "AES-256"}},
        {CipherHint::kSerpentTwofishAes, {"Serpent", "Twofish", "AES-256"}},
        {CipherHint::kAesSerpent, {"AES-256", "Serpent"}},
        {CipherHint::kAesTwofishSerpent, {"AES-256", "Twofish", "Serpent"}},
        {CipherHint::kSerpentTwofish, {"Serpent", "Twofish"}},
        {CipherHint::kKuznyechikCamellia, {"Kuznyechik", "Camellia-256"}},
        {CipherHint::kTwofishKuznyechik, {"Twofish", "Kuznyechik"}},
        {CipherHint::kSerpentCamellia, {"Serpent", "Camellia-256"}},
        {CipherHint::kAesKuznyechik, {"AES-256", "Kuznyechik"}},
        {CipherHint::kCamelliaSerpentKuznyechik, {"Camellia-256", "Serpent", "Kuznyechik"}},
    }};
    for (const CipherSuite& suite : suites) if (suite.hint == hint) return suite;
    throw std::invalid_argument("Unknown VeraCrypt cipher suite");
}

std::size_t XtsKeyBytes(CipherHint cipher) {
    return Suite(cipher).ciphers.size() * kXtsKeyBytesPerCipher;
}

std::uint64_t CheckedAdd(std::uint64_t left, std::uint64_t right, const char* message) {
    if (right > std::numeric_limits<std::uint64_t>::max() - left) throw std::overflow_error(message);
    return left + right;
}

std::uint64_t CheckedMul(std::uint64_t left, std::uint64_t right, const char* message) {
    if (left != 0 && right > std::numeric_limits<std::uint64_t>::max() / left) throw std::overflow_error(message);
    return left * right;
}

std::uint32_t Crc32(const std::uint8_t* data, std::size_t size) {
    std::uint32_t crc = 0xffffffffU;
    for (std::size_t index = 0; index < size; ++index) {
        crc ^= data[index];
        for (int bit = 0; bit < 8; ++bit) crc = (crc >> 1) ^ ((crc & 1U) == 0 ? 0U : 0xedb88320U);
    }
    return ~crc;
}

void WriteBigEndianU32(std::uint8_t* destination, std::uint32_t value) {
    destination[0] = static_cast<std::uint8_t>(value >> 24);
    destination[1] = static_cast<std::uint8_t>(value >> 16);
    destination[2] = static_cast<std::uint8_t>(value >> 8);
    destination[3] = static_cast<std::uint8_t>(value);
}

void WriteBigEndianU16(std::uint8_t* destination, std::uint16_t value) {
    destination[0] = static_cast<std::uint8_t>(value >> 8);
    destination[1] = static_cast<std::uint8_t>(value);
}

void WriteBigEndianU64(std::uint8_t* destination, std::uint64_t value) {
    for (std::size_t index = 0; index < 8; ++index) {
        destination[index] = static_cast<std::uint8_t>(value >> ((7 - index) * 8));
    }
}

std::vector<KdfHint> KdfCandidates(KdfHint hint) {
    if (hint != KdfHint::kAuto) return {hint};
    return {
        KdfHint::kPbkdf2HmacSha512,
        KdfHint::kPbkdf2HmacSha256,
        KdfHint::kPbkdf2HmacBlake2s,
        KdfHint::kPbkdf2HmacWhirlpool,
        KdfHint::kPbkdf2HmacStreebog,
        KdfHint::kArgon2id,
    };
}

std::vector<CipherHint> CipherCandidates(CipherHint hint) {
    if (hint == CipherHint::kAuto) {
        return {
            CipherHint::kAes,
            CipherHint::kSerpent,
            CipherHint::kTwofish,
            CipherHint::kCamellia,
            CipherHint::kKuznyechik,
            CipherHint::kTwofishAes,
            CipherHint::kSerpentTwofishAes,
            CipherHint::kAesSerpent,
            CipherHint::kAesTwofishSerpent,
            CipherHint::kSerpentTwofish,
            CipherHint::kKuznyechikCamellia,
            CipherHint::kTwofishKuznyechik,
            CipherHint::kSerpentCamellia,
            CipherHint::kAesKuznyechik,
            CipherHint::kCamelliaSerpentKuznyechik,
        };
    }
    const auto numeric_hint = static_cast<std::uint8_t>(hint);
    if (numeric_hint >= static_cast<std::uint8_t>(CipherHint::kAes) &&
        numeric_hint <= static_cast<std::uint8_t>(CipherHint::kCamelliaSerpentKuznyechik)) return {hint};
    throw CoreException(CoreError::kUnsupportedAlgorithm);
}

bool IsCreatableCipher(CipherHint hint) {
    switch (hint) {
        case CipherHint::kAes:
        case CipherHint::kSerpent:
        case CipherHint::kTwofish:
        case CipherHint::kCamellia:
        case CipherHint::kTwofishAes:
        case CipherHint::kSerpentTwofishAes:
        case CipherHint::kAesSerpent:
        case CipherHint::kAesTwofishSerpent:
        case CipherHint::kSerpentTwofish:
            return true;
        default:
            return false;
    }
}

void VeraCryptXtsTransform(
        CipherHint suite_hint, const std::uint8_t* key, std::size_t key_size, std::uint8_t* data,
        std::size_t size, std::uint64_t data_unit, bool encrypt) {
    const CipherSuite& suite = Suite(suite_hint);
    const std::size_t suite_key_bytes = XtsKeyBytes(suite_hint);
    if (key == nullptr || key_size != suite_key_bytes || data == nullptr || size == 0 || (size % 16) != 0) {
        throw std::invalid_argument("Invalid VeraCrypt XTS transform buffer");
    }
    std::array<std::uint8_t, 16> tweak {};
    for (std::size_t index = 0; index < 8; ++index) tweak[index] = static_cast<std::uint8_t>(data_unit >> (index * 8));
    Botan::secure_vector<std::uint8_t> buffer(data, data + size);
    const auto transform = [&](std::size_t index, Botan::Cipher_Dir direction) {
        auto mode = Botan::Cipher_Mode::create_or_throw(std::string(suite.ciphers.begin()[index]) + "/XTS", direction);
        std::array<std::uint8_t, kXtsKeyBytesPerCipher> pair_key {};
        const std::size_t primary_offset = index * kCipherKeyBytes;
        const std::size_t secondary_offset = suite.ciphers.size() * kCipherKeyBytes + primary_offset;
        std::memcpy(pair_key.data(), key + primary_offset, kCipherKeyBytes);
        std::memcpy(pair_key.data() + kCipherKeyBytes, key + secondary_offset, kCipherKeyBytes);
        mode->set_key(pair_key.data(), pair_key.size());
        mode->start(tweak.data(), tweak.size());
        mode->finish(buffer);
        std::fill(pair_key.begin(), pair_key.end(), 0);
    };
    if (encrypt) {
        for (std::size_t index = 0; index < suite.ciphers.size(); ++index) transform(index, Botan::Cipher_Dir::Encryption);
    } else {
        for (std::size_t index = suite.ciphers.size(); index-- > 0;) transform(index, Botan::Cipher_Dir::Decryption);
    }
    if (buffer.size() != size) throw std::runtime_error("VeraCrypt XTS changed the data-unit size");
    std::memcpy(data, buffer.data(), size);
}

bool TryDecryptHeader(
        const std::array<std::uint8_t, kVeraCryptHeaderSize>& encrypted,
        const SecureBytes& derived,
        KdfHint kdf,
        CipherHint cipher,
        OpenedVolume* opened,
        bool* corrupt_header,
        bool* unsupported_header) {
    const std::size_t key_bytes = XtsKeyBytes(cipher);
    if (derived.size() < key_bytes) return false;
    std::array<std::uint8_t, kVeraCryptHeaderSize> header = encrypted;
    try {
        VeraCryptXtsTransform(cipher, derived.data(), key_bytes, header.data() + kVeraCryptHeaderSaltSize,
                               kVeraCryptHeaderSize - kVeraCryptHeaderSaltSize, 0, false);
        HeaderMetadata metadata = ParseDecryptedVeraCryptHeader(header.data(), header.size());
        if ((metadata.flags & 0x01U) != 0) {
            *unsupported_header = true;
            std::fill(header.begin(), header.end(), 0);
            return false;
        }
        std::vector<std::uint8_t> master(header.begin() + 256, header.begin() + 256 + key_bytes);
        std::vector<std::uint8_t> decrypted(header.begin(), header.end());
        *opened = {metadata, cipher, kdf, false, false, 0, 0, SecureBytes(std::move(master)), SecureBytes(std::move(decrypted))};
        std::fill(header.begin(), header.end(), 0);
        return true;
    } catch (const std::exception&) {
        const bool has_signature = header[64] == 'V' && header[65] == 'E' && header[66] == 'R' && header[67] == 'A';
        if (has_signature) *corrupt_header = true;
        std::fill(header.begin(), header.end(), 0);
        return false;
    }
}

std::array<std::uint8_t, kVeraCryptHeaderSize> EncryptHeader(
        const SecureBytes& decrypted_template, const SecureBytes& processed_password, KdfHint kdf, std::int32_t pim,
        CipherHint cipher) {
    if (decrypted_template.size() != kVeraCryptHeaderSize) throw std::invalid_argument("Missing decrypted VeraCrypt header template");
    std::array<std::uint8_t, kVeraCryptHeaderSize> header {};
    std::copy(decrypted_template.data(), decrypted_template.data() + decrypted_template.size(), header.begin());
    FillSecureRandom(header.data(), kVeraCryptHeaderSaltSize);
    const std::size_t key_bytes = XtsKeyBytes(cipher);
    if (key_bytes > kKeyAreaSize) throw std::invalid_argument("VeraCrypt cipher master key exceeds header key area");
    // The header layout retains the actual data master key but refreshes every
    // reserved/key-area byte whose value is not required for data decryption.
    FillSecureRandom(header.data() + kPaddingOffset, kHeaderCrcOffset - kPaddingOffset);
    FillSecureRandom(header.data() + kKeyAreaOffset + key_bytes, kKeyAreaSize - key_bytes);
    WriteBigEndianU32(header.data() + kKeyAreaCrcOffset, Crc32(header.data() + kKeyAreaOffset, kKeyAreaSize));
    WriteBigEndianU32(header.data() + kHeaderCrcOffset, Crc32(header.data() + kVeraCryptHeaderSaltSize,
                                                               kHeaderCrcOffset - kVeraCryptHeaderSaltSize));
    SecureBytes derived = DeriveVeraCryptHeaderKey(processed_password, header.data(), kVeraCryptHeaderSaltSize, kdf, pim);
    try {
        VeraCryptXtsTransform(cipher, derived.data(), key_bytes, header.data() + kVeraCryptHeaderSaltSize,
                               kVeraCryptHeaderSize - kVeraCryptHeaderSaltSize, 0, true);
    } catch (...) {
        std::fill(header.begin(), header.end(), 0);
        throw;
    }
    return header;
}

void VerifyBackupCredentials(
        const FdRandomAccess& container, const SecureBytes& decrypted_header, std::uint64_t current_header_offset,
        const SecureBytes& processed_password, KdfHint kdf, std::int32_t pim, CipherHint cipher) {
    std::array<std::uint8_t, kVeraCryptHeaderSize> encrypted {};
    try {
        container.ReadAt(current_header_offset, encrypted.data(), encrypted.size());
        SecureBytes derived = DeriveVeraCryptHeaderKey(
                processed_password, decrypted_header.data(), kVeraCryptHeaderSaltSize, kdf, pim);
        OpenedVolume verified {};
        bool corrupt = false;
        bool unsupported = false;
        if (!TryDecryptHeader(encrypted, derived, kdf, cipher, &verified, &corrupt, &unsupported)) {
            throw CoreException(CoreError::kInvalidCredentialsOrFormat);
        }
    } catch (...) {
        std::fill(encrypted.begin(), encrypted.end(), 0);
        throw;
    }
    std::fill(encrypted.begin(), encrypted.end(), 0);
}

}  // namespace

/**
 * Holds Botan XTS modes for the lifetime of one unlocked volume. XTS must be
 * restarted for every VeraCrypt 512-byte data unit, but its cipher key
 * schedule must not be rebuilt for every unit.
 */
class XtsTransformContext final {
public:
    XtsTransformContext(CipherHint suite_hint, const std::uint8_t* key, std::size_t key_size)
        : suite_hint_(suite_hint) {
        const CipherSuite& suite = Suite(suite_hint_);
        if (key == nullptr || key_size != XtsKeyBytes(suite_hint_)) {
            throw std::invalid_argument("Invalid VeraCrypt XTS session key");
        }
        encrypt_.reserve(suite.ciphers.size());
        decrypt_.reserve(suite.ciphers.size());
        for (std::size_t index = 0; index < suite.ciphers.size(); ++index) {
            std::array<std::uint8_t, kXtsKeyBytesPerCipher> pair_key {};
            const std::size_t primary_offset = index * kCipherKeyBytes;
            const std::size_t secondary_offset = suite.ciphers.size() * kCipherKeyBytes + primary_offset;
            std::memcpy(pair_key.data(), key + primary_offset, kCipherKeyBytes);
            std::memcpy(pair_key.data() + kCipherKeyBytes, key + secondary_offset, kCipherKeyBytes);
            try {
                const std::string algorithm = std::string(suite.ciphers.begin()[index]) + "/XTS";
                auto encryption = Botan::Cipher_Mode::create_or_throw(algorithm, Botan::Cipher_Dir::Encryption);
                auto decryption = Botan::Cipher_Mode::create_or_throw(algorithm, Botan::Cipher_Dir::Decryption);
                encryption->set_key(pair_key.data(), pair_key.size());
                decryption->set_key(pair_key.data(), pair_key.size());
                encrypt_.push_back(std::move(encryption));
                decrypt_.push_back(std::move(decryption));
            } catch (...) {
                std::fill(pair_key.begin(), pair_key.end(), 0);
                throw;
            }
            std::fill(pair_key.begin(), pair_key.end(), 0);
        }
    }

    XtsTransformContext(const XtsTransformContext&) = delete;
    XtsTransformContext& operator=(const XtsTransformContext&) = delete;

    ~XtsTransformContext() {
        for (const auto& mode : encrypt_) mode->clear();
        for (const auto& mode : decrypt_) mode->clear();
    }

    void TransformSector(std::uint64_t physical_offset, std::uint8_t* data, bool encrypt) {
        if (data == nullptr || (physical_offset % kSectorBytes) != 0) {
            throw std::invalid_argument("Invalid VeraCrypt XTS sector transform");
        }
        std::array<std::uint8_t, 16> tweak {};
        const std::uint64_t data_unit = VeraCryptDataUnitNumberForPhysicalOffset(physical_offset);
        for (std::size_t index = 0; index < 8; ++index) tweak[index] = static_cast<std::uint8_t>(data_unit >> (index * 8));
        auto process = [&](Botan::Cipher_Mode& mode) {
            mode.start(tweak.data(), tweak.size());
            if (mode.process(data, kSectorBytes) != kSectorBytes) {
                throw std::runtime_error("VeraCrypt XTS sector transform was incomplete");
            }
        };
        if (encrypt) {
            for (const auto& mode : encrypt_) process(*mode);
        } else {
            for (auto it = decrypt_.rbegin(); it != decrypt_.rend(); ++it) process(**it);
        }
        std::fill(tweak.begin(), tweak.end(), 0);
    }

private:
    CipherHint suite_hint_;
    std::vector<std::unique_ptr<Botan::Cipher_Mode>> encrypt_;
    std::vector<std::unique_ptr<Botan::Cipher_Mode>> decrypt_;
};

/**
 * Runtime-sized independent XTS workers. Botan Cipher_Mode is mutable, so
 * each worker owns a distinct key schedule. The caller only submits a range
 * and waits; every sector is transformed by a worker thread (including AES).
 */
class XtsParallelExecutor final {
public:
    XtsParallelExecutor(
            CipherHint cipher, const std::uint8_t* key, std::size_t key_size,
            std::size_t requested_workers = 0) {
        topology_ = DetectCpuTopology();
        const std::size_t count = requested_workers == 0
                ? std::max<std::size_t>(1, topology_.worker_count)
                : std::max<std::size_t>(1, requested_workers);
        __android_log_print(ANDROID_LOG_INFO, "RongVaultVc",
                            "XTS topology allowed=%zu performance=%zu workers=%zu freq=%s%s%s",
                            topology_.allowed_cpus.size(), topology_.performance_cpus.size(), count,
                            topology_.frequency_data_available ? "yes" : "no",
                            topology_.degradation_reason.empty() ? "" : " degradation=",
                            topology_.degradation_reason.empty() ? "" : topology_.degradation_reason.c_str());
        lanes_.reserve(count);
        workers_.reserve(count);
        for (std::size_t lane = 0; lane < count; ++lane) {
            lanes_.push_back(std::make_unique<XtsTransformContext>(cipher, key, key_size));
            workers_.emplace_back([this, lane] { Worker(lane); });
        }
    }

    XtsParallelExecutor(const XtsParallelExecutor&) = delete;
    XtsParallelExecutor& operator=(const XtsParallelExecutor&) = delete;

    ~XtsParallelExecutor() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopping_ = true;
            ++generation_;
        }
        work_ready_.notify_all();
        for (std::thread& worker : workers_) if (worker.joinable()) worker.join();
    }

    void Transform(std::uint64_t physical_offset, std::uint8_t* data, std::size_t length, bool encrypt) {
        if (data == nullptr || length < kParallelXtsMinimumBytes || length % kSectorBytes != 0 ||
            physical_offset % kSectorBytes != 0) {
            throw std::invalid_argument("Invalid parallel VeraCrypt XTS block");
        }
        const std::size_t sectors = length / kSectorBytes;
        const std::size_t channels = workers_.size();
        std::vector<std::size_t> starts(channels);
        std::vector<std::size_t> counts(channels);
        const std::size_t base = sectors / channels;
        const std::size_t extra = sectors % channels;
        std::size_t next = 0;
        for (std::size_t lane = 0; lane < channels; ++lane) {
            starts[lane] = next;
            counts[lane] = base + (lane < extra ? 1 : 0);
            next += counts[lane];
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            physical_offset_ = physical_offset;
            data_ = data;
            encrypt_ = encrypt;
            starts_ = starts;
            counts_ = counts;
            completed_workers_ = 0;
            failure_ = nullptr;
            ++generation_;
        }
        work_ready_.notify_all();
        std::unique_lock<std::mutex> lock(mutex_);
        work_complete_.wait(lock, [this] { return completed_workers_ == workers_.size(); });
        if (failure_ != nullptr) std::rethrow_exception(failure_);
    }

private:
    void TransformRange(std::size_t lane, std::uint64_t physical_offset, std::uint8_t* data,
                        std::size_t first_sector, std::size_t sector_count, bool encrypt) {
        for (std::size_t sector = 0; sector < sector_count; ++sector) {
            const std::size_t index = first_sector + sector;
            lanes_[lane]->TransformSector(physical_offset + index * kSectorBytes, data + index * kSectorBytes, encrypt);
        }
    }

    void Worker(std::size_t lane) {
        std::string affinity_failure;
        if (!topology_.performance_cpus.empty()) {
            const int cpu = topology_.performance_cpus[lane % topology_.performance_cpus.size()];
            const CpuAffinityLease affinity(cpu);
            if (!affinity.bound()) {
                affinity_failure = affinity.failure_reason();
                __android_log_print(ANDROID_LOG_WARN, "RongVaultVc",
                                    "XTS worker %zu could not bind CPU %d: %s; using system scheduling",
                                    lane, cpu, affinity_failure.c_str());
            } else {
                __android_log_print(ANDROID_LOG_DEBUG, "RongVaultVc",
                                    "XTS worker %zu requested_cpu=%d actual_cpu=%d",
                                    lane, cpu, sched_getcpu());
            }
        }
        std::uint64_t observed_generation = 0;
        for (;;) {
            std::uint64_t physical_offset = 0;
            std::uint8_t* data = nullptr;
            std::size_t start = 0;
            std::size_t count = 0;
            bool encrypt = false;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                work_ready_.wait(lock, [this, &observed_generation] { return stopping_ || generation_ != observed_generation; });
                if (stopping_) return;
                observed_generation = generation_;
                physical_offset = physical_offset_;
                data = data_;
                start = starts_[lane];
                count = counts_[lane];
                encrypt = encrypt_;
            }
            try {
                TransformRange(lane, physical_offset, data, start, count, encrypt);
            } catch (...) {
                std::lock_guard<std::mutex> lock(mutex_);
                if (failure_ == nullptr) failure_ = std::current_exception();
            }
            {
                std::lock_guard<std::mutex> lock(mutex_);
                ++completed_workers_;
            }
            work_complete_.notify_one();
        }
    }

    CpuTopology topology_;
    std::vector<std::unique_ptr<XtsTransformContext>> lanes_;
    std::vector<std::thread> workers_;
    std::mutex mutex_;
    std::condition_variable work_ready_;
    std::condition_variable work_complete_;
    bool stopping_ = false;
    std::uint64_t generation_ = 0;
    std::uint64_t physical_offset_ = 0;
    std::uint8_t* data_ = nullptr;
    bool encrypt_ = false;
    std::vector<std::size_t> starts_;
    std::vector<std::size_t> counts_;
    std::size_t completed_workers_ = 0;
    std::exception_ptr failure_;
};
/** A bounded, write-through plaintext page cache owned by one native session. */
class SecureBlockCache final {
public:
    using PageLoader = std::function<void(std::uint64_t offset, std::uint8_t* target, std::size_t length)>;

    SecureBlockCache() : pages_(kCachePageCount) {
        for (Page& page : pages_) page.bytes.resize(kCachePageBytes);
    }

    SecureBlockCache(const SecureBlockCache&) = delete;
    SecureBlockCache& operator=(const SecureBlockCache&) = delete;

    ~SecureBlockCache() { Clear(); }

    std::size_t Read(
            std::uint64_t logical_offset, std::uint8_t* target, std::size_t length,
            std::uint64_t volume_size, const PageLoader& loader) {
        std::size_t copied = 0;
        while (copied < length) {
            const std::uint64_t position = logical_offset + copied;
            const std::uint64_t page_offset = position - (position % kCachePageBytes);
            const std::size_t page_length = static_cast<std::size_t>(std::min<std::uint64_t>(
                    kCachePageBytes, volume_size - page_offset));
            Page& page = FindOrLoad(page_offset, page_length, loader);
            const std::size_t within_page = static_cast<std::size_t>(position - page_offset);
            const std::size_t count = std::min(page_length - within_page, length - copied);
            std::memcpy(target + copied, page.bytes.data() + within_page, count);
            copied += count;
        }
        return copied;
    }

    void Invalidate(std::uint64_t logical_offset, std::size_t length) {
        if (length == 0) return;
        const std::uint64_t after_last = logical_offset + length;
        for (Page& page : pages_) {
            if (page.offset != kInvalidOffset && page.offset < after_last && logical_offset < page.offset + page.length) {
                Wipe(page);
            }
        }
    }

    void Clear() noexcept {
        for (Page& page : pages_) Wipe(page);
    }

    std::pair<std::uint64_t, std::uint64_t> Stats() const noexcept { return {hits_, misses_}; }

private:
    static constexpr std::uint64_t kInvalidOffset = std::numeric_limits<std::uint64_t>::max();

    struct Page final {
        std::uint64_t offset = kInvalidOffset;
        std::size_t length = 0;
        std::uint64_t last_use = 0;
        std::vector<std::uint8_t> bytes;
    };

    Page& FindOrLoad(std::uint64_t page_offset, std::size_t page_length, const PageLoader& loader) {
        for (Page& page : pages_) {
            if (page.offset == page_offset) {
                page.last_use = ++use_counter_;
                ++hits_;
                return page;
            }
        }
        ++misses_;
        Page* selected = &pages_.front();
        for (Page& page : pages_) {
            if (page.offset == kInvalidOffset) {
                selected = &page;
                break;
            }
            if (page.last_use < selected->last_use) selected = &page;
        }
        Wipe(*selected);
        loader(page_offset, selected->bytes.data(), page_length);
        selected->offset = page_offset;
        selected->length = page_length;
        selected->last_use = ++use_counter_;
        return *selected;
    }

    static void Wipe(Page& page) noexcept {
        volatile std::uint8_t* pointer = page.bytes.empty() ? nullptr : page.bytes.data();
        for (std::size_t index = 0; pointer != nullptr && index < page.bytes.size(); ++index) pointer[index] = 0;
        page.offset = kInvalidOffset;
        page.length = 0;
        page.last_use = 0;
    }

    std::vector<Page> pages_;
    std::uint64_t use_counter_ = 0;
    std::uint64_t hits_ = 0;
    std::uint64_t misses_ = 0;
};

std::uint64_t VeraCryptDataUnitNumberForPhysicalOffset(std::uint64_t physical_offset) {
    if ((physical_offset % kSectorBytes) != 0) {
        throw std::invalid_argument("VeraCrypt physical data-unit offset is not 512-byte aligned");
    }
    return physical_offset / kSectorBytes;
}

namespace {

OpenedVolume OpenVeraCryptVolumeImpl(
        const FdRandomAccess& container,
        const OpenRequest& request,
        const std::vector<FdRandomAccess>& keyfiles,
        bool validate_data_area_bounds,
        bool open_hidden_volume) {
    SecureBytes password = ApplyVeraCryptKeyfiles(request.password, keyfiles);
    const std::uint64_t container_size = container.size();
    if (container_size < kHeaderGroupBytes) throw CoreException(CoreError::kInvalidCredentialsOrFormat);
    const std::uint64_t primary_offset = open_hidden_volume ? kHiddenHeaderOffset : 0;
    const std::uint64_t backup_offset = open_hidden_volume ? container_size - kHiddenHeaderOffset : container_size - kHeaderGroupBytes;
    const std::array<std::uint64_t, 2> locations {primary_offset, backup_offset};
    bool used_backup = false;
    bool saw_corrupt_header = false;
    bool saw_unsupported_header = false;
    for (const std::uint64_t location : locations) {
        std::array<std::uint8_t, kVeraCryptHeaderSize> header {};
        container.ReadAt(location, header.data(), header.size());
        for (const KdfHint kdf : KdfCandidates(request.kdf)) {
            // All candidate ciphers for a KDF use the same 192-byte header
            // key. Derive it once, especially important for Argon2id.
            SecureBytes derived = DeriveVeraCryptHeaderKey(
                    password, header.data(), kVeraCryptHeaderSaltSize, kdf, request.pim);
            for (const CipherHint cipher : CipherCandidates(request.cipher)) {
                OpenedVolume opened {};
                if (TryDecryptHeader(header, derived, kdf, cipher, &opened, &saw_corrupt_header, &saw_unsupported_header)) {
                    if (validate_data_area_bounds && (opened.metadata.encrypted_area_offset > container_size ||
                        opened.metadata.encrypted_area_size > container_size - opened.metadata.encrypted_area_offset ||
                        opened.metadata.volume_data_size > opened.metadata.encrypted_area_size)) {
                        throw CoreException(CoreError::kCorruptHeader);
                    }
                    opened.used_backup_header = used_backup;
                    opened.hidden_volume = open_hidden_volume;
                    opened.primary_header_offset = primary_offset;
                    opened.backup_header_offset = backup_offset;
                    std::fill(header.begin(), header.end(), 0);
                    return opened;
                }
            }
        }
        std::fill(header.begin(), header.end(), 0);
        used_backup = true;
    }
    if (saw_unsupported_header) throw CoreException(CoreError::kUnsupportedAlgorithm);
    if (saw_corrupt_header) throw CoreException(CoreError::kCorruptHeader);
    throw CoreException(CoreError::kInvalidCredentialsOrFormat);
}

}  // namespace

OpenedVolume OpenVeraCryptVolume(
        const FdRandomAccess& container,
        const OpenRequest& request,
        const std::vector<FdRandomAccess>& keyfiles,
        const OpenProgressCallback& progress) {
    if (!request.auto_detect_volume) {
        return OpenVeraCryptVolumeImpl(container, request, keyfiles, true, request.open_hidden_volume);
    }
    // AUTO is deliberately KDF-major: a SHA-512 hidden header must not wait
    // for every outer-header KDF (and its backup header) first. Within each
    // KDF, primary headers are checked before backups and the outer header is
    // retained when both headers validate with the same credentials.
    SecureBytes password = ApplyVeraCryptKeyfiles(request.password, keyfiles);
    const std::uint64_t container_size = container.size();
    if (container_size < kHeaderGroupBytes) throw CoreException(CoreError::kInvalidCredentialsOrFormat);
    struct Candidate { std::uint64_t offset; bool hidden; bool backup; };
    const std::array<Candidate, 4> candidates {{
        {0, false, false},
        {kHiddenHeaderOffset, true, false},
        {container_size - kHeaderGroupBytes, false, true},
        {container_size - kHiddenHeaderOffset, true, true},
    }};
    const auto kdfs = KdfCandidates(request.kdf);
    const auto ciphers = CipherCandidates(request.cipher);
    const std::uint32_t total = static_cast<std::uint32_t>(kdfs.size() * candidates.size() * ciphers.size());
    std::uint32_t completed = 0;
    bool saw_corrupt_header = false;
    bool saw_unsupported_header = false;
    bool saw_insufficient_memory = false;
    for (const KdfHint kdf : kdfs) {
        for (const Candidate& candidate : candidates) {
            if (progress && !progress(completed, total)) throw CoreException(CoreError::kCancelled);
            std::array<std::uint8_t, kVeraCryptHeaderSize> header {};
            try {
                container.ReadAt(candidate.offset, header.data(), header.size());
                // Header salts differ, so PBKDF2 derives an independent key for
                // each location. Keeping this candidate boundary also limits
                // Argon2 memory to one derivation at a time.
                SecureBytes derived = DeriveVeraCryptHeaderKey(
                        password, header.data(), kVeraCryptHeaderSaltSize, kdf, request.pim);
                for (const CipherHint cipher : ciphers) {
                    OpenedVolume opened {};
                    if (TryDecryptHeader(header, derived, kdf, cipher, &opened, &saw_corrupt_header, &saw_unsupported_header)) {
                        const bool invalid_bounds = opened.metadata.encrypted_area_offset > container_size ||
                            opened.metadata.encrypted_area_size > container_size - opened.metadata.encrypted_area_offset ||
                            opened.metadata.volume_data_size > opened.metadata.encrypted_area_size;
                        if (!invalid_bounds) {
                            opened.used_backup_header = candidate.backup;
                            opened.hidden_volume = candidate.hidden;
                            opened.primary_header_offset = candidate.hidden ? kHiddenHeaderOffset : 0;
                            opened.backup_header_offset = candidate.hidden ? container_size - kHiddenHeaderOffset : container_size - kHeaderGroupBytes;
                            std::fill(header.begin(), header.end(), 0);
                            return opened;
                        }
                        saw_corrupt_header = true;
                    }
                    ++completed;
                }
            } catch (const CoreException& error) {
                if (error.error() == CoreError::kCancelled) throw;
                if (error.error() == CoreError::kUnsupportedAlgorithm) saw_unsupported_header = true;
                else if (error.error() == CoreError::kCorruptHeader) saw_corrupt_header = true;
                else if (error.error() == CoreError::kInsufficientMemory) saw_insufficient_memory = true;
                else if (error.error() != CoreError::kInvalidCredentialsOrFormat) throw;
                completed += static_cast<std::uint32_t>(ciphers.size());
            } catch (const std::bad_alloc&) {
                // An Argon2 candidate can exceed the device budget. It must
                // not prevent a later, valid PBKDF2/other-header candidate.
                saw_insufficient_memory = true;
                completed += static_cast<std::uint32_t>(ciphers.size());
            }
            std::fill(header.begin(), header.end(), 0);
        }
    }
    if (progress && !progress(total, total)) throw CoreException(CoreError::kCancelled);
    if (saw_unsupported_header) throw CoreException(CoreError::kUnsupportedAlgorithm);
    if (saw_corrupt_header) throw CoreException(CoreError::kCorruptHeader);
    if (saw_insufficient_memory) throw CoreException(CoreError::kInsufficientMemory);
    throw CoreException(CoreError::kInvalidCredentialsOrFormat);
}

std::shared_ptr<NativeVolumeSession> CreateNormalVeraCryptVolume(
        FdRandomAccess container,
        const CreateRequest& request,
        const std::vector<FdRandomAccess>& keyfiles,
        const CreateProgressCallback& progress) {
    if (!container.writable()) throw CoreException(CoreError::kReadOnlySource);
    if (request.hidden_volume) throw std::invalid_argument("Hidden volumes require the two-stage creation flow");
    if (!IsCreatableCipher(request.cipher) || request.kdf == KdfHint::kAuto) {
        throw CoreException(CoreError::kUnsupportedAlgorithm);
    }
    if (request.container_size < kHeaderGroupBytes * 2 + kMinimumCreatedDataBytes ||
        request.container_size > kMaximumCreatedDataBytes ||
        (request.container_size % request.sector_size) != 0) {
        throw std::invalid_argument("Container size cannot hold aligned VeraCrypt headers and a filesystem");
    }
    const std::uint64_t data_offset = kHeaderGroupBytes;
    const std::uint64_t data_size = request.container_size - kHeaderGroupBytes * 2;
    if ((data_size % request.sector_size) != 0 || (data_size % kSectorBytes) != 0) {
        throw std::invalid_argument("Created VeraCrypt data area is not sector aligned");
    }

    container.Resize(request.container_size);
    // Before any data or filesystem write, invalidate every header slot. A
    // failed full format must never leave an earlier valid container behind.
    std::array<std::uint8_t, kVeraCryptHeaderSize> invalid_header {};
    container.WriteAt(0, invalid_header.data(), invalid_header.size());
    container.WriteAt(kHiddenHeaderOffset, invalid_header.data(), invalid_header.size());
    container.WriteAt(request.container_size - kHeaderGroupBytes, invalid_header.data(), invalid_header.size());
    container.WriteAt(request.container_size - kHiddenHeaderOffset, invalid_header.data(), invalid_header.size());
    container.Sync();

    const std::size_t master_key_bytes = XtsKeyBytes(request.cipher);
    std::vector<std::uint8_t> master_key(master_key_bytes);
    std::vector<std::uint8_t> template_bytes(kVeraCryptHeaderSize);
    FillSecureRandom(master_key.data(), master_key.size());
    FillSecureRandom(template_bytes.data(), template_bytes.size());
    try {
        template_bytes[64] = 'V'; template_bytes[65] = 'E'; template_bytes[66] = 'R'; template_bytes[67] = 'A';
        WriteBigEndianU16(template_bytes.data() + 68, kCreatedHeaderVersion);
        WriteBigEndianU16(template_bytes.data() + 70, kCreatedRequiredProgramVersion);
        WriteBigEndianU64(template_bytes.data() + 92, 0);
        WriteBigEndianU64(template_bytes.data() + 100, data_size);
        WriteBigEndianU64(template_bytes.data() + 108, data_offset);
        WriteBigEndianU64(template_bytes.data() + 116, data_size);
        WriteBigEndianU32(template_bytes.data() + 124, 0);
        WriteBigEndianU32(template_bytes.data() + 128, request.sector_size);
        std::copy(master_key.begin(), master_key.end(), template_bytes.begin() + kKeyAreaOffset);
        WriteBigEndianU32(template_bytes.data() + kKeyAreaCrcOffset,
                          Crc32(template_bytes.data() + kKeyAreaOffset, kKeyAreaSize));
        WriteBigEndianU32(template_bytes.data() + kHeaderCrcOffset,
                          Crc32(template_bytes.data() + kVeraCryptHeaderSaltSize,
                                kHeaderCrcOffset - kVeraCryptHeaderSaltSize));

        HeaderMetadata metadata {
                kCreatedHeaderVersion, kCreatedRequiredProgramVersion, 0, data_size, data_size, data_offset, 0, request.sector_size,
        };
        OpenedVolume opened {
                metadata, request.cipher, request.kdf, false, false, 0, request.container_size - kHeaderGroupBytes,
                SecureBytes(std::move(master_key)), SecureBytes(std::move(template_bytes)),
        };
        auto session = std::make_shared<NativeVolumeSession>(std::move(container), std::move(opened), std::nullopt);

        // Full formatting starts with encrypted random data so no plaintext or
        // prior container data survives in unallocated filesystem sectors.
        std::array<std::uint8_t, 128 * 1024> initialization {};
        for (std::uint64_t offset = 0; offset < data_size;) {
            if (!progress(1, offset, data_size)) throw CoreException(CoreError::kCancelled);
            const std::size_t count = static_cast<std::size_t>(std::min<std::uint64_t>(initialization.size(), data_size - offset));
            FillSecureRandom(initialization.data(), count);
            session->Write(offset, initialization.data(), count);
            std::fill(initialization.begin(), initialization.begin() + count, 0);
            offset += count;
            if (!progress(1, offset, data_size)) throw CoreException(CoreError::kCancelled);
        }
        if (!progress(2, 0, 0)) throw CoreException(CoreError::kCancelled);
        const FatFsType filesystem = request.filesystem == CreateFileSystem::kExFat ? FatFsType::kExFat : FatFsType::kFat;
        session->FormatFatFs(filesystem);
        if (!progress(3, data_size, data_size)) throw CoreException(CoreError::kCancelled);
        SecureBytes processed_password = ApplyVeraCryptKeyfiles(request.password, keyfiles);
        session->FinalizeCreatedHeaders(processed_password, request.pim);
        return session;
    } catch (...) {
        std::fill(master_key.begin(), master_key.end(), 0);
        std::fill(template_bytes.begin(), template_bytes.end(), 0);
        throw;
    }
}

OpenedVolume OpenVeraCryptHeaderBackup(
        const FdRandomAccess& header_backup,
        const OpenRequest& request,
        const std::vector<FdRandomAccess>& keyfiles) {
    if (header_backup.size() != kHeaderGroupBytes) {
        throw std::invalid_argument("VeraCrypt header backup must be exactly 128 KiB");
    }
    if (request.auto_detect_volume) throw std::invalid_argument("Header backup restore requires a concrete volume type");
    return OpenVeraCryptVolumeImpl(header_backup, request, keyfiles, false, request.open_hidden_volume);
}

void RestoreVeraCryptHeader(
        FdRandomAccess& container,
        const FdRandomAccess& header_backup,
        const OpenRequest& request,
        const std::vector<FdRandomAccess>& keyfiles) {
    if (!container.writable()) throw CoreException(CoreError::kReadOnlySource);
    if (header_backup.size() != kHeaderGroupBytes) throw std::invalid_argument("VeraCrypt header backup must be exactly 128 KiB");
    const std::uint64_t container_size = container.size();
    if (container_size < kHeaderGroupBytes * 2) throw std::invalid_argument("Container is too small for VeraCrypt header restoration");
    // Verify both sides using the requested normal/hidden credentials. Do not
    // use a valid header from a different volume as a restoration source.
    const OpenedVolume target = OpenVeraCryptVolume(container, request, keyfiles);
    const OpenedVolume source = OpenVeraCryptHeaderBackup(header_backup, request, keyfiles);
    if (target.hidden_volume != source.hidden_volume ||
        target.metadata.encrypted_area_offset != source.metadata.encrypted_area_offset ||
        target.metadata.encrypted_area_size != source.metadata.encrypted_area_size ||
        target.metadata.volume_data_size != source.metadata.volume_data_size ||
        target.metadata.sector_size != source.metadata.sector_size) {
        throw CoreException(CoreError::kCorruptHeader);
    }
    // A header backup has one selected normal or hidden header at the same
    // primary and backup offset because its total size is exactly 128 KiB.
    // Restore only that type, never the other header class in the group.
    std::array<std::uint8_t, kVeraCryptHeaderSize> buffer {};
    try {
        header_backup.ReadAt(source.primary_header_offset, buffer.data(), buffer.size());
        // Commit the backup copy first. A cancellation or I/O interruption can
        // then still leave a usable header at the end of the container.
        container.WriteAt(target.backup_header_offset, buffer.data(), buffer.size());
        container.Sync();
        container.WriteAt(target.primary_header_offset, buffer.data(), buffer.size());
        container.Sync();
    } catch (...) {
        std::fill(buffer.begin(), buffer.end(), 0);
        throw;
    }
    std::fill(buffer.begin(), buffer.end(), 0);
}

NativeVolumeSession::NativeVolumeSession(
        FdRandomAccess container, OpenedVolume opened, std::optional<EncryptedRange> protected_hidden_range)
    : container_(std::move(container)), metadata_(opened.metadata), cipher_(opened.cipher), master_key_(std::move(opened.master_key)),
      kdf_(opened.kdf), decrypted_header_(std::move(opened.decrypted_header)),
      primary_header_offset_(opened.primary_header_offset), backup_header_offset_(opened.backup_header_offset),
      hidden_volume_(opened.hidden_volume), used_backup_header_(opened.used_backup_header),
      protected_hidden_range_(protected_hidden_range) {
    if (master_key_.size() != XtsKeyBytes(cipher_)) {
        throw std::invalid_argument("Unlocked cipher suite cannot create a native session");
    }
    xts_ = std::make_unique<XtsTransformContext>(cipher_, master_key_.data(), master_key_.size());
    parallel_xts_ = std::make_unique<XtsParallelExecutor>(cipher_, master_key_.data(), master_key_.size());
    read_cache_ = std::make_unique<SecureBlockCache>();
    io_counters_at_open_ = container_.counters();
}

NativeVolumeSession::~NativeVolumeSession() {
    UnmountFileSystem();
    if (read_cache_ != nullptr) read_cache_->Clear();
    read_cache_.reset();
    parallel_xts_.reset();
    xts_.reset();
    std::fill(write_scratch_.begin(), write_scratch_.end(), 0);
}

VolumeInfo NativeVolumeSession::info() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return {
            metadata_.volume_data_size,
            metadata_.encrypted_area_offset,
            metadata_.encrypted_area_size,
            metadata_.sector_size,
            cipher_,
            kdf_,
            hidden_volume_,
            used_backup_header_,
    };
}

VolumePerformanceCounters NativeVolumeSession::performance_counters() const {
    std::lock_guard<std::mutex> lock(mutex_);
    const RandomAccessCounters io = container_.counters();
    const auto cache_stats = read_cache_ != nullptr ? read_cache_->Stats() : std::pair<std::uint64_t, std::uint64_t>{0, 0};
    return {
            io.read_syscalls - io_counters_at_open_.read_syscalls,
            io.write_syscalls - io_counters_at_open_.write_syscalls,
            io.read_bytes - io_counters_at_open_.read_bytes,
            io.write_bytes - io_counters_at_open_.write_bytes,
            batched_reads_,
            batched_writes_,
            rmw_sectors_,
            xts_data_units_,
            cache_stats.first,
            cache_stats.second,
    };
}

bool NativeVolumeSession::hidden_volume_protection_triggered() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return hidden_volume_protection_triggered_;
}

void NativeVolumeSession::CheckRange(std::uint64_t logical_offset, std::size_t length) const {
    if (length > std::numeric_limits<std::uint64_t>::max() - logical_offset ||
        logical_offset + length > metadata_.volume_data_size) {
        throw std::out_of_range("Volume logical range is outside the encrypted data area");
    }
}

void NativeVolumeSession::TransformSectorAtPhysicalOffset(std::uint64_t physical_offset, std::uint8_t* data, bool encrypt) {
    xts_->TransformSector(physical_offset, data, encrypt);
    ++xts_data_units_;
}

void NativeVolumeSession::TransformAlignedBlockAtPhysicalOffset(
        std::uint64_t physical_offset, std::uint8_t* data, std::size_t length, bool encrypt) {
    if (length == 0 || length % kSectorBytes != 0 || physical_offset % kSectorBytes != 0) {
        throw std::invalid_argument("Invalid aligned VeraCrypt XTS block");
    }
#if !defined(VC_CORE_DISABLE_PARALLEL_XTS)
    if (length >= kParallelXtsMinimumBytes && parallel_xts_ != nullptr) {
        parallel_xts_->Transform(physical_offset, data, length, encrypt);
        xts_data_units_ += length / kSectorBytes;
        return;
    }
#endif
    for (std::size_t offset = 0; offset < length; offset += kSectorBytes)
        TransformSectorAtPhysicalOffset(physical_offset + offset, data + offset, encrypt);
}

void NativeVolumeSession::CheckHiddenVolumeProtectionRange(std::uint64_t logical_offset, std::size_t length) {
    if (!protected_hidden_range_.has_value() || length == 0) return;
    const std::uint64_t first_sector = logical_offset / kSectorBytes;
    const std::uint64_t final_byte = CheckedAdd(logical_offset, static_cast<std::uint64_t>(length) - 1,
                                                "Hidden-volume protection range overflows");
    const std::uint64_t final_sector = final_byte / kSectorBytes;
    const std::uint64_t first_physical = CheckedAdd(
            metadata_.encrypted_area_offset, CheckedMul(first_sector, kSectorBytes, "Hidden-volume sector range overflows"),
            "Hidden-volume physical range overflows");
    const std::uint64_t after_last_physical = CheckedAdd(
            metadata_.encrypted_area_offset,
            CheckedMul(CheckedAdd(final_sector, 1, "Hidden-volume sector end overflows"), kSectorBytes,
                        "Hidden-volume sector range overflows"),
            "Hidden-volume physical range overflows");
    const EncryptedRange protected_range = *protected_hidden_range_;
    const std::uint64_t protected_after = CheckedAdd(protected_range.offset, protected_range.size,
                                                     "Hidden-volume protected range overflows");
    if (first_physical < protected_after &&
        protected_range.offset < after_last_physical) {
        hidden_volume_protection_triggered_ = true;
        throw CoreException(CoreError::kHiddenVolumeRisk);
    }
}

std::size_t NativeVolumeSession::ReadUncachedLocked(
        std::uint64_t logical_offset, std::uint8_t* target, std::size_t length) {
    if (length != 0 && target == nullptr) throw std::invalid_argument("Read target is null");
    CheckRange(logical_offset, length);
    std::size_t completed = 0;
    std::array<std::uint8_t, kSectorBytes> sector_data {};
    const auto read_sector = [&](std::uint64_t sector, std::size_t within_sector, std::size_t copied) {
        const std::uint64_t physical_offset = metadata_.encrypted_area_offset + sector * kSectorBytes;
        container_.ReadAt(physical_offset, sector_data.data(), sector_data.size());
        TransformSectorAtPhysicalOffset(physical_offset, sector_data.data(), false);
        std::memcpy(target + completed, sector_data.data() + within_sector, copied);
        std::fill(sector_data.begin(), sector_data.end(), 0);
    };
    if ((logical_offset % kSectorBytes) != 0 && completed < length) {
        const std::size_t within_sector = static_cast<std::size_t>(logical_offset % kSectorBytes);
        const std::size_t copied = std::min(kSectorBytes - within_sector, length);
        read_sector(logical_offset / kSectorBytes, within_sector, copied);
        completed += copied;
    }
    while (completed < length) {
        const std::uint64_t position = logical_offset + completed;
        const std::size_t remaining = length - completed;
        if (remaining < kSectorBytes) {
            read_sector(position / kSectorBytes, 0, remaining);
            completed += remaining;
            continue;
        }
        const std::size_t block_size = std::min(kMaximumIoBlockBytes, remaining - (remaining % kSectorBytes));
        const std::uint64_t physical_offset = metadata_.encrypted_area_offset + position;
        container_.ReadAt(physical_offset, target + completed, block_size);
        ++batched_reads_;
        TransformAlignedBlockAtPhysicalOffset(physical_offset, target + completed, block_size, false);
        completed += block_size;
    }
    return completed;
}

std::size_t NativeVolumeSession::Read(std::uint64_t logical_offset, std::uint8_t* target, std::size_t length) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (length != 0 && target == nullptr) throw std::invalid_argument("Read target is null");
    CheckRange(logical_offset, length);
    if (length != 0 && length < kCachedReadMaximumBytes && read_cache_ != nullptr) {
        return read_cache_->Read(logical_offset, target, length, metadata_.volume_data_size,
                [this](std::uint64_t offset, std::uint8_t* page, std::size_t page_length) {
                    ReadUncachedLocked(offset, page, page_length);
                });
    }
    return ReadUncachedLocked(logical_offset, target, length);
}

#if defined(VC_CORE_ENABLE_BENCHMARK_STATS)
std::size_t NativeVolumeSession::ReadUncachedForBenchmark(
        std::uint64_t logical_offset, std::uint8_t* target, std::size_t length) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (length != 0 && target == nullptr) throw std::invalid_argument("Uncached read target is null");
    CheckRange(logical_offset, length);
    return ReadUncachedLocked(logical_offset, target, length);
}

std::size_t NativeVolumeSession::ReadLegacySector(
        std::uint64_t logical_offset, std::uint8_t* target, std::size_t length) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (length != 0 && target == nullptr) throw std::invalid_argument("Legacy read target is null");
    CheckRange(logical_offset, length);
    std::array<std::uint8_t, kSectorBytes> sector_data {};
    for (std::size_t completed = 0; completed < length;) {
        const std::uint64_t position = CheckedAdd(logical_offset, completed, "Legacy read position overflows");
        const std::uint64_t sector = position / kSectorBytes;
        const std::size_t within = static_cast<std::size_t>(position % kSectorBytes);
        const std::size_t copied = std::min(kSectorBytes - within, length - completed);
        const std::uint64_t physical = CheckedAdd(
                metadata_.encrypted_area_offset, CheckedMul(sector, kSectorBytes, "Legacy read sector overflows"),
                "Legacy read physical offset overflows");
        container_.ReadAt(physical, sector_data.data(), sector_data.size());
        VeraCryptXtsTransform(cipher_, master_key_.data(), master_key_.size(), sector_data.data(), sector_data.size(),
                               VeraCryptDataUnitNumberForPhysicalOffset(physical), false);
        std::memcpy(target + completed, sector_data.data() + within, copied);
        completed += copied;
    }
    std::fill(sector_data.begin(), sector_data.end(), 0);
    return length;
}

std::size_t NativeVolumeSession::WriteLegacySector(
        std::uint64_t logical_offset, const std::uint8_t* source, std::size_t length) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (length != 0 && source == nullptr) throw std::invalid_argument("Legacy write source is null");
    if (!container_.writable()) throw CoreException(CoreError::kReadOnlySource);
    if (hidden_volume_protection_triggered_) throw CoreException(CoreError::kHiddenVolumeRisk);
    CheckRange(logical_offset, length);
    CheckHiddenVolumeProtectionRange(logical_offset, length);
    std::array<std::uint8_t, kSectorBytes> sector_data {};
    for (std::size_t completed = 0; completed < length;) {
        const std::uint64_t position = CheckedAdd(logical_offset, completed, "Legacy write position overflows");
        const std::uint64_t sector = position / kSectorBytes;
        const std::size_t within = static_cast<std::size_t>(position % kSectorBytes);
        const std::size_t copied = std::min(kSectorBytes - within, length - completed);
        const std::uint64_t physical = CheckedAdd(
                metadata_.encrypted_area_offset, CheckedMul(sector, kSectorBytes, "Legacy write sector overflows"),
                "Legacy write physical offset overflows");
        container_.ReadAt(physical, sector_data.data(), sector_data.size());
        VeraCryptXtsTransform(cipher_, master_key_.data(), master_key_.size(), sector_data.data(), sector_data.size(),
                               VeraCryptDataUnitNumberForPhysicalOffset(physical), false);
        std::memcpy(sector_data.data() + within, source + completed, copied);
        VeraCryptXtsTransform(cipher_, master_key_.data(), master_key_.size(), sector_data.data(), sector_data.size(),
                               VeraCryptDataUnitNumberForPhysicalOffset(physical), true);
        container_.WriteAt(physical, sector_data.data(), sector_data.size());
        completed += copied;
    }
    std::fill(sector_data.begin(), sector_data.end(), 0);
    return length;
}
#endif

std::size_t NativeVolumeSession::Write(std::uint64_t logical_offset, const std::uint8_t* source, std::size_t length) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (length != 0 && source == nullptr) throw std::invalid_argument("Write source is null");
    if (!container_.writable()) throw CoreException(CoreError::kReadOnlySource);
    if (hidden_volume_protection_triggered_) {
        throw CoreException(CoreError::kHiddenVolumeRisk);
    }
    // A FAT/exFAT metadata or data write can allocate a formerly free tail
    // cluster. Do not permit a later hidden-create operation to trust an
    // analysis performed before this write.
    if (!hidden_volume_ && length != 0) analyzed_hidden_capacity_.reset();
    CheckRange(logical_offset, length);
    CheckHiddenVolumeProtectionRange(logical_offset, length);
    // Write-through policy: no dirty plaintext is retained. Invalidate every
    // page touched by the complete logical request before the first write.
    if (read_cache_ != nullptr && length != 0) read_cache_->Invalidate(logical_offset, length);
    std::array<std::uint8_t, kSectorBytes> sector_data {};
    std::size_t completed = 0;
    while (completed < length) {
        const std::uint64_t position = logical_offset + completed;
        const std::uint64_t sector = position / kSectorBytes;
        const std::size_t within_sector = static_cast<std::size_t>(position % kSectorBytes);
        const std::size_t copied = std::min(kSectorBytes - within_sector, length - completed);
        const std::uint64_t physical_offset = metadata_.encrypted_area_offset + sector * kSectorBytes;
        if (within_sector == 0 && length - completed >= kSectorBytes) {
            const std::size_t remaining = length - completed;
            const std::size_t block_size = std::min(kMaximumIoBlockBytes, remaining - (remaining % kSectorBytes));
            write_scratch_.resize(block_size);
            try {
                std::memcpy(write_scratch_.data(), source + completed, block_size);
                TransformAlignedBlockAtPhysicalOffset(physical_offset, write_scratch_.data(), block_size, true);
                container_.WriteAt(physical_offset, write_scratch_.data(), write_scratch_.size());
            } catch (...) {
                std::fill(write_scratch_.begin(), write_scratch_.end(), 0);
                throw;
            }
            ++batched_writes_;
            std::fill(write_scratch_.begin(), write_scratch_.end(), 0);
            completed += block_size;
            continue;
        }
        container_.ReadAt(physical_offset, sector_data.data(), sector_data.size());
        ++rmw_sectors_;
        TransformSectorAtPhysicalOffset(physical_offset, sector_data.data(), false);
        std::memcpy(sector_data.data() + within_sector, source + completed, copied);
        TransformSectorAtPhysicalOffset(physical_offset, sector_data.data(), true);
        container_.WriteAt(physical_offset, sector_data.data(), sector_data.size());
        std::fill(sector_data.begin(), sector_data.end(), 0);
        completed += copied;
    }
    return completed;
}

void NativeVolumeSession::Flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    container_.Sync();
}

FileSystemType NativeVolumeSession::MountFileSystem() {
    std::lock_guard<std::mutex> lock(filesystem_mutex_);
    if (fatfs_ != nullptr) return fatfs_->Mount();
    if (ntfs_ != nullptr) return ntfs_->Mount();
    auto fatfs = std::make_unique<FatFsVolume>(*this);
    try {
        const FileSystemType type = fatfs->Mount();
        fatfs_ = std::move(fatfs);
        return type;
    } catch (const CoreException&) {
        auto ntfs = std::make_unique<NtfsVolume>(*this);
        try {
            const FileSystemType type = ntfs->Mount();
            ntfs_ = std::move(ntfs);
            return type;
        } catch (...) {
            throw;
        }
    }
}

FatFsType NativeVolumeSession::FormatFatFs(FatFsType requested_type) {
    std::lock_guard<std::mutex> lock(filesystem_mutex_);
    if (fatfs_ == nullptr) fatfs_ = std::make_unique<FatFsVolume>(*this);
    return fatfs_->FormatAndMount(requested_type);
}

void NativeVolumeSession::UnmountFileSystem() noexcept {
    std::lock_guard<std::mutex> lock(filesystem_mutex_);
    if (fatfs_ != nullptr) {
        fatfs_->Unmount();
        fatfs_.reset();
    }
    if (ntfs_ != nullptr) {
        ntfs_->Unmount();
        ntfs_.reset();
    }
}

#if defined(VC_CORE_ENABLE_SELF_TESTS)
/** Debug-only byte-equivalence test for every currently implemented suite. */
extern "C" void VcCoreParallelXtsSelfTest() {
    // CPU_SETSIZE - 1 is outside any supported Android device's online CPU
    // set. It must fail cleanly and leave the caller on normal system
    // scheduling, exercising the affinity-degradation path.
    std::string affinity_failure;
    if (BindCurrentThreadToCpu(CPU_SETSIZE - 1, &affinity_failure) || affinity_failure.empty()) {
        throw std::runtime_error("Affinity failure fallback self-test did not fail as expected");
    }
    constexpr std::array<CipherHint, 15> kSuites {{
            CipherHint::kAes, CipherHint::kSerpent, CipherHint::kTwofish,
            CipherHint::kCamellia, CipherHint::kKuznyechik, CipherHint::kTwofishAes,
            CipherHint::kSerpentTwofishAes, CipherHint::kAesSerpent,
            CipherHint::kAesTwofishSerpent, CipherHint::kSerpentTwofish,
            CipherHint::kKuznyechikCamellia, CipherHint::kTwofishKuznyechik,
            CipherHint::kSerpentCamellia, CipherHint::kAesKuznyechik,
            CipherHint::kCamelliaSerpentKuznyechik,
    }};
    for (const CipherHint suite : kSuites) {
        std::vector<std::uint8_t> key(XtsKeyBytes(suite));
        std::vector<std::uint8_t> plain(128 * 1024);
        for (std::size_t index = 0; index < key.size(); ++index) key[index] = static_cast<std::uint8_t>(index * 17U + 3U);
        for (std::size_t index = 0; index < plain.size(); ++index) plain[index] = static_cast<std::uint8_t>(index * 29U + 11U);
        std::vector<std::uint8_t> encrypted = plain;
        XtsTransformContext encryption_context(suite, key.data(), key.size());
        for (std::size_t offset = 0; offset < encrypted.size(); offset += kSectorBytes) {
            encryption_context.TransformSector(131072 + offset, encrypted.data() + offset, true);
        }
        std::vector<std::uint8_t> parallel = plain;
        const std::size_t detected_workers = DetectCpuTopology().worker_count;
        for (const std::size_t workers : {std::size_t{1}, std::size_t{2}, detected_workers}) {
            parallel = plain;
            XtsParallelExecutor parallel_executor(suite, key.data(), key.size(), workers);
            parallel_executor.Transform(131072, parallel.data(), parallel.size(), true);
            if (parallel != encrypted) throw std::runtime_error("Parallel XTS output differs from serial output");
        }
        XtsTransformContext decryption_context(suite, key.data(), key.size());
        for (std::size_t offset = 0; offset < encrypted.size(); offset += kSectorBytes) {
            decryption_context.TransformSector(131072 + offset, encrypted.data() + offset, false);
        }
        if (plain != encrypted) throw std::runtime_error("Serial XTS decrypt round trip failed");
        std::fill(key.begin(), key.end(), 0);
        std::fill(plain.begin(), plain.end(), 0);
        std::fill(encrypted.begin(), encrypted.end(), 0);
        std::fill(parallel.begin(), parallel.end(), 0);
    }
}
#endif

std::vector<FileSystemEntry> NativeVolumeSession::ListDirectory(const std::string& relative_path) {
    std::lock_guard<std::mutex> lock(filesystem_mutex_);
    if (fatfs_ != nullptr) return fatfs_->ListDirectory(relative_path);
    if (ntfs_ != nullptr) return ntfs_->ListDirectory(relative_path);
    throw CoreException(CoreError::kUnsupportedFileSystem);
}

FileSystemEntry NativeVolumeSession::StatFileSystemEntry(const std::string& relative_path) {
    std::lock_guard<std::mutex> lock(filesystem_mutex_);
    if (fatfs_ != nullptr) return fatfs_->Stat(relative_path);
    if (ntfs_ != nullptr) return ntfs_->Stat(relative_path);
    throw CoreException(CoreError::kUnsupportedFileSystem);
}

std::unique_ptr<FileSystemFile> NativeVolumeSession::OpenFileSystemFile(
        const std::string& relative_path, bool writable, bool create, bool truncate) {
    std::lock_guard<std::mutex> lock(filesystem_mutex_);
    if (fatfs_ != nullptr) return fatfs_->OpenFile(relative_path, writable, create, truncate);
    if (ntfs_ != nullptr) return ntfs_->OpenFile(relative_path, writable, create, truncate);
    throw CoreException(CoreError::kUnsupportedFileSystem);
}

void NativeVolumeSession::CreateFatFsDirectory(const std::string& relative_path) {
    std::lock_guard<std::mutex> lock(filesystem_mutex_);
    if (ntfs_ != nullptr) throw CoreException(CoreError::kReadOnlySource);
    if (fatfs_ == nullptr) throw CoreException(CoreError::kUnsupportedFileSystem);
    fatfs_->CreateDirectory(relative_path);
}

void NativeVolumeSession::DeleteFatFsEntry(const std::string& relative_path) {
    std::lock_guard<std::mutex> lock(filesystem_mutex_);
    if (ntfs_ != nullptr) throw CoreException(CoreError::kReadOnlySource);
    if (fatfs_ == nullptr) throw CoreException(CoreError::kUnsupportedFileSystem);
    fatfs_->Delete(relative_path);
}

void NativeVolumeSession::RenameFatFsEntry(
        const std::string& from_relative_path, const std::string& to_relative_path) {
    std::lock_guard<std::mutex> lock(filesystem_mutex_);
    if (ntfs_ != nullptr) throw CoreException(CoreError::kReadOnlySource);
    if (fatfs_ == nullptr) throw CoreException(CoreError::kUnsupportedFileSystem);
    fatfs_->Rename(from_relative_path, to_relative_path);
}

HiddenVolumeCapacity NativeVolumeSession::AnalyzeHiddenVolumeCapacity() {
    std::lock_guard<std::mutex> lock(filesystem_mutex_);
    if (hidden_volume_) throw std::invalid_argument("Hidden capacity must be analyzed from an outer volume session");
    if (fatfs_ == nullptr || !fatfs_->mounted()) throw CoreException(CoreError::kUnsupportedFileSystem);
    const FatFsTailFreeRange range = fatfs_->FindTailFreeRange();
    if (range.logical_offset > metadata_.volume_data_size ||
        range.size > metadata_.volume_data_size - range.logical_offset ||
        range.logical_offset > std::numeric_limits<std::uint64_t>::max() - metadata_.encrypted_area_offset) {
        throw CoreException(CoreError::kCorruptHeader);
    }
    const HiddenVolumeCapacity result {range.size, metadata_.encrypted_area_offset + range.logical_offset};
    // Do not take filesystem_mutex_ while holding mutex_: filesystem writes
    // may already hold the former while entering Write().
    std::lock_guard<std::mutex> session_lock(mutex_);
    analyzed_hidden_capacity_ = result;
    return result;
}

std::shared_ptr<NativeVolumeSession> NativeVolumeSession::CreateHiddenVolume(
        const CreateRequest& request, const std::vector<FdRandomAccess>& keyfiles, const CreateProgressCallback& progress) {
    if (!request.hidden_volume) throw std::invalid_argument("Hidden creation requires hidden-volume options");
    if (!IsCreatableCipher(request.cipher) || request.kdf == KdfHint::kAuto) {
        throw CoreException(CoreError::kUnsupportedAlgorithm);
    }
    if (request.container_size < kMinimumCreatedDataBytes ||
        request.container_size > kMaximumCreatedDataBytes ||
        request.container_size % request.sector_size != 0 || request.container_size % kSectorBytes != 0) {
        throw std::invalid_argument("Hidden volume size is not a supported aligned filesystem size");
    }

    // Serializing through mutex_ prevents a concurrent outer FAT/exFAT write
    // from invalidating the capacity between validation and the final header
    // commit. Do not acquire filesystem_mutex_ here: the prior scan owns that
    // synchronization boundary and filesystem writes enter this mutex below.
    std::lock_guard<std::mutex> lock(mutex_);
    if (hidden_volume_ || !container_.writable()) throw CoreException(CoreError::kReadOnlySource);
    if (protected_hidden_range_.has_value()) {
        throw CoreException(CoreError::kHiddenVolumeRisk);
    }
    if (!analyzed_hidden_capacity_.has_value()) {
        throw CoreException(CoreError::kHiddenVolumeRisk);
    }
    const HiddenVolumeCapacity capacity = *analyzed_hidden_capacity_;
    if (request.container_size > capacity.maximum_size ||
        capacity.encrypted_area_offset % request.sector_size != 0 ||
        capacity.encrypted_area_offset % kSectorBytes != 0) {
        throw CoreException(CoreError::kHiddenVolumeRisk);
    }
    const std::uint64_t container_size = container_.size();
    if (container_size < kHeaderGroupBytes * 2 ||
        capacity.encrypted_area_offset > container_size ||
        request.container_size > container_size - capacity.encrypted_area_offset) {
        throw CoreException(CoreError::kCorruptHeader);
    }

    // A failed creation must never leave a decryptable old hidden header.
    std::array<std::uint8_t, kVeraCryptHeaderSize> invalid_header {};
    container_.WriteAt(kHiddenHeaderOffset, invalid_header.data(), invalid_header.size());
    container_.WriteAt(container_size - kHiddenHeaderOffset, invalid_header.data(), invalid_header.size());
    container_.Sync();

    std::vector<std::uint8_t> master_key(XtsKeyBytes(request.cipher));
    std::vector<std::uint8_t> template_bytes(kVeraCryptHeaderSize);
    FillSecureRandom(master_key.data(), master_key.size());
    FillSecureRandom(template_bytes.data(), template_bytes.size());
    try {
        template_bytes[64] = 'V'; template_bytes[65] = 'E'; template_bytes[66] = 'R'; template_bytes[67] = 'A';
        WriteBigEndianU16(template_bytes.data() + 68, kCreatedHeaderVersion);
        WriteBigEndianU16(template_bytes.data() + 70, kCreatedRequiredProgramVersion);
        WriteBigEndianU64(template_bytes.data() + 92, 0);
        WriteBigEndianU64(template_bytes.data() + 100, request.container_size);
        WriteBigEndianU64(template_bytes.data() + 108, capacity.encrypted_area_offset);
        WriteBigEndianU64(template_bytes.data() + 116, request.container_size);
        WriteBigEndianU32(template_bytes.data() + 124, 0);
        WriteBigEndianU32(template_bytes.data() + 128, request.sector_size);
        std::copy(master_key.begin(), master_key.end(), template_bytes.begin() + kKeyAreaOffset);
        WriteBigEndianU32(template_bytes.data() + kKeyAreaCrcOffset,
                          Crc32(template_bytes.data() + kKeyAreaOffset, kKeyAreaSize));
        WriteBigEndianU32(template_bytes.data() + kHeaderCrcOffset,
                          Crc32(template_bytes.data() + kVeraCryptHeaderSaltSize,
                                kHeaderCrcOffset - kVeraCryptHeaderSaltSize));

        HeaderMetadata metadata {
                kCreatedHeaderVersion, kCreatedRequiredProgramVersion, 0, request.container_size, request.container_size,
                capacity.encrypted_area_offset, 0, request.sector_size,
        };
        OpenedVolume opened {
                metadata, request.cipher, request.kdf, false, true, kHiddenHeaderOffset,
                container_size - kHiddenHeaderOffset, SecureBytes(std::move(master_key)), SecureBytes(std::move(template_bytes)),
        };
        auto hidden = std::make_shared<NativeVolumeSession>(container_.Duplicate(), std::move(opened), std::nullopt);
        std::array<std::uint8_t, 128 * 1024> initialization {};
        for (std::uint64_t offset = 0; offset < request.container_size;) {
            if (!progress(1, offset, request.container_size)) throw CoreException(CoreError::kCancelled);
            const std::size_t count = static_cast<std::size_t>(std::min<std::uint64_t>(
                    initialization.size(), request.container_size - offset));
            FillSecureRandom(initialization.data(), count);
            hidden->Write(offset, initialization.data(), count);
            std::fill(initialization.begin(), initialization.begin() + count, 0);
            offset += count;
            if (!progress(1, offset, request.container_size)) throw CoreException(CoreError::kCancelled);
        }
        if (!progress(2, 0, 0)) throw CoreException(CoreError::kCancelled);
        hidden->FormatFatFs(request.filesystem == CreateFileSystem::kExFat ? FatFsType::kExFat : FatFsType::kFat);
        if (!progress(3, request.container_size, request.container_size)) throw CoreException(CoreError::kCancelled);
        SecureBytes processed_password = ApplyVeraCryptKeyfiles(request.password, keyfiles);
        hidden->FinalizeCreatedHeaders(processed_password, request.pim);

        // From now on outer writes which reach this range must latch the
        // existing hidden-volume protection failure rather than corrupt data.
        protected_hidden_range_ = EncryptedRange {capacity.encrypted_area_offset, request.container_size};
        analyzed_hidden_capacity_.reset();
        return hidden;
    } catch (...) {
        std::fill(master_key.begin(), master_key.end(), 0);
        std::fill(template_bytes.begin(), template_bytes.end(), 0);
        analyzed_hidden_capacity_.reset();
        throw;
    }
}

void NativeVolumeSession::FinalizeCreatedHeaders(const SecureBytes& processed_password, std::int32_t pim) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!container_.writable()) throw CoreException(CoreError::kReadOnlySource);
    auto backup_header = EncryptHeader(decrypted_header_, processed_password, kdf_, pim, cipher_);
    try {
        container_.WriteAt(backup_header_offset_, backup_header.data(), backup_header.size());
        container_.Sync();
        auto primary_header = EncryptHeader(decrypted_header_, processed_password, kdf_, pim, cipher_);
        try {
            container_.WriteAt(primary_header_offset_, primary_header.data(), primary_header.size());
            container_.Sync();
            std::vector<std::uint8_t> next_template(primary_header.begin(), primary_header.end());
            std::copy(decrypted_header_.data() + kVeraCryptHeaderSaltSize,
                      decrypted_header_.data() + decrypted_header_.size(),
                      next_template.begin() + kVeraCryptHeaderSaltSize);
            decrypted_header_ = SecureBytes(std::move(next_template));
            used_backup_header_ = false;
        } catch (...) {
            std::fill(primary_header.begin(), primary_header.end(), 0);
            throw;
        }
        std::fill(primary_header.begin(), primary_header.end(), 0);
    } catch (...) {
        std::fill(backup_header.begin(), backup_header.end(), 0);
        throw;
    }
    std::fill(backup_header.begin(), backup_header.end(), 0);
}

void NativeVolumeSession::BackupHeader(
        int output_fd, const OpenRequest& options, const std::vector<FdRandomAccess>& keyfiles) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (output_fd < 0) throw std::invalid_argument("Header backup destination descriptor is invalid");
    if (container_.SameFile(output_fd)) {
        throw std::invalid_argument("Header backup destination must differ from the source container");
    }
    if (options.protect_hidden_volume || !options.writable) {
        throw std::invalid_argument("Header backup requires a writable non-protection credential request");
    }
    if (options.open_hidden_volume != hidden_volume_) {
        throw std::invalid_argument("Header backup volume type does not match the session");
    }
    const CipherHint requested_cipher = options.cipher == CipherHint::kAuto ? cipher_ : options.cipher;
    const KdfHint requested_kdf = options.kdf == KdfHint::kAuto ? kdf_ : options.kdf;
    if (requested_cipher != cipher_ || requested_kdf != kdf_) {
        throw std::invalid_argument("Header backup credentials must use the session cipher and KDF");
    }
    SecureBytes processed_password = ApplyVeraCryptKeyfiles(options.password, keyfiles);
    const std::uint64_t current_header_offset = used_backup_header_ ? backup_header_offset_ : primary_header_offset_;
    VerifyBackupCredentials(container_, decrypted_header_, current_header_offset, processed_password,
                            requested_kdf, options.pim, requested_cipher);
    auto refreshed_header = EncryptHeader(decrypted_header_, processed_password, requested_kdf, options.pim, cipher_);
    auto destination = FdRandomAccess::Open(output_fd, true);
    std::array<std::uint8_t, kHeaderGroupBytes> buffer {};
    try {
        // A backup contains only the credential-verified normal or hidden
        // header. The other slot is fresh random data, never copied without
        // its own credentials.
        FillSecureRandom(buffer.data(), buffer.size());
        const std::size_t selected_offset = hidden_volume_ ? kHiddenHeaderOffset : 0;
        std::copy(refreshed_header.begin(), refreshed_header.end(), buffer.begin() + selected_offset);
        destination.Resize(0);
        destination.WriteAt(0, buffer.data(), buffer.size());
        destination.Sync();
    } catch (...) {
        std::fill(buffer.begin(), buffer.end(), 0);
        std::fill(refreshed_header.begin(), refreshed_header.end(), 0);
        throw;
    }
    std::fill(buffer.begin(), buffer.end(), 0);
    std::fill(refreshed_header.begin(), refreshed_header.end(), 0);
}

void NativeVolumeSession::ChangeCredentials(const OpenRequest& options, const std::vector<FdRandomAccess>& keyfiles) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!container_.writable()) throw std::runtime_error("Volume is read-only");
    if (options.protect_hidden_volume) throw std::invalid_argument("Hidden-volume protection credentials cannot update a header");
    if (options.open_hidden_volume != hidden_volume_) throw std::invalid_argument("Credential update volume type does not match the session");
    const CipherHint target_cipher = options.cipher == CipherHint::kAuto ? cipher_ : options.cipher;
    if (target_cipher != cipher_) throw std::invalid_argument("Changing a VeraCrypt data cipher requires re-encrypting the volume");
    const KdfHint target_kdf = options.kdf == KdfHint::kAuto ? kdf_ : options.kdf;
    SecureBytes processed_password = ApplyVeraCryptKeyfiles(options.password, keyfiles);
    auto backup_header = EncryptHeader(decrypted_header_, processed_password, target_kdf, options.pim, cipher_);
    try {
        container_.WriteAt(backup_header_offset_, backup_header.data(), backup_header.size());
        container_.Sync();
        auto primary_header = EncryptHeader(decrypted_header_, processed_password, target_kdf, options.pim, cipher_);
        try {
            container_.WriteAt(primary_header_offset_, primary_header.data(), primary_header.size());
            container_.Sync();
            std::vector<std::uint8_t> next_template(primary_header.begin(), primary_header.end());
            // Store plaintext metadata for future updates: reconstitute it
            // from the old template, retaining only the newly generated salt.
            std::copy(primary_header.begin(), primary_header.begin() + kVeraCryptHeaderSaltSize, next_template.begin());
            std::copy(decrypted_header_.data() + kVeraCryptHeaderSaltSize,
                      decrypted_header_.data() + decrypted_header_.size(), next_template.begin() + kVeraCryptHeaderSaltSize);
            decrypted_header_ = SecureBytes(std::move(next_template));
            kdf_ = target_kdf;
            used_backup_header_ = false;
        } catch (...) {
            std::fill(primary_header.begin(), primary_header.end(), 0);
            throw;
        }
        std::fill(primary_header.begin(), primary_header.end(), 0);
    } catch (...) {
        std::fill(backup_header.begin(), backup_header.end(), 0);
        throw;
    }
    std::fill(backup_header.begin(), backup_header.end(), 0);
}

}  // namespace vc_core
