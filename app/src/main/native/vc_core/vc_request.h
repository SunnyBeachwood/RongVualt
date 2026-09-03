#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace vc_core {

constexpr std::int32_t kVeraCryptMaximumPim = 2147468;

class SecureBytes final {
public:
    SecureBytes() = default;
    explicit SecureBytes(std::vector<std::uint8_t> value);
    SecureBytes(const SecureBytes&) = delete;
    SecureBytes& operator=(const SecureBytes&) = delete;
    SecureBytes(SecureBytes&& other) noexcept;
    SecureBytes& operator=(SecureBytes&& other) noexcept;
    ~SecureBytes();

    const std::uint8_t* data() const noexcept { return value_.data(); }
    std::uint8_t* mutable_data() noexcept { return value_.data(); }
    std::size_t size() const noexcept { return value_.size(); }

private:
    void clear() noexcept;
    std::vector<std::uint8_t> value_;
};

enum class CipherHint : std::uint8_t {
    kAuto = 0,
    kAes = 1,
    kSerpent = 2,
    kTwofish = 3,
    kCamellia = 4,
    kKuznyechik = 5,
    kTwofishAes = 6,
    kSerpentTwofishAes = 7,
    kAesSerpent = 8,
    kAesTwofishSerpent = 9,
    kSerpentTwofish = 10,
    kKuznyechikCamellia = 11,
    kTwofishKuznyechik = 12,
    kSerpentCamellia = 13,
    kAesKuznyechik = 14,
    kCamelliaSerpentKuznyechik = 15,
};

enum class KdfHint : std::uint8_t {
    kAuto = 0,
    kPbkdf2HmacSha512 = 1,
    kPbkdf2HmacSha256 = 2,
    kPbkdf2HmacBlake2s = 3,
    kPbkdf2HmacWhirlpool = 4,
    kPbkdf2HmacStreebog = 5,
    kArgon2id = 6,
};

struct OpenRequest final {
    bool open_hidden_volume = false;
    /** Try a normal header first, then a hidden header. Opened sessions stay concrete. */
    bool auto_detect_volume = false;
    bool writable = false;
    std::int32_t pim = 0;
    CipherHint cipher = CipherHint::kAuto;
    KdfHint kdf = KdfHint::kAuto;
    std::uint16_t keyfile_count = 0;
    SecureBytes password;
    bool protect_hidden_volume = false;
    std::int32_t protection_pim = 0;
    std::uint16_t protection_keyfile_count = 0;
    SecureBytes protection_password;
};

enum class CreateFileSystem : std::uint8_t {
    kFat = 1,
    kExFat = 2,
};

/** Native-only creation parameters. Container locations never cross JNI. */
struct CreateRequest final {
    std::uint64_t container_size = 0;
    bool hidden_volume = false;
    std::int32_t pim = 0;
    CipherHint cipher = CipherHint::kAuto;
    KdfHint kdf = KdfHint::kAuto;
    CreateFileSystem filesystem = CreateFileSystem::kExFat;
    std::uint32_t sector_size = 512;
    std::uint16_t keyfile_count = 0;
    SecureBytes password;
};

/** Parses the versioned Kotlin NativeRequestCodec open request. */
OpenRequest ParseOpenRequest(const std::uint8_t* bytes, std::size_t size);

/** Parses the versioned Kotlin NativeRequestCodec create request. */
CreateRequest ParseCreateRequest(const std::uint8_t* bytes, std::size_t size);

}  // namespace vc_core
