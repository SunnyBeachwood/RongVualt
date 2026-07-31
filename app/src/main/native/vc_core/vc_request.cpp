#include "vc_request.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace vc_core {
namespace {

constexpr std::uint32_t kMagic = 0x56435251;  // VCRQ
constexpr std::uint16_t kVersion = 2;
constexpr std::uint8_t kOpenOperation = 1;
constexpr std::uint8_t kCreateOperation = 2;
constexpr std::uint8_t kKnownFlags = 0x0F;
constexpr std::size_t kHeaderSize = 16;
constexpr std::size_t kMaxPasswordBytes = 128;
constexpr std::size_t kMaxKeyfiles = 1024;

class Reader final {
public:
    Reader(const std::uint8_t* data, std::size_t size) : current_(data), end_(data + size) {
        if (data == nullptr && size != 0) throw std::invalid_argument("Null request data");
    }

    std::uint8_t ReadU8() {
        Require(1);
        return *current_++;
    }

    std::uint16_t ReadU16() {
        const std::uint16_t high = ReadU8();
        const std::uint16_t low = ReadU8();
        return static_cast<std::uint16_t>((high << 8) | low);
    }

    std::uint32_t ReadU32() {
        const std::uint32_t one = ReadU8();
        const std::uint32_t two = ReadU8();
        const std::uint32_t three = ReadU8();
        const std::uint32_t four = ReadU8();
        return (one << 24) | (two << 16) | (three << 8) | four;
    }

    std::uint64_t ReadU64() {
        const std::uint64_t high = ReadU32();
        const std::uint64_t low = ReadU32();
        return (high << 32) | low;
    }

    SecureBytes ReadPassword() {
        const std::size_t length = ReadU16();
        if (length > kMaxPasswordBytes) throw std::invalid_argument("Password exceeds VeraCrypt limit");
        Require(length);
        std::vector<std::uint8_t> value(current_, current_ + length);
        current_ += length;
        return SecureBytes(std::move(value));
    }

    bool at_end() const noexcept { return current_ == end_; }

private:
    void Require(std::size_t count) const {
        if (count > static_cast<std::size_t>(end_ - current_)) {
            throw std::invalid_argument("Truncated native request");
        }
    }

    const std::uint8_t* current_;
    const std::uint8_t* end_;
};

bool IsKnownCipher(std::uint8_t value) { return value <= static_cast<std::uint8_t>(CipherHint::kCamelliaSerpentKuznyechik); }
bool IsKnownKdf(std::uint8_t value) { return value <= static_cast<std::uint8_t>(KdfHint::kArgon2id); }
bool IsCreateFileSystem(std::uint8_t value) { return value == static_cast<std::uint8_t>(CreateFileSystem::kFat) || value == static_cast<std::uint8_t>(CreateFileSystem::kExFat); }

std::int32_t ToSigned(std::uint32_t value) {
    if (value > static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max())) {
        throw std::invalid_argument("PIM is outside the supported range");
    }
    return static_cast<std::int32_t>(value);
}

}  // namespace

SecureBytes::SecureBytes(std::vector<std::uint8_t> value) : value_(std::move(value)) {}

SecureBytes::SecureBytes(SecureBytes&& other) noexcept : value_(std::move(other.value_)) {}

SecureBytes& SecureBytes::operator=(SecureBytes&& other) noexcept {
    if (this != &other) {
        clear();
        value_ = std::move(other.value_);
    }
    return *this;
}

SecureBytes::~SecureBytes() { clear(); }

void SecureBytes::clear() noexcept {
    volatile std::uint8_t* pointer = value_.empty() ? nullptr : value_.data();
    for (std::size_t index = 0; pointer != nullptr && index < value_.size(); ++index) pointer[index] = 0;
    value_.clear();
}

OpenRequest ParseOpenRequest(const std::uint8_t* bytes, std::size_t size) {
    if (size < kHeaderSize) throw std::invalid_argument("Native request is too short");
    Reader reader(bytes, size);
    if (reader.ReadU32() != kMagic) throw std::invalid_argument("Unknown native request magic");
    const std::uint16_t version = reader.ReadU16();
    if (version != 1 && version != kVersion) throw std::invalid_argument("Unsupported native request version");
    if (reader.ReadU8() != kOpenOperation) throw std::invalid_argument("Unexpected native request operation");

    const std::uint8_t flags = reader.ReadU8();
    if ((flags & ~kKnownFlags) != 0) throw std::invalid_argument("Unknown native request flags");

    OpenRequest request;
    request.open_hidden_volume = (flags & 0x01) != 0;
    request.auto_detect_volume = (flags & 0x08) != 0;
    if (request.auto_detect_volume && request.open_hidden_volume) {
        throw std::invalid_argument("Automatic volume detection cannot select a hidden volume");
    }
    request.writable = (flags & 0x02) != 0;
    request.protect_hidden_volume = (flags & 0x04) != 0;
    if (request.protect_hidden_volume && !request.writable) {
        throw std::invalid_argument("Hidden-volume protection requires writable access");
    }

    request.pim = ToSigned(reader.ReadU32());
    if (request.pim < 0) throw std::invalid_argument("PIM cannot be negative");
    const std::uint8_t cipher = reader.ReadU8();
    const std::uint8_t kdf = reader.ReadU8();
    if (!IsKnownCipher(cipher) || !IsKnownKdf(kdf)) throw std::invalid_argument("Unknown cipher or KDF hint");
    request.cipher = static_cast<CipherHint>(cipher);
    request.kdf = static_cast<KdfHint>(kdf);
    request.keyfile_count = reader.ReadU16();
    if (request.keyfile_count > kMaxKeyfiles) throw std::invalid_argument("Too many keyfiles");
    request.password = reader.ReadPassword();

    if (request.protect_hidden_volume) {
        request.protection_pim = ToSigned(reader.ReadU32());
        if (request.protection_pim < 0) throw std::invalid_argument("Hidden-volume protection PIM cannot be negative");
        request.protection_keyfile_count = reader.ReadU16();
        if (request.protection_keyfile_count > kMaxKeyfiles) throw std::invalid_argument("Too many hidden-volume keyfiles");
        request.protection_password = reader.ReadPassword();
    }
    if (!reader.at_end()) throw std::invalid_argument("Trailing native request data");
    return request;
}

CreateRequest ParseCreateRequest(const std::uint8_t* bytes, std::size_t size) {
    if (size < kHeaderSize + 4 + 1 + 1 + 1 + 4 + 2 + 2) {
        throw std::invalid_argument("Native create request is too short");
    }
    Reader reader(bytes, size);
    if (reader.ReadU32() != kMagic) throw std::invalid_argument("Unknown native request magic");
    if (reader.ReadU16() != kVersion) throw std::invalid_argument("Unsupported native request version");
    if (reader.ReadU8() != kCreateOperation) throw std::invalid_argument("Unexpected native request operation");
    const std::uint8_t flags = reader.ReadU8();
    if ((flags & ~0x01U) != 0) throw std::invalid_argument("Unknown native create request flags");

    CreateRequest request;
    request.container_size = reader.ReadU64();
    request.hidden_volume = (flags & 0x01U) != 0;
    request.pim = ToSigned(reader.ReadU32());
    if (request.pim < 0) throw std::invalid_argument("PIM cannot be negative");
    const std::uint8_t cipher = reader.ReadU8();
    const std::uint8_t kdf = reader.ReadU8();
    const std::uint8_t filesystem = reader.ReadU8();
    if (!IsKnownCipher(cipher) || !IsKnownKdf(kdf) || !IsCreateFileSystem(filesystem)) {
        throw std::invalid_argument("Unknown create cipher, KDF, or filesystem");
    }
    request.cipher = static_cast<CipherHint>(cipher);
    request.kdf = static_cast<KdfHint>(kdf);
    request.filesystem = static_cast<CreateFileSystem>(filesystem);
    request.sector_size = reader.ReadU32();
    if (request.sector_size != 512 && request.sector_size != 1024 &&
        request.sector_size != 2048 && request.sector_size != 4096) {
        throw std::invalid_argument("Unsupported VeraCrypt logical sector size");
    }
    request.keyfile_count = reader.ReadU16();
    if (request.keyfile_count > kMaxKeyfiles) throw std::invalid_argument("Too many keyfiles");
    request.password = reader.ReadPassword();
    if (!reader.at_end()) throw std::invalid_argument("Trailing native create request data");
    return request;
}

}  // namespace vc_core
