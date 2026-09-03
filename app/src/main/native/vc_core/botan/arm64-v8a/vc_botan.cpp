/*
* Botan 3.12.0 Amalgamation
* (C) 1999-2023 The Botan Authors
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "vc_botan.h"

#include <array>
#include <bit>
#include <chrono>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <functional>
#include <iosfwd>
#include <iterator>
#include <locale>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>


namespace Botan {

/**
* AES-128
*/
class AES_128 final : public Block_Cipher_Fixed_Params<16, 16> {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;

      std::string provider() const override;

      std::string name() const override { return "AES-128"; }

      std::unique_ptr<BlockCipher> new_object() const override { return std::make_unique<AES_128>(); }

      size_t parallelism() const override;

      bool has_keying_material() const override;

   private:
      void key_schedule(std::span<const uint8_t> key) override;

#if defined(BOTAN_HAS_AES_VPERM)
      void vperm_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void vperm_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void vperm_key_schedule(const uint8_t key[], size_t length);
#endif

#if defined(BOTAN_HAS_AES_NI)
      void aesni_key_schedule(const uint8_t key[], size_t length);
#endif

#if defined(BOTAN_HAS_AES_POWER8) || defined(BOTAN_HAS_AES_ARMV8) || defined(BOTAN_HAS_AES_NI)
      void hw_aes_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void hw_aes_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
#endif

#if defined(BOTAN_HAS_AES_VAES)
      void x86_vaes_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void x86_vaes_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
#endif

      secure_vector<uint32_t> m_EK, m_DK;
};

/**
* AES-192
*/
class AES_192 final : public Block_Cipher_Fixed_Params<16, 24> {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;

      std::string provider() const override;

      std::string name() const override { return "AES-192"; }

      std::unique_ptr<BlockCipher> new_object() const override { return std::make_unique<AES_192>(); }

      size_t parallelism() const override;
      bool has_keying_material() const override;

   private:
#if defined(BOTAN_HAS_AES_VPERM)
      void vperm_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void vperm_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void vperm_key_schedule(const uint8_t key[], size_t length);
#endif

#if defined(BOTAN_HAS_AES_NI)
      void aesni_key_schedule(const uint8_t key[], size_t length);
#endif

#if defined(BOTAN_HAS_AES_POWER8) || defined(BOTAN_HAS_AES_ARMV8) || defined(BOTAN_HAS_AES_NI)
      void hw_aes_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void hw_aes_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
#endif

#if defined(BOTAN_HAS_AES_VAES)
      void x86_vaes_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void x86_vaes_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
#endif

      void key_schedule(std::span<const uint8_t> key) override;

      secure_vector<uint32_t> m_EK, m_DK;
};

/**
* AES-256
*/
class AES_256 final : public Block_Cipher_Fixed_Params<16, 32> {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;

      std::string provider() const override;

      std::string name() const override { return "AES-256"; }

      std::unique_ptr<BlockCipher> new_object() const override { return std::make_unique<AES_256>(); }

      size_t parallelism() const override;
      bool has_keying_material() const override;

   private:
#if defined(BOTAN_HAS_AES_VPERM)
      void vperm_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void vperm_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void vperm_key_schedule(const uint8_t key[], size_t length);
#endif

#if defined(BOTAN_HAS_AES_NI)
      void aesni_key_schedule(const uint8_t key[], size_t length);
#endif

#if defined(BOTAN_HAS_AES_POWER8) || defined(BOTAN_HAS_AES_ARMV8) || defined(BOTAN_HAS_AES_NI)
      void hw_aes_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void hw_aes_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
#endif

#if defined(BOTAN_HAS_AES_VAES)
      void x86_vaes_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void x86_vaes_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
#endif

      void key_schedule(std::span<const uint8_t> key) override;

      secure_vector<uint32_t> m_EK, m_DK;
};

}  // namespace Botan

namespace Botan {

/**
 * Helper class to ease unmarshalling of concatenated fixed-length values
 */
class BufferSlicer final {
   public:
      explicit BufferSlicer(std::span<const uint8_t> buffer) : m_remaining(buffer) {}

      template <concepts::contiguous_container ContainerT>
      auto copy(const size_t count) {
         const auto result = take(count);
         return ContainerT(result.begin(), result.end());
      }

      auto copy_as_vector(const size_t count) { return copy<std::vector<uint8_t>>(count); }

      auto copy_as_secure_vector(const size_t count) { return copy<secure_vector<uint8_t>>(count); }

      std::span<const uint8_t> take(const size_t count) {
         BOTAN_STATE_CHECK(remaining() >= count);
         auto result = m_remaining.first(count);
         m_remaining = m_remaining.subspan(count);
         return result;
      }

      template <size_t count>
      std::span<const uint8_t, count> take() {
         BOTAN_STATE_CHECK(remaining() >= count);
         auto result = m_remaining.first<count>();
         m_remaining = m_remaining.subspan(count);
         return result;
      }

      template <concepts::contiguous_strong_type T>
      StrongSpan<const T> take(const size_t count) {
         return StrongSpan<const T>(take(count));
      }

      uint8_t take_byte() { return take(1)[0]; }

      void copy_into(std::span<uint8_t> sink) {
         const auto data = take(sink.size());
         std::copy(data.begin(), data.end(), sink.begin());
      }

      void skip(const size_t count) { take(count); }

      size_t remaining() const { return m_remaining.size(); }

      bool empty() const { return m_remaining.empty(); }

   private:
      std::span<const uint8_t> m_remaining;
};

}  // namespace Botan

namespace Botan {

/**
* Zeroize memory contents in a way that a compiler should not elide,
* using some system specific technique.
*
* Use this function to scrub memory just before deallocating it, or on
* a stack buffer before returning from the function.
*
* @param ptr a pointer to memory to scrub
* @param n the number of bytes pointed to by ptr
*/
BOTAN_TEST_API void secure_zeroize_buffer(void* ptr, size_t n);

/**
 * @param buf a pointer to the start of the region
 * @param n the number of elements in buf
 */
template <std::unsigned_integral T>
inline void zeroize_buffer(T buf[], size_t n) {
   if(n > 0) {
      std::memset(buf, 0, sizeof(T) * n);
   }
}

template <std::unsigned_integral T>
inline void unchecked_copy_memory(T* out, const T* in, size_t n) {
   if(in != nullptr && out != nullptr && n > 0) {
      std::memmove(out, in, sizeof(T) * n);
   }
}

/**
* Return true if any of the provided arguments are null
*/
template <typename... Ptrs>
bool any_null_pointers(Ptrs... ptr) {
   static_assert((... && std::is_pointer_v<Ptrs>), "All arguments must be pointers");
   return (... || (ptr == nullptr));
}

inline std::span<const uint8_t> as_span_of_bytes(const char* s, size_t len) {
   const uint8_t* b = reinterpret_cast<const uint8_t*>(s);
   return std::span{b, len};
}

inline std::span<const uint8_t> as_span_of_bytes(const std::string& s) {
   return as_span_of_bytes(s.data(), s.size());
}

inline std::span<const uint8_t> as_span_of_bytes(std::string_view s) {
   return as_span_of_bytes(s.data(), s.size());
}

inline std::span<const uint8_t> cstr_as_span_of_bytes(const char* s) {
   return as_span_of_bytes(s, std::strlen(s));
}

inline std::string bytes_to_string(std::span<const uint8_t> bytes) {
   return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

}  // namespace Botan

namespace Botan {

/**
 * Defines the strategy for handling the final block of input data in the
 * handle_unaligned_data() method of the AlignmentBuffer<>.
 *
 * - is_not_special:   the final block is treated like any other block
 * - must_be_deferred: the final block is not emitted while bulk processing (typically add_data())
 *                     but is deferred until manually consumed (typically final_result())
 *
 * The AlignmentBuffer<> assumes data to be "the final block" if no further
 * input data is available in the BufferSlicer<>. This might result in some
 * performance overhead when using the must_be_deferred strategy.
 */
enum class AlignmentBufferFinalBlock : uint8_t {
   is_not_special = 0,
   must_be_deferred = 1,
};

/**
 * @brief Alignment buffer helper
 *
 * Many algorithms have an intrinsic block size in which they consume input
 * data. When streaming arbitrary data chunks to such algorithms we must store
 * some data intermittently to honor the algorithm's alignment requirements.
 *
 * This helper encapsulates such an alignment buffer. The API of this class is
 * designed to minimize user errors in the algorithm implementations. Therefore,
 * it is strongly opinionated on its use case. Don't try to use it for anything
 * but the described circumstance.
 *
 * @tparam T                     the element type of the internal buffer
 * @tparam BLOCK_SIZE            the buffer size to use for the alignment buffer
 * @tparam FINAL_BLOCK_STRATEGY  defines whether the final input data block is
 *                               retained in handle_unaligned_data() and must be
 *                               manually consumed
 */
template <typename T,
          size_t BLOCK_SIZE,
          AlignmentBufferFinalBlock FINAL_BLOCK_STRATEGY = AlignmentBufferFinalBlock::is_not_special>
   requires(BLOCK_SIZE > 0)
class AlignmentBuffer final {
   public:
      AlignmentBuffer() = default;

      ~AlignmentBuffer() { secure_zeroize_buffer(m_buffer.data(), sizeof(T) * m_buffer.size()); }

      AlignmentBuffer(const AlignmentBuffer& other) = default;
      AlignmentBuffer(AlignmentBuffer&& other) noexcept = default;
      AlignmentBuffer& operator=(const AlignmentBuffer& other) = default;
      AlignmentBuffer& operator=(AlignmentBuffer&& other) noexcept = default;

      void clear() {
         zeroize_buffer(m_buffer.data(), m_buffer.size());
         m_position = 0;
      }

      /**
       * Fills the currently unused bytes of the buffer with zero bytes
       */
      void fill_up_with_zeros() {
         if(!ready_to_consume()) {
            zeroize_buffer(&m_buffer[m_position], elements_until_alignment());
            m_position = m_buffer.size();
         }
      }

      /**
       * Appends the provided @p elements to the buffer. The user has to make
       * sure that @p elements fits in the remaining capacity of the buffer.
       */
      void append(std::span<const T> elements) {
         BOTAN_ASSERT_NOMSG(elements.size() <= elements_until_alignment());
         std::copy(elements.begin(), elements.end(), m_buffer.begin() + m_position);
         m_position += elements.size();
      }

      /**
       * Allows direct modification of the first @p elements in the buffer.
       * This is a low-level accessor that neither takes the buffer's current
       * capacity into account nor does it change the internal cursor.
       * Beware not to overwrite unconsumed bytes.
       */
      std::span<T> directly_modify_first(size_t elements) {
         BOTAN_ASSERT_NOMSG(size() >= elements);
         return std::span(m_buffer).first(elements);
      }

      /**
       * Allows direct modification of the last @p elements in the buffer.
       * This is a low-level accessor that neither takes the buffer's current
       * capacity into account nor does it change the internal cursor.
       * Beware not to overwrite unconsumed bytes.
       */
      std::span<T> directly_modify_last(size_t elements) {
         BOTAN_ASSERT_NOMSG(size() >= elements);
         return std::span(m_buffer).last(elements);
      }

      /**
       * Once the buffer reached alignment, this can be used to consume as many
       * input bytes from the given @p slider as possible. The output always
       * contains data elements that are a multiple of the intrinsic block size.
       *
       * @returns a view onto the aligned data from @p slicer and the number of
       *          full blocks that are represented by this view.
       */
      [[nodiscard]] std::tuple<std::span<const uint8_t>, size_t> aligned_data_to_process(BufferSlicer& slicer) const {
         BOTAN_ASSERT_NOMSG(in_alignment());

         // When the final block is to be deferred, the last block must not be
         // selected for processing if there is no (unaligned) extra input data.
         const size_t defer = (defers_final_block()) ? 1 : 0;
         const size_t full_blocks_to_process = (slicer.remaining() - defer) / m_buffer.size();
         return {slicer.take(full_blocks_to_process * m_buffer.size()), full_blocks_to_process};
      }

      /**
       * Once the buffer reached alignment, this can be used to consume full
       * blocks from the input data represented by @p slicer.
       *
       * @returns a view onto the next full block from @p slicer or std::nullopt
       *          if not enough data is available in @p slicer.
       */
      [[nodiscard]] std::optional<std::span<const uint8_t>> next_aligned_block_to_process(BufferSlicer& slicer) const {
         BOTAN_ASSERT_NOMSG(in_alignment());

         // When the final block is to be deferred, the last block must not be
         // selected for processing if there is no (unaligned) extra input data.
         const size_t defer = (defers_final_block()) ? 1 : 0;
         if(slicer.remaining() < m_buffer.size() + defer) {
            return std::nullopt;
         }

         return slicer.take(m_buffer.size());
      }

      /**
       * Intermittently buffers potentially unaligned data provided in @p
       * slicer. If the internal buffer already contains some elements, data is
       * appended. Once a full block is collected, it is returned to the caller
       * for processing.
       *
       * @param slicer the input data source to be (partially) consumed
       * @returns a view onto a full block once enough data was collected, or
       *          std::nullopt if no full block is available yet
       */
      [[nodiscard]] std::optional<std::span<const T>> handle_unaligned_data(BufferSlicer& slicer) {
         // When the final block is to be deferred, we would need to store and
         // hold a buffer that contains exactly one block until more data is
         // passed or it is explicitly consumed.
         const size_t defer = (defers_final_block()) ? 1 : 0;

         if(in_alignment() && slicer.remaining() >= m_buffer.size() + defer) {
            // We are currently in alignment and the passed-in data source
            // contains enough data to benefit from aligned processing.
            // Therefore, we don't copy anything into the intermittent buffer.
            return std::nullopt;
         }

         // Fill the buffer with as much input data as needed to reach alignment
         // or until the input source is depleted.
         const auto elements_to_consume = std::min(m_buffer.size() - m_position, slicer.remaining());
         append(slicer.take(elements_to_consume));

         // If we collected enough data, we push out one full block. When
         // deferring the final block is enabled, we additionally check that
         // more input data is available to continue processing a consecutive
         // block.
         if(ready_to_consume() && (!defers_final_block() || !slicer.empty())) {
            return consume();
         } else {
            return std::nullopt;
         }
      }

      /**
       * Explicitly consume the currently collected block. It is the caller's
       * responsibility to ensure that the buffer is filled fully. After
       * consumption, the buffer is cleared and ready to collect new data.
       */
      [[nodiscard]] std::span<const T> consume() {
         BOTAN_ASSERT_NOMSG(ready_to_consume());
         m_position = 0;
         return m_buffer;
      }

      /**
       * Explicitly consumes however many bytes are currently stored in the
       * buffer. After consumption, the buffer is cleared and ready to collect
       * new data.
       */
      [[nodiscard]] std::span<const T> consume_partial() {
         const auto elements = elements_in_buffer();
         m_position = 0;
         return std::span(m_buffer).first(elements);
      }

      constexpr size_t size() const { return m_buffer.size(); }

      size_t elements_in_buffer() const { return m_position; }

      size_t elements_until_alignment() const { return m_buffer.size() - m_position; }

      /**
       * @returns true if the buffer is empty (i.e. contains no unaligned data)
       */
      bool in_alignment() const { return m_position == 0; }

      /**
       * @returns true if the buffer is full (i.e. a block is ready to be consumed)
       */
      bool ready_to_consume() const { return m_position == m_buffer.size(); }

      constexpr bool defers_final_block() const {
         return FINAL_BLOCK_STRATEGY == AlignmentBufferFinalBlock::must_be_deferred;
      }

   private:
      std::array<T, BLOCK_SIZE> m_buffer = {};
      size_t m_position = 0;
};

}  // namespace Botan



namespace Botan {

/**
 * Swap the byte order of an unsigned integer
 */
template <std::unsigned_integral T>
   requires(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8)
inline constexpr T reverse_bytes(T x) {
   if constexpr(sizeof(T) == 1) {
      return x;
   } else if constexpr(sizeof(T) == 2) {
#if BOTAN_COMPILER_HAS_BUILTIN(__builtin_bswap16)
      return static_cast<T>(__builtin_bswap16(x));
#else
      return static_cast<T>((x << 8) | (x >> 8));
#endif
   } else if constexpr(sizeof(T) == 4) {
#if BOTAN_COMPILER_HAS_BUILTIN(__builtin_bswap32)
      return static_cast<T>(__builtin_bswap32(x));
#else
      // MSVC at least recognizes this as a bswap
      return static_cast<T>(((x & 0x000000FF) << 24) | ((x & 0x0000FF00) << 8) | ((x & 0x00FF0000) >> 8) |
                            ((x & 0xFF000000) >> 24));
#endif
   } else if constexpr(sizeof(T) == 8) {
#if BOTAN_COMPILER_HAS_BUILTIN(__builtin_bswap64)
      return static_cast<T>(__builtin_bswap64(x));
#else
      uint32_t hi = static_cast<uint32_t>(x >> 32);
      uint32_t lo = static_cast<uint32_t>(x);

      hi = reverse_bytes(hi);
      lo = reverse_bytes(lo);

      return (static_cast<T>(lo) << 32) | hi;
#endif
   }
}

}  // namespace Botan

/**
* @file  target_info.h
*
* Automatically generated from
 * generated by tools/generate-botan-arm64.ps1 from Botan 3.12.0
*
* Target
*  - Compiler: C:\Users\QinQin\AppData\Local\Android\Sdk\ndk\28.2.13676358\toolchains\llvm\prebuilt\windows-x86_64\bin\aarch64-linux-android35-clang++.cmd -fstack-protector -pthread -std=c++20 -D_REENTRANT -O3
*  - Arch: arm64
*  - OS: android
*/

/* NOLINTBEGIN(*-macro-usage,*-macro-to-enum) */

/*
* Configuration
*/
#define BOTAN_CT_VALUE_BARRIER_USE_ASM

[[maybe_unused]] static constexpr bool OptimizeForSize = false;



/*
* Compiler Information
*/
#define BOTAN_BUILD_COMPILER_IS_CLANG

#define BOTAN_COMPILER_INVOCATION_STRING "C:\Users\QinQin\AppData\Local\Android\Sdk\ndk\28.2.13676358\toolchains\llvm\prebuilt\windows-x86_64\bin\aarch64-linux-android35-clang++.cmd -fstack-protector -pthread -O3"

#define BOTAN_USE_GCC_INLINE_ASM


/*
* External tool settings
*/




/*
* CPU feature information
*/
#define BOTAN_TARGET_ARCH "arm64"

#define BOTAN_TARGET_ARCH_IS_ARM64

#define BOTAN_TARGET_ARCH_IS_ARM_FAMILY

#define BOTAN_TARGET_ARCH_SUPPORTS_ARMV8CRYPTO
#define BOTAN_TARGET_ARCH_SUPPORTS_ARMV8SHA512
#define BOTAN_TARGET_ARCH_SUPPORTS_ARMV8SM3
#define BOTAN_TARGET_ARCH_SUPPORTS_ARMV8SM4
#define BOTAN_TARGET_ARCH_SUPPORTS_NEON


/*
* Operating system information
*/
#define BOTAN_TARGET_OS_IS_ANDROID

#define BOTAN_TARGET_OS_HAS_ARC4RANDOM
#define BOTAN_TARGET_OS_HAS_ATOMICS
#define BOTAN_TARGET_OS_HAS_CLOCK_GETTIME
#define BOTAN_TARGET_OS_HAS_DEV_RANDOM
#define BOTAN_TARGET_OS_HAS_GETAUXVAL
#define BOTAN_TARGET_OS_HAS_POSIX1
#define BOTAN_TARGET_OS_HAS_POSIX_MLOCK
#define BOTAN_TARGET_OS_HAS_PRCTL
#define BOTAN_TARGET_OS_HAS_SOCKETS
#define BOTAN_TARGET_OS_HAS_SYSTEM_CLOCK
#define BOTAN_TARGET_OS_HAS_THREAD_LOCAL


/*
* System paths
*/
#define BOTAN_INSTALL_PREFIX R"(C:\Users\QinQin\AppData\Local\Temp\eds-botan-83aa7d53-120c-4591-aec7-41360a4539df\Botan-3.12.0\eds-arm64-amalgamation\install)"
#define BOTAN_INSTALL_HEADER_DIR R"(include/botan-3)"
#define BOTAN_INSTALL_LIB_DIR R"(C:\Users\QinQin\AppData\Local\Temp\eds-botan-83aa7d53-120c-4591-aec7-41360a4539df\Botan-3.12.0\eds-arm64-amalgamation\install\lib)"
#define BOTAN_LIB_LINK ""
#define BOTAN_LINK_FLAGS "-fstack-protector -pthread"


/* NOLINTEND(*-macro-usage,*-macro-to-enum) */

namespace Botan::CT {

/**
* This function returns its argument, but (if called in a non-constexpr context)
* attempts to prevent the compiler from reasoning about the value or the possible
* range of values. Such optimizations have a way of breaking constant time code.
*
* The method that is use is decided at configuration time based on the target
* compiler and architecture (see `ct_value_barrier` blocks in `src/build-data/cc`).
* The decision can be overridden by the user with the configure.py option
* `--ct-value-barrier-type=`
*
* There are three options currently possible in the data files and with the
* option:
*
*  * `asm`: Use an inline assembly expression which (currently) prevents Clang
*    and GCC from optimizing based on the possible value of the input expression.
*
*  * `volatile`: Launder the input through a volatile variable. This is likely
*    to cause significant performance regressions since the value must be
*    actually stored and loaded back from memory each time.
*
*  * `none`: disable constant time barriers entirely. This is used
*    with MSVC, which is not known to perform optimizations that break
*    constant time code and which does not support GCC-style inline asm.
*
*/
template <std::unsigned_integral T>
   requires(!std::same_as<bool, T>)
constexpr inline T value_barrier(T x) {
   if(std::is_constant_evaluated()) {
      return x;
   } else {
#if defined(BOTAN_CT_VALUE_BARRIER_USE_ASM)
      /*
      * We may want a "stronger" statement such as
      *     asm volatile("" : "+r,m"(x) : : "memory);
      * (see https://theunixzoo.co.uk/blog/2021-10-14-preventing-optimisations.html)
      * however the current approach seems sufficient with current compilers,
      * and is minimally damaging with regards to degrading code generation.
      */
      asm("" : "+r"(x) : /* no input */);  // NOLINT(*-no-assembler)
      return x;
#elif defined(BOTAN_CT_VALUE_BARRIER_USE_VOLATILE)
      volatile T vx = x;
      return vx;
#else
      return x;
#endif
   }
}

}  // namespace Botan::CT

namespace Botan {

/**
* If top bit of arg is set, return |1| (all bits set). Otherwise return |0| (all bits unset)
*/
template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr T ct_expand_top_bit(T a) {
   const T top = CT::value_barrier<T>(a >> (sizeof(T) * 8 - 1));
   return static_cast<T>(0) - top;
}

/**
* If arg is zero, return |1|. Otherwise return |0|
*/
template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr T ct_is_zero(T x) {
   return ct_expand_top_bit<T>(~x & (x - 1));
}

/**
* If arg is zero, return the size_t `s`. Otherwise return the size_t zero.
*/
template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr size_t ct_if_is_zero_ret(T x, size_t s) {
   /*
   Similar to `return ct_is_zero(x) & s` but has to account for possibility that
   sizeof(T) is smaller than sizeof(size_t) which would lead to incomplete masking
   */
   const T a = ~x & (x - 1);
   const size_t a_top = static_cast<size_t>(CT::value_barrier<T>(a >> (sizeof(T) * 8 - 1)));
   const size_t mask = static_cast<size_t>(0) - a_top;
   return mask & s;
}

/**
* Power of 2 test. T should be an unsigned integer type
* @param arg an integer value
* @return true iff arg is 2^n for some n > 0
*/
template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr bool is_power_of_2(T arg) {
   return (arg != 0) && (arg != 1) && ((arg & static_cast<T>(arg - 1)) == 0);
}

/**
* Return the index of the highest set bit
* T is an unsigned integer type
* @param n an integer value
* @return index of the highest set bit in n
*/
template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr size_t high_bit(T n) {
   size_t hb = 0;

   for(size_t s = 8 * sizeof(T) / 2; s > 0; s /= 2) {
      // Equivalent to: ((n >> s) == 0) ? 0 : s;
      const size_t z = s - ct_if_is_zero_ret<T>(n >> s, s);
      hb += z;
      n >>= z;
   }

   hb += n;

   return hb;
}

/**
* Return the number of significant bytes in n
* @param n an integer value
* @return number of significant bytes in n
*/
template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr size_t significant_bytes(T n) {
   size_t b = 0;

   for(size_t s = 8 * sizeof(T) / 2; s >= 8; s /= 2) {
      // Equivalent to: ((n >> s) == 0) ? 0 : s;
      const size_t z = s - ct_if_is_zero_ret<T>(n >> s, s);
      b += z / 8;
      n >>= z;
   }

   b += (n != 0);

   return b;
}

/**
* Count the trailing zero bits in n
* @param n an integer value
* @return maximum x st 2^x divides n
*/
template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr size_t ctz(T n) {
   /*
   * If n == 0 then this function will compute 8*sizeof(T)-1, so
   * initialize lb to 1 if n == 0 to produce the expected result.
   */
   size_t lb = ct_if_is_zero_ret<T>(n, 1);

   for(size_t s = 8 * sizeof(T) / 2; s > 0; s /= 2) {
      const T range = (static_cast<T>(1) << s) - 1;
      // Equivalent to: ((n & range) == 0) ? s : 0;
      const size_t z = ct_if_is_zero_ret<T>(n & range, s);
      lb += z;
      n >>= z;
   }

   return lb;
}

template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr T floor_log2(T n) {
   BOTAN_ARG_CHECK(n != 0, "log2(0) is not defined");
   return static_cast<T>(high_bit(n) - 1);
}

template <std::unsigned_integral T>
constexpr uint8_t ceil_log2(T x)
   requires(sizeof(T) < 32)
{
   if(x >> (sizeof(T) * 8 - 1)) {
      return sizeof(T) * 8;
   }

   uint8_t result = 0;
   T compare = 1;

   while(compare < x) {
      compare <<= 1;
      result++;
   }

   return result;
}

/**
 * Ceil of an unsigned integer division. @p b must not be zero.
 *
 * @param a divident
 * @param b divisor
 *
 * @returns ceil(a/b)
 */
template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr T ceil_division(T a, T b) {
   return (a + b - 1) / b;
}

/**
 * Return the number of bytes necessary to contain @p bits bits.
 */
template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr T ceil_tobytes(T bits) {
   return (bits + 7) / 8;
}

// Potentially variable time ctz used for OCB
BOTAN_FORCE_INLINE constexpr size_t var_ctz64(uint64_t n) {
#if BOTAN_COMPILER_HAS_BUILTIN(__builtin_ctzll)
   if(n == 0) {
      return 64;
   }
   return __builtin_ctzll(n);
#else
   return ctz<uint64_t>(n);
#endif
}

template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr T bit_permute_step(T x, T mask, size_t shift) {
   /*
   See https://reflectionsonsecurity.wordpress.com/2014/05/11/efficient-bit-permutation-using-delta-swaps/
   and http://programming.sirrida.de/bit_perm.html
   */
   const T swap = ((x >> shift) ^ x) & mask;
   return (x ^ swap) ^ (swap << shift);
}

template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr void swap_bits(T& x, T& y, T mask, size_t shift) {
   const T swap = ((x >> shift) ^ y) & mask;
   x ^= swap << shift;
   y ^= swap;
}

/**
* Bitwise selection
*
* If mask is |1| returns a
* If mask is |0| returns b
* If mask is some other value returns a or b depending on the bit
*/
template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr T choose(T mask, T a, T b) {
   //return (mask & a) | (~mask & b);
   return (b ^ (mask & (a ^ b)));
}

template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr T majority(T a, T b, T c) {
   /*
   Considering each bit of a, b, c individually

   If a xor b is set, then c is the deciding vote.

   If a xor b is not set then either a and b are both set or both unset.
   In either case the value of c doesn't matter, and examining b (or a)
   allows us to determine which case we are in.
   */
   return choose(a ^ b, c, b);
}

/**
 * @returns the reversed bits in @p b.
 */
template <std::unsigned_integral T>
inline constexpr T ct_reverse_bits(T b) {
   auto extend = [](uint8_t m) -> T {
      T mask = 0;
      for(size_t i = 0; i < sizeof(T); ++i) {
         mask |= T(m) << i * 8;
      }
      return mask;
   };

   // First reverse bits in each byte...
   // From: https://stackoverflow.com/a/2602885
   b = (b & extend(0xF0)) >> 4 | (b & extend(0x0F)) << 4;
   b = (b & extend(0xCC)) >> 2 | (b & extend(0x33)) << 2;
   b = (b & extend(0xAA)) >> 1 | (b & extend(0x55)) << 1;

   // ... then swap the bytes
   return reverse_bytes(b);
}

/**
 * Calculates the number of 1-bits in an unsigned integer in constant-time.
 * This operation is also known as "population count" or hamming weight.
 *
 * Modern compilers will recognize this pattern and replace it by a hardware
 * instruction, if available. This is the SWAR (SIMD within a register)
 * algorithm. See: https://nimrod.blog/posts/algorithms-behind-popcount/#swar-algorithm
 *
 * Note: C++20 provides std::popcount(), but there's no guarantee that this
 *       is implemented in constant-time.
 *
 * @param x an unsigned integer
 * @returns the number of 1-bits in the provided value
 */
template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr uint8_t ct_popcount(T x) {
   constexpr size_t s = sizeof(T);
   static_assert(s <= 8, "T is not a suitable unsigned integer value");
   if constexpr(s == 8) {
      x = x - ((x >> 1) & 0x5555555555555555);
      x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
      x = (x + (x >> 4)) & 0xF0F0F0F0F0F0F0F;
      return (x * 0x101010101010101) >> 56;
   } else if constexpr(s == 4) {
      x = x - ((x >> 1) & 0x55555555);
      x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
      x = (x + (x >> 4)) & 0x0F0F0F0F;
      return (x * 0x01010101) >> 24;
   } else {
      // s < 4
      return ct_popcount(static_cast<uint32_t>(x));
   }
}

/**
* Compile-time polynomial multiplication in GF(2^8) modulo an irreducible
* polynomial POLY
*
* When T is larger than a byte, the function computes the product of each byte
* of T and returns the packed result.
*
* This function is intended only for use at compile-time, and in particular
* should not be used with secret inputs.
*
* TODO(Botan4) this function should be consteval, but that hits bugs in
* older versions of GCC and Clang.
*/
template <uint8_t POLY, std::unsigned_integral T>
constexpr T poly_mul(T x, uint8_t y) {
   // The constant 0x010101... as a T
   constexpr T lo_bit = (static_cast<T>(-1) / 255);

   // The constant 0x7F7F7F... as a T
   constexpr T mask = static_cast<T>(~(lo_bit << 7));

   constexpr T poly = POLY;

   T r = 0;
   while(x > 0 && y > 0) {
      if((y & 1) != 0) {
         r ^= x;
      }
      const T carry = ((x >> 7) & lo_bit) * poly;
      x = ((x & mask) << 1) ^ carry;
      y >>= 1;
   }
   return r;
}

}  // namespace Botan

namespace Botan {

constexpr size_t BLAKE2B_BLOCKBYTES = 128;

/**
* BLAKE2B
*/
class BLAKE2b final : public HashFunction,
                      public SymmetricAlgorithm {
   public:
      /**
      * @param output_bits the output size of BLAKE2b in bits
      */
      explicit BLAKE2b(size_t output_bits = 512);

      size_t hash_block_size() const override { return 128; }

      size_t output_length() const override { return m_output_bits / 8; }

      size_t key_size() const { return m_key_size; }

      Key_Length_Specification key_spec() const override;

      std::unique_ptr<HashFunction> new_object() const override;
      std::string name() const override;
      void clear() override;
      bool has_keying_material() const override;

      std::unique_ptr<HashFunction> copy_state() const override;

   protected:
      friend class BLAKE2bMAC;

      void key_schedule(std::span<const uint8_t> key) override;

      void add_data(std::span<const uint8_t> input) override;
      void final_result(std::span<uint8_t> out) override;

   private:
      void state_init();
      void compress(const uint8_t* data, size_t blocks, uint64_t increment);

      const size_t m_output_bits;

      AlignmentBuffer<uint8_t, BLAKE2B_BLOCKBYTES, AlignmentBufferFinalBlock::must_be_deferred> m_buffer;

      secure_vector<uint64_t> m_H;
      uint64_t m_T[2];
      uint64_t m_F;

      size_t m_key_size;
      secure_vector<uint8_t> m_padded_key_buffer;
};

}  // namespace Botan

namespace Botan {

/**
 * BLAKE2s
 */
class BLAKE2s final : public HashFunction {
   private:
      static constexpr size_t block_size = 64;

   public:
      explicit BLAKE2s(size_t output_bits = 256);
      ~BLAKE2s() override;

      BLAKE2s(const BLAKE2s&) = default;
      BLAKE2s& operator=(const BLAKE2s&) = delete;
      BLAKE2s(BLAKE2s&&) = delete;
      BLAKE2s& operator=(BLAKE2s&&) = delete;

      std::string name() const override;

      size_t output_length() const override { return m_outlen; }

      size_t hash_block_size() const override { return block_size; }

      std::unique_ptr<HashFunction> copy_state() const override;

      std::unique_ptr<HashFunction> new_object() const override { return std::make_unique<BLAKE2s>(m_outlen << 3); }

      void clear() override;

   private:
      void add_data(std::span<const uint8_t> input) override;
      void final_result(std::span<uint8_t> output) override;
      void state_init(size_t outlen);
      void compress(bool last, std::span<const uint8_t> buf);

   private:
      uint64_t m_bytes_processed = 0;
      AlignmentBuffer<uint8_t, block_size, AlignmentBufferFinalBlock::must_be_deferred> m_buffer;

      std::array<uint32_t, 8> m_h{};  // chained state
      size_t m_outlen = 0;            // digest size
};

}  // namespace Botan

namespace Botan {

/**
 * @brief Helper class to ease in-place marshalling of concatenated fixed-length
 *        values.
 *
 * The size of the final buffer must be known from the start, reallocations are
 * not performed.
 */
class BufferStuffer final {
   public:
      constexpr explicit BufferStuffer(std::span<uint8_t> buffer) : m_buffer(buffer) {}

      /**
       * @returns a span for the next @p bytes bytes in the concatenated buffer.
       *          Checks that the buffer is not exceeded.
       */
      constexpr std::span<uint8_t> next(size_t bytes) {
         BOTAN_STATE_CHECK(m_buffer.size() >= bytes);

         auto result = m_buffer.first(bytes);
         m_buffer = m_buffer.subspan(bytes);
         return result;
      }

      template <size_t bytes>
      constexpr std::span<uint8_t, bytes> next() {
         BOTAN_STATE_CHECK(m_buffer.size() >= bytes);

         auto result = m_buffer.first<bytes>();
         m_buffer = m_buffer.subspan(bytes);
         return result;
      }

      template <concepts::contiguous_strong_type StrongT>
      StrongSpan<StrongT> next(size_t bytes) {
         return StrongSpan<StrongT>(next(bytes));
      }

      /**
       * @returns a reference to the next single byte in the buffer
       */
      constexpr uint8_t& next_byte() { return next(1)[0]; }

      constexpr void append(std::span<const uint8_t> buffer) {
         auto sink = next(buffer.size());
         std::copy(buffer.begin(), buffer.end(), sink.begin());
      }

      constexpr void append(uint8_t b, size_t repeat = 1) {
         auto sink = next(repeat);
         std::fill(sink.begin(), sink.end(), b);
      }

      constexpr bool full() const { return m_buffer.empty(); }

      constexpr size_t remaining_capacity() const { return m_buffer.size(); }

   private:
      std::span<uint8_t> m_buffer;
};

}  // namespace Botan

namespace Botan {

/**
* Struct representing a particular date and time
*/
class BOTAN_TEST_API calendar_point final {
   public:
      /** The year */
      uint32_t year() const { return m_year; }

      /** The month, 1 through 12 for Jan to Dec */
      uint32_t month() const { return m_month; }

      /** The day of the month, 1 through 31 (or 28 or 30 based on month */
      uint32_t day() const { return m_day; }

      /** Hour in 24-hour form, 0 to 23 */
      uint32_t hour() const { return m_hour; }

      /** Minutes in the hour, 0 to 60 */
      uint32_t minutes() const { return m_minutes; }

      /** Seconds in the minute, 0 to 60, but might be slightly
      larger to deal with leap seconds on some systems
      */
      uint32_t seconds() const { return m_seconds; }

      /**
      * Initialize a calendar_point
      * @param y the year
      * @param mon the month
      * @param d the day
      * @param h the hour
      * @param min the minute
      * @param sec the second
      */
      calendar_point(uint32_t y, uint32_t mon, uint32_t d, uint32_t h, uint32_t min, uint32_t sec) :
            m_year(y), m_month(mon), m_day(d), m_hour(h), m_minutes(min), m_seconds(sec) {}

      /**
      * Convert a time_point to a calendar_point
      * @param time_point a time point from the system clock
      */
      explicit calendar_point(const std::chrono::system_clock::time_point& time_point);

      /**
      * Return seconds since epoch
      */
      uint64_t seconds_since_epoch() const;

      /**
      * Returns an STL timepoint object
      *
      * Note this throws an exception if the time is not representable
      * in the system time_t
      */
      std::chrono::system_clock::time_point to_std_timepoint() const;

      /**
      * Returns a human readable string of the struct's components.
      * Formatting might change over time. Currently it is RFC339 'iso-date-time'.
      */
      std::string to_string() const;

   private:
      uint32_t m_year;
      uint32_t m_month;
      uint32_t m_day;
      uint32_t m_hour;
      uint32_t m_minutes;
      uint32_t m_seconds;
};

}  // namespace Botan

namespace Botan {

/**
* Camellia-128
*/
class Camellia_128 final : public Block_Cipher_Fixed_Params<16, 16> {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;

      std::string name() const override { return "Camellia-128"; }

      std::string provider() const override;
      size_t parallelism() const override;

      std::unique_ptr<BlockCipher> new_object() const override { return std::make_unique<Camellia_128>(); }

      bool has_keying_material() const override;

   private:
      void key_schedule(std::span<const uint8_t> key) override;

#if defined(BOTAN_HAS_CAMELLIA_AVX2_GFNI)
      static void avx2_gfni_encrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
      static void avx2_gfni_decrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
#endif

#if defined(BOTAN_HAS_CAMELLIA_AVX512_GFNI)
      static void avx512_gfni_encrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
      static void avx512_gfni_decrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
#endif

#if defined(BOTAN_HAS_CAMELLIA_HWAES)
      static void hwaes_encrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
      static void hwaes_decrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
#endif

      secure_vector<uint64_t> m_SK;
};

/**
* Camellia-192
*/
class Camellia_192 final : public Block_Cipher_Fixed_Params<16, 24> {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;

      std::string name() const override { return "Camellia-192"; }

      std::string provider() const override;
      size_t parallelism() const override;

      std::unique_ptr<BlockCipher> new_object() const override { return std::make_unique<Camellia_192>(); }

      bool has_keying_material() const override;

   private:
      void key_schedule(std::span<const uint8_t> key) override;

#if defined(BOTAN_HAS_CAMELLIA_AVX2_GFNI)
      static void avx2_gfni_encrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
      static void avx2_gfni_decrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
#endif

#if defined(BOTAN_HAS_CAMELLIA_AVX512_GFNI)
      static void avx512_gfni_encrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
      static void avx512_gfni_decrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
#endif

#if defined(BOTAN_HAS_CAMELLIA_HWAES)
      static void hwaes_encrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
      static void hwaes_decrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
#endif

      secure_vector<uint64_t> m_SK;
};

/**
* Camellia-256
*/
class Camellia_256 final : public Block_Cipher_Fixed_Params<16, 32> {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;

      std::string name() const override { return "Camellia-256"; }

      std::string provider() const override;
      size_t parallelism() const override;

      std::unique_ptr<BlockCipher> new_object() const override { return std::make_unique<Camellia_256>(); }

      bool has_keying_material() const override;

   private:
      void key_schedule(std::span<const uint8_t> key) override;

#if defined(BOTAN_HAS_CAMELLIA_AVX2_GFNI)
      static void avx2_gfni_encrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
      static void avx2_gfni_decrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
#endif

#if defined(BOTAN_HAS_CAMELLIA_AVX512_GFNI)
      static void avx512_gfni_encrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
      static void avx512_gfni_decrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
#endif

#if defined(BOTAN_HAS_CAMELLIA_HWAES)
      static void hwaes_encrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
      static void hwaes_decrypt(const uint8_t in[], uint8_t out[], size_t blocks, std::span<const uint64_t> SK);
#endif

      secure_vector<uint64_t> m_SK;
};

}  // namespace Botan

namespace Botan {

/**
* Block Cipher Cascade
*/
class Cascade_Cipher final : public BlockCipher {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      size_t block_size() const override { return m_block_size; }

      Key_Length_Specification key_spec() const override {
         return Key_Length_Specification(m_cipher1->maximum_keylength() + m_cipher2->maximum_keylength());
      }

      void clear() override;
      std::string name() const override;
      std::unique_ptr<BlockCipher> new_object() const override;

      bool has_keying_material() const override;

      /**
      * Create a cascade of two block ciphers
      * @param cipher1 the first cipher
      * @param cipher2 the second cipher
      */
      Cascade_Cipher(std::unique_ptr<BlockCipher> cipher1, std::unique_ptr<BlockCipher> cipher2);

   private:
      void key_schedule(std::span<const uint8_t> key) override;

      std::unique_ptr<BlockCipher> m_cipher1, m_cipher2;
      size_t m_block_size;
};

}  // namespace Botan

namespace Botan {

// TODO convert these to take arguments as spans or std::string_view

/**
* Convert a sequence of UCS-2 (big endian) characters to a UTF-8 string
* This is used for ASN.1 BMPString type
* @param ucs2 the sequence of UCS-2 characters
* @param len length of ucs2 in bytes, must be a multiple of 2
*/
BOTAN_TEST_API std::string ucs2_to_utf8(const uint8_t ucs2[], size_t len);

/**
 * Convert a UTF-8 string to a sequence of UCS-2 (big endian) characters
 * This is used for ASN.1 BMPString type
 * @param utf8 the UTF-8 string
 * @return a vector of bytes containing the UCS-2 (big endian) encoding
 * @throws Decoding_Error if the input is not valid UTF-8 (including overlong encodings,
 *         surrogate code points, or values outside Unicode), or if a code point exceeds
 *         U+FFFF and cannot be represented in UCS-2
 */
BOTAN_TEST_API std::vector<uint8_t> utf8_to_ucs2(const std::string& utf8);

/**
* Convert a sequence of UCS-4 (big endian) characters to a UTF-8 string
* This is used for ASN.1 UniversalString type
* @param ucs4 the sequence of UCS-4 characters
* @param len length of ucs4 in bytes, must be a multiple of 4
*/
BOTAN_TEST_API std::string ucs4_to_utf8(const uint8_t ucs4[], size_t len);

/**
 * Convert a UTF-8 string to a sequence of UCS-4 (big endian) characters
 * This is used for ASN.1 UniversalString type
 * @param utf8 the UTF-8 string
 * @return a vector of bytes containing the UCS-4 (big endian) encoding
 * @throws Decoding_Error if the input is not valid UTF-8 (including overlong encodings,
 *         surrogate code points, or values outside the Unicode scalar value range U+0000..U+10FFFF)
 */
BOTAN_TEST_API std::vector<uint8_t> utf8_to_ucs4(const std::string& utf8);

BOTAN_TEST_API std::string latin1_to_utf8(const uint8_t latin1[], size_t len);

/**
* Return true if this string seems to contain a valid sequence of UTF-8
*/
bool is_valid_utf8(const std::string& str);

/**
* Return a string containing 'c', quoted and possibly escaped
*
* This is used when creating an error message nothing an invalid character
* in some codex (for example during hex decoding)
*
* Currently this function escapes tab, newlines and carriage return
* as "\t", "\n", and "\r", and also escapes characters > 0x7F as
* "\xHH" where HH is the hex code.
*/
std::string format_char_for_display(char c);

}  // namespace Botan

namespace Botan {

/**
* Perform encoding using the base provided
* @param base object giving access to the encodings specifications
* @param output an array of at least base.encode_max_output bytes
* @param input is some binary data
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding chars will be applied if needed
* @return number of bytes written to output
*/
template <class Base>
size_t base_encode(const Base& base,
                   char output[],
                   const uint8_t input[],
                   size_t input_length,
                   size_t& input_consumed,
                   bool final_inputs) {
   input_consumed = 0;

   // TODO(Botan4) Check if we can use just base. or Base:: here instead
   constexpr size_t encoding_bytes_in = std::remove_reference_t<Base>::encoding_bytes_in();
   constexpr size_t encoding_bytes_out = std::remove_reference_t<Base>::encoding_bytes_out();

   size_t input_remaining = input_length;
   size_t output_produced = 0;

   while(input_remaining >= encoding_bytes_in) {
      base.encode(output + output_produced, input + input_consumed);

      input_consumed += encoding_bytes_in;
      output_produced += encoding_bytes_out;
      input_remaining -= encoding_bytes_in;
   }

   if(final_inputs && input_remaining) {
      std::array<uint8_t, encoding_bytes_in> remainder{};
      for(size_t i = 0; i != input_remaining; ++i) {
         remainder[i] = input[input_consumed + i];
      }

      base.encode(output + output_produced, remainder.data());

      const size_t bits_consumed = base.bits_consumed();
      const size_t remaining_bits_before_padding = base.remaining_bits_before_padding();

      size_t empty_bits = 8 * (encoding_bytes_in - input_remaining);
      size_t index = output_produced + encoding_bytes_out - 1;
      while(empty_bits >= remaining_bits_before_padding) {
         output[index--] = '=';
         empty_bits -= bits_consumed;
      }

      input_consumed += input_remaining;
      output_produced += encoding_bytes_out;
   }

   return output_produced;
}

template <typename Base>
std::string base_encode_to_string(const Base& base, const uint8_t input[], size_t input_length) {
   const size_t output_length = base.encode_max_output(input_length);
   std::string output(output_length, 0);

   size_t consumed = 0;
   size_t produced = 0;

   if(output_length > 0) {
      produced = base_encode(base, &output.front(), input, input_length, consumed, true);
   }

   BOTAN_ASSERT_EQUAL(consumed, input_length, "Consumed the entire input");
   BOTAN_ASSERT_EQUAL(produced, output.size(), "Produced expected size");

   return output;
}

/**
* Perform decoding using the base provided
* @param base object giving access to the encodings specifications
* @param output an array of at least Base::decode_max_output bytes
* @param input some base input
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding is allowed
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
template <typename Base>
size_t base_decode(const Base& base,
                   uint8_t output[],
                   const char input[],
                   size_t input_length,
                   size_t& input_consumed,
                   bool final_inputs,
                   bool ignore_ws = true) {
   // TODO(Botan4) Check if we can use just base. or Base:: here instead
   constexpr size_t decoding_bytes_in = std::remove_reference_t<Base>::decoding_bytes_in();
   constexpr size_t decoding_bytes_out = std::remove_reference_t<Base>::decoding_bytes_out();

   uint8_t* out_ptr = output;
   std::array<uint8_t, decoding_bytes_in> decode_buf{};
   size_t decode_buf_pos = 0;
   size_t final_truncate = 0;

   clear_mem(output, base.decode_max_output(input_length));

   for(size_t i = 0; i != input_length; ++i) {
      const uint8_t bin = base.lookup_binary_value(input[i]);

      // This call might throw Invalid_Argument
      if(base.check_bad_char(bin, input[i], ignore_ws)) {
         decode_buf[decode_buf_pos] = bin;
         ++decode_buf_pos;
      }

      /*
      * If we're at the end of the input, pad with 0s and truncate
      */
      if(final_inputs && (i == input_length - 1)) {
         if(decode_buf_pos) {
            for(size_t j = decode_buf_pos; j < decoding_bytes_in; ++j) {
               decode_buf[j] = 0;
            }

            final_truncate = decoding_bytes_in - decode_buf_pos;
            decode_buf_pos = decoding_bytes_in;
         }
      }

      if(decode_buf_pos == decoding_bytes_in) {
         base.decode(out_ptr, decode_buf.data());

         out_ptr += decoding_bytes_out;
         decode_buf_pos = 0;
         input_consumed = i + 1;
      }
   }

   while(input_consumed < input_length && base.lookup_binary_value(input[input_consumed]) == 0x80) {
      ++input_consumed;
   }

   const size_t written = (out_ptr - output) - base.bytes_to_remove(final_truncate);

   return written;
}

template <typename Base>
size_t base_decode_full(const Base& base, uint8_t output[], const char input[], size_t input_length, bool ignore_ws) {
   size_t consumed = 0;
   const size_t written = base_decode(base, output, input, input_length, consumed, true, ignore_ws);

   if(consumed != input_length) {
      throw Invalid_Argument(base.name() + " decoding failed, input did not have full bytes");
   }

   return written;
}

template <typename Vector, typename Base>
Vector base_decode_to_vec(const Base& base, const char input[], size_t input_length, bool ignore_ws) {
   const size_t output_length = base.decode_max_output(input_length);
   Vector bin(output_length);

   const size_t written = base_decode_full(base, bin.data(), input, input_length, ignore_ws);

   bin.resize(written);
   return bin;
}

}  // namespace Botan

namespace Botan {

namespace detail {

/**
 * Helper function that performs range size-checks as required given the
 * selected output and input range types. If all lengths are known at compile
 * time, this check will be performed at compile time as well. It will then
 * instantiate an output range and concatenate the input ranges' contents.
 */
template <ranges::spanable_range OutR, ranges::spanable_range... Rs>
constexpr OutR concatenate(Rs&&... ranges)
   requires(concepts::reservable_container<OutR> || ranges::statically_spanable_range<OutR>)
{
   OutR result{};

   // Prepare and validate the output range and construct a lambda that does the
   // actual filling of the result buffer.
   // (if no input ranges are given, GCC claims that fill_fn is unused)
   [[maybe_unused]] auto fill_fn = [&] {
      if constexpr(concepts::reservable_container<OutR>) {
         // dynamically allocate the correct result byte length
         const size_t total_size = (ranges.size() + ... + 0);
         result.reserve(total_size);

         // fill the result buffer using a back-inserter
         return [&result](auto&& range) {
            std::copy(
               std::ranges::begin(range), std::ranges::end(range), std::back_inserter(unwrap_strong_type(result)));
         };
      } else {
         if constexpr((ranges::statically_spanable_range<Rs> && ... && true)) {
            // all input ranges have a static extent, so check the total size at compile time
            // (work around an issue in MSVC that warns `total_size` is unused)
            [[maybe_unused]] constexpr size_t total_size = (decltype(std::span{ranges})::extent + ... + 0);
            static_assert(result.size() == total_size, "size of result buffer does not match the sum of input buffers");
         } else {
            // at least one input range has a dynamic extent, so check the total size at runtime
            const size_t total_size = (ranges.size() + ... + 0);
            BOTAN_ARG_CHECK(result.size() == total_size,
                            "result buffer has static extent that does not match the sum of input buffers");
         }

         // fill the result buffer and hold the current output-iterator position
         return [itr = std::ranges::begin(result)](auto&& range) mutable {
            std::copy(std::ranges::begin(range), std::ranges::end(range), itr);
            std::advance(itr, std::ranges::size(range));
         };
      }
   }();

   // perform the actual concatenation
   (fill_fn(std::forward<Rs>(ranges)), ...);

   return result;
}

}  // namespace detail

/**
 * Concatenate an arbitrary number of buffers. Performs range-checks as needed.
 *
 * The output type can be auto-detected based on the input ranges, or explicitly
 * specified by the caller. If all input ranges have a static extent, the total
 * size is calculated at compile time and a statically sized std::array<> is used.
 * Otherwise this tries to use the type of the first input range as output type.
 *
 * Alternatively, the output container type can be specified explicitly.
 */
template <typename OutR = detail::AutoDetect, ranges::spanable_range... Rs>
constexpr auto concat(Rs&&... ranges)
   requires(all_same_v<std::ranges::range_value_t<Rs>...>)
{
   if constexpr(std::same_as<detail::AutoDetect, OutR>) {
      // Try to auto-detect a reasonable output type given the input ranges
      static_assert(sizeof...(Rs) > 0, "Cannot auto-detect the output type if not a single input range is provided.");
      using candidate_result_t = std::remove_cvref_t<std::tuple_element_t<0, std::tuple<Rs...>>>;
      using result_range_value_t = std::remove_cvref_t<std::ranges::range_value_t<candidate_result_t>>;

      if constexpr((ranges::statically_spanable_range<Rs> && ...)) {
         // If all input ranges have a static extent, we can calculate the total size at compile time
         // and therefore can use a statically sized output container. This is constexpr.
         constexpr size_t total_size = (decltype(std::span{ranges})::extent + ... + 0);
         using out_array_t = std::array<result_range_value_t, total_size>;
         return detail::concatenate<out_array_t>(std::forward<Rs>(ranges)...);
      } else {
         // If at least one input range has a dynamic extent, we must use a dynamically allocated output container.
         // We assume that the user wants to use the first input range's container type as output type.
         static_assert(
            concepts::reservable_container<candidate_result_t>,
            "First input range has static extent, but a dynamically allocated output range is required. Please explicitly specify a dynamically allocatable output type.");
         return detail::concatenate<candidate_result_t>(std::forward<Rs>(ranges)...);
      }
   } else {
      // The caller has explicitly specified the output type
      return detail::concatenate<OutR>(std::forward<Rs>(ranges)...);
   }
}

}  // namespace Botan

#if defined(BOTAN_HAS_CPUID_DETECTION)

namespace Botan {

class BOTAN_TEST_API CPUFeature final {
   public:
      enum Bit : uint32_t /* NOLINT(*-use-enum-class) */ {
         NEON = (1U << 0),
         SVE = (1U << 1),
         AES = (1U << 16),
         PMULL = (1U << 17),
         SHA1 = (1U << 18),
         SHA2 = (1U << 19),
         SHA3 = (1U << 20),
         SHA2_512 = (1U << 21),
         SM3 = (1U << 22),
         SM4 = (1U << 23),

         SIMD_4X32 = NEON,
         HW_AES = AES,
         HW_CLMUL = PMULL,
      };

      CPUFeature(Bit b) : m_bit(b) {}  // NOLINT(*-explicit-conversions)

      uint32_t as_u32() const { return static_cast<uint32_t>(m_bit); }

      std::string to_string() const;

      static std::optional<CPUFeature> from_string(std::string_view s);

   private:
      Bit m_bit;
};

}  // namespace Botan
#endif

namespace Botan {

#if !defined(BOTAN_HAS_CPUID_DETECTION)
// A no-op CPUFeature
class BOTAN_TEST_API CPUFeature final {
   public:
      enum Bit : uint32_t {};

      uint32_t as_u32() const;

      CPUFeature(Bit) {}

      static std::optional<CPUFeature> from_string(std::string_view);

      std::string to_string() const;
};
#endif

/**
* A class handling runtime CPU feature detection. It is limited to
* just the features necessary to implement CPU specific code in Botan,
* rather than being a general purpose utility.
*/
class BOTAN_TEST_API CPUID final {
   public:
      typedef CPUFeature Feature;

      /**
      * Probe the CPU and see what extensions are supported
      */
      static void initialize();

      /**
      * Return a possibly empty string containing list of known CPU
      * extensions. Each name will be separated by a space, and the ordering
      * will be arbitrary. This list only contains values that are useful to
      * Botan (for example FMA instructions are not checked).
      *
      * Example outputs "sse2 ssse3 rdtsc", "neon arm_aes", "altivec"
      */
      static std::string to_string();

      /**
      * Check if a feature is supported returning the associated string if so
      *
      * This is a helper function used to implement provider()
      */
      static std::optional<std::string> check(CPUID::Feature feat) {
         if(state().has_bit(feat.as_u32())) {
            return feat.to_string();
         } else {
            return {};
         }
      }

      /**
      * Check if a feature is supported returning the associated string if so
      *
      * This is a helper function used to implement provider()
      */
      static std::optional<std::string> check(CPUID::Feature feat1, CPUID::Feature feat2) {
         if(state().has_bit((feat1.as_u32() | feat2.as_u32()))) {
            // Typically feat2 is a secondary feature that is almost but not
            // completely implied by feat1 (ex: AVX2 + BMI2) which we have to
            // check for completeness, but don't reflect into the provider name.
            return feat1.to_string();
         } else {
            return {};
         }
      }

      /**
      * Check if a feature is supported
      */
      static bool has(CPUID::Feature feat) { return state().has_bit(feat.as_u32()); }

      /**
      * Check if two features are both supported
      */
      static bool has(CPUID::Feature feat1, CPUID::Feature feat2) {
         return state().has_bit(feat1.as_u32() | feat2.as_u32());
      }

      /*
      * Clear a CPUID bit
      * Call CPUID::initialize to reset
      *
      * This is only exposed for testing and should never be called within the library
      */
      static void clear_cpuid_bit(CPUID::Feature bit) { state().clear_cpuid_bit(bit.as_u32()); }

      static std::optional<CPUID::Feature> bit_from_string(std::string_view tok);

      /**
      * A common helper for the various CPUID implementations
      */
      template <typename T>
      static inline uint32_t if_set(uint64_t cpuid, T flag, CPUID::Feature bit, uint32_t allowed) {
         const uint64_t flag64 = static_cast<uint64_t>(flag);
         if((cpuid & flag64) == flag64) {
            return (bit.as_u32() & allowed);
         } else {
            return 0;
         }
      }

   private:
      static inline bool is_set(uint32_t allowed, CPUID::Feature bit) {
         const uint32_t feat_bit = bit.as_u32();
         return ((allowed & feat_bit) == feat_bit);
      }

      struct CPUID_Data {
         public:
            CPUID_Data();

            CPUID_Data(const CPUID_Data& other) = default;
            CPUID_Data(CPUID_Data&& other) = default;
            CPUID_Data& operator=(const CPUID_Data& other) = default;
            CPUID_Data& operator=(CPUID_Data&& other) = default;
            ~CPUID_Data() = default;

            void clear_cpuid_bit(uint32_t bit) { m_processor_features &= ~bit; }

            bool has_bit(uint32_t bit) const { return (m_processor_features & bit) == bit; }

            uint32_t bitset() const { return m_processor_features; }

         private:
#if defined(BOTAN_HAS_CPUID_DETECTION)
            static uint32_t detect_cpu_features(uint32_t allowed_bits);
#endif

            uint32_t m_processor_features;
      };

      static CPUID_Data& state() {
         static CPUID::CPUID_Data g_cpuid;
         return g_cpuid;
      }
};

}  // namespace Botan

namespace Botan {

/**
 * @brief Helper class to create a RAII-style cleanup callback
 *
 * Ensures that the cleanup callback given in the object's constructor is called
 * when the object is destroyed. Use this to ensure some cleanup code runs when
 * leaving the current scope.
 */
template <std::invocable FunT>
class scoped_cleanup final {
   public:
      explicit scoped_cleanup(FunT cleanup) : m_cleanup(std::move(cleanup)) {}

      scoped_cleanup(const scoped_cleanup&) = delete;
      scoped_cleanup& operator=(const scoped_cleanup&) = delete;

      scoped_cleanup(scoped_cleanup&& other) noexcept : m_cleanup(std::move(other.m_cleanup)) { other.disengage(); }

      scoped_cleanup& operator=(scoped_cleanup&& other) noexcept {
         if(this != &other) {
            m_cleanup = std::move(other.m_cleanup);
            other.disengage();
         }
         return *this;
      }

      ~scoped_cleanup() {
         if(m_cleanup.has_value()) {
            (*m_cleanup)();  // NOLINT(bugprone-exception-escape) clang-tidy bug
         }
      }

      /**
       * Disengage the cleanup callback, i.e., prevent it from being called
       */
      void disengage() noexcept { m_cleanup.reset(); }

   private:
      std::optional<FunT> m_cleanup;
};

}  // namespace Botan


#if defined(BOTAN_HAS_VALGRIND)
   #include <valgrind/memcheck.h>
#endif

namespace Botan::CT {

/// @name Constant Time Check Annotation Helpers
/// @{

/**
* Use valgrind to mark the contents of memory as being undefined.
* Valgrind will accept operations which manipulate undefined values,
* but will warn if an undefined value is used to decided a conditional
* jump or a load/store address. So if we poison all of our inputs we
* can confirm that the operations in question are truly const time
* when compiled by whatever compiler is in use.
*
* Even better, the VALGRIND_MAKE_MEM_* macros work even when the
* program is not run under valgrind (though with a few cycles of
* overhead, which is unfortunate in final binaries as these
* annotations tend to be used in fairly important loops).
*
* This approach was first used in ctgrind (https://github.com/agl/ctgrind)
* but calling the valgrind mecheck API directly works just as well and
* doesn't require a custom patched valgrind.
*/
template <typename T>
constexpr inline void poison(const T* p, size_t n) {
#if defined(BOTAN_HAS_VALGRIND)
   if(!std::is_constant_evaluated()) {
      VALGRIND_MAKE_MEM_UNDEFINED(p, n * sizeof(T));
   }
#endif

   BOTAN_UNUSED(p, n);
}

template <typename T>
constexpr inline void unpoison(const T* p, size_t n) {
#if defined(BOTAN_HAS_VALGRIND)
   if(!std::is_constant_evaluated()) {
      VALGRIND_MAKE_MEM_DEFINED(p, n * sizeof(T));
   }
#endif

   BOTAN_UNUSED(p, n);
}

/**
 * Checks whether CT::poison() and CT::unpoison() actually have an effect.
 *
 * If the build is not instrumented and/or not run using an analysis tool like
 * valgrind, the functions are no-ops and the return value is false.
 *
 * @returns true if CT::poison() and CT::unpoison() are effective
 */
inline bool poison_has_effect() {
#if defined(BOTAN_HAS_VALGRIND)
   return RUNNING_ON_VALGRIND;
#else
   return false;
#endif
}

/// @}

/// @name Constant Time Check Annotation Convenience overloads
/// @{

template <typename T>
concept custom_poisonable = requires(const T& v) { v._const_time_poison(); };
template <typename T>
concept custom_unpoisonable = requires(const T& v) { v._const_time_unpoison(); };

/**
 * Poison a single integral object
 */
template <std::integral T>
constexpr void poison(const T& p) {
   poison(&p, 1);
}

template <std::integral T>
constexpr void unpoison(const T& p) {
   unpoison(&p, 1);
}

/**
 * Poison a contiguous buffer of trivial objects (e.g. integers and such)
 */
template <ranges::spanable_range R>
   requires std::is_trivially_copyable_v<std::ranges::range_value_t<R>> && (!custom_poisonable<R>)
constexpr void poison(const R& r) {
   const std::span s{r};
   poison(s.data(), s.size());
}

template <ranges::spanable_range R>
   requires std::is_trivially_copyable_v<std::ranges::range_value_t<R>> && (!custom_unpoisonable<R>)
constexpr void unpoison(const R& r) {
   const std::span s{r};
   unpoison(s.data(), s.size());
}

/**
 * Poison a class type that provides a public `_const_time_poison()` method
 * For instance: BigInt, CT::Mask<>, FrodoMatrix, ...
 */
template <custom_poisonable T>
constexpr void poison(const T& x) {
   x._const_time_poison();
}

template <custom_unpoisonable T>
constexpr void unpoison(const T& x) {
   x._const_time_unpoison();
}

/**
 * Poison an optional object if it has a value.
 */
template <typename T>
   requires requires(const T& v) { ::Botan::CT::poison(v); }
constexpr void poison(const std::optional<T>& x) {
   if(x.has_value()) {
      poison(*x);
   }
}

template <typename T>
   requires requires(const T& v) { ::Botan::CT::unpoison(v); }
constexpr void unpoison(const std::optional<T>& x) {
   if(x.has_value()) {
      unpoison(*x);
   }
}

/// @}

/// @name Higher-level Constant Time Check Annotation Helpers
/// @{

template <typename T>
concept poisonable = requires(const T& v) { ::Botan::CT::poison(v); };
template <typename T>
concept unpoisonable = requires(const T& v) { ::Botan::CT::unpoison(v); };

/**
 * Poison a range of objects by calling `poison` on each element.
 */
template <std::ranges::range R>
   requires poisonable<std::ranges::range_value_t<R>>
constexpr void poison_range(const R& r) {
   for(const auto& v : r) {
      poison(v);
   }
}

template <std::ranges::range R>
   requires unpoisonable<std::ranges::range_value_t<R>>
constexpr void unpoison_range(const R& r) {
   for(const auto& v : r) {
      unpoison(v);
   }
}

/**
 * Poisons an arbitrary number of values in a single call.
 * Mostly syntactic sugar to save clutter (i.e. lines-of-code).
 */
template <poisonable... Ts>
   requires(sizeof...(Ts) > 0)
constexpr void poison_all(const Ts&... ts) {
   (poison(ts), ...);
}

template <unpoisonable... Ts>
   requires(sizeof...(Ts) > 0)
constexpr void unpoison_all(const Ts&... ts) {
   (unpoison(ts), ...);
}

/**
 * Poisons an arbitrary number of poisonable values, and unpoisons them when the
 * returned object runs out-of-scope
 *
 * Use this when you want to poison a value that remains valid longer than the
 * scope you are currently in. For instance, a private key structure that is a
 * member of a Signature_Operation object, that may be used for multiple
 * signatures.
 */
template <typename... Ts>
   requires(sizeof...(Ts) > 0) && (poisonable<Ts> && ...) && (unpoisonable<Ts> && ...)
[[nodiscard]] constexpr auto scoped_poison(const Ts&... xs) {
   auto scope = scoped_cleanup([&] { unpoison_all(xs...); });
   poison_all(xs...);
   return scope;
}

/**
 * Poisons an r-value @p v and forwards it as the return value.
 */
template <poisonable T>
[[nodiscard]] decltype(auto) driveby_poison(T&& v)
   requires(std::is_rvalue_reference_v<decltype(v)>)
{
   poison(v);
   return std::forward<T>(v);
}

/**
 * Unpoisons an r-value @p v and forwards it as the return value.
 */
template <unpoisonable T>
[[nodiscard]] decltype(auto) driveby_unpoison(T&& v)
   requires(std::is_rvalue_reference_v<decltype(v)>)
{
   unpoison(v);
   return std::forward<T>(v);
}

/// @}

/**
* A Choice is used for constant-time conditionals.
*
* Internally it always is either |0| (all 0 bits) or |1| (all 1 bits)
* and measures are taken to block compilers from reasoning about the
* expected value of a Choice.
*/
class Choice final {
   public:
      using underlying_type = word;

      /**
      * If v == 0 return an unset (false) Choice, otherwise a set Choice
      */
      template <typename T>
         requires std::unsigned_integral<T> && (!std::same_as<bool, T>)
      constexpr static Choice from_int(T v) {
         if constexpr(sizeof(T) <= sizeof(underlying_type)) {
            return !Choice(ct_is_zero<underlying_type>(v));
         } else {
            // Mask of T that is either |0| or |1|
            const T v_is_0 = ct_is_zero<T>(value_barrier<T>(v));

            // We want the mask to be set if v != 0 so we must check that
            // v_is_0 is itself zero.
            //
            // Also sizeof(T) may not equal sizeof(underlying_type) so we must
            // use ct_is_zero<underlying_type>. It's ok to either truncate or
            // zero extend v_is_0 to 32 bits since we know it is |0| or |1|
            // so even just the low bit is sufficient.
            return Choice(ct_is_zero<underlying_type>(static_cast<underlying_type>(v_is_0)));
         }
      }

      /**
      * Return a bitmask |1| if the choice is set, or |0| otherwise
      */
      template <typename T>
         requires std::unsigned_integral<T> && (!std::same_as<bool, T>)
      constexpr T into_bitmask() const {
         if constexpr(sizeof(T) <= sizeof(underlying_type)) {
            // The inner mask is already |0| or |1| so just truncate
            return static_cast<T>(value());
         } else {
            return ~ct_is_zero<T>(value());
         }
      }

      /**
      * Create a Choice directly from a mask value - this assumes v is either |0| or |1|
      */
      constexpr static Choice from_mask(underlying_type v) { return Choice(v); }

      constexpr static Choice yes() { return !no(); }

      constexpr static Choice no() { return Choice(0); }

      constexpr Choice operator!() const { return Choice(~value()); }

      constexpr Choice operator&&(const Choice& other) const { return Choice(value() & other.value()); }

      constexpr Choice operator||(const Choice& other) const { return Choice(value() | other.value()); }

      constexpr Choice operator!=(const Choice& other) const { return Choice(value() ^ other.value()); }

      constexpr Choice operator==(const Choice& other) const { return !(*this != other); }

      /**
      * Unsafe conversion to bool
      *
      * This conversion itself is (probably) constant time, but once the
      * choice is reduced to a simple bool, it's entirely possible for the
      * compiler to perform range analysis on the values, since there are just
      * the two. As a consequence even if the caller is not using this in an
      * obviously branchy way (`if(choice.as_bool()) ...`) a smart compiler
      * may introduce branches depending on the value.
      */
      constexpr bool as_bool() const { return m_value != 0; }

      /// Return the masked value
      constexpr underlying_type value() const { return value_barrier(m_value); }

      constexpr Choice(const Choice& other) = default;
      constexpr Choice(Choice&& other) = default;
      constexpr Choice& operator=(const Choice& other) noexcept = default;
      constexpr Choice& operator=(Choice&& other) noexcept = default;
      constexpr ~Choice() = default;

   private:
      constexpr explicit Choice(underlying_type v) : m_value(CT::value_barrier<underlying_type>(v)) {}

      underlying_type m_value;
};

/**
* A concept for a type which is conditionally assignable
*/
template <typename T>
concept ct_conditional_assignable = requires(T lhs, const T& rhs, Choice c) { lhs.conditional_assign(c, rhs); };

/**
* A Mask type used for constant-time operations. A Mask<T> always has value
* either |0| (all bits cleared) or |1| (all bits set). All operations in a Mask<T>
* are intended to compile to code which does not contain conditional jumps.
* This must be verified with tooling (eg binary disassembly or using valgrind)
* since you never know what a compiler might do.
*/
template <typename T>
class Mask final {
   public:
      static_assert(std::is_unsigned_v<T> && !std::is_same_v<bool, T>,
                    "Only unsigned integer types are supported by CT::Mask");

      Mask(const Mask<T>& other) = default;
      Mask(Mask<T>&& other) = default;
      Mask<T>& operator=(const Mask<T>& other) = default;
      Mask<T>& operator=(Mask<T>&& other) = default;
      ~Mask() = default;

      /**
      * Derive a Mask from a Mask of a larger type
      */
      template <typename U>
      constexpr explicit Mask(Mask<U> o) : m_mask(static_cast<T>(o.value())) {
         static_assert(sizeof(U) > sizeof(T), "sizes ok");
      }

      /**
      * Return a Mask<T> of |1| (all bits set)
      */
      static constexpr Mask<T> set() { return Mask<T>(static_cast<T>(~0)); }

      /**
      * Return a Mask<T> of |0| (all bits cleared)
      */
      static constexpr Mask<T> cleared() { return Mask<T>(0); }

      /**
      * Return a Mask<T> which is set if v is != 0
      */
      static constexpr Mask<T> expand(T v) { return ~Mask<T>::is_zero(value_barrier<T>(v)); }

      /**
      * Return a Mask<T> which is set if v is true
      */
      static constexpr Mask<T> expand_bool(bool v) { return Mask<T>::expand(static_cast<T>(v)); }

      /**
      * Return a Mask<T> which is set if choice is set
      */
      static constexpr Mask<T> from_choice(Choice c) {
         if constexpr(sizeof(T) <= sizeof(Choice::underlying_type)) {
            // Take advantage of the fact that Choice's mask is always
            // either |0| or |1|
            return Mask<T>(static_cast<T>(c.value()));
         } else {
            return ~Mask<T>::is_zero(c.value());
         }
      }

      /**
      * Return a Mask<T> which is set if the top bit of v is set
      */
      static constexpr Mask<T> expand_top_bit(T v) { return Mask<T>(ct_expand_top_bit<T>(v)); }

      /**
       * Return a Mask<T> which is set if the given @p bit of @p v is set.
       * @p bit must be from 0 (LSB) to (sizeof(T) * 8 - 1) (MSB).
       */
      static constexpr Mask<T> expand_bit(T v, size_t bit) {
         return CT::Mask<T>::expand_top_bit(v << (sizeof(v) * 8 - 1 - bit));
      }

      /**
      * Return a Mask<T> which is set if m is set
      */
      template <typename U>
      static constexpr Mask<T> expand(Mask<U> m) {
         static_assert(sizeof(U) < sizeof(T), "sizes ok");
         return ~Mask<T>::is_zero(m.value());
      }

      /**
      * Return a Mask<T> which is set if v is == 0 or cleared otherwise
      */
      static constexpr Mask<T> is_zero(T x) { return Mask<T>(ct_is_zero<T>(value_barrier<T>(x))); }

      /**
      * Return a Mask<T> which is set if x == y
      */
      static constexpr Mask<T> is_equal(T x, T y) {
         const T diff = value_barrier(x) ^ value_barrier(y);
         return Mask<T>::is_zero(diff);
      }

      /**
      * Return a Mask<T> which is set if x < y
      */
      static constexpr Mask<T> is_lt(T x, T y) {
         T u = x ^ ((x ^ y) | ((x - y) ^ x));
         return Mask<T>::expand_top_bit(u);
      }

      /**
      * Return a Mask<T> which is set if x > y
      */
      static constexpr Mask<T> is_gt(T x, T y) { return Mask<T>::is_lt(y, x); }

      /**
      * Return a Mask<T> which is set if x <= y
      */
      static constexpr Mask<T> is_lte(T x, T y) { return ~Mask<T>::is_gt(x, y); }

      /**
      * Return a Mask<T> which is set if x >= y
      */
      static constexpr Mask<T> is_gte(T x, T y) { return ~Mask<T>::is_lt(x, y); }

      static constexpr Mask<T> is_within_range(T v, T l, T u) {
         //return Mask<T>::is_gte(v, l) & Mask<T>::is_lte(v, u);

         const T v_lt_l = v ^ ((v ^ l) | ((v - l) ^ v));
         const T v_gt_u = u ^ ((u ^ v) | ((u - v) ^ u));
         const T either = value_barrier(v_lt_l) | value_barrier(v_gt_u);
         return ~Mask<T>::expand_top_bit(either);
      }

      static constexpr Mask<T> is_any_of(T v, std::initializer_list<T> accepted) {
         T accept = 0;

         for(auto a : accepted) {
            const T diff = a ^ v;
            const T eq_zero = value_barrier<T>(~diff & (diff - 1));
            accept |= eq_zero;
         }

         return Mask<T>::expand_top_bit(accept);
      }

      /**
      * AND-combine two masks
      */
      Mask<T>& operator&=(Mask<T> o) {
         m_mask &= o.value();
         return (*this);
      }

      /**
      * XOR-combine two masks
      */
      Mask<T>& operator^=(Mask<T> o) {
         m_mask ^= o.value();
         return (*this);
      }

      /**
      * OR-combine two masks
      */
      Mask<T>& operator|=(Mask<T> o) {
         m_mask |= o.value();
         return (*this);
      }

      /**
      * AND-combine two masks
      */
      friend Mask<T> operator&(Mask<T> x, Mask<T> y) { return Mask<T>(x.value() & y.value()); }

      /**
      * XOR-combine two masks
      */
      friend Mask<T> operator^(Mask<T> x, Mask<T> y) { return Mask<T>(x.value() ^ y.value()); }

      /**
      * OR-combine two masks
      */
      friend Mask<T> operator|(Mask<T> x, Mask<T> y) { return Mask<T>(x.value() | y.value()); }

      /**
      * Negate this mask
      */
      constexpr Mask<T> operator~() const { return Mask<T>(~value()); }

      /**
      * Return x if the mask is set, or otherwise zero
      */
      constexpr T if_set_return(T x) const { return value() & x; }

      /**
      * Return x if the mask is cleared, or otherwise zero
      */
      constexpr T if_not_set_return(T x) const { return ~value() & x; }

      /**
      * If this mask is set, return x, otherwise return y
      */
      constexpr T select(T x, T y) const { return choose(value(), x, y); }

      constexpr T select_and_unpoison(T x, T y) const {
         T r = this->select(x, y);
         CT::unpoison(r);
         return r;
      }

      /**
      * If this mask is set, return x, otherwise return y
      */
      Mask<T> select_mask(Mask<T> x, Mask<T> y) const { return Mask<T>(select(x.value(), y.value())); }

      /**
      * Conditionally set output to x or y, depending on if mask is set or
      * cleared (resp)
      */
      constexpr void select_n(T output[], const T x[], const T y[], size_t len) const {
         const T mask = value();
         for(size_t i = 0; i != len; ++i) {
            output[i] = choose(mask, x[i], y[i]);
         }
      }

      /**
      * If this mask is set, zero out buf, otherwise do nothing
      */
      constexpr void if_set_zero_out(T buf[], size_t elems) {
         for(size_t i = 0; i != elems; ++i) {
            buf[i] = this->if_not_set_return(buf[i]);
         }
      }

      /**
     * If this mask is set, swap x and y
     */
      template <typename U>
      void conditional_swap(U& x, U& y) const
         requires(sizeof(U) <= sizeof(T))
      {
         auto cnd = Mask<U>(*this);
         U t0 = cnd.select(y, x);
         U t1 = cnd.select(x, y);
         x = t0;
         y = t1;
      }

      /**
      * Return the value of the mask, unpoisoned
      */
      constexpr T unpoisoned_value() const {
         T r = value();
         CT::unpoison(r);
         return r;
      }

      /**
      * Unsafe conversion to bool
      *
      * This conversion itself is (probably) constant time, but once the
      * mask is reduced to a simple bool, it's entirely possible for the
      * compiler to perform range analysis on the values, since there are just
      * the two. As a consequence even if the caller is not using this in an
      * obviously branchy way (`if(mask.as_bool()) ...`) a smart compiler
      * may introduce branches depending on the value.
      */
      constexpr bool as_bool() const { return unpoisoned_value() != 0; }

      /**
      * Return a Choice based on this mask
      */
      constexpr CT::Choice as_choice() const {
         if constexpr(sizeof(T) >= sizeof(Choice::underlying_type)) {
            return CT::Choice::from_mask(static_cast<Choice::underlying_type>(unpoisoned_value()));
         } else {
            return CT::Choice::from_int(unpoisoned_value());
         }
      }

      /**
      * Return the underlying value of the mask
      */
      constexpr T value() const { return value_barrier<T>(m_mask); }

      constexpr void _const_time_poison() const { CT::poison(m_mask); }

      constexpr void _const_time_unpoison() const { CT::unpoison(m_mask); }

   private:
      constexpr explicit Mask(T m) : m_mask(m) {}

      T m_mask;
};

/**
* A CT::Option<T> is either a valid T, or not
*
* To maintain constant time behavior a value must always be stored.
* A CT::Choice tracks if the value is valid or not. It is not possible
* to access the inner value if the Choice is unset.
*/
template <typename T>
class Option final {
   public:
      /// Construct an Option which contains the specified value, and is set or not
      constexpr Option(T v, Choice valid) : m_has_value(valid), m_value(std::move(v)) {}

      /// Construct a set option with the provided value
      constexpr explicit Option(T v) : Option(std::move(v), Choice::yes()) {}

      /// Construct an unset option with a default inner value
      constexpr Option()
         requires std::default_initializable<T>
            : Option(T(), Choice::no()) {}

      /// Return true if this Option contains a value
      constexpr Choice has_value() const { return m_has_value; }

      /**
      * Apply a function to the inner value and return a new Option
      * which contains that value. This is constant time only if @p f is.
      *
      * @note The function will always be called, even if the Option is None. It
      *       must be prepared to handle any possible state of T.
      */
      template <std::invocable<const T&> F>
      constexpr auto transform(F f) const -> Option<std::remove_cvref_t<std::invoke_result_t<F, const T&>>> {
         return {f(m_value), m_has_value};
      }

      /// Either returns the value or throws an exception
      constexpr const T& value() const {
         BOTAN_STATE_CHECK(m_has_value.as_bool());
         return m_value;
      }

      /// Returns either the inner value or the alternative, in constant time
      ///
      /// This variant is used for types which explicitly define a function
      /// conditional_assign which takes a CT::Choice as the conditional.
      constexpr T value_or(T other) const
         requires ct_conditional_assignable<T>
      {
         other.conditional_assign(m_has_value, m_value);
         return other;
      }

      /// Returns either the inner value or the alternative, in constant time
      ///
      /// This variant is used for integer types where CT::Mask can perform
      /// a constant time selection
      constexpr T value_or(T other) const
         requires std::unsigned_integral<T>
      {
         auto mask = CT::Mask<T>::from_choice(m_has_value);
         return mask.select(m_value, other);
      }

      /// Convert this Option into a std::optional
      ///
      /// This is not constant time, leaking if the Option had a
      /// value or not
      constexpr std::optional<T> as_optional_vartime() const {
         if(m_has_value.as_bool()) {
            return {m_value};
         } else {
            return {};
         }
      }

      /// Return a new CT::Option that is set if @p also is set as well
      constexpr CT::Option<T> operator&&(CT::Choice also) { return CT::Option<T>(m_value, m_has_value && also); }

   private:
      Choice m_has_value;
      T m_value;
};

/**
* Conditional memory copy (constant time)
*
* If mask is set, then sets dest to if_set, otherwise sets dest to if_unset
*/
template <typename T>
constexpr inline Mask<T> conditional_copy_mem(Mask<T> mask, T* dest, const T* if_set, const T* if_unset, size_t elems) {
   mask.select_n(dest, if_set, if_unset, elems);
   return mask;
}

template <typename T>
constexpr inline Mask<T> conditional_copy_mem(T cnd, T* dest, const T* if_set, const T* if_unset, size_t elems) {
   const auto mask = CT::Mask<T>::expand(cnd);
   return CT::conditional_copy_mem(mask, dest, if_set, if_unset, elems);
}

/**
* Conditional memory assignment (constant time)
*
* If mask is set overwrites dest with src
*/
template <typename T>
constexpr inline Mask<T> conditional_assign_mem(T cnd, T* dest, const T* src, size_t elems) {
   const auto mask = CT::Mask<T>::expand(cnd);
   mask.select_n(dest, src, dest, elems);
   return mask;
}

/**
* Conditional memory assignment (constant time)
*
* If mask is set overwrites dest with src
*/
template <typename T>
constexpr inline Mask<T> conditional_assign_mem(Choice cnd, T* dest, const T* src, size_t elems) {
   const auto mask = CT::Mask<T>::from_choice(cnd);
   mask.select_n(dest, src, dest, elems);
   return mask;
}

template <typename T>
constexpr inline void conditional_swap(bool cnd, T& x, T& y) {
   const auto swap = CT::Mask<T>::expand(cnd);
   swap.conditional_swap(x, y);
}

template <typename T>
constexpr inline void conditional_swap_ptr(bool cnd, T& x, T& y) {
   uintptr_t xp = reinterpret_cast<uintptr_t>(x);
   uintptr_t yp = reinterpret_cast<uintptr_t>(y);

   conditional_swap<uintptr_t>(cnd, xp, yp);

   x = reinterpret_cast<T>(xp);  // NOLINT(*-no-int-to-ptr)
   y = reinterpret_cast<T>(yp);  // NOLINT(*-no-int-to-ptr)
}

template <typename T>
constexpr inline CT::Mask<T> all_zeros(const T elem[], size_t len) {
   T sum = 0;
   for(size_t i = 0; i != len; ++i) {
      sum |= elem[i];
   }
   return CT::Mask<T>::is_zero(sum);
}

/**
* Compare two arrays of equal size and return a Mask indicating if
* they are equal or not. The mask is set if they are identical.
*/
template <typename T>
constexpr inline CT::Mask<T> is_equal(const T x[], const T y[], size_t len) {
   if(std::is_constant_evaluated()) {
      T difference = 0;

      for(size_t i = 0; i != len; ++i) {
         difference = difference | (x[i] ^ y[i]);
      }

      return CT::Mask<T>::is_zero(difference);
   } else {
      volatile T difference = 0;

      for(size_t i = 0; i != len; ++i) {
         difference = difference | (x[i] ^ y[i]);
      }

      return CT::Mask<T>::is_zero(difference);
   }
}

/**
* Compare two spans and return a Mask which is set iff they were identical.
*
* If the spans are of different length then the function returns early without
* looking at either span
*/
template <typename T>
constexpr inline CT::Mask<T> is_equal(std::span<const T> x, std::span<const T> y) {
   if(x.size() != y.size()) {
      return CT::Mask<T>::cleared();
   }

   return is_equal(x.data(), y.data(), x.size());
}

/**
* Compare two arrays of equal size and return a Mask indicating if
* they are equal or not. The mask is set if they differ.
*/
template <typename T>
constexpr inline CT::Mask<T> is_not_equal(const T x[], const T y[], size_t len) {
   return ~CT::is_equal(x, y, len);
}

/**
* Constant time conditional copy out with offset
*
* If accept is set and offset <= input_length, sets output[0..] to
* input[offset:input_length] and returns input_length - offset. The
* remaining bytes of output are zeroized.
*
* Otherwise, output is zeroized, and returns an empty Ct::Option
*
* The input and output spans may not overlap, and output must be at
* least as large as input.
*
* This function attempts to avoid leaking the following to side channels
*  - if accept was set or not
*  - the value of offset
*  - the value of input
*
* This function leaks the length of the input
*/
BOTAN_TEST_API
CT::Option<size_t> copy_output(CT::Choice accept,
                               std::span<uint8_t> output,
                               std::span<const uint8_t> input,
                               size_t offset);

size_t count_leading_zero_bytes(std::span<const uint8_t> input);

secure_vector<uint8_t> strip_leading_zeros(std::span<const uint8_t> input);

}  // namespace Botan::CT

#if defined(BOTAN_BUILD_COMPILER_IS_MSVC)
   #include <intrin.h>
#endif

namespace Botan {

/**
* Perform a 64x64->128 bit multiplication
*/
constexpr inline void mul64x64_128(uint64_t a, uint64_t b, uint64_t* lo, uint64_t* hi) {
   if(!std::is_constant_evaluated()) {
#if defined(BOTAN_BUILD_COMPILER_IS_MSVC) && defined(BOTAN_TARGET_ARCH_IS_X86_64)
      *lo = _umul128(a, b, hi);
      return;

#elif defined(BOTAN_BUILD_COMPILER_IS_MSVC) && defined(BOTAN_TARGET_ARCH_IS_ARM64)
      *lo = a * b;
      *hi = __umulh(a, b);
      return;
#endif
   }

#if defined(BOTAN_TARGET_HAS_NATIVE_UINT128)
   const uint128_t r = static_cast<uint128_t>(a) * b;
   *hi = (r >> 64) & 0xFFFFFFFFFFFFFFFF;
   *lo = (r) & 0xFFFFFFFFFFFFFFFF;
#else

   /*
   * Do a 64x64->128 multiply using four 32x32->64 multiplies plus
   * some adds and shifts.
   */
   const size_t HWORD_BITS = 32;
   const uint32_t HWORD_MASK = 0xFFFFFFFF;

   const uint32_t a_hi = (a >> HWORD_BITS);
   const uint32_t a_lo = (a & HWORD_MASK);
   const uint32_t b_hi = (b >> HWORD_BITS);
   const uint32_t b_lo = (b & HWORD_MASK);

   const uint64_t x0 = static_cast<uint64_t>(a_hi) * b_hi;
   const uint64_t x1 = static_cast<uint64_t>(a_lo) * b_hi;
   const uint64_t x2 = static_cast<uint64_t>(a_hi) * b_lo;
   const uint64_t x3 = static_cast<uint64_t>(a_lo) * b_lo;

   // this cannot overflow as (2^32-1)^2 + 2^32-1 + 2^32-1 = 2^64-1
   const uint64_t middle = x2 + (x3 >> HWORD_BITS) + (x1 & HWORD_MASK);

   // likewise these cannot overflow
   *hi = x0 + (middle >> HWORD_BITS) + (x1 >> HWORD_BITS);
   *lo = (middle << HWORD_BITS) + (x3 & HWORD_MASK);
#endif
}

}  // namespace Botan

namespace Botan {

class donna128 final {
   public:
      constexpr explicit donna128(uint64_t l = 0, uint64_t h = 0) : m_lo(l), m_hi(h) {}

      template <std::unsigned_integral T>
      constexpr friend donna128 operator>>(const donna128& x, T shift) {
         donna128 z = x;

         if(shift > 64) {
            z.m_lo = z.m_hi >> (shift - 64);
            z.m_hi = 0;
         } else if(shift == 64) {
            z.m_lo = z.m_hi;
            z.m_hi = 0;
         } else if(shift > 0) {
            const uint64_t carry = z.m_hi << static_cast<size_t>(64 - shift);
            z.m_hi >>= shift;
            z.m_lo >>= shift;
            z.m_lo |= carry;
         }

         return z;
      }

      template <std::unsigned_integral T>
      constexpr friend donna128 operator<<(const donna128& x, T shift) {
         donna128 z = x;
         if(shift > 64) {
            z.m_hi = z.m_lo << (shift - 64);
            z.m_lo = 0;
         } else if(shift == 64) {
            z.m_hi = z.m_lo;
            z.m_lo = 0;
         } else if(shift > 0) {
            const uint64_t carry = z.m_lo >> static_cast<size_t>(64 - shift);
            z.m_lo = (z.m_lo << shift);
            z.m_hi = (z.m_hi << shift) | carry;
         }

         return z;
      }

      constexpr friend uint64_t operator&(const donna128& x, uint64_t mask) { return x.m_lo & mask; }

      constexpr uint64_t operator&=(uint64_t mask) {
         m_hi = 0;
         m_lo &= mask;
         return m_lo;
      }

      constexpr donna128& operator+=(const donna128& x) {
         m_lo += x.m_lo;
         m_hi += x.m_hi;

         const uint64_t carry = CT::Mask<uint64_t>::is_lt(m_lo, x.m_lo).if_set_return(1);
         m_hi += carry;
         return *this;
      }

      constexpr donna128& operator+=(uint64_t x) {
         m_lo += x;
         const uint64_t carry = CT::Mask<uint64_t>::is_lt(m_lo, x).if_set_return(1);
         m_hi += carry;
         return *this;
      }

      constexpr uint64_t lo() const { return m_lo; }

      constexpr uint64_t hi() const { return m_hi; }

      constexpr explicit operator uint64_t() const { return lo(); }

   private:
      uint64_t m_lo = 0;
      uint64_t m_hi = 0;
};

template <std::integral T>
constexpr inline donna128 operator*(const donna128& x, T y) {
   BOTAN_ARG_CHECK(x.hi() == 0, "High 64 bits of donna128 set to zero during multiply");

   uint64_t lo = 0;
   uint64_t hi = 0;
   mul64x64_128(x.lo(), static_cast<uint64_t>(y), &lo, &hi);
   return donna128(lo, hi);
}

template <std::integral T>
constexpr inline donna128 operator*(T y, const donna128& x) {
   return x * y;
}

constexpr inline donna128 operator+(const donna128& x, const donna128& y) {
   donna128 z = x;
   z += y;
   return z;
}

constexpr inline donna128 operator+(const donna128& x, uint64_t y) {
   donna128 z = x;
   z += y;
   return z;
}

constexpr inline donna128 operator|(const donna128& x, const donna128& y) {
   return donna128(x.lo() | y.lo(), x.hi() | y.hi());
}

constexpr inline donna128 operator|(const donna128& x, uint64_t y) {
   return donna128(x.lo() | y, x.hi());
}

constexpr inline uint64_t carry_shift(const donna128& a, size_t shift) {
   return (a >> shift).lo();
}

constexpr inline uint64_t combine_lower(const donna128& a, size_t s1, const donna128& b, size_t s2) {
   const donna128 z = (a >> s1) | (b << s2);
   return z.lo();
}

#if defined(BOTAN_TARGET_HAS_NATIVE_UINT128)
inline uint64_t carry_shift(const uint128_t a, size_t shift) {
   return static_cast<uint64_t>(a >> shift);
}

inline uint64_t combine_lower(const uint128_t a, size_t s1, const uint128_t b, size_t s2) {
   return static_cast<uint64_t>((a >> s1) | (b << s2));
}
#endif

}  // namespace Botan

namespace Botan {

/**
* No_Filesystem_Access Exception
*/
class No_Filesystem_Access final : public Exception {
   public:
      No_Filesystem_Access() : Exception("No filesystem access enabled.") {}
};

BOTAN_TEST_API bool has_filesystem_impl();

BOTAN_TEST_API std::vector<std::string> get_files_recursive(std::string_view dir);

}  // namespace Botan

namespace Botan {

namespace fmt_detail {

inline void do_fmt(std::ostringstream& oss, std::string_view format) {
   oss << format;
}

template <typename T, typename... Ts>
void do_fmt(std::ostringstream& oss, std::string_view format, const T& val, const Ts&... rest) {
   size_t i = 0;

   while(i < format.size()) {
      if(format[i] == '{' && (format.size() > (i + 1)) && format.at(i + 1) == '}') {
         oss << val;
         return do_fmt(oss, format.substr(i + 2), rest...);
      } else {
         oss << format[i];
      }

      i += 1;
   }
}

}  // namespace fmt_detail

/**
* Simple formatter utility.
*
* Should be replaced with std::format once that's available on all our
* supported compilers.
*
* '{}' markers in the format string are replaced by the arguments.
* Unlike std::format, there is no support for escaping or for any kind
* of conversion flags.
*/
template <typename... T>
std::string fmt(std::string_view format, const T&... args) {
   std::ostringstream oss;
   oss.imbue(std::locale::classic());
   fmt_detail::do_fmt(oss, format, args...);
   return oss.str();
}

}  // namespace Botan

namespace Botan {

// Helper for defining GFNI constants
consteval uint64_t gfni_matrix(std::string_view s) {
   uint64_t matrix = 0;
   size_t bit_cnt = 0;
   uint8_t row = 0;

   for(const char c : s) {
      if(c == ' ' || c == '\n') {
         continue;
      }
      if(c != '0' && c != '1') {
         throw std::runtime_error("gfni_matrix: invalid bit value");
      }

      if(c == '1') {
         row |= 0x80 >> (7 - bit_cnt % 8);
      }
      bit_cnt++;

      if(bit_cnt % 8 == 0) {
         matrix <<= 8;
         matrix |= row;
         row = 0;
      }
   }

   if(bit_cnt != 64) {
      throw std::runtime_error("gfni_matrix: invalid bit count");
   }

   return matrix;
}

}  // namespace Botan

namespace Botan {

/**
* HMAC
*/
class HMAC final : public MessageAuthenticationCode {
   public:
      void clear() override;
      std::string name() const override;
      std::unique_ptr<MessageAuthenticationCode> new_object() const override;

      size_t output_length() const override;

      Key_Length_Specification key_spec() const override;

      bool has_keying_material() const override;

      /**
      * @param hash the hash to use for HMACing
      */
      explicit HMAC(std::unique_ptr<HashFunction> hash);

   private:
      void add_data(std::span<const uint8_t> input) override;
      void final_result(std::span<uint8_t> output) override;
      void key_schedule(std::span<const uint8_t> key) override;

      std::unique_ptr<HashFunction> m_hash;
      secure_vector<uint8_t> m_ikey, m_okey;
      size_t m_hash_output_length;
      size_t m_hash_block_size;
};

}  // namespace Botan

namespace Botan {

template <std::unsigned_integral T>
constexpr inline std::optional<T> checked_add(T a, T b) {
   const T r = a + b;
   if(r < a || r < b) {
      return {};
   }
   return r;
}

template <std::unsigned_integral T>
constexpr std::optional<T> checked_sub(T a, T b) {
   if(b > a) {
      return {};
   }
   return a - b;
}

template <std::unsigned_integral T, std::unsigned_integral... Ts>
   requires all_same_v<T, Ts...>
constexpr inline std::optional<T> checked_add(T a, T b, Ts... rest) {
   if(auto r = checked_add(a, b)) {
      return checked_add(r.value(), rest...);
   } else {
      return {};
   }
}

template <std::unsigned_integral T>
constexpr inline std::optional<T> checked_mul(T a, T b) {
   // Multiplication by 1U is a hack to work around C's insane
   // integer promotion rules.
   // https://stackoverflow.com/questions/24795651
   const T r = (1U * a) * b;
   // If a == 0 then the multiply certainly did not overflow
   // Otherwise r / a == b unless overflow occurred
   if(a != 0 && r / a != b) {
      return {};
   }
   return r;
}

template <typename RT, typename ExceptionType, typename AT>
   requires std::integral<strong_type_wrapped_type<RT>> && std::integral<strong_type_wrapped_type<AT>>
constexpr RT checked_cast_to_or_throw(AT i, std::string_view error_msg_on_fail) {
   const auto unwrapped_input = unwrap_strong_type(i);

   const auto unwrapped_result = static_cast<strong_type_wrapped_type<RT>>(unwrapped_input);
   if(unwrapped_input != static_cast<strong_type_wrapped_type<AT>>(unwrapped_result)) [[unlikely]] {
      throw ExceptionType(error_msg_on_fail);
   }

   return wrap_strong_type<RT>(unwrapped_result);
}

template <typename RT, typename AT>
   requires std::integral<strong_type_wrapped_type<RT>> && std::integral<strong_type_wrapped_type<AT>>
constexpr RT checked_cast_to(AT i) {
   return checked_cast_to_or_throw<RT, Internal_Error>(i, "Error during integer conversion");
}

/**
* SWAR (SIMD within a word) byte-by-byte comparison
*
* This individually compares each byte of the provided words.
* It returns a mask which contains, for each byte, 0xFF if
* the byte in @p a was less than the byte in @p b. Otherwise the
* mask is 00.
*
* This implementation assumes that the high bits of each byte
* in both @p a and @p b are clear! It is possible to support the
* full range of bytes, but this requires additional comparisons.
*/
template <std::unsigned_integral T>
constexpr T swar_lt(T a, T b) {
   // The constant 0x808080... as a T
   constexpr T hi1 = (static_cast<T>(-1) / 255) << 7;
   // The constant 0x7F7F7F... as a T
   constexpr T lo7 = static_cast<T>(~hi1);
   T r = (lo7 - a + b) & hi1;
   // Currently the mask is 80 if lt, otherwise 00. Convert to FF/00
   return (r << 1) - (r >> 7);
}

/**
* SWAR (SIMD within a word) byte-by-byte comparison
*
* This individually compares each byte of the provided words.
* It returns a mask which contains, for each byte, 0x80 if
* the byte in @p a was less than the byte in @p b. Otherwise the
* mask is 00.
*
* This implementation assumes that the high bits of each byte
* in both @p lower and @p upper are clear! It is possible to support the
* full range of bytes, but this requires additional comparisons.
*/
template <std::unsigned_integral T>
constexpr T swar_in_range(T v, T lower, T upper) {
   // The constant 0x808080... as a T
   constexpr T hi1 = (static_cast<T>(-1) / 255) << 7;
   // The constant 0x7F7F7F... as a T
   constexpr T lo7 = ~hi1;

   const T sub = ((v | hi1) - (lower & lo7)) ^ ((v ^ (~lower)) & hi1);
   const T a_lo = sub & lo7;
   const T a_hi = sub & hi1;
   return (lo7 - a_lo + upper) & hi1 & ~a_hi;
}

/**
* Return the index of the first byte with the high bit set
*/
template <std::unsigned_integral T>
constexpr size_t index_of_first_set_byte(T v) {
   // The constant 0x010101... as a T
   constexpr T lo1 = (static_cast<T>(-1) / 255);
   // The constant 0x808080... as a T
   constexpr T hi1 = lo1 << 7;
   // How many bits to shift in order to get the top byte
   constexpr size_t bits = (sizeof(T) * 8) - 8;

   return static_cast<size_t>((((((v & hi1) - 1) & lo1) * lo1) >> bits) - 1);
}

}  // namespace Botan

/*
* GCC and Clang use string identifiers to tag ISA extensions (eg using the
* `target` function attribute).
*
* This file consolidates the actual definition of such target attributes
*/

#if defined(BOTAN_TARGET_ARCH_IS_X86_FAMILY)

   #define BOTAN_FN_ISA_SIMD_4X32 BOTAN_FUNC_ISA("ssse3")
   #define BOTAN_FN_ISA_SIMD_2X64 BOTAN_FUNC_ISA("ssse3")
   #define BOTAN_FN_ISA_SIMD_4X64 BOTAN_FUNC_ISA("avx2")
   #define BOTAN_FN_ISA_SIMD_8X64 BOTAN_FN_ISA_AVX512
   #define BOTAN_FN_ISA_CLMUL BOTAN_FUNC_ISA("pclmul,ssse3")
   #define BOTAN_FN_ISA_AESNI BOTAN_FUNC_ISA("aes,ssse3")
   #define BOTAN_FN_ISA_SHANI BOTAN_FUNC_ISA("sha,ssse3,sse4.1")
   #define BOTAN_FN_ISA_SHA512 BOTAN_FUNC_ISA("sha512,avx2")
   #define BOTAN_FN_ISA_BMI2 BOTAN_FUNC_ISA("bmi,bmi2")
   #define BOTAN_FN_ISA_RNG BOTAN_FUNC_ISA("rdrnd")
   #define BOTAN_FN_ISA_SSE2 BOTAN_FUNC_ISA("sse2")
   #define BOTAN_FN_ISA_AVX2 BOTAN_FUNC_ISA("avx2")
   #define BOTAN_FN_ISA_AVX2_BMI2 BOTAN_FUNC_ISA("avx2,bmi,bmi2")
   #define BOTAN_FN_ISA_AVX2_GFNI BOTAN_FUNC_ISA("avx2,gfni")
   #define BOTAN_FN_ISA_AVX2_VAES BOTAN_FUNC_ISA("vaes,avx2")
   #define BOTAN_FN_ISA_AVX2_SM3 BOTAN_FUNC_ISA("sm3,avx2")
   #define BOTAN_FN_ISA_AVX2_SM4 BOTAN_FUNC_ISA("sm4,avx2")
   #define BOTAN_FN_ISA_AVX512 \
      BOTAN_FUNC_ISA("avx512f,avx512dq,avx512bw,avx512vl,avx512vbmi,avx512vbmi2,avx512bitalg,avx512ifma")
   #define BOTAN_FN_ISA_AVX512_CLMUL \
      BOTAN_FUNC_ISA("avx512f,avx512dq,avx512bw,avx512vl,avx512vbmi,avx512vbmi2,avx512bitalg,pclmul,vpclmulqdq")
   #define BOTAN_FN_ISA_AVX512_BMI2 \
      BOTAN_FUNC_ISA("avx512f,avx512dq,avx512bw,avx512vl,avx512vbmi,avx512vbmi2,avx512bitalg,avx512ifma,bmi,bmi2")
   #define BOTAN_FN_ISA_AVX512_GFNI \
      BOTAN_FUNC_ISA("avx512f,avx512dq,avx512bw,avx512vl,avx512vbmi,avx512vbmi2,avx512bitalg,avx512ifma,gfni")

   #define BOTAN_FN_ISA_HWAES BOTAN_FN_ISA_AESNI
#endif

#if defined(BOTAN_TARGET_ARCH_IS_ARM64)

   #define BOTAN_FN_ISA_SIMD_4X32 BOTAN_FUNC_ISA("+simd")
   #define BOTAN_FN_ISA_CLMUL BOTAN_FUNC_ISA("+crypto+aes")
   #define BOTAN_FN_ISA_AES BOTAN_FUNC_ISA("+crypto+aes")
   #define BOTAN_FN_ISA_SHA2 BOTAN_FUNC_ISA("+crypto+sha2")
   #define BOTAN_FN_ISA_SM3 BOTAN_FUNC_ISA("arch=armv8.2-a+sm4")
   #define BOTAN_FN_ISA_SM4 BOTAN_FUNC_ISA("arch=armv8.2-a+sm4")
   #define BOTAN_FN_ISA_SHA512 BOTAN_FUNC_ISA("arch=armv8.2-a+sha3")

   #define BOTAN_FN_ISA_HWAES BOTAN_FN_ISA_AES
#endif

#if defined(BOTAN_TARGET_ARCH_IS_ARM32)
   #define BOTAN_FN_ISA_SIMD_4X32 BOTAN_FUNC_ISA("fpu=neon")
#endif

#if defined(BOTAN_TARGET_ARCH_IS_PPC_FAMILY)

   #define BOTAN_FN_ISA_SIMD_4X32 BOTAN_FUNC_ISA("altivec")
   #define BOTAN_FN_ISA_CLMUL BOTAN_FUNC_ISA("vsx,crypto")
   #define BOTAN_FN_ISA_AES BOTAN_FUNC_ISA("vsx,crypto")
   #define BOTAN_FN_ISA_RNG BOTAN_FUNC_ISA("cpu=power9")

   #define BOTAN_FN_ISA_HWAES BOTAN_FN_ISA_AES
#endif

#if defined(BOTAN_TARGET_ARCH_IS_LOONGARCH64)

   #define BOTAN_FN_ISA_SIMD_4X32 BOTAN_FUNC_ISA("lsx")

#endif

#if defined(BOTAN_TARGET_ARCH_IS_WASM)

   #define BOTAN_FN_ISA_SIMD_4X32 BOTAN_FUNC_ISA("simd128")
   #define BOTAN_FN_ISA_SIMD_2X64 BOTAN_FUNC_ISA("simd128")

#endif


namespace Botan {

/**
 * Integer encoding defined in NIST SP.800-185 that can be unambiguously
 * parsed from the beginning of the string.
 *
 * This function does not allocate any memory and requires the caller to
 * provide a sufficiently large @p buffer. For a given @p x, this will
 * need exactly keccak_int_encoding_size() bytes. For an arbitrary @p x
 * it will generate keccak_max_int_encoding_size() bytes at most.
 *
 * @param buffer  buffer to write the left-encoding of @p x to.
 *                It is assumed that the buffer will hold at least
 *                keccak_int_encoding_size() bytes.
 * @param x       the integer to be left-encoded
 * @return        the byte span that represents the bytes written to @p buffer.
 */
BOTAN_TEST_API std::span<const uint8_t> keccak_int_left_encode(std::span<uint8_t> buffer, size_t x);

/**
 * Integer encoding defined in NIST SP.800-185 that can be unambiguously
 * parsed from the end of the string.
 *
 * This function does not allocate any memory and requires the caller to
 * provide a sufficiently large @p buffer. For a given @p x, this will
 * need exactly keccak_int_encoding_size() bytes. For an arbitrary @p x
 * it will generate keccak_max_int_encoding_size() bytes at most.
 *
 * @param out  buffer to write the right-encoding of @p x to.
 *             It is assumed that the buffer will hold at least
 *             keccak_int_encoding_size() bytes.
 * @param x    the integer to be right-encoded
 * @return     the byte span that represents the bytes written to @p buffer.
 */
BOTAN_TEST_API std::span<const uint8_t> keccak_int_right_encode(std::span<uint8_t> out, size_t x);

/**
 * @returns the required bytes for encodings of keccak_int_left_encode() or
 *          keccak_int_right_encode() given an integer @p x
 */
BOTAN_TEST_API size_t keccak_int_encoding_size(size_t x);

/**
 * @returns the maximum required bytes for encodings of keccak_int_left_encode() or
 *          keccak_int_right_encode()
 */
constexpr size_t keccak_max_int_encoding_size() {
   return sizeof(size_t) + 1 /* the length tag */;
}

template <typename T>
concept updatable_object = requires(T& a, std::span<const uint8_t> span) { a.update(span); };

template <typename T>
concept appendable_object = requires(T& a, std::span<const uint8_t> s) { a.insert(a.end(), s.begin(), s.end()); };

template <typename T>
concept absorbing_object = updatable_object<T> || appendable_object<T>;

/**
 * This is a combination of the functions encode_string() and bytepad() defined
 * in NIST SP.800-185 Section 2.3. Additionally, the result is directly streamed
 * into the provided XOF to avoid unnecessary memory allocation or a byte vector.
 *
 * @param sink         the XOF or byte vector to absorb the @p byte_strings into
 * @param padding_mod  the modulus value to create a padding for (NIST calls this 'w')
 * @param byte_strings a variable-length list of byte strings to be encoded and
 *                     absorbed into the given @p xof
 * @returns the number of bytes absorbed into the @p xof
 */
template <absorbing_object T, typename... Ts>
   requires(std::constructible_from<std::span<const uint8_t>, Ts> && ...)
size_t keccak_absorb_padded_strings_encoding(T& sink, size_t padding_mod, Ts... byte_strings) {
   BOTAN_ASSERT_NOMSG(padding_mod > 0);

   // used as temporary storage for all integer encodings in this function
   std::array<uint8_t, keccak_max_int_encoding_size()> int_encoding_buffer{};

   // absorbs byte strings and counts the number of absorbed bytes
   size_t bytes_absorbed = 0;
   auto absorb = [&](std::span<const uint8_t> bytes) {
      if constexpr(updatable_object<T>) {
         sink.update(bytes);
      } else if constexpr(appendable_object<T>) {
         sink.insert(sink.end(), bytes.begin(), bytes.end());
      }
      bytes_absorbed += bytes.size();
   };

   // encodes a given string and absorbs it into the XOF straight away
   auto encode_string_and_absorb = [&](std::span<const uint8_t> bytes) {
      absorb(keccak_int_left_encode(int_encoding_buffer, bytes.size() * 8));
      absorb(bytes);
   };

   // absorbs as many zero-bytes as requested into the XOF
   auto absorb_padding = [&](size_t padding_bytes) {
      for(size_t i = 0; i < padding_bytes; ++i) {
         const uint8_t zero_byte = 0;
         absorb({&zero_byte, 1});
      }
   };

   // implementation of bytepad(encode_string(Ts) || ...) that absorbs the result
   // staight into the given xof
   absorb(keccak_int_left_encode(int_encoding_buffer, padding_mod));
   (encode_string_and_absorb(byte_strings), ...);
   absorb_padding(padding_mod - (bytes_absorbed % padding_mod));

   return bytes_absorbed;
}

}  // namespace Botan

namespace Botan {

/**
 * A generic sponge construction with a fixed state size defined in terms of
 * "words" of an unsigned integral type.
 *
 * This is meant to be used as a base class for specific sponge constructions
 * like Keccak or Ascon.
 */
template <size_t words, std::unsigned_integral word = uint64_t>
class Sponge {
   public:
      using word_t = word;
      using state_t = std::array<word, words>;
      constexpr static size_t word_bytes = sizeof(word);
      constexpr static size_t word_bits = word_bytes * 8;

      struct Config final {
            size_t bit_rate;        /// The number of bits that using algorithms can modify between permutations
            state_t initial_state;  /// The state of the sponge state at initialization
      };

   public:
      constexpr explicit Sponge(Config config) : m_S(config.initial_state), m_S_cursor(0), m_bit_rate(config.bit_rate) {
         BOTAN_ARG_CHECK(m_bit_rate % word_bits == 0 && m_bit_rate < words * word_bits, "Invalid sponge bit rate");
      }

      constexpr static size_t state_bytes() { return sizeof(state_t); }

      constexpr static size_t state_bits() { return state_bytes() * 8; }

      constexpr size_t bit_rate() const { return m_bit_rate; }

      constexpr size_t byte_rate() const { return m_bit_rate / 8; }

      constexpr size_t bit_capacity() const { return state_bits() - bit_rate(); }

      constexpr size_t byte_capacity() const { return state_bytes() - byte_rate(); }

      constexpr auto& state() { return m_S; }

      size_t cursor() const { return m_S_cursor; }

      size_t& _cursor() { return m_S_cursor; }

   protected:
      void reset_cursor() { m_S_cursor = 0; }

   private:
      state_t m_S;
      size_t m_S_cursor;
      size_t m_bit_rate;
};

}  // namespace Botan

namespace Botan {

struct KeccakPadding {
      uint64_t padding;  /// The padding bits in little-endian order
      uint8_t bit_len;   /// The number of relevant bits in 'padding'

      /// NIST FIPS 202 Section 6.1
      static constexpr KeccakPadding sha3() { return {.padding = 0b10 /* little-endian */, .bit_len = 2}; }

      /// NIST FIPS 202 Section 6.2
      static constexpr KeccakPadding shake() { return {.padding = 0b1111, .bit_len = 4}; }

      /// NIST SP.800-185 Section 3.3
      static constexpr KeccakPadding cshake() { return {.padding = 0b00, .bit_len = 2}; }

      /// Keccak submission, prior to the introduction of an algorithm specific padding
      static constexpr KeccakPadding keccak1600() { return {.padding = 0, .bit_len = 0}; }
};

/**
* KECCAK FIPS
*
* This file implements Keccak[c] which is specified by NIST FIPS 202 [1], where
* "c" is the variable capacity of this hash primitive. Keccak[c] is not  a
* general purpose hash function, but used as the basic primitive for algorithms
* such as SHA-3 and KMAC. This is not to be confused with the "informal" general purpose hash
* function which is referred to as "Keccak" and apparently refers to the final
* submission version of the Keccak submission in the SHA-3 contest, possibly
* what is released by NIST under the name "KECCAK - Final Algorithm Package" [2].
* See also the file keccak.h for the details how the keccak hash function is defined
* in terms of the Keccak[c] – a detail which cannot be found in [1].
*
*
*
* [1] FIPS PUB 202 – FEDERAL INFORMATION PROCESSING STANDARDS PUBLICATION – SHA-3 Standard: Permutation-Based Hash and Extendable-Output Functions
*       https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.202.pdf#page=28
* [2] https://csrc.nist.gov/projects/hash-functions/sha-3-project
*/
class Keccak_Permutation final : public Sponge<25, uint64_t> {
   public:
      struct Config {
            size_t capacity_bits;
            KeccakPadding padding;
      };

   public:
      /**
        * @brief Instantiate a Keccak permutation
        *
        * @param config Keccak parameter configuration
        */
      constexpr explicit Keccak_Permutation(Config config) :
            Sponge({.bit_rate = state_bits() - config.capacity_bits, .initial_state = {}}), m_padding(config.padding) {}

      void clear();
      std::string provider() const;

      /**
      * @brief Absorb input data into the Keccak sponge
      *
      * This method can be called multiple times with arbitrary-length buffers.
      *
      * @param input the input data
      */
      void absorb(std::span<const uint8_t> input);

      /**
      * @brief Expand output data from the current Keccak state
      *
      * This method can be called multiple times with arbitrary-length buffers.
      *
      * @param output the designated output memory
      */
      void squeeze(std::span<uint8_t> output);

      /**
      * @brief Add final padding (as provided in the constructor) and permute
      */
      void finish();

      /**
       * The Keccak permutation function
       */
      void permute();

   private:
#if defined(BOTAN_HAS_KECCAK_PERM_BMI2)
      void permute_bmi2();
#endif

#if defined(BOTAN_HAS_KECCAK_PERM_AVX512)
      void permute_avx512();
#endif

   private:
      KeccakPadding m_padding;
};

}  // namespace Botan

namespace Botan {

/**
* Bit rotation left by a compile-time constant amount
* @param input the input word
* @return input rotated left by ROT bits
*/
template <size_t ROT, std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr T rotl(T input)
   requires(ROT > 0 && ROT < 8 * sizeof(T))
{
   return static_cast<T>((input << ROT) | (input >> (8 * sizeof(T) - ROT)));
}

/**
* Bit rotation right by a compile-time constant amount
* @param input the input word
* @return input rotated right by ROT bits
*/
template <size_t ROT, std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr T rotr(T input)
   requires(ROT > 0 && ROT < 8 * sizeof(T))
{
   return static_cast<T>((input >> ROT) | (input << (8 * sizeof(T) - ROT)));
}

/**
* SHA-2 Sigma style function
*/
template <size_t R1, size_t R2, size_t S, std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr T sigma(T x) {
   return rotr<R1>(x) ^ rotr<R2>(x) ^ (x >> S);
}

/**
* SHA-2 Sigma style function
*/
template <size_t R1, size_t R2, size_t R3, std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr T rho(T x) {
   return rotr<R1>(x) ^ rotr<R2>(x) ^ rotr<R3>(x);
}

/**
* Bit rotation left, variable rotation amount
* @param input the input word
* @param rot the number of bits to rotate, must be between 0 and sizeof(T)*8-1
* @return input rotated left by rot bits
*/
template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr T rotl_var(T input, size_t rot) {
   return rot ? static_cast<T>((input << rot) | (input >> (sizeof(T) * 8 - rot))) : input;
}

/**
* Bit rotation right, variable rotation amount
* @param input the input word
* @param rot the number of bits to rotate, must be between 0 and sizeof(T)*8-1
* @return input rotated right by rot bits
*/
template <std::unsigned_integral T>
BOTAN_FORCE_INLINE constexpr T rotr_var(T input, size_t rot) {
   return rot ? static_cast<T>((input >> rot) | (input << (sizeof(T) * 8 - rot))) : input;
}

}  // namespace Botan

namespace Botan {

BOTAN_FORCE_INLINE void Keccak_Permutation_round(uint64_t T[25], const uint64_t A[25], uint64_t RC) {
   const uint64_t C0 = A[0] ^ A[5] ^ A[10] ^ A[15] ^ A[20];
   const uint64_t C1 = A[1] ^ A[6] ^ A[11] ^ A[16] ^ A[21];
   const uint64_t C2 = A[2] ^ A[7] ^ A[12] ^ A[17] ^ A[22];
   const uint64_t C3 = A[3] ^ A[8] ^ A[13] ^ A[18] ^ A[23];
   const uint64_t C4 = A[4] ^ A[9] ^ A[14] ^ A[19] ^ A[24];

   const uint64_t D0 = rotl<1>(C0) ^ C3;
   const uint64_t D1 = rotl<1>(C1) ^ C4;
   const uint64_t D2 = rotl<1>(C2) ^ C0;
   const uint64_t D3 = rotl<1>(C3) ^ C1;
   const uint64_t D4 = rotl<1>(C4) ^ C2;

   const uint64_t B00 = A[0] ^ D1;
   const uint64_t B01 = rotl<44>(A[6] ^ D2);
   const uint64_t B02 = rotl<43>(A[12] ^ D3);
   const uint64_t B03 = rotl<21>(A[18] ^ D4);
   const uint64_t B04 = rotl<14>(A[24] ^ D0);
   T[0] = B00 ^ (~B01 & B02) ^ RC;
   T[1] = B01 ^ (~B02 & B03);
   T[2] = B02 ^ (~B03 & B04);
   T[3] = B03 ^ (~B04 & B00);
   T[4] = B04 ^ (~B00 & B01);

   const uint64_t B05 = rotl<28>(A[3] ^ D4);
   const uint64_t B06 = rotl<20>(A[9] ^ D0);
   const uint64_t B07 = rotl<3>(A[10] ^ D1);
   const uint64_t B08 = rotl<45>(A[16] ^ D2);
   const uint64_t B09 = rotl<61>(A[22] ^ D3);
   T[5] = B05 ^ (~B06 & B07);
   T[6] = B06 ^ (~B07 & B08);
   T[7] = B07 ^ (~B08 & B09);
   T[8] = B08 ^ (~B09 & B05);
   T[9] = B09 ^ (~B05 & B06);

   const uint64_t B10 = rotl<1>(A[1] ^ D2);
   const uint64_t B11 = rotl<6>(A[7] ^ D3);
   const uint64_t B12 = rotl<25>(A[13] ^ D4);
   const uint64_t B13 = rotl<8>(A[19] ^ D0);
   const uint64_t B14 = rotl<18>(A[20] ^ D1);
   T[10] = B10 ^ (~B11 & B12);
   T[11] = B11 ^ (~B12 & B13);
   T[12] = B12 ^ (~B13 & B14);
   T[13] = B13 ^ (~B14 & B10);
   T[14] = B14 ^ (~B10 & B11);

   const uint64_t B15 = rotl<27>(A[4] ^ D0);
   const uint64_t B16 = rotl<36>(A[5] ^ D1);
   const uint64_t B17 = rotl<10>(A[11] ^ D2);
   const uint64_t B18 = rotl<15>(A[17] ^ D3);
   const uint64_t B19 = rotl<56>(A[23] ^ D4);
   T[15] = B15 ^ (~B16 & B17);
   T[16] = B16 ^ (~B17 & B18);
   T[17] = B17 ^ (~B18 & B19);
   T[18] = B18 ^ (~B19 & B15);
   T[19] = B19 ^ (~B15 & B16);

   const uint64_t B20 = rotl<62>(A[2] ^ D3);
   const uint64_t B21 = rotl<55>(A[8] ^ D4);
   const uint64_t B22 = rotl<39>(A[14] ^ D0);
   const uint64_t B23 = rotl<41>(A[15] ^ D1);
   const uint64_t B24 = rotl<2>(A[21] ^ D2);
   T[20] = B20 ^ (~B21 & B22);
   T[21] = B21 ^ (~B22 & B23);
   T[22] = B22 ^ (~B23 & B24);
   T[23] = B23 ^ (~B24 & B20);
   T[24] = B24 ^ (~B20 & B21);
}

}  // namespace Botan

namespace Botan {

/**
* Kuznyechik
*/
class Kuznyechik final : public Botan::Block_Cipher_Fixed_Params<16, 32> {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;

      std::string name() const override { return "Kuznyechik"; }

      std::unique_ptr<BlockCipher> new_object() const override { return std::make_unique<Kuznyechik>(); }

      bool has_keying_material() const override;

   private:
      void key_schedule(std::span<const uint8_t> key) override;
      secure_vector<uint64_t> m_rke;
      secure_vector<uint64_t> m_rkd;
};

}  // namespace Botan

/**
 * @file loadstor.h
 *
 * @brief This header contains various helper functions to load and store
 *        unsigned integers in big- or little-endian byte order.
 *
 * Storing integer values in various ways (same for BE and LE):
 * @code {.cpp}
 *
 *   std::array<uint8_t, 8> bytes = store_le(some_uint64);
 *   std::array<uint8_t, 12> bytes = store_le(some_uint32_1, some_uint32_2, some_uint32_3, ...);
 *   auto bytes = store_le<std::vector<uint8_t>>(some_uint64);
 *   auto bytes = store_le<MyContainerStrongType>(some_uint64);
 *   auto bytes = store_le<std::vector<uint8_t>>(vector_of_ints);
 *   auto bytes = store_le<secure_vector<uint8_t>>(some_uint32_1, some_uint32_2, some_uint32_3, ...);
 *   store_le(bytes, some_uint64);
 *   store_le(concatenated_bytes, some_uint64_1, some_uint64_2, some_uint64_3, ...);
 *   store_le(concatenated_bytes, vector_of_ints);
 *   copy_out_le(short_concated_bytes, vector_of_ints); // stores as many bytes as required in the output buffer
 *
 * @endcode
 *
 * Loading integer values in various ways (same for BE and LE):
 * @code {.cpp}
 *
 *   uint64_t some_uint64 = load_le(bytes_8);
 *   auto some_int32s = load_le<std::vector<uint32_t>>(concatenated_bytes);
 *   auto some_int32s = load_le<std::vector<MyIntStrongType>>(concatenated_bytes);
 *   auto some_int32s = load_le(some_strong_typed_bytes);
 *   auto strong_int  = load_le<MyStrongTypedInteger>(concatenated_bytes);
 *   load_le(concatenated_bytes, out_some_uint64);
 *   load_le(concatenated_bytes, out_some_uint64_1, out_some_uint64_2, out_some_uint64_3, ...);
 *   load_le(out_vector_of_ints, concatenated_bytes);
 *
 * @endcode
 */

namespace Botan {

static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little,
              "Mixed endian systems are not supported");

/**
* Byte extraction
* @param byte_num which byte to extract, 0 == highest byte
* @param input the value to extract from
* @return byte byte_num of input
*/
template <typename T>
inline constexpr uint8_t get_byte_var(size_t byte_num, T input) {
   return static_cast<uint8_t>(input >> (((~byte_num) & (sizeof(T) - 1)) << 3));
}

/**
* Byte extraction
* @param input the value to extract from
* @return byte byte number B of input
*/
template <size_t B, typename T>
inline constexpr uint8_t get_byte(T input)
   requires(B < sizeof(T))
{
   const size_t shift = ((~B) & (sizeof(T) - 1)) << 3;
   return static_cast<uint8_t>((input >> shift) & 0xFF);
}

/**
* Make a uint16_t from two bytes
* @param i0 the first byte
* @param i1 the second byte
* @return i0 || i1
*/
inline constexpr uint16_t make_uint16(uint8_t i0, uint8_t i1) {
   return static_cast<uint16_t>((static_cast<uint16_t>(i0) << 8) | i1);
}

/**
* Make a uint32_t from four bytes
* @param i0 the first byte
* @param i1 the second byte
* @param i2 the third byte
* @param i3 the fourth byte
* @return i0 || i1 || i2 || i3
*/
inline constexpr uint32_t make_uint32(uint8_t i0, uint8_t i1, uint8_t i2, uint8_t i3) {
   return ((static_cast<uint32_t>(i0) << 24) | (static_cast<uint32_t>(i1) << 16) | (static_cast<uint32_t>(i2) << 8) |
           (static_cast<uint32_t>(i3)));
}

/**
* Make a uint64_t from eight bytes
* @param i0 the first byte
* @param i1 the second byte
* @param i2 the third byte
* @param i3 the fourth byte
* @param i4 the fifth byte
* @param i5 the sixth byte
* @param i6 the seventh byte
* @param i7 the eighth byte
* @return i0 || i1 || i2 || i3 || i4 || i5 || i6 || i7
*/
inline constexpr uint64_t make_uint64(
   uint8_t i0, uint8_t i1, uint8_t i2, uint8_t i3, uint8_t i4, uint8_t i5, uint8_t i6, uint8_t i7) {
   return ((static_cast<uint64_t>(i0) << 56) | (static_cast<uint64_t>(i1) << 48) | (static_cast<uint64_t>(i2) << 40) |
           (static_cast<uint64_t>(i3) << 32) | (static_cast<uint64_t>(i4) << 24) | (static_cast<uint64_t>(i5) << 16) |
           (static_cast<uint64_t>(i6) << 8) | (static_cast<uint64_t>(i7)));
}

namespace detail {

/**
 * @returns the opposite endianness of the specified endianness
 *
 * Note this assumes that there are only two endian orderings; we
 * do not supported mixed endian systems
 */
consteval std::endian opposite(std::endian endianness) {
   if(endianness == std::endian::big) {
      return std::endian::little;
   } else {
      // We already verified via static assert earlier in this file that we are
      // running on either a big endian or little endian system
      return std::endian::big;
   }
}

/**
 * Models a custom type that provides factory methods to be loaded in big- or
 * little-endian byte order.
 */
template <typename T>
concept custom_loadable = requires(std::span<const uint8_t, sizeof(T)> data) {
   { T::load_be(data) } -> std::same_as<T>;
   { T::load_le(data) } -> std::same_as<T>;
};

/**
 * Models a custom type that provides store methods to be stored in big- or
 * little-endian byte order.
 */
template <typename T>
concept custom_storable = requires(std::span<uint8_t, sizeof(T)> data, const T value) {
   { value.store_be(data) };
   { value.store_le(data) };
};

/**
 * Models a type that can be loaded/stored from/to a byte range.
 */
template <typename T>
concept unsigned_integralish =
   std::unsigned_integral<strong_type_wrapped_type<T>> ||
   (std::is_enum_v<T> && std::unsigned_integral<std::underlying_type_t<T>>) ||
   (custom_loadable<strong_type_wrapped_type<T>> || custom_storable<strong_type_wrapped_type<T>>);

template <typename T>
struct wrapped_type_helper_with_enum {
      using type = strong_type_wrapped_type<T>;
};

template <typename T>
   requires std::is_enum_v<T>
struct wrapped_type_helper_with_enum<T> {
      using type = std::underlying_type_t<T>;
};

template <unsigned_integralish T>
using wrapped_type = typename wrapped_type_helper_with_enum<T>::type;

template <unsigned_integralish InT>
constexpr auto unwrap_strong_type_or_enum(InT t) {
   if constexpr(std::is_enum_v<InT>) {
      // TODO: C++23: use std::to_underlying(in) instead
      return static_cast<std::underlying_type_t<InT>>(t);
   } else {
      return Botan::unwrap_strong_type(t);
   }
}

template <unsigned_integralish OutT, std::unsigned_integral T>
constexpr auto wrap_strong_type_or_enum(T t) {
   if constexpr(std::is_enum_v<OutT>) {
      return static_cast<OutT>(t);
   } else {
      return Botan::wrap_strong_type<OutT>(t);
   }
}

/**
 * Manually load a word from a range in either big or little endian byte order.
 *
 * This is only used at compile time.
 */
template <std::endian endianness, std::unsigned_integral OutT, ranges::contiguous_range<uint8_t> InR>
inline constexpr OutT fallback_load_any(const InR& in_range) {
   std::span in{in_range};
   // clang-format off
   if constexpr(endianness == std::endian::big) {
      return [&]<size_t... i>(std::index_sequence<i...>) {
         return static_cast<OutT>(((static_cast<OutT>(in[i]) << ((sizeof(OutT) - i - 1) * 8)) | ...));
      } (std::make_index_sequence<sizeof(OutT)>());
   } else {
      static_assert(endianness == std::endian::little);
      return [&]<size_t... i>(std::index_sequence<i...>) {
         return static_cast<OutT>(((static_cast<OutT>(in[i]) << (i * 8)) | ...));
      } (std::make_index_sequence<sizeof(OutT)>());
   }
   // clang-format on
}

/**
 * Manually store a word into a range in either big or little endian byte order.
 *
 * This will be used only at compile time.
 */
template <std::endian endianness, std::unsigned_integral InT, ranges::contiguous_output_range<uint8_t> OutR>
inline constexpr void fallback_store_any(InT in, OutR&& out_range /* NOLINT(*-std-forward) */) {
   std::span out{out_range};
   // clang-format off
   if constexpr(endianness == std::endian::big) {
      [&]<size_t... i>(std::index_sequence<i...>) {
         ((out[i] = get_byte<i>(in)), ...);
      } (std::make_index_sequence<sizeof(InT)>());
   } else {
      static_assert(endianness == std::endian::little);
      [&]<size_t... i>(std::index_sequence<i...>) {
         ((out[i] = get_byte<sizeof(InT) - i - 1>(in)), ...);
      } (std::make_index_sequence<sizeof(InT)>());
   }
   // clang-format on
}

/**
 * Load a word from a range in either big or little endian byte order
 *
 * This is the base implementation, all other overloads are just convenience
 * wrappers. It is assumed that the range has the correct size for the word.
 *
 * Template arguments of all overloads of load_any() share the same semantics:
 *
 *   1.  std::endian     Either `std::endian::big` or `std::endian::little`, that
 *                       will eventually select the byte order translation mode
 *                       implemented in this base function.
 *
 *   2.  Output type     Either `AutoDetect`, an unsigned integer or a container
 *                       holding an unsigned integer type. `AutoDetect` means
 *                       that the caller did not explicitly specify the type and
 *                       expects the type to be inferred from the input.
 *
 *   3+. Argument types  Typically, those are input and output ranges of bytes
 *                       or unsigned integers. Or one or more unsigned integers
 *                       acting as output parameters.
 *
 * @param in_range a fixed-length byte range
 * @return T loaded from @p in_range, as a big-endian value
 */
template <std::endian endianness, unsigned_integralish WrappedOutT, ranges::contiguous_range<uint8_t> InR>
   requires(!custom_loadable<strong_type_wrapped_type<WrappedOutT>>)
inline constexpr WrappedOutT load_any(InR&& in_range) {
   using OutT = detail::wrapped_type<WrappedOutT>;
   ranges::assert_exact_byte_length<sizeof(OutT)>(in_range);

   return detail::wrap_strong_type_or_enum<WrappedOutT>([&]() -> OutT {
      // At compile time we cannot use `typecast_copy` as it uses `std::memcpy`
      // internally to copy ranges on a byte-by-byte basis, which is not allowed
      // in a `constexpr` context.
      if(std::is_constant_evaluated()) /* TODO: C++23: if consteval {} */ {
         return fallback_load_any<endianness, OutT>(std::forward<InR>(in_range));
      } else {
         const std::span in{in_range};
         if constexpr(sizeof(OutT) == 1) {
            return static_cast<OutT>(in[0]);
         } else if constexpr(endianness == std::endian::native) {
            return typecast_copy<OutT>(in);
         } else {
            static_assert(opposite(endianness) == std::endian::native);
            return reverse_bytes(typecast_copy<OutT>(in));
         }
      }
   }());
}

/**
 * Load a custom object from a range in either big or little endian byte order
 *
 * This is the base implementation for custom objects (e.g. SIMD type wrappres),
 * all other overloads are just convenience overloads.
 *
 * @param in_range a fixed-length byte range
 * @return T loaded from @p in_range, as a big-endian value
 */
template <std::endian endianness, unsigned_integralish WrappedOutT, ranges::contiguous_range<uint8_t> InR>
   requires(custom_loadable<strong_type_wrapped_type<WrappedOutT>>)
inline constexpr WrappedOutT load_any(const InR& in_range) {
   using OutT = detail::wrapped_type<WrappedOutT>;
   ranges::assert_exact_byte_length<sizeof(OutT)>(in_range);
   const std::span<const uint8_t, sizeof(OutT)> ins{in_range};
   if constexpr(endianness == std::endian::big) {
      return wrap_strong_type<WrappedOutT>(OutT::load_be(ins));
   } else {
      return wrap_strong_type<WrappedOutT>(OutT::load_le(ins));
   }
}

/**
 * Load many unsigned integers
 * @param in   a fixed-length span to some bytes
 * @param outs a arbitrary-length parameter list of unsigned integers to be loaded
 */
template <std::endian endianness, typename OutT, ranges::contiguous_range<uint8_t> InR, unsigned_integralish... Ts>
   requires(sizeof...(Ts) > 0) && ((std::same_as<AutoDetect, OutT> && all_same_v<Ts...>) ||
                                   (unsigned_integralish<OutT> && all_same_v<OutT, Ts...>))
inline constexpr void load_any(const InR& in, Ts&... outs) {
   ranges::assert_exact_byte_length<(sizeof(Ts) + ...)>(in);
   auto load_one = [off = 0]<typename T>(auto i, T& o) mutable {
      o = load_any<endianness, T>(i.subspan(off).template first<sizeof(T)>());
      off += sizeof(T);
   };

   (load_one(std::span{in}, outs), ...);
}

/**
 * Load a variable number of words from @p in into @p out.
 * The byte length of the @p out and @p in ranges must match.
 *
 * @param out the output range of words
 * @param in the input range of bytes
 */
template <std::endian endianness,
          typename OutT,
          ranges::contiguous_output_range OutR,
          ranges::contiguous_range<uint8_t> InR>
   requires(unsigned_integralish<std::ranges::range_value_t<OutR>> &&
            (std::same_as<AutoDetect, OutT> || std::same_as<OutT, std::ranges::range_value_t<OutR>>))
inline constexpr void load_any(OutR&& out /* NOLINT(*-std-forward) */, const InR& in) {
   ranges::assert_equal_byte_lengths(out, in);
   using element_type = std::ranges::range_value_t<OutR>;

   auto load_elementwise = [&] {
      constexpr size_t bytes_per_element = sizeof(element_type);
      std::span<const uint8_t> in_s(in);
      for(auto& out_elem : out) {
         out_elem = load_any<endianness, element_type>(in_s.template first<bytes_per_element>());
         in_s = in_s.subspan(bytes_per_element);
      }
   };

   // At compile time we cannot use `typecast_copy` as it uses `std::memcpy`
   // internally to copy ranges on a byte-by-byte basis, which is not allowed
   // in a `constexpr` context.
   if(std::is_constant_evaluated()) /* TODO: C++23: if consteval {} */ {
      load_elementwise();
   } else {
      if constexpr(endianness == std::endian::native && !custom_loadable<element_type>) {
         typecast_copy(out, in);
      } else {
         load_elementwise();
      }
   }
}

//
// Type inference overloads
//

/**
 * Load one or more unsigned integers, auto-detect the output type if
 * possible. Otherwise, use the specified integer or integer container type.
 *
 * @param in_range a statically-sized range with some bytes
 * @return T loaded from in
 */
template <std::endian endianness, typename OutT, ranges::contiguous_range<uint8_t> InR>
   requires(std::same_as<AutoDetect, OutT> ||
            ((ranges::statically_spanable_range<OutT> || concepts::resizable_container<OutT>) &&
             unsigned_integralish<typename OutT::value_type>))
inline constexpr auto load_any(InR&& in_range) {
   auto out = []([[maybe_unused]] const auto& in) {
      if constexpr(std::same_as<AutoDetect, OutT>) {
         if constexpr(ranges::statically_spanable_range<InR>) {
            constexpr size_t extent = decltype(std::span{in})::extent;

            // clang-format off
            using type =
               std::conditional_t<extent == 1, uint8_t,
               std::conditional_t<extent == 2, uint16_t,
               std::conditional_t<extent == 4, uint32_t,
               std::conditional_t<extent == 8, uint64_t, void>>>>;
            // clang-format on

            static_assert(
               !std::is_void_v<type>,
               "Cannot determine the output type based on a statically sized bytearray with length other than those: 1, 2, 4, 8");

            return type{};
         } else {
            static_assert(
               !std::same_as<AutoDetect, OutT>,
               "cannot infer return type from a dynamic range at compile time, please specify it explicitly");
         }
      } else if constexpr(concepts::resizable_container<OutT>) {
         const size_t in_bytes = std::span{in}.size_bytes();
         constexpr size_t out_elem_bytes = sizeof(typename OutT::value_type);
         BOTAN_ARG_CHECK(in_bytes % out_elem_bytes == 0,
                         "Input range is not word-aligned with the requested output range");
         return OutT(in_bytes / out_elem_bytes);
      } else {
         return OutT{};
      }
   }(in_range);

   using out_type = decltype(out);
   if constexpr(unsigned_integralish<out_type>) {
      out = load_any<endianness, out_type>(std::forward<InR>(in_range));
   } else {
      static_assert(ranges::contiguous_range<out_type>);
      using out_range_type = std::ranges::range_value_t<out_type>;
      load_any<endianness, out_range_type>(out, std::forward<InR>(in_range));
   }
   return out;
}

//
// Legacy load functions that work on raw pointers and arrays
//

/**
 * Load a word from @p in at some offset @p off
 * @param in a pointer to some bytes
 * @param off an offset into the array
 * @return off'th T of in, as a big-endian value
 */
template <std::endian endianness, unsigned_integralish OutT>
inline constexpr OutT load_any(const uint8_t in[], size_t off) {
   // asserts that *in points to enough bytes to read at offset off
   constexpr size_t out_size = sizeof(OutT);
   return load_any<endianness, OutT>(std::span<const uint8_t, out_size>(in + off * out_size, out_size));
}

/**
 * Load many words from @p in
 * @param in   a pointer to some bytes
 * @param outs a arbitrary-length parameter list of unsigned integers to be loaded
 */
template <std::endian endianness, typename OutT, unsigned_integralish... Ts>
   requires(sizeof...(Ts) > 0 && all_same_v<Ts...> &&
            ((std::same_as<AutoDetect, OutT> && all_same_v<Ts...>) ||
             (unsigned_integralish<OutT> && all_same_v<OutT, Ts...>)))
inline constexpr void load_any(const uint8_t in[], Ts&... outs) {
   constexpr auto bytes = (sizeof(outs) + ...);
   // asserts that *in points to the correct amount of memory
   load_any<endianness, OutT>(std::span<const uint8_t, bytes>(in, bytes), outs...);
}

/**
 * Load a variable number of words from @p in into @p out.
 * @param out the output array of words
 * @param in the input array of bytes
 * @param count how many words are in in
 */
template <std::endian endianness, typename OutT, unsigned_integralish T>
   requires(std::same_as<AutoDetect, OutT> || std::same_as<T, OutT>)
inline constexpr void load_any(T out[], const uint8_t in[], size_t count) {
   // asserts that *in and *out point to the correct amount of memory
   load_any<endianness, OutT>(std::span<T>(out, count), std::span<const uint8_t>(in, count * sizeof(T)));
}

}  // namespace detail

/**
 * Load "something" in little endian byte order
 * See the documentation of this file for more details.
 */
template <typename OutT = detail::AutoDetect, typename... ParamTs>
inline constexpr auto load_le(ParamTs&&... params) {
   return detail::load_any<std::endian::little, OutT>(std::forward<ParamTs>(params)...);
}

/**
 * Load "something" in big endian byte order
 * See the documentation of this file for more details.
 */
template <typename OutT = detail::AutoDetect, typename... ParamTs>
inline constexpr auto load_be(ParamTs&&... params) {
   return detail::load_any<std::endian::big, OutT>(std::forward<ParamTs>(params)...);
}

namespace detail {

/**
 * Store a word in either big or little endian byte order into a range
 *
 * This is the base implementation, all other overloads are just convenience
 * wrappers. It is assumed that the range has the correct size for the word.
 *
 * Template arguments of all overloads of store_any() share the same semantics
 * as those of load_any(). See the documentation of this function for more
 * details.
 *
 * @param wrapped_in an unsigned integral to be stored
 * @param out_range  a byte range to store the word into
 */
template <std::endian endianness, unsigned_integralish WrappedInT, ranges::contiguous_output_range<uint8_t> OutR>
   requires(!custom_storable<strong_type_wrapped_type<WrappedInT>>)
inline constexpr void store_any(WrappedInT wrapped_in, OutR&& out_range) {
   const auto in = detail::unwrap_strong_type_or_enum(wrapped_in);
   using InT = decltype(in);
   ranges::assert_exact_byte_length<sizeof(in)>(out_range);
   const std::span out{out_range};

   // At compile time we cannot use `typecast_copy` as it uses `std::memcpy`
   // internally to copy ranges on a byte-by-byte basis, which is not allowed
   // in a `constexpr` context.
   if(std::is_constant_evaluated()) /* TODO: C++23: if consteval {} */ {
      return fallback_store_any<endianness, InT>(in, std::forward<OutR>(out_range));
   } else {
      if constexpr(sizeof(InT) == 1) {
         out[0] = static_cast<uint8_t>(in);
      } else if constexpr(endianness == std::endian::native) {
         typecast_copy(out, in);
      } else {
         static_assert(opposite(endianness) == std::endian::native);
         typecast_copy(out, reverse_bytes(in));
      }
   }
}

/**
 * Store a custom word in either big or little endian byte order into a range
 *
 * This is the base implementation for storing custom objects, all other
 * overloads are just convenience overloads.
 *
 * @param wrapped_in a custom object to be stored
 * @param out_range  a byte range to store the word into
 */
template <std::endian endianness, unsigned_integralish WrappedInT, ranges::contiguous_output_range<uint8_t> OutR>
   requires(custom_storable<strong_type_wrapped_type<WrappedInT>>)
inline constexpr void store_any(WrappedInT wrapped_in, const OutR& out_range) {
   const auto in = detail::unwrap_strong_type_or_enum(wrapped_in);
   using InT = decltype(in);
   ranges::assert_exact_byte_length<sizeof(in)>(out_range);
   const std::span<uint8_t, sizeof(InT)> outs{out_range};
   if constexpr(endianness == std::endian::big) {
      in.store_be(outs);
   } else {
      in.store_le(outs);
   }
}

/**
 * Store many unsigned integers words into a byte range
 * @param out a sized range of some bytes
 * @param ins a arbitrary-length parameter list of unsigned integers to be stored
 */
template <std::endian endianness,
          typename InT,
          ranges::contiguous_output_range<uint8_t> OutR,
          unsigned_integralish... Ts>
   requires(sizeof...(Ts) > 0) && ((std::same_as<AutoDetect, InT> && all_same_v<Ts...>) ||
                                   (unsigned_integralish<InT> && all_same_v<InT, Ts...>))
inline constexpr void store_any(OutR&& out /* NOLINT(*-std-forward) */, Ts... ins) {
   ranges::assert_exact_byte_length<(sizeof(Ts) + ...)>(out);
   auto store_one = [off = 0]<typename T>(auto o, T i) mutable {
      store_any<endianness, T>(i, o.subspan(off).template first<sizeof(T)>());
      off += sizeof(T);
   };

   (store_one(std::span{out}, ins), ...);
}

/**
 * Store a variable number of words given in @p in into @p out.
 * The byte lengths of @p in and @p out must be consistent.
 * @param out the output range of bytes
 * @param in the input range of words
 */
template <std::endian endianness,
          typename InT,
          ranges::contiguous_output_range<uint8_t> OutR,
          ranges::spanable_range InR>
   requires(std::same_as<AutoDetect, InT> || std::same_as<InT, std::ranges::range_value_t<InR>>)
inline constexpr void store_any(OutR&& out /* NOLINT(*-std-forward) */, const InR& in) {
   ranges::assert_equal_byte_lengths(out, in);
   using element_type = std::ranges::range_value_t<InR>;

   auto store_elementwise = [&] {
      constexpr size_t bytes_per_element = sizeof(element_type);
      std::span<uint8_t> out_s(out);
      for(auto in_elem : in) {
         store_any<endianness, element_type>(out_s.template first<bytes_per_element>(), in_elem);
         out_s = out_s.subspan(bytes_per_element);
      }
   };

   // At compile time we cannot use `typecast_copy` as it uses `std::memcpy`
   // internally to copy ranges on a byte-by-byte basis, which is not allowed
   // in a `constexpr` context.
   if(std::is_constant_evaluated()) /* TODO: C++23: if consteval {} */ {
      store_elementwise();
   } else {
      if constexpr(endianness == std::endian::native && !custom_storable<element_type>) {
         typecast_copy(out, in);
      } else {
         store_elementwise();
      }
   }
}

//
// Type inference overloads
//

/**
 * Infer InT from a single unsigned integer input parameter.
 *
 * TODO: we might consider dropping this overload (i.e. out-range as second
 *       parameter) and make this a "special case" of the overload below, that
 *       takes a variadic number of input parameters.
 *
 * @param in an unsigned integer to be stored
 * @param out_range a range of bytes to store the word into
 */
template <std::endian endianness, typename InT, unsigned_integralish T, ranges::contiguous_output_range<uint8_t> OutR>
   requires std::same_as<AutoDetect, InT>
inline constexpr void store_any(T in, OutR&& out_range) {
   store_any<endianness, T>(in, std::forward<OutR>(out_range));
}

/**
 * The caller provided some integer values in a collection but did not provide
 * the output container. Let's create one for them, fill it with one of the
 * overloads above and return it. This will default to a std::array if the
 * caller did not specify the desired output container type.
 *
 * @param in_range a range of words that should be stored
 * @return a container of bytes that contains the stored words
 */
template <std::endian endianness, typename OutR, ranges::spanable_range InR>
   requires(std::same_as<AutoDetect, OutR> ||
            (ranges::statically_spanable_range<OutR> && std::default_initializable<OutR>) ||
            concepts::resizable_byte_buffer<OutR>)
inline constexpr auto store_any(InR&& in_range) {
   auto out = []([[maybe_unused]] const auto& in) {
      if constexpr(std::same_as<AutoDetect, OutR>) {
         if constexpr(ranges::statically_spanable_range<InR>) {
            constexpr size_t bytes = decltype(std::span{in})::extent * sizeof(std::ranges::range_value_t<InR>);
            return std::array<uint8_t, bytes>();
         } else {
            static_assert(
               !std::same_as<AutoDetect, OutR>,
               "cannot infer a suitable result container type from the given parameters at compile time, please specify it explicitly");
         }
      } else if constexpr(concepts::resizable_byte_buffer<OutR>) {
         return OutR(std::span{in}.size_bytes());
      } else {
         return OutR{};
      }
   }(in_range);

   store_any<endianness, std::ranges::range_value_t<InR>>(out, std::forward<InR>(in_range));
   return out;
}

/**
 * The caller provided some integer values but did not provide the output
 * container. Let's create one for them, fill it with one of the overloads above
 * and return it. This will default to a std::array if the caller did not
 * specify the desired output container type.
 *
 * @param ins some words that should be stored
 * @return a container of bytes that contains the stored words
 */
template <std::endian endianness, typename OutR, unsigned_integralish... Ts>
   requires all_same_v<Ts...>
inline constexpr auto store_any(Ts... ins) {
   return store_any<endianness, OutR>(std::array{ins...});
}

//
// Legacy store functions that work on raw pointers and arrays
//

/**
 * Store a single unsigned integer into a raw pointer
 * @param in the input unsigned integer
 * @param out the byte array to write to
 */
template <std::endian endianness, typename InT, unsigned_integralish T>
   requires(std::same_as<AutoDetect, InT> || std::same_as<T, InT>)
inline constexpr void store_any(T in, uint8_t out[]) {
   // asserts that *out points to enough bytes to write into
   store_any<endianness, InT>(in, std::span<uint8_t, sizeof(T)>(out, sizeof(T)));
}

/**
 * Store many unsigned integers words into a raw pointer
 * @param ins a arbitrary-length parameter list of unsigned integers to be stored
 * @param out the byte array to write to
 */
template <std::endian endianness, typename InT, unsigned_integralish T0, unsigned_integralish... Ts>
   requires(std::same_as<AutoDetect, InT> || std::same_as<T0, InT>) && all_same_v<T0, Ts...>
inline constexpr void store_any(uint8_t out[], T0 in0, Ts... ins) {
   constexpr auto bytes = sizeof(in0) + (sizeof(ins) + ... + 0);
   // asserts that *out points to the correct amount of memory
   store_any<endianness, T0>(std::span<uint8_t, bytes>(out, bytes), in0, ins...);
}

}  // namespace detail

/**
 * Store "something" in little endian byte order
 * See the documentation of this file for more details.
 */
template <typename ModifierT = detail::AutoDetect, typename... ParamTs>
inline constexpr auto store_le(ParamTs&&... params) {
   return detail::store_any<std::endian::little, ModifierT>(std::forward<ParamTs>(params)...);
}

/**
 * Store "something" in big endian byte order
 * See the documentation of this file for more details.
 */
template <typename ModifierT = detail::AutoDetect, typename... ParamTs>
inline constexpr auto store_be(ParamTs&&... params) {
   return detail::store_any<std::endian::big, ModifierT>(std::forward<ParamTs>(params)...);
}

namespace detail {

template <std::endian endianness, unsigned_integralish T>
inline size_t copy_out_any_word_aligned_portion(std::span<uint8_t>& out, std::span<const T>& in) {
   const size_t full_words = out.size() / sizeof(T);
   const size_t full_word_bytes = full_words * sizeof(T);
   const size_t remaining_bytes = out.size() - full_word_bytes;
   BOTAN_ASSERT_NOMSG(in.size_bytes() >= full_word_bytes + remaining_bytes);

   // copy full words
   store_any<endianness, T>(out.first(full_word_bytes), in.first(full_words));
   out = out.subspan(full_word_bytes);
   in = in.subspan(full_words);

   return remaining_bytes;
}

}  // namespace detail

/**
 * Partially copy a subset of @p in into @p out using big-endian
 * byte order.
 */
template <ranges::spanable_range InR>
inline void copy_out_be(std::span<uint8_t> out, const InR& in) {
   using T = std::ranges::range_value_t<InR>;
   std::span<const T> in_s{in};
   const auto remaining_bytes = detail::copy_out_any_word_aligned_portion<std::endian::big>(out, in_s);

   // copy remaining bytes as a partial word
   for(size_t i = 0; i < remaining_bytes; ++i) {
      out[i] = get_byte_var(i, in_s.front());
   }
}

/**
 * Partially copy a subset of @p in into @p out using little-endian
 * byte order.
 */
template <ranges::spanable_range InR>
inline void copy_out_le(std::span<uint8_t> out, const InR& in) {
   using T = std::ranges::range_value_t<InR>;
   std::span<const T> in_s{in};
   const auto remaining_bytes = detail::copy_out_any_word_aligned_portion<std::endian::little>(out, in_s);

   // copy remaining bytes as a partial word
   for(size_t i = 0; i < remaining_bytes; ++i) {
      out[i] = get_byte_var(sizeof(T) - 1 - i, in_s.front());
   }
}

}  // namespace Botan


namespace Botan {

enum class MD_Endian : uint8_t {
   Little,
   Big,
};

template <typename T>
concept md_hash_implementation =
   concepts::contiguous_container<typename T::digest_type> &&
   requires(typename T::digest_type& digest, std::span<const uint8_t> input, size_t blocks) {
      { T::init(digest) } -> std::same_as<void>;
      { T::compress_n(digest, input, blocks) } -> std::same_as<void>;
      T::bit_endianness;
      T::byte_endianness;
      T::block_bytes;
      T::output_bytes;
      T::ctr_bytes;
   } && T::block_bytes >= 64 && is_power_of_2(T::block_bytes) && T::output_bytes >= 16 && T::ctr_bytes >= 8 &&
   is_power_of_2(T::ctr_bytes) && T::ctr_bytes < T::block_bytes;

template <md_hash_implementation MD>
class MerkleDamgard_Hash final {
   public:
      MerkleDamgard_Hash() { clear(); }

      void update(std::span<const uint8_t> input) {
         BufferSlicer in(input);

         while(!in.empty()) {
            if(const auto one_block = m_buffer.handle_unaligned_data(in)) {
               MD::compress_n(m_digest, one_block.value(), 1);
            }

            if(m_buffer.in_alignment()) {
               const auto [aligned_data, full_blocks] = m_buffer.aligned_data_to_process(in);
               if(full_blocks > 0) {
                  MD::compress_n(m_digest, aligned_data, full_blocks);
               }
            }
         }

         m_count += input.size();
      }

      void final(std::span<uint8_t> output) {
         append_padding_bit();
         append_counter_and_finalize();
         copy_output(output);
         clear();
      }

      void clear() {
         MD::init(m_digest);
         m_buffer.clear();
         m_count = 0;
      }

   private:
      void append_padding_bit() {
         BOTAN_ASSERT_NOMSG(!m_buffer.ready_to_consume());
         if constexpr(MD::bit_endianness == MD_Endian::Big) {
            const uint8_t final_byte = 0x80;
            m_buffer.append({&final_byte, 1});
         } else {
            const uint8_t final_byte = 0x01;
            m_buffer.append({&final_byte, 1});
         }
      }

      void append_counter_and_finalize() {
         // Compress the remaining data if the final data block does not provide
         // enough space for the counter bytes.
         if(m_buffer.elements_until_alignment() < MD::ctr_bytes) {
            m_buffer.fill_up_with_zeros();
            MD::compress_n(m_digest, m_buffer.consume(), 1);
         }

         // Make sure that any remaining bytes in the very last block are zero.
         BOTAN_ASSERT_NOMSG(m_buffer.elements_until_alignment() >= MD::ctr_bytes);
         m_buffer.fill_up_with_zeros();

         // Replace a bunch of the right-most zero-padding with the counter bytes.
         const uint64_t bit_count = m_count * 8;
         auto last_bytes = m_buffer.directly_modify_last(sizeof(bit_count));
         if constexpr(MD::byte_endianness == MD_Endian::Big) {
            store_be(bit_count, last_bytes.data());
         } else {
            store_le(bit_count, last_bytes.data());
         }

         // Compress the very last block.
         MD::compress_n(m_digest, m_buffer.consume(), 1);
      }

      void copy_output(std::span<uint8_t> output) {
         BOTAN_ASSERT_NOMSG(output.size() >= MD::output_bytes);

         if constexpr(MD::byte_endianness == MD_Endian::Big) {
            copy_out_be(output.first(MD::output_bytes), m_digest);
         } else {
            copy_out_le(output.first(MD::output_bytes), m_digest);
         }
      }

   private:
      typename MD::digest_type m_digest;
      uint64_t m_count = 0;

      AlignmentBuffer<uint8_t, MD::block_bytes> m_buffer;
};

}  // namespace Botan

#if defined(BOTAN_TARGET_OS_HAS_THREADS)
   #include <thread>
#endif

namespace Botan::OS {

/*
* This header is internal (not installed) and these functions are not
* intended to be called by applications. However they are given public
* visibility (using BOTAN_TEST_API macro) for the tests. This also probably
* allows them to be overridden by the application on ELF systems, but
* this hasn't been tested.
*/

/**
* @return process ID assigned by the operating system.
*
* On Unix and Windows systems, this always returns a result
*
* On systems where there is no processes to speak of (for example on baremetal
* systems or within a unikernel), this function returns zero.
*/
uint32_t BOTAN_TEST_API get_process_id();

/**
* @return CPU processor clock, if available
*
* On Windows, calls QueryPerformanceCounter.
*
* Under GCC or Clang on supported platforms the hardware cycle counter is queried.
* Currently supported processors are x86, PPC, Alpha, SPARC, IA-64, S/390x, and HP-PA.
* If no CPU cycle counter is available on this system, returns zero.
*/
uint64_t BOTAN_TEST_API get_cpu_cycle_counter();

size_t BOTAN_TEST_API get_cpu_available();

/**
* If this system supports getauxval (or an equivalent interface,
* like FreeBSD's elf_aux_info) queries AT_HWCAP and AT_HWCAP2
* and returns both.
*
* Otherwise returns nullopt.
*/
std::optional<std::pair<unsigned long, unsigned long>> get_auxval_hwcap();

/*
* @return best resolution timestamp available
*
* The epoch and update rate of this clock is arbitrary and depending
* on the hardware it may not tick at a constant rate.
*
* Uses hardware cycle counter, if available.
* On POSIX platforms clock_gettime is used with a monotonic timer
*
* As a final fallback std::chrono::high_resolution_clock is used.
*
* On systems that are lacking a real time clock, this may return 0
*/
uint64_t BOTAN_TEST_API get_high_resolution_clock();

/**
* @return system clock (reflecting wall clock) with best resolution
* available, normalized to nanoseconds resolution, using Unix epoch.
*
* If the system does not have a real time clock this function will throw
* Not_Implemented
*/
uint64_t BOTAN_TEST_API get_system_timestamp_ns();

/**
* Format a time
*
* Converts the time_t to a local time representation,
* then invokes std::put_time with the specified format.
*/
std::string BOTAN_TEST_API format_time(time_t time, const std::string& format);

/**
* @return maximum amount of memory (in bytes) Botan could/should
* hypothetically allocate for the memory poool. Reads environment
* variable "BOTAN_MLOCK_POOL_SIZE", set to "0" to disable pool.
*/
size_t get_memory_locking_limit();

/**
* Return the size of a memory page, if that can be derived on the
* current system. Otherwise returns some default value (eg 4096)
*/
size_t system_page_size();

/**
* Read the value of an environment variable, setting it to value_out if it
* exists.  Returns false and sets value_out to empty string if no such variable
* is set. If the process seems to be running in a privileged state (such as
* setuid) then always returns false and does not examine the environment.
*/
bool read_env_variable(std::string& value_out, std::string_view var_name);

/**
* Read the value of an environment variable and convert it to an
* integer. If not set or conversion fails, returns the default value.
*
* If the process seems to be running in a privileged state (such as setuid)
* then always returns nullptr, similar to glibc's secure_getenv.
*/
size_t read_env_variable_sz(std::string_view var_name, size_t def_value = 0);

/**
* Request count pages of RAM which are locked into memory using mlock,
* VirtualLock, or some similar OS specific API. Free it with free_locked_pages.
*
* Returns an empty list on failure. This function is allowed to return fewer
* than count pages.
*
* The contents of the allocated pages are undefined.
*
* Each page is preceded by and followed by a page which is marked
* as noaccess, such that accessing it will cause a crash. This turns
* out of bound reads/writes into crash events.
*
* @param count requested number of locked pages
*/
std::vector<void*> allocate_locked_pages(size_t count);

/**
* Free memory allocated by allocate_locked_pages
* @param pages a list of pages returned by allocate_locked_pages
*/
void free_locked_pages(const std::vector<void*>& pages);

/**
* Set the MMU to prohibit access to this page
*/
void page_prohibit_access(void* page);

/**
* Set the MMU to allow R/W access to this page
*/
void page_allow_access(void* page);

/**
* Set a ID to a page's range expressed by size bytes
*/
void page_named(void* page, size_t size);

#if defined(BOTAN_TARGET_OS_HAS_THREADS)
void set_thread_name(std::thread& thread, const std::string& name);
#endif

/**
* Run a probe instruction to test for support for a CPU instruction.
* Runs in system-specific env that catches illegal instructions; this
* function always fails if the OS doesn't provide this.
* Returns value of probe_fn, if it could run.
* If error occurs, returns negative number.
* This allows probe_fn to indicate errors of its own, if it wants.
* For example the instruction might not only be only available on some
* CPUs, but also buggy on some subset of these - the probe function
* can test to make sure the instruction works properly before
* indicating that the instruction is available.
*
* @warning on Unix systems uses signal handling in a way that is not
* thread safe. It should only be called in a single-threaded context
* (ie, at static init time).
*
* If probe_fn throws an exception the result is undefined.
*
* Return codes:
* -1 illegal instruction detected
*/
int BOTAN_TEST_API run_cpu_instruction_probe(const std::function<int()>& probe_fn);

/**
* Represents a terminal state
*/
class BOTAN_UNSTABLE_API Echo_Suppression /* NOLINT(*special-member-functions) */ {
   public:
      /**
      * Reenable echo on this terminal. Can be safely called
      * multiple times. May throw if an error occurs.
      */
      virtual void reenable_echo() = 0;

      /**
      * Implicitly calls reenable_echo, but swallows/ignored all
      * errors which would leave the terminal in an invalid state.
      */
      virtual ~Echo_Suppression() = default;
};

/**
* Suppress echo on the terminal
* Returns null if this operation is not supported on the current system.
*/
std::unique_ptr<Echo_Suppression> BOTAN_UNSTABLE_API suppress_echo_on_terminal();

}  // namespace Botan::OS

namespace Botan {

/**
* Parse a SCAN-style algorithm name
* @param scan_name the name
* @return the name components
*/
std::vector<std::string> parse_algorithm_name(std::string_view scan_name);

/**
* Split a string
* @param str the input string
* @param delim the delimiter
* @return string split by delim
*/
BOTAN_TEST_API std::vector<std::string> split_on(std::string_view str, char delim);

/**
* Join a string
* @param strs strings to join
* @param delim the delimiter
* @return string joined by delim
*/
std::string string_join(const std::vector<std::string>& strs, char delim);

/**
* Convert a decimal string to a number
* @param str the string to convert
* @return number value of the string
*/
BOTAN_TEST_API uint32_t to_u32bit(std::string_view str);

/**
* Convert a decimal string to a number
* @param str the string to convert
* @return number value of the string
*/
uint16_t to_uint16(std::string_view str);

/**
* Convert a string representation of an IPv4 address to a number
* @param ip_str the string representation
* @return integer IPv4 address
*/
std::optional<uint32_t> BOTAN_TEST_API string_to_ipv4(std::string_view ip_str);

/**
* Convert an IPv4 address to a string
* @param ip_addr the IPv4 address to convert
* @return string representation of the IPv4 address
*/
std::string BOTAN_TEST_API ipv4_to_string(uint32_t ip_addr);

/**
* Convert a string representation of an IPv6 address to a 16-byte big-endian
* array. Accepts the full form (eight colon-separated hex groups), the
* "::"-compressed form (exactly one run of zero groups elided), and combinations
* such as "2001:db8::1". Does not currently accept the IPv4-in-IPv6 trailing
* dotted-quad form (e.g. "::ffff:192.0.2.1") or surrounding brackets.
*/
std::optional<std::array<uint8_t, 16>> BOTAN_TEST_API string_to_ipv6(std::string_view ip_str);

/**
* Convert an IPv6 address to normalized string format. Zero compression ("::")
* is not applied.
*/
std::string BOTAN_TEST_API ipv6_to_string(std::span<const uint8_t, 16> ip_addr);

std::map<std::string, std::string> read_cfg(std::istream& is);

/**
* Accepts key value pairs delimited by commas:
*
* "" (returns empty map)
* "K=V" (returns map {'K': 'V'})
* "K1=V1,K2=V2"
* "K1=V1,K2=V2,K3=V3"
* "K1=V1,K2=V2,K3=a_value\,with\,commas_and_\=equals"
*
* Values may be empty, keys must be non-empty and unique. Duplicate
* keys cause an exception.
*
* Within both key and value, comma and equals can be escaped with
* backslash. Backslash can also be escaped.
*/
BOTAN_TEST_API
std::map<std::string, std::string> read_kv(std::string_view kv);

std::string tolower_string(std::string_view str);

/**
* Check if the given hostname is a match for the specified wildcard
*/
BOTAN_TEST_API
bool host_wildcard_match(std::string_view wildcard, std::string_view host);

/**
* If name is a valid DNS name, return it canonicalized
*
* Otherwise throws Decoding_Error
*/
BOTAN_TEST_API std::string check_and_canonicalize_dns_name(std::string_view name);

}  // namespace Botan

namespace Botan {

/**
* Polynomial doubling in GF(2^n)
*/
void BOTAN_TEST_API poly_double_n(uint8_t out[], const uint8_t in[], size_t n);

/**
* Returns true iff poly_double_n is implemented for this size.
*/
inline bool poly_double_supported_size(size_t n) {
   return (n == 8 || n == 16 || n == 24 || n == 32 || n == 64 || n == 128);
}

inline void poly_double_n(uint8_t buf[], size_t n) {
   return poly_double_n(buf, buf, n);
}

/*
* Little endian convention - used for XTS
*/
void BOTAN_TEST_API poly_double_n_le(uint8_t out[], const uint8_t in[], size_t n);

/*
* Tweak block update step for XTS
*
* Assumes tweak is BS * n bytes long.
*
* The first block remains unmodified.
* The remaining n-1 blocks are set to the successive doublings of the first block
*/
void xts_compute_tweak_block(uint8_t tweak[], size_t BS, size_t n);

}  // namespace Botan

namespace Botan {

/**
* Prefetch an array
*
* This function returns a uint64_t which is accumulated from values
* read from the array. This may help confuse the compiler sufficiently
* to not elide otherwise "useless" reads. The return value will always
* be zero.
*/
uint64_t prefetch_array_raw(size_t bytes, const void* array) noexcept;

/**
* Prefetch several arrays
*
* This function returns a uint64_t which is accumulated from values
* read from the array. This may help confuse the compiler sufficiently
* to not elide otherwise "useless" reads. The return value will always
* be zero.
*/
template <std::unsigned_integral T, size_t... Ns>
T prefetch_arrays(T (&... arr)[Ns]) noexcept {
   return (static_cast<T>(prefetch_array_raw(sizeof(T) * Ns, arr)) & ...);
}

}  // namespace Botan

namespace Botan {

/**
* Integer rounding
*
* Returns an integer z such that n <= z <= n + align_to
* and z % align_to == 0
*
* @param n an integer
* @param align_to the alignment boundary
* @return n rounded up to a multiple of align_to
*/
constexpr inline size_t round_up(size_t n, size_t align_to) {
   // Arguably returning n in this case would also be sensible
   BOTAN_ARG_CHECK(align_to != 0, "align_to must not be 0");

   if(n % align_to > 0) {
      const size_t adj = align_to - (n % align_to);
      BOTAN_ARG_CHECK(n + adj >= n, "Integer overflow during rounding");
      n += adj;
   }
   return n;
}

}  // namespace Botan

namespace Botan {

/**
A class encapsulating a SCAN name (similar to JCE conventions)
http://www.users.zetnet.co.uk/hopwood/crypto/scan/
*/
class SCAN_Name final {
   public:
      /**
      * Create a SCAN_Name
      * @param algo_spec A SCAN-format name
      */
      explicit SCAN_Name(const char* algo_spec);

      /**
      * Create a SCAN_Name
      * @param algo_spec A SCAN-format name
      */
      explicit SCAN_Name(std::string_view algo_spec);

      /**
      * @return original input string
      */
      const std::string& to_string() const { return m_orig_algo_spec; }

      /**
      * @return algorithm name
      */
      const std::string& algo_name() const { return m_alg_name; }

      /**
      * @return number of arguments
      */
      size_t arg_count() const { return m_args.size(); }

      /**
      * @param lower is the lower bound
      * @param upper is the upper bound
      * @return if the number of arguments is between lower and upper
      */
      bool arg_count_between(size_t lower, size_t upper) const {
         return ((arg_count() >= lower) && (arg_count() <= upper));
      }

      /**
      * @param i which argument
      * @return ith argument
      */
      std::string arg(size_t i) const;

      /**
      * @param i which argument
      * @param def_value the default value
      * @return ith argument or the default value
      */
      std::string arg(size_t i, std::string_view def_value) const;

      /**
      * @param i which argument
      * @param def_value the default value
      * @return ith argument as an integer, or the default value
      */
      size_t arg_as_integer(size_t i, size_t def_value) const;

      /**
      * @param i which argument
      * @return ith argument as an integer
      */
      size_t arg_as_integer(size_t i) const;

      /**
      * @return cipher mode (if any)
      */
      std::string cipher_mode() const { return (!m_mode_info.empty()) ? m_mode_info[0] : ""; }

      /**
      * @return cipher mode padding (if any)
      */
      std::string cipher_mode_pad() const { return (m_mode_info.size() >= 2) ? m_mode_info[1] : ""; }

   private:
      std::string m_orig_algo_spec;
      std::string m_alg_name;
      std::vector<std::string> m_args;
      std::vector<std::string> m_mode_info;
};

// This is unrelated but it is convenient to stash it here
template <typename T>
std::vector<std::string> probe_providers_of(std::string_view algo_spec,
                                            const std::vector<std::string>& possible = {"base"}) {
   std::vector<std::string> providers;
   for(auto&& prov : possible) {
      auto o = T::create(algo_spec, prov);
      if(o) {
         providers.push_back(prov);  // available
      }
   }
   return providers;
}

}  // namespace Botan

namespace Botan {

/**
* Serpent is the most conservative of the AES finalists
* https://www.cl.cam.ac.uk/~rja14/serpent.html
*/
class Serpent final : public Block_Cipher_Fixed_Params<16, 16, 32, 8> {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string provider() const override;

      std::string name() const override { return "Serpent"; }

      std::unique_ptr<BlockCipher> new_object() const override { return std::make_unique<Serpent>(); }

      size_t parallelism() const override { return 4; }

      bool has_keying_material() const override;

   private:
#if defined(BOTAN_HAS_SERPENT_SIMD)
      void simd_encrypt_4(const uint8_t in[16 * 4], uint8_t out[16 * 4]) const;
      void simd_decrypt_4(const uint8_t in[16 * 4], uint8_t out[16 * 4]) const;
#endif

#if defined(BOTAN_HAS_SERPENT_AVX2)
      void avx2_encrypt_8(const uint8_t in[16 * 8], uint8_t out[16 * 8]) const;
      void avx2_decrypt_8(const uint8_t in[16 * 8], uint8_t out[16 * 8]) const;
#endif

#if defined(BOTAN_HAS_SERPENT_AVX512)
      void avx512_encrypt_16(const uint8_t in[16 * 16], uint8_t out[16 * 16]) const;
      void avx512_decrypt_16(const uint8_t in[16 * 16], uint8_t out[16 * 16]) const;
#endif

      void key_schedule(std::span<const uint8_t> key) override;

      secure_vector<uint32_t> m_round_key;
};

}  // namespace Botan

namespace Botan::Serpent_F {

// Concept for types that support bitwise operations (unsigned integers or SIMD types)
template <typename T>
concept BitsliceT = requires(T& a, const T& b) {
   a ^= b;
   a &= b;
   a |= b;
   ~a;
};

template <size_t S>
BOTAN_FORCE_INLINE uint32_t shl(uint32_t v) {
   return v << S;
}

/*
* Serpent's Linear Transform
*/
template <BitsliceT T>
BOTAN_FORCE_INLINE void transform(T& B0, T& B1, T& B2, T& B3) {
   B0 = rotl<13>(B0);
   B2 = rotl<3>(B2);
   B1 ^= B0 ^ B2;
   B3 ^= B2 ^ shl<3>(B0);
   B1 = rotl<1>(B1);
   B3 = rotl<7>(B3);
   B0 ^= B1 ^ B3;
   B2 ^= B3 ^ shl<7>(B1);
   B0 = rotl<5>(B0);
   B2 = rotl<22>(B2);
}

/*
* Serpent's Inverse Linear Transform
*/
template <BitsliceT T>
BOTAN_FORCE_INLINE void i_transform(T& B0, T& B1, T& B2, T& B3) {
   B2 = rotr<22>(B2);
   B0 = rotr<5>(B0);
   B2 ^= B3 ^ shl<7>(B1);
   B0 ^= B1 ^ B3;
   B3 = rotr<7>(B3);
   B1 = rotr<1>(B1);
   B3 ^= B2 ^ shl<3>(B0);
   B1 ^= B0 ^ B2;
   B2 = rotr<3>(B2);
   B0 = rotr<13>(B0);
}

class Key_Inserter final {
   public:
      explicit Key_Inserter(const uint32_t* RK) : m_RK(RK) {}

      template <BitsliceT T>
      BOTAN_FORCE_INLINE void operator()(size_t R, T& B0, T& B1, T& B2, T& B3) const {
         B0 ^= m_RK[4 * R];
         B1 ^= m_RK[4 * R + 1];
         B2 ^= m_RK[4 * R + 2];
         B3 ^= m_RK[4 * R + 3];
      }

   private:
      const uint32_t* m_RK;
};

}  // namespace Botan::Serpent_F

namespace Botan::Serpent_F {

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxE0(T& a, T& b, T& c, T& d) {
   d ^= a;
   T t0 = b;
   b &= d;
   t0 ^= c;
   b ^= a;
   a |= d;
   a ^= t0;
   t0 ^= d;
   d ^= c;
   c |= b;
   c ^= t0;
   t0 = ~t0;
   t0 |= b;
   b ^= d;
   b ^= t0;
   d |= a;
   b ^= d;
   t0 ^= d;
   d = a;
   a = b;
   b = t0;
}

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxE1(T& a, T& b, T& c, T& d) {
   a = ~a;
   c = ~c;
   T t0 = a;
   a &= b;
   c ^= a;
   a |= d;
   d ^= c;
   b ^= a;
   a ^= t0;
   t0 |= b;
   b ^= d;
   c |= a;
   c &= t0;
   a ^= b;
   b &= c;
   b ^= a;
   a &= c;
   t0 ^= a;
   a = c;
   c = d;
   d = b;
   b = t0;
}

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxE2(T& a, T& b, T& c, T& d) {
   T t0 = a;
   a &= c;
   a ^= d;
   c ^= b;
   c ^= a;
   d |= t0;
   d ^= b;
   t0 ^= c;
   b = d;
   d |= t0;
   d ^= a;
   a &= b;
   t0 ^= a;
   b ^= d;
   b ^= t0;
   a = c;
   c = b;
   b = d;
   d = ~t0;
}

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxE3(T& a, T& b, T& c, T& d) {
   T t0 = a;
   a |= d;
   d ^= b;
   b &= t0;
   t0 ^= c;
   c ^= d;
   d &= a;
   t0 |= b;
   d ^= t0;
   a ^= b;
   t0 &= a;
   b ^= d;
   t0 ^= c;
   b |= a;
   b ^= c;
   a ^= d;
   c = b;
   b |= d;
   a ^= b;
   b = c;
   c = d;
   d = t0;
}

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxE4(T& a, T& b, T& c, T& d) {
   b ^= d;
   d = ~d;
   c ^= d;
   d ^= a;
   T t0 = b;
   b &= d;
   b ^= c;
   t0 ^= d;
   a ^= t0;
   c &= t0;
   c ^= a;
   a &= b;
   d ^= a;
   t0 |= b;
   t0 ^= a;
   a |= d;
   a ^= c;
   c &= d;
   a = ~a;
   t0 ^= c;
   c = a;
   a = b;
   b = t0;
}

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxE5(T& a, T& b, T& c, T& d) {
   a ^= b;
   b ^= d;
   d = ~d;
   T t0 = b;
   b &= a;
   c ^= d;
   b ^= c;
   c |= t0;
   t0 ^= d;
   d &= b;
   d ^= a;
   t0 ^= b;
   t0 ^= c;
   c ^= a;
   a &= d;
   c = ~c;
   a ^= t0;
   t0 |= d;
   t0 ^= c;
   c = a;
   a = b;
   b = d;
   d = t0;
}

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxE6(T& a, T& b, T& c, T& d) {
   c = ~c;
   T t0 = d;
   d &= a;
   a ^= t0;
   d ^= c;
   c |= t0;
   b ^= d;
   c ^= a;
   a |= b;
   c ^= b;
   t0 ^= a;
   a |= d;
   a ^= c;
   t0 ^= d;
   t0 ^= a;
   d = ~d;
   c &= t0;
   d ^= c;
   c = t0;
}

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxE7(T& a, T& b, T& c, T& d) {
   T t0 = b;
   b |= c;
   b ^= d;
   t0 ^= c;
   c ^= b;
   d |= t0;
   d &= a;
   t0 ^= c;
   d ^= b;
   b |= t0;
   b ^= a;
   a |= t0;
   a ^= c;
   b ^= t0;
   c ^= b;
   b &= a;
   b ^= t0;
   c = ~c;
   c |= a;
   t0 ^= c;
   c = b;
   b = d;
   d = a;
   a = t0;
}

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxD0(T& a, T& b, T& c, T& d) {
   c = ~c;
   T t0 = b;
   b |= a;
   t0 = ~t0;
   b ^= c;
   c |= t0;
   b ^= d;
   a ^= t0;
   c ^= a;
   a &= d;
   t0 ^= a;
   a |= b;
   a ^= c;
   d ^= t0;
   c ^= b;
   d ^= a;
   d ^= b;
   c &= d;
   t0 ^= c;
   c = b;
   b = t0;
}

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxD1(T& a, T& b, T& c, T& d) {
   T t0 = b;
   b ^= d;
   d &= b;
   t0 ^= c;
   d ^= a;
   a |= b;
   c ^= d;
   a ^= t0;
   a |= c;
   b ^= d;
   a ^= b;
   b |= d;
   b ^= a;
   t0 = ~t0;
   t0 ^= b;
   b |= a;
   b ^= a;
   b |= t0;
   d ^= b;
   b = a;
   a = t0;
   t0 = c;
   c = d;
   d = t0;
}

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxD2(T& a, T& b, T& c, T& d) {
   c ^= d;
   d ^= a;
   T t0 = d;
   d &= c;
   d ^= b;
   b |= c;
   b ^= t0;
   t0 &= d;
   c ^= d;
   t0 &= a;
   t0 ^= c;
   c &= b;
   c |= a;
   d = ~d;
   c ^= d;
   a ^= d;
   a &= b;
   d ^= t0;
   d ^= a;
   a = b;
   b = t0;
}

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxD3(T& a, T& b, T& c, T& d) {
   T t0 = c;
   c ^= b;
   a ^= c;
   t0 &= c;
   t0 ^= a;
   a &= b;
   b ^= d;
   d |= t0;
   c ^= d;
   a ^= d;
   b ^= t0;
   d &= c;
   d ^= b;
   b ^= a;
   b |= c;
   a ^= d;
   b ^= t0;
   a ^= b;
   t0 = a;
   a = c;
   c = d;
   d = t0;
}

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxD4(T& a, T& b, T& c, T& d) {
   T t0 = c;
   c &= d;
   c ^= b;
   b |= d;
   b &= a;
   t0 ^= c;
   t0 ^= b;
   b &= c;
   a = ~a;
   d ^= t0;
   b ^= d;
   d &= a;
   d ^= c;
   a ^= b;
   c &= a;
   d ^= a;
   c ^= t0;
   c |= d;
   d ^= a;
   c ^= b;
   b = d;
   d = t0;
}

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxD5(T& a, T& b, T& c, T& d) {
   b = ~b;
   T t0 = d;
   c ^= b;
   d |= a;
   d ^= c;
   c |= b;
   c &= a;
   t0 ^= d;
   c ^= t0;
   t0 |= a;
   t0 ^= b;
   b &= c;
   b ^= d;
   t0 ^= c;
   d &= t0;
   t0 ^= b;
   d ^= t0;
   t0 = ~t0;
   d ^= a;
   a = b;
   b = t0;
   t0 = d;
   d = c;
   c = t0;
}

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxD6(T& a, T& b, T& c, T& d) {
   a ^= c;
   T t0 = c;
   c &= a;
   t0 ^= d;
   c = ~c;
   d ^= b;
   c ^= d;
   t0 |= a;
   a ^= c;
   d ^= t0;
   t0 ^= b;
   b &= d;
   b ^= a;
   a ^= d;
   a |= c;
   d ^= b;
   t0 ^= a;
   a = b;
   b = c;
   c = t0;
}

template <BitsliceT T>
BOTAN_FORCE_INLINE void SBoxD7(T& a, T& b, T& c, T& d) {
   T t0 = c;
   c ^= a;
   a &= d;
   t0 |= d;
   c = ~c;
   d ^= b;
   b |= a;
   a ^= c;
   c &= t0;
   d &= t0;
   b ^= c;
   c ^= a;
   a |= c;
   t0 ^= b;
   a ^= d;
   d ^= t0;
   t0 |= a;
   d ^= c;
   t0 ^= c;
   c = b;
   b = a;
   a = d;
   d = t0;
}

}  // namespace Botan::Serpent_F

namespace Botan {

/**
* SHA-224
*/
class SHA_224 final : public HashFunction {
   public:
      using digest_type = secure_vector<uint32_t>;

      static constexpr MD_Endian byte_endianness = MD_Endian::Big;
      static constexpr MD_Endian bit_endianness = MD_Endian::Big;
      static constexpr size_t block_bytes = 64;
      static constexpr size_t output_bytes = 28;
      static constexpr size_t ctr_bytes = 8;

      static void compress_n(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
      static void init(digest_type& digest);

   public:
      std::string name() const override { return "SHA-224"; }

      size_t output_length() const override { return output_bytes; }

      size_t hash_block_size() const override { return block_bytes; }

      std::unique_ptr<HashFunction> new_object() const override;

      std::unique_ptr<HashFunction> copy_state() const override;

      void clear() override { m_md.clear(); }

      std::string provider() const override;

   private:
      void add_data(std::span<const uint8_t> input) override;

      void final_result(std::span<uint8_t> output) override;

   private:
      MerkleDamgard_Hash<SHA_224> m_md;
};

/**
* SHA-256
*/
class SHA_256 final : public HashFunction {
   public:
      using digest_type = secure_vector<uint32_t>;

      static constexpr MD_Endian byte_endianness = MD_Endian::Big;
      static constexpr MD_Endian bit_endianness = MD_Endian::Big;
      static constexpr size_t block_bytes = 64;
      static constexpr size_t output_bytes = 32;
      static constexpr size_t ctr_bytes = 8;

      static void compress_n(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
      static void init(digest_type& digest);

   public:
      std::string name() const override { return "SHA-256"; }

      size_t output_length() const override { return output_bytes; }

      size_t hash_block_size() const override { return block_bytes; }

      std::unique_ptr<HashFunction> new_object() const override;

      std::unique_ptr<HashFunction> copy_state() const override;

      void clear() override { m_md.clear(); }

      std::string provider() const override;

   public:
      static void compress_digest(digest_type& digest, std::span<const uint8_t> input, size_t blocks);

#if defined(BOTAN_HAS_SHA2_32_ARMV8)
      static void compress_digest_armv8(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
#endif

#if defined(BOTAN_HAS_SHA2_32_SIMD)
      static void compress_digest_x86_simd(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
#endif

#if defined(BOTAN_HAS_SHA2_32_X86_AVX2)
      static void compress_digest_x86_avx2(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
#endif

#if defined(BOTAN_HAS_SHA2_32_X86)
      static void compress_digest_x86(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
#endif

   private:
      void add_data(std::span<const uint8_t> input) override;

      void final_result(std::span<uint8_t> output) override;

   private:
      MerkleDamgard_Hash<SHA_256> m_md;
};

}  // namespace Botan

namespace Botan {

/*
* SHA-256 F1 Function
*/
BOTAN_FORCE_INLINE void SHA2_32_F(uint32_t A,
                                  uint32_t B,
                                  uint32_t C,
                                  uint32_t& D,
                                  uint32_t E,
                                  uint32_t F,
                                  uint32_t G,
                                  uint32_t& H,
                                  uint32_t& M1,
                                  uint32_t M2,
                                  uint32_t M3,
                                  uint32_t M4,
                                  uint32_t magic) {
   H += magic + rho<6, 11, 25>(E) + choose(E, F, G) + M1;
   D += H;
   H += rho<2, 13, 22>(A) + majority(A, B, C);
   M1 += sigma<17, 19, 10>(M2) + M3 + sigma<7, 18, 3>(M4);
}

/*
* SHA-256 F1 Function (No Message Expansion)
*/
BOTAN_FORCE_INLINE void SHA2_32_F(
   uint32_t A, uint32_t B, uint32_t C, uint32_t& D, uint32_t E, uint32_t F, uint32_t G, uint32_t& H, uint32_t M) {
   H += rho<6, 11, 25>(E) + choose(E, F, G) + M;
   D += H;
   H += rho<2, 13, 22>(A) + majority(A, B, C);
}

}  // namespace Botan

namespace Botan {

/**
* SHA-384
*/
class SHA_384 final : public HashFunction {
   public:
      using digest_type = secure_vector<uint64_t>;

      static constexpr MD_Endian byte_endianness = MD_Endian::Big;
      static constexpr MD_Endian bit_endianness = MD_Endian::Big;
      static constexpr size_t block_bytes = 128;
      static constexpr size_t output_bytes = 48;
      static constexpr size_t ctr_bytes = 16;

      static void compress_n(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
      static void init(digest_type& digest);

   public:
      std::string name() const override { return "SHA-384"; }

      size_t output_length() const override { return output_bytes; }

      size_t hash_block_size() const override { return block_bytes; }

      std::unique_ptr<HashFunction> new_object() const override;

      std::unique_ptr<HashFunction> copy_state() const override;

      std::string provider() const override;

      void clear() override { m_md.clear(); }

   private:
      void add_data(std::span<const uint8_t> input) override;

      void final_result(std::span<uint8_t> output) override;

   private:
      MerkleDamgard_Hash<SHA_384> m_md;
};

/**
* SHA-512
*/
class SHA_512 final : public HashFunction {
   public:
      using digest_type = secure_vector<uint64_t>;

      static constexpr MD_Endian byte_endianness = MD_Endian::Big;
      static constexpr MD_Endian bit_endianness = MD_Endian::Big;
      static constexpr size_t block_bytes = 128;
      static constexpr size_t output_bytes = 64;
      static constexpr size_t ctr_bytes = 16;

      static void compress_n(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
      static void init(digest_type& digest);

   public:
      std::string name() const override { return "SHA-512"; }

      size_t output_length() const override { return output_bytes; }

      size_t hash_block_size() const override { return block_bytes; }

      std::unique_ptr<HashFunction> new_object() const override;

      std::unique_ptr<HashFunction> copy_state() const override;

      std::string provider() const override;

      void clear() override { m_md.clear(); }

   public:
      static void compress_digest(digest_type& digest, std::span<const uint8_t> input, size_t blocks);

#if defined(BOTAN_HAS_SHA2_64_X86_AVX2)
      static void compress_digest_x86_avx2(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
#endif

#if defined(BOTAN_HAS_SHA2_64_X86_AVX512)
      static void compress_digest_x86_avx512(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
#endif

#if defined(BOTAN_HAS_SHA2_64_X86)
      static void compress_digest_x86(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
#endif

#if defined(BOTAN_HAS_SHA2_64_ARMV8)
      static void compress_digest_armv8(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
#endif

   private:
      void add_data(std::span<const uint8_t> input) override;

      void final_result(std::span<uint8_t> output) override;

   private:
      MerkleDamgard_Hash<SHA_512> m_md;
};

/**
* SHA-512/256
*/
class SHA_512_256 final : public HashFunction {
   public:
      using digest_type = secure_vector<uint64_t>;

      static constexpr MD_Endian byte_endianness = MD_Endian::Big;
      static constexpr MD_Endian bit_endianness = MD_Endian::Big;
      static constexpr size_t block_bytes = 128;
      static constexpr size_t output_bytes = 32;
      static constexpr size_t ctr_bytes = 16;

      static void compress_n(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
      static void init(digest_type& digest);

   public:
      std::string name() const override { return "SHA-512-256"; }

      size_t output_length() const override { return output_bytes; }

      size_t hash_block_size() const override { return block_bytes; }

      std::unique_ptr<HashFunction> new_object() const override;

      std::unique_ptr<HashFunction> copy_state() const override;

      std::string provider() const override;

      void clear() override { m_md.clear(); }

   private:
      void add_data(std::span<const uint8_t> input) override;

      void final_result(std::span<uint8_t> output) override;

   private:
      MerkleDamgard_Hash<SHA_512_256> m_md;
};

}  // namespace Botan

namespace Botan {

/*
* SHA-512 F1 Function
*/
BOTAN_FORCE_INLINE void SHA2_64_F(uint64_t A,
                                  uint64_t B,
                                  uint64_t C,
                                  uint64_t& D,
                                  uint64_t E,
                                  uint64_t F,
                                  uint64_t G,
                                  uint64_t& H,
                                  uint64_t& M1,
                                  uint64_t M2,
                                  uint64_t M3,
                                  uint64_t M4,
                                  uint64_t magic) {
   H += magic + rho<14, 18, 41>(E) + choose(E, F, G) + M1;
   D += H;
   H += rho<28, 34, 39>(A) + majority(A, B, C);
   M1 += sigma<19, 61, 6>(M2) + M3 + sigma<1, 8, 7>(M4);
}

/*
* SHA-512 F1 Function (No Message Expansion)
*/
BOTAN_FORCE_INLINE void SHA2_64_F(
   uint64_t A, uint64_t B, uint64_t C, uint64_t& D, uint64_t E, uint64_t F, uint64_t G, uint64_t& H, uint64_t M) {
   H += rho<14, 18, 41>(E) + choose(E, F, G) + M;
   D += H;
   H += rho<28, 34, 39>(A) + majority(A, B, C);
}

}  // namespace Botan

namespace Botan {

/**
* SHA-3
*/
class SHA_3 : public HashFunction {
   public:
      /**
      * @param output_bits the size of the hash output; must be one of
      *                    224, 256, 384, or 512
      */
      explicit SHA_3(size_t output_bits);

      size_t hash_block_size() const override { return m_keccak.byte_rate(); }

      size_t output_length() const override { return m_output_length; }

      std::unique_ptr<HashFunction> new_object() const override;
      std::unique_ptr<HashFunction> copy_state() const override;
      std::string name() const override;
      void clear() override;
      std::string provider() const override;

   private:
      void add_data(std::span<const uint8_t> input) override;
      void final_result(std::span<uint8_t> out) override;

   private:
      Keccak_Permutation m_keccak;
      size_t m_output_length;
};

/**
* SHA-3-224
*/
class SHA_3_224 final : public SHA_3 {
   public:
      SHA_3_224() : SHA_3(224) {}
};

/**
* SHA-3-256
*/
class SHA_3_256 final : public SHA_3 {
   public:
      SHA_3_256() : SHA_3(256) {}
};

/**
* SHA-3-384
*/
class SHA_3_384 final : public SHA_3 {
   public:
      SHA_3_384() : SHA_3(384) {}
};

/**
* SHA-3-512
*/
class SHA_3_512 final : public SHA_3 {
   public:
      SHA_3_512() : SHA_3(512) {}
};

}  // namespace Botan

#if defined(BOTAN_TARGET_ARCH_SUPPORTS_SSSE3)
   #include <immintrin.h>
   #define BOTAN_SIMD_USE_SSSE3

#elif defined(BOTAN_TARGET_ARCH_SUPPORTS_ALTIVEC)
   #include <altivec.h>
   #undef vector
   #undef bool
   #define BOTAN_SIMD_USE_ALTIVEC
   #ifdef __VSX__
      #define BOTAN_SIMD_USE_VSX
   #endif

#elif defined(BOTAN_TARGET_ARCH_SUPPORTS_NEON)
   #include <arm_neon.h>
   #include <bit>
   #define BOTAN_SIMD_USE_NEON

#elif defined(BOTAN_TARGET_ARCH_SUPPORTS_LSX)
   #include <lsxintrin.h>
   #define BOTAN_SIMD_USE_LSX

#elif defined(BOTAN_TARGET_ARCH_SUPPORTS_SIMD128)
   #include <wasm_simd128.h>
   #define BOTAN_SIMD_USE_SIMD128

#else
   #error "No SIMD instruction set enabled"
#endif

namespace Botan {

// NOLINTBEGIN(portability-simd-intrinsics)

/**
* 4x32 bit SIMD register
*
* This class is not a general purpose SIMD type, and only offers instructions
* needed for evaluation of specific crypto primitives. For example it does not
* currently have equality operators of any kind.
*
* Implemented for SSE2, VMX (Altivec), ARMv7/Aarch64 NEON, LoongArch LSX and Wasm SIMD128
*/
class SIMD_4x32 final {
   public:
#if defined(BOTAN_SIMD_USE_SSSE3) || defined(BOTAN_SIMD_USE_LSX)
      using native_simd_type = __m128i;
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
      using native_simd_type = __vector unsigned int;
#elif defined(BOTAN_SIMD_USE_NEON)
      using native_simd_type = uint32x4_t;
#elif defined(BOTAN_SIMD_USE_SIMD128)
      using native_simd_type = v128_t;
#endif

      SIMD_4x32& operator=(const SIMD_4x32& other) = default;
      SIMD_4x32(const SIMD_4x32& other) = default;

      SIMD_4x32& operator=(SIMD_4x32&& other) = default;
      SIMD_4x32(SIMD_4x32&& other) = default;

      ~SIMD_4x32() = default;

      /* NOLINTBEGIN(*-prefer-member-initializer) */

      /**
      * Zero initialize SIMD register with 4 32-bit elements
      */
      BOTAN_FN_ISA_SIMD_4X32 SIMD_4x32() noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)
         m_simd = _mm_setzero_si128();
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_simd = vec_splat_u32(0);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_simd = vdupq_n_u32(0);
#elif defined(BOTAN_SIMD_USE_LSX)
         m_simd = __lsx_vldi(0);
#elif defined(BOTAN_SIMD_USE_SIMD128)
         m_simd = wasm_u32x4_const_splat(0);
#endif
      }

      /**
      * Load SIMD register with 4 32-bit elements
      */
      BOTAN_FN_ISA_SIMD_4X32 SIMD_4x32(uint32_t B0, uint32_t B1, uint32_t B2, uint32_t B3) noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)
         m_simd = _mm_set_epi32(B3, B2, B1, B0);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         __vector unsigned int val = {B0, B1, B2, B3};
         m_simd = val;
#elif defined(BOTAN_SIMD_USE_NEON)
         // Better way to do this?
         const uint32_t B[4] = {B0, B1, B2, B3};
         m_simd = vld1q_u32(B);
#elif defined(BOTAN_SIMD_USE_LSX)
         // Better way to do this?
         const uint32_t B[4] = {B0, B1, B2, B3};
         m_simd = __lsx_vld(B, 0);
#elif defined(BOTAN_SIMD_USE_SIMD128)
         m_simd = wasm_u32x4_make(B0, B1, B2, B3);
#endif
      }

      /* NOLINTEND(*-prefer-member-initializer) */

      /**
      * Load SIMD register with one 32-bit element repeated
      */
      static SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 splat(uint32_t B) noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)
         return SIMD_4x32(_mm_set1_epi32(B));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vdupq_n_u32(B));
#elif defined(BOTAN_SIMD_USE_LSX)
         return SIMD_4x32(__lsx_vreplgr2vr_w(B));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         return SIMD_4x32(wasm_u32x4_splat(B));
#else
         return SIMD_4x32(B, B, B, B);
#endif
      }

      /**
      * Load SIMD register with one 8-bit element repeated
      */
      static SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 splat_u8(uint8_t B) noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)
         return SIMD_4x32(_mm_set1_epi8(B));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vreinterpretq_u32_u8(vdupq_n_u8(B)));
#elif defined(BOTAN_SIMD_USE_LSX)
         return SIMD_4x32(__lsx_vreplgr2vr_b(B));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         return SIMD_4x32(wasm_u8x16_splat(B));
#else
         const uint32_t B4 = make_uint32(B, B, B, B);
         return SIMD_4x32(B4, B4, B4, B4);
#endif
      }

      /**
      * Load a SIMD register with little-endian convention
      */
      static SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 load_le(const void* in) noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)
         return SIMD_4x32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(in)));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         uint32_t R0 = Botan::load_le<uint32_t>(reinterpret_cast<const uint8_t*>(in), 0);
         uint32_t R1 = Botan::load_le<uint32_t>(reinterpret_cast<const uint8_t*>(in), 1);
         uint32_t R2 = Botan::load_le<uint32_t>(reinterpret_cast<const uint8_t*>(in), 2);
         uint32_t R3 = Botan::load_le<uint32_t>(reinterpret_cast<const uint8_t*>(in), 3);
         __vector unsigned int val = {R0, R1, R2, R3};
         return SIMD_4x32(val);
#elif defined(BOTAN_SIMD_USE_NEON)
         SIMD_4x32 l(vld1q_u32(static_cast<const uint32_t*>(in)));
         if constexpr(std::endian::native == std::endian::big) {
            return l.bswap();
         } else {
            return l;
         }
#elif defined(BOTAN_SIMD_USE_LSX)
         return SIMD_4x32(__lsx_vld(in, 0));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         return SIMD_4x32(wasm_v128_load(in));
#endif
      }

      /**
      * Load a SIMD register with big-endian convention
      */
      static SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 load_be(const void* in) noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3) || defined(BOTAN_SIMD_USE_LSX) || defined(BOTAN_SIMD_USE_SIMD128)
         return load_le(in).bswap();

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         uint32_t R0 = Botan::load_be<uint32_t>(reinterpret_cast<const uint8_t*>(in), 0);
         uint32_t R1 = Botan::load_be<uint32_t>(reinterpret_cast<const uint8_t*>(in), 1);
         uint32_t R2 = Botan::load_be<uint32_t>(reinterpret_cast<const uint8_t*>(in), 2);
         uint32_t R3 = Botan::load_be<uint32_t>(reinterpret_cast<const uint8_t*>(in), 3);
         __vector unsigned int val = {R0, R1, R2, R3};
         return SIMD_4x32(val);

#elif defined(BOTAN_SIMD_USE_NEON)
         SIMD_4x32 l(vld1q_u32(static_cast<const uint32_t*>(in)));
         if constexpr(std::endian::native == std::endian::little) {
            return l.bswap();
         } else {
            return l;
         }
#endif
      }

      static SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 load_le(std::span<const uint8_t, 16> in) {
         return SIMD_4x32::load_le(in.data());
      }

      static SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 load_be(std::span<const uint8_t, 16> in) {
         return SIMD_4x32::load_be(in.data());
      }

      void BOTAN_FN_ISA_SIMD_4X32 store_le(uint32_t out[4]) const noexcept {
         this->store_le(reinterpret_cast<uint8_t*>(out));
      }

      void BOTAN_FN_ISA_SIMD_4X32 store_be(uint32_t out[4]) const noexcept {
         this->store_be(reinterpret_cast<uint8_t*>(out));
      }

      void BOTAN_FN_ISA_SIMD_4X32 store_le(uint64_t out[2]) const noexcept {
         this->store_le(reinterpret_cast<uint8_t*>(out));
      }

      /**
      * Load a SIMD register with little-endian convention
      */
      void BOTAN_FN_ISA_SIMD_4X32 store_le(uint8_t out[]) const noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)

         _mm_storeu_si128(reinterpret_cast<__m128i*>(out), raw());

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         union {
               __vector unsigned int V;
               uint32_t R[4];
         } vec{};

         // NOLINTNEXTLINE(*-union-access)
         vec.V = raw();
         // NOLINTNEXTLINE(*-union-access)
         Botan::store_le(out, vec.R[0], vec.R[1], vec.R[2], vec.R[3]);

#elif defined(BOTAN_SIMD_USE_NEON)
         if constexpr(std::endian::native == std::endian::little) {
            vst1q_u8(out, vreinterpretq_u8_u32(m_simd));
         } else {
            vst1q_u8(out, vreinterpretq_u8_u32(bswap().m_simd));
         }
#elif defined(BOTAN_SIMD_USE_LSX)
         __lsx_vst(raw(), out, 0);
#elif defined(BOTAN_SIMD_USE_SIMD128)
         wasm_v128_store(out, m_simd);
#endif
      }

      /**
      * Load a SIMD register with big-endian convention
      */
      BOTAN_FN_ISA_SIMD_4X32 void store_be(uint8_t out[]) const noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3) || defined(BOTAN_SIMD_USE_LSX) || defined(BOTAN_SIMD_USE_SIMD128)

         bswap().store_le(out);

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         union {
               __vector unsigned int V;
               uint32_t R[4];
         } vec{};

         // NOLINTNEXTLINE(*-union-access)
         vec.V = m_simd;
         // NOLINTNEXTLINE(*-union-access)
         Botan::store_be(out, vec.R[0], vec.R[1], vec.R[2], vec.R[3]);

#elif defined(BOTAN_SIMD_USE_NEON)
         if constexpr(std::endian::native == std::endian::little) {
            vst1q_u8(out, vreinterpretq_u8_u32(bswap().m_simd));
         } else {
            vst1q_u8(out, vreinterpretq_u8_u32(m_simd));
         }
#endif
      }

      void BOTAN_FN_ISA_SIMD_4X32 store_be(std::span<uint8_t, 16> out) const { this->store_be(out.data()); }

      void BOTAN_FN_ISA_SIMD_4X32 store_le(std::span<uint8_t, 16> out) const { this->store_le(out.data()); }

      /*
      * This is used for SHA-2/SHACAL2
      */
      SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 sigma0() const noexcept {
#if BOTAN_COMPILER_HAS_BUILTIN(__builtin_crypto_vshasigmaw) && defined(_ARCH_PWR8)
         return SIMD_4x32(__builtin_crypto_vshasigmaw(raw(), 1, 0));
#else
         const SIMD_4x32 r1 = this->rotr<2>();
         const SIMD_4x32 r2 = this->rotr<13>();
         const SIMD_4x32 r3 = this->rotr<22>();
         return (r1 ^ r2 ^ r3);
#endif
      }

      /*
      * This is used for SHA-2/SHACAL2
      */
      SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 sigma1() const noexcept {
#if BOTAN_COMPILER_HAS_BUILTIN(__builtin_crypto_vshasigmaw) && defined(_ARCH_PWR8)
         return SIMD_4x32(__builtin_crypto_vshasigmaw(raw(), 1, 0xF));
#else
         const SIMD_4x32 r1 = this->rotr<6>();
         const SIMD_4x32 r2 = this->rotr<11>();
         const SIMD_4x32 r3 = this->rotr<25>();
         return (r1 ^ r2 ^ r3);
#endif
      }

      /**
      * Left rotation by a compile time constant
      */
      template <size_t ROT>
      BOTAN_FN_ISA_SIMD_4X32 SIMD_4x32 rotl() const noexcept
         requires(ROT > 0 && ROT < 32)
      {
#if defined(BOTAN_SIMD_USE_SSSE3)
         if constexpr(ROT == 8) {
            const auto shuf_rotl_8 = _mm_set_epi64x(0x0e0d0c0f0a09080b, 0x0605040702010003);
            return SIMD_4x32(_mm_shuffle_epi8(raw(), shuf_rotl_8));
         } else if constexpr(ROT == 16) {
            const auto shuf_rotl_16 = _mm_set_epi64x(0x0d0c0f0e09080b0a, 0x0504070601000302);
            return SIMD_4x32(_mm_shuffle_epi8(raw(), shuf_rotl_16));
         } else if constexpr(ROT == 24) {
            const auto shuf_rotl_24 = _mm_set_epi64x(0x0c0f0e0d080b0a09, 0x0407060500030201);
            return SIMD_4x32(_mm_shuffle_epi8(raw(), shuf_rotl_24));
         } else {
            return SIMD_4x32(_mm_xor_si128(_mm_slli_epi32(raw(), static_cast<int>(ROT)),
                                           _mm_srli_epi32(raw(), static_cast<int>(32 - ROT))));
         }

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         const unsigned int r = static_cast<unsigned int>(ROT);
         __vector unsigned int rot = {r, r, r, r};
         return SIMD_4x32(vec_rl(m_simd, rot));

#elif defined(BOTAN_SIMD_USE_NEON)

   #if defined(BOTAN_TARGET_ARCH_IS_ARM64)

         if constexpr(ROT == 8) {
            const uint8_t maskb[16] = {3, 0, 1, 2, 7, 4, 5, 6, 11, 8, 9, 10, 15, 12, 13, 14};
            const uint8x16_t mask = vld1q_u8(maskb);
            return SIMD_4x32(vreinterpretq_u32_u8(vqtbl1q_u8(vreinterpretq_u8_u32(m_simd), mask)));
         } else if constexpr(ROT == 16) {
            return SIMD_4x32(vreinterpretq_u32_u16(vrev32q_u16(vreinterpretq_u16_u32(m_simd))));
         }
   #endif
         return SIMD_4x32(
            vorrq_u32(vshlq_n_u32(m_simd, static_cast<int>(ROT)), vshrq_n_u32(m_simd, static_cast<int>(32 - ROT))));
#elif defined(BOTAN_SIMD_USE_LSX)
         return SIMD_4x32(__lsx_vrotri_w(raw(), 32 - ROT));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         return SIMD_4x32(wasm_v128_or(wasm_i32x4_shl(m_simd, ROT), wasm_u32x4_shr(m_simd, 32 - ROT)));
#endif
      }

      /**
      * Right rotation by a compile time constant
      */
      template <size_t ROT>
      SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 rotr() const noexcept {
         return this->rotl<32 - ROT>();
      }

      /**
      * Add elements of a SIMD vector
      */
      SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 operator+(const SIMD_4x32& other) const noexcept {
         SIMD_4x32 retval(*this);
         retval += other;
         return retval;
      }

      /**
      * Subtract elements of a SIMD vector
      */
      SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 operator-(const SIMD_4x32& other) const noexcept {
         SIMD_4x32 retval(*this);
         retval -= other;
         return retval;
      }

      /**
      * XOR elements of a SIMD vector
      */
      SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 operator^(const SIMD_4x32& other) const noexcept {
         SIMD_4x32 retval(*this);
         retval ^= other;
         return retval;
      }

      /**
      * Binary OR elements of a SIMD vector
      */
      SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 operator|(const SIMD_4x32& other) const noexcept {
         SIMD_4x32 retval(*this);
         retval |= other;
         return retval;
      }

      /**
      * Binary AND elements of a SIMD vector
      */
      SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 operator&(const SIMD_4x32& other) const noexcept {
         SIMD_4x32 retval(*this);
         retval &= other;
         return retval;
      }

      void BOTAN_FN_ISA_SIMD_4X32 operator+=(const SIMD_4x32& other) noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)
         m_simd = _mm_add_epi32(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_simd = vec_add(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_simd = vaddq_u32(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_LSX)
         m_simd = __lsx_vadd_w(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_SIMD128)
         m_simd = wasm_i32x4_add(m_simd, other.m_simd);
#endif
      }

      void BOTAN_FN_ISA_SIMD_4X32 operator-=(const SIMD_4x32& other) noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)
         m_simd = _mm_sub_epi32(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_simd = vec_sub(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_simd = vsubq_u32(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_LSX)
         m_simd = __lsx_vsub_w(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_SIMD128)
         m_simd = wasm_i32x4_sub(m_simd, other.m_simd);
#endif
      }

      void BOTAN_FN_ISA_SIMD_4X32 operator^=(const SIMD_4x32& other) noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)
         m_simd = _mm_xor_si128(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_simd = vec_xor(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_simd = veorq_u32(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_LSX)
         m_simd = __lsx_vxor_v(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_SIMD128)
         m_simd = wasm_v128_xor(m_simd, other.m_simd);
#endif
      }

      void BOTAN_FN_ISA_SIMD_4X32 operator^=(uint32_t other) noexcept { *this ^= SIMD_4x32::splat(other); }

      void BOTAN_FN_ISA_SIMD_4X32 operator|=(const SIMD_4x32& other) noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)
         m_simd = _mm_or_si128(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_simd = vec_or(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_simd = vorrq_u32(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_LSX)
         m_simd = __lsx_vor_v(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_SIMD128)
         m_simd = wasm_v128_or(m_simd, other.m_simd);
#endif
      }

      void BOTAN_FN_ISA_SIMD_4X32 operator&=(const SIMD_4x32& other) noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)
         m_simd = _mm_and_si128(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_simd = vec_and(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_simd = vandq_u32(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_LSX)
         m_simd = __lsx_vand_v(m_simd, other.m_simd);
#elif defined(BOTAN_SIMD_USE_SIMD128)
         m_simd = wasm_v128_and(m_simd, other.m_simd);
#endif
      }

      template <int SHIFT>
      SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 shl() const noexcept
         requires(SHIFT > 0 && SHIFT < 32)
      {
#if defined(BOTAN_SIMD_USE_SSSE3)
         return SIMD_4x32(_mm_slli_epi32(m_simd, SHIFT));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const unsigned int s = static_cast<unsigned int>(SHIFT);
         const __vector unsigned int shifts = {s, s, s, s};
         return SIMD_4x32(vec_sl(m_simd, shifts));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vshlq_n_u32(m_simd, SHIFT));
#elif defined(BOTAN_SIMD_USE_LSX)
         return SIMD_4x32(__lsx_vslli_w(m_simd, SHIFT));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         return SIMD_4x32(wasm_i32x4_shl(m_simd, SHIFT));
#endif
      }

      template <int SHIFT>
      SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 shr() const noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)
         return SIMD_4x32(_mm_srli_epi32(m_simd, SHIFT));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const unsigned int s = static_cast<unsigned int>(SHIFT);
         const __vector unsigned int shifts = {s, s, s, s};
         return SIMD_4x32(vec_sr(m_simd, shifts));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vshrq_n_u32(m_simd, SHIFT));
#elif defined(BOTAN_SIMD_USE_LSX)
         return SIMD_4x32(__lsx_vsrli_w(m_simd, SHIFT));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         return SIMD_4x32(wasm_u32x4_shr(m_simd, SHIFT));
#endif
      }

      SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 operator~() const noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)
         return SIMD_4x32(_mm_xor_si128(m_simd, _mm_set1_epi32(0xFFFFFFFF)));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         return SIMD_4x32(vec_nor(m_simd, m_simd));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vmvnq_u32(m_simd));
#elif defined(BOTAN_SIMD_USE_LSX)
         return SIMD_4x32(__lsx_vnor_v(m_simd, m_simd));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         return SIMD_4x32(wasm_v128_not(m_simd));
#endif
      }

      // (~reg) & other
      SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 andc(const SIMD_4x32& other) const noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)
         return SIMD_4x32(_mm_andnot_si128(m_simd, other.m_simd));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         /*
         AltiVec does arg1 & ~arg2 rather than SSE's ~arg1 & arg2
         so swap the arguments
         */
         return SIMD_4x32(vec_andc(other.m_simd, m_simd));
#elif defined(BOTAN_SIMD_USE_NEON)
         // NEON is also a & ~b
         return SIMD_4x32(vbicq_u32(other.m_simd, m_simd));
#elif defined(BOTAN_SIMD_USE_LSX)
         // LSX is ~a & b
         return SIMD_4x32(__lsx_vandn_v(m_simd, other.m_simd));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         // SIMD128 is a & ~b
         return SIMD_4x32(wasm_v128_andnot(other.m_simd, m_simd));
#endif
      }

      /**
      * Return copy *this with each word byte swapped
      */
      BOTAN_FN_ISA_SIMD_4X32 SIMD_4x32 bswap() const noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)
         const auto idx = _mm_set_epi8(12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3);

         return SIMD_4x32(_mm_shuffle_epi8(raw(), idx));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
   #ifdef BOTAN_SIMD_USE_VSX
         return SIMD_4x32(vec_revb(m_simd));
   #else
         const __vector unsigned char rev[1] = {
            {3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12},
         };

         return SIMD_4x32(vec_perm(m_simd, m_simd, rev[0]));
   #endif

#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(m_simd))));
#elif defined(BOTAN_SIMD_USE_LSX)
         return SIMD_4x32(__lsx_vshuf4i_b(m_simd, 0b00011011));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         return SIMD_4x32(wasm_i8x16_shuffle(m_simd, m_simd, 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12));
#endif
      }

      template <size_t I>
      SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 shift_elems_left() const noexcept
         requires(I <= 3)
      {
#if defined(BOTAN_SIMD_USE_SSSE3)
         return SIMD_4x32(_mm_slli_si128(raw(), 4 * I));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vextq_u32(vdupq_n_u32(0), raw(), 4 - I));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const __vector unsigned int zero = vec_splat_u32(0);

         const __vector unsigned char shuf[3] = {
            {16, 17, 18, 19, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
            {16, 17, 18, 19, 20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7},
            {16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 0, 1, 2, 3},
         };

         return SIMD_4x32(vec_perm(raw(), zero, shuf[I - 1]));
#elif defined(BOTAN_SIMD_USE_LSX)
         return SIMD_4x32(__lsx_vbsll_v(raw(), 4 * I));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         if constexpr(I == 0) {
            return SIMD_4x32(m_simd);
         }

         const auto zero = wasm_u32x4_const_splat(0);
         if constexpr(I == 1) {
            return SIMD_4x32(wasm_i8x16_shuffle(m_simd, zero, 16, 16, 16, 16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11));
         }
         if constexpr(I == 2) {
            return SIMD_4x32(wasm_i8x16_shuffle(m_simd, zero, 16, 16, 16, 16, 16, 16, 16, 16, 0, 1, 2, 3, 4, 5, 6, 7));
         }

         return SIMD_4x32(wasm_i8x16_shuffle(m_simd, zero, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 1, 2, 3));
#endif
      }

      template <size_t I>
      SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 shift_elems_right() const noexcept
         requires(I <= 3)
      {
#if defined(BOTAN_SIMD_USE_SSSE3)
         return SIMD_4x32(_mm_srli_si128(raw(), 4 * I));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vextq_u32(raw(), vdupq_n_u32(0), I));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const __vector unsigned int zero = vec_splat_u32(0);

         const __vector unsigned char shuf[3] = {
            {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19},
            {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23},
            {12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27},
         };

         return SIMD_4x32(vec_perm(raw(), zero, shuf[I - 1]));
#elif defined(BOTAN_SIMD_USE_LSX)
         return SIMD_4x32(__lsx_vbsrl_v(raw(), 4 * I));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         if constexpr(I == 0) {
            return SIMD_4x32(m_simd);
         }

         const auto zero = wasm_u32x4_const_splat(0);
         if constexpr(I == 1) {
            return SIMD_4x32(
               wasm_i8x16_shuffle(m_simd, zero, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16));
         }
         if constexpr(I == 2) {
            return SIMD_4x32(
               wasm_i8x16_shuffle(m_simd, zero, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16));
         }

         return SIMD_4x32(
            wasm_i8x16_shuffle(m_simd, zero, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16));
#endif
      }

      /**
      * 4x4 Transposition on SIMD registers
      */
      static void BOTAN_FN_ISA_SIMD_4X32 transpose(SIMD_4x32& B0,
                                                   SIMD_4x32& B1,
                                                   SIMD_4x32& B2,
                                                   SIMD_4x32& B3) noexcept {
#if defined(BOTAN_SIMD_USE_SSSE3)
         const __m128i T0 = _mm_unpacklo_epi32(B0.m_simd, B1.m_simd);
         const __m128i T1 = _mm_unpacklo_epi32(B2.m_simd, B3.m_simd);
         const __m128i T2 = _mm_unpackhi_epi32(B0.m_simd, B1.m_simd);
         const __m128i T3 = _mm_unpackhi_epi32(B2.m_simd, B3.m_simd);

         B0.m_simd = _mm_unpacklo_epi64(T0, T1);
         B1.m_simd = _mm_unpackhi_epi64(T0, T1);
         B2.m_simd = _mm_unpacklo_epi64(T2, T3);
         B3.m_simd = _mm_unpackhi_epi64(T2, T3);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const __vector unsigned int T0 = vec_mergeh(B0.m_simd, B2.m_simd);
         const __vector unsigned int T1 = vec_mergeh(B1.m_simd, B3.m_simd);
         const __vector unsigned int T2 = vec_mergel(B0.m_simd, B2.m_simd);
         const __vector unsigned int T3 = vec_mergel(B1.m_simd, B3.m_simd);

         B0.m_simd = vec_mergeh(T0, T1);
         B1.m_simd = vec_mergel(T0, T1);
         B2.m_simd = vec_mergeh(T2, T3);
         B3.m_simd = vec_mergel(T2, T3);

#elif defined(BOTAN_SIMD_USE_NEON) && defined(BOTAN_TARGET_ARCH_IS_ARM32)
         const uint32x4x2_t T0 = vzipq_u32(B0.m_simd, B2.m_simd);
         const uint32x4x2_t T1 = vzipq_u32(B1.m_simd, B3.m_simd);
         const uint32x4x2_t O0 = vzipq_u32(T0.val[0], T1.val[0]);
         const uint32x4x2_t O1 = vzipq_u32(T0.val[1], T1.val[1]);

         B0.m_simd = O0.val[0];
         B1.m_simd = O0.val[1];
         B2.m_simd = O1.val[0];
         B3.m_simd = O1.val[1];

#elif defined(BOTAN_SIMD_USE_NEON) && defined(BOTAN_TARGET_ARCH_IS_ARM64)
         const uint32x4_t T0 = vzip1q_u32(B0.m_simd, B2.m_simd);
         const uint32x4_t T2 = vzip2q_u32(B0.m_simd, B2.m_simd);
         const uint32x4_t T1 = vzip1q_u32(B1.m_simd, B3.m_simd);
         const uint32x4_t T3 = vzip2q_u32(B1.m_simd, B3.m_simd);

         B0.m_simd = vzip1q_u32(T0, T1);
         B1.m_simd = vzip2q_u32(T0, T1);
         B2.m_simd = vzip1q_u32(T2, T3);
         B3.m_simd = vzip2q_u32(T2, T3);
#elif defined(BOTAN_SIMD_USE_LSX)
         const __m128i T0 = __lsx_vilvl_w(B2.raw(), B0.raw());
         const __m128i T1 = __lsx_vilvh_w(B2.raw(), B0.raw());
         const __m128i T2 = __lsx_vilvl_w(B3.raw(), B1.raw());
         const __m128i T3 = __lsx_vilvh_w(B3.raw(), B1.raw());
         B0.m_simd = __lsx_vilvl_w(T2, T0);
         B1.m_simd = __lsx_vilvh_w(T2, T0);
         B2.m_simd = __lsx_vilvl_w(T3, T1);
         B3.m_simd = __lsx_vilvh_w(T3, T1);
#elif defined(BOTAN_SIMD_USE_SIMD128)
         const auto T0 = wasm_i32x4_shuffle(B0.m_simd, B2.m_simd, 0, 4, 1, 5);
         const auto T2 = wasm_i32x4_shuffle(B0.m_simd, B2.m_simd, 2, 6, 3, 7);
         const auto T1 = wasm_i32x4_shuffle(B1.m_simd, B3.m_simd, 0, 4, 1, 5);
         const auto T3 = wasm_i32x4_shuffle(B1.m_simd, B3.m_simd, 2, 6, 3, 7);

         B0.m_simd = wasm_i32x4_shuffle(T0, T1, 0, 4, 1, 5);
         B1.m_simd = wasm_i32x4_shuffle(T0, T1, 2, 6, 3, 7);
         B2.m_simd = wasm_i32x4_shuffle(T2, T3, 0, 4, 1, 5);
         B3.m_simd = wasm_i32x4_shuffle(T2, T3, 2, 6, 3, 7);
#endif
      }

      static inline SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 choose(const SIMD_4x32& mask,
                                                            const SIMD_4x32& a,
                                                            const SIMD_4x32& b) noexcept {
#if defined(BOTAN_SIMD_USE_ALTIVEC)
         return SIMD_4x32(vec_sel(b.raw(), a.raw(), mask.raw()));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vbslq_u32(mask.raw(), a.raw(), b.raw()));
#elif defined(BOTAN_SIMD_USE_LSX)
         return SIMD_4x32(__lsx_vbitsel_v(b.raw(), a.raw(), mask.raw()));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         return SIMD_4x32(wasm_v128_bitselect(a.raw(), b.raw(), mask.raw()));
#else
         return (mask & a) ^ mask.andc(b);
#endif
      }

      /**
      * Byte-granularity blend: for each byte position, select from @p a where
      * the corresponding mask byte is 0xFF, or from @p b where it is 0x00.
      *
      * Each byte of @p mask must be either 0x00 or 0xFF; other values produce
      * undefined results.
      */
      static inline SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 byte_blend(const SIMD_4x32& mask,
                                                                const SIMD_4x32& a,
                                                                const SIMD_4x32& b) noexcept {
         return SIMD_4x32::choose(mask, a, b);
      }

      /**
      * Byte-granularity blend: for each byte position, select from @p a where
      * the corresponding mask byte is 0xFF, or from @p b where it is 0x00.
      *
      * Each byte of @p mask must be either 0x00 or 0xFF; other values produce
      * undefined results.
      */
      static inline SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 byte_blend(uint32_t mask,
                                                                const SIMD_4x32& a,
                                                                const SIMD_4x32& b) noexcept {
         return SIMD_4x32::byte_blend(SIMD_4x32::splat(mask), a, b);
      }

      static inline SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 majority(const SIMD_4x32& x,
                                                              const SIMD_4x32& y,
                                                              const SIMD_4x32& z) noexcept {
         return SIMD_4x32::choose(x ^ y, z, y);
      }

      /**
      * Byte shuffle
      *
      * This function assumes that each byte of idx is <= 16; it may produce incorrect
      * results if this does not hold.
      */
      static inline SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 byte_shuffle(const SIMD_4x32& tbl, const SIMD_4x32& idx) {
#if defined(BOTAN_SIMD_USE_SSSE3)
         return SIMD_4x32(_mm_shuffle_epi8(tbl.raw(), idx.raw()));
#elif defined(BOTAN_SIMD_USE_NEON)
         const uint8x16_t tbl8 = vreinterpretq_u8_u32(tbl.raw());
         const uint8x16_t idx8 = vreinterpretq_u8_u32(idx.raw());

   #if defined(BOTAN_TARGET_ARCH_IS_ARM32)
         const uint8x8x2_t tbl2 = {vget_low_u8(tbl8), vget_high_u8(tbl8)};

         return SIMD_4x32(
            vreinterpretq_u32_u8(vcombine_u8(vtbl2_u8(tbl2, vget_low_u8(idx8)), vtbl2_u8(tbl2, vget_high_u8(idx8)))));
   #else
         return SIMD_4x32(vreinterpretq_u32_u8(vqtbl1q_u8(tbl8, idx8)));
   #endif

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const auto r = vec_perm(reinterpret_cast<__vector signed char>(tbl.raw()),
                                 reinterpret_cast<__vector signed char>(tbl.raw()),
                                 reinterpret_cast<__vector unsigned char>(idx.raw()));
         return SIMD_4x32(reinterpret_cast<__vector unsigned int>(r));
#elif defined(BOTAN_SIMD_USE_LSX)
         return SIMD_4x32(__lsx_vshuf_b(tbl.raw(), tbl.raw(), idx.raw()));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         return SIMD_4x32(wasm_i8x16_swizzle(tbl.raw(), idx.raw()));
#endif
      }

      /**
      * Byte shuffle with masking
      *
      * If the index is >= 128 then the output byte is set to zero.
      *
      * Warning: for indices between 16 and 128 this function may have different
      * behaviors depending on the CPU; possibly the output is zero, tbl[idx % 16],
      * or even undefined.
      */
      inline static SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 masked_byte_shuffle(const SIMD_4x32& tbl, const SIMD_4x32& idx) {
#if defined(BOTAN_SIMD_USE_ALTIVEC)
         const auto zero = vec_splat_s8(0x00);
         const auto mask = vec_cmplt(reinterpret_cast<__vector signed char>(idx.raw()), zero);
         const auto r = vec_perm(reinterpret_cast<__vector signed char>(tbl.raw()),
                                 reinterpret_cast<__vector signed char>(tbl.raw()),
                                 reinterpret_cast<__vector unsigned char>(idx.raw()));
         return SIMD_4x32(reinterpret_cast<__vector unsigned int>(vec_sel(r, zero, mask)));
#elif defined(BOTAN_SIMD_USE_LSX)
         /*
         * The behavior of vshuf.b unfortunately differs among microarchitectures
         * when the index is larger than the available elements. In LA664 CPUs,
         * larger indices result in a zero byte, which is exactly what we want.
         * Unfortunately on LA464 machines, the output is instead undefined.
         *
         * So we must use a slower sequence that handles the larger indices.
         * If we had a way of knowing at compile time that we are on an LA664
         * or later, we could use __lsx_vshuf_b without the comparison or select.
         */
         const auto zero = __lsx_vldi(0);
         const auto r = __lsx_vshuf_b(zero, tbl.raw(), idx.raw());
         const auto mask = __lsx_vslti_bu(idx.raw(), 16);
         return SIMD_4x32(__lsx_vbitsel_v(zero, r, mask));
#else
         // ARM, x86 and Wasm byte shuffles have the behavior we want for out of range idx
         return SIMD_4x32::byte_shuffle(tbl, idx);
#endif
      }

      static inline SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 alignr4(const SIMD_4x32& a, const SIMD_4x32& b) {
#if defined(BOTAN_SIMD_USE_SSSE3)
         return SIMD_4x32(_mm_alignr_epi8(a.raw(), b.raw(), 4));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vextq_u32(b.raw(), a.raw(), 1));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const __vector unsigned char mask = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
         return SIMD_4x32(vec_perm(b.raw(), a.raw(), mask));
#elif defined(BOTAN_SIMD_USE_LSX)
         const auto mask = SIMD_4x32(0x07060504, 0x0B0A0908, 0x0F0E0D0C, 0x13121110);
         return SIMD_4x32(__lsx_vshuf_b(a.raw(), b.raw(), mask.raw()));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         return SIMD_4x32(
            wasm_i8x16_shuffle(b.raw(), a.raw(), 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19));
#endif
      }

      static inline SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 alignr8(const SIMD_4x32& a, const SIMD_4x32& b) {
#if defined(BOTAN_SIMD_USE_SSSE3)
         return SIMD_4x32(_mm_alignr_epi8(a.raw(), b.raw(), 8));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vextq_u32(b.raw(), a.raw(), 2));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const __vector unsigned char mask = {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
         return SIMD_4x32(vec_perm(b.raw(), a.raw(), mask));
#elif defined(BOTAN_SIMD_USE_LSX)
         return SIMD_4x32(__lsx_vshuf4i_d(a.raw(), b.raw(), 0b0011));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         return SIMD_4x32(
            wasm_i8x16_shuffle(b.raw(), a.raw(), 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23));
#endif
      }

      /**
      If the topmost bit of x is set, return a vector of all ones, otherwise a vector of all zeros
      ie: (v >> 127) ? splat(0xFFFFFFFF) : zero;

      Most of the implementations work by doing an arithmetic shift of 31 to smear the top bits
      of each word, followed by a broadcast of the top word.
      */
      inline SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 top_bit_mask() const {
#if defined(BOTAN_SIMD_USE_SSSE3)
         return SIMD_4x32(_mm_shuffle_epi32(_mm_srai_epi32(raw(), 31), 0b11111111));
#elif defined(BOTAN_SIMD_USE_NEON)
   #if defined(BOTAN_TARGET_ARCH_IS_ARM32)
         int32x4_t v = vshrq_n_s32(vreinterpretq_s32_u32(raw()), 31);
         int32x2_t hi = vget_high_s32(v);
         return SIMD_4x32(vreinterpretq_u32_s32(vdupq_lane_s32(hi, 1)));
   #else
         return SIMD_4x32(vreinterpretq_u32_s32(vdupq_laneq_s32(vshrq_n_s32(vreinterpretq_s32_u32(raw()), 31), 3)));
   #endif
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const __vector unsigned int shift = vec_splats(31U);
         const __vector signed int shifted = vec_sra(reinterpret_cast<__vector signed int>(raw()), shift);
         return SIMD_4x32(reinterpret_cast<__vector unsigned int>(vec_splat(shifted, 3)));
#elif defined(BOTAN_SIMD_USE_LSX)
         return SIMD_4x32(__lsx_vshuf4i_w(__lsx_vsrai_w(raw(), 31), 0xFF));
#elif defined(BOTAN_SIMD_USE_SIMD128)
         return SIMD_4x32(wasm_i32x4_splat(wasm_i32x4_extract_lane(wasm_i32x4_shr(raw(), 31), 3)));
#endif
      }

      /**
      * Swap the upper and lower 64-bit halves of the vector
      */
      inline SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 swap_halves() const {
#if defined(BOTAN_SIMD_USE_SSSE3)
         return SIMD_4x32(_mm_shuffle_epi32(raw(), 0b01001110));
#else
         return SIMD_4x32::alignr8(*this, *this);
#endif
      }

      native_simd_type BOTAN_FN_ISA_SIMD_4X32 raw() const noexcept { return m_simd; }

      explicit BOTAN_FN_ISA_SIMD_4X32 SIMD_4x32(native_simd_type x) noexcept : m_simd(x) {}

   private:
      native_simd_type m_simd;
};

// NOLINTEND(portability-simd-intrinsics)

template <size_t R>
inline SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 rotl(SIMD_4x32 input) {
   return input.rotl<R>();
}

template <size_t R>
inline SIMD_4x32 BOTAN_FN_ISA_SIMD_4X32 rotr(SIMD_4x32 input) {
   return input.rotr<R>();
}

// For Serpent:
template <size_t S>
inline SIMD_4x32 shl(SIMD_4x32 input) {
   return input.shl<S>();
}

}  // namespace Botan

namespace Botan {

namespace detail {

template <typename T>
concept SpongeLike = std::unsigned_integral<decltype(T::word_bytes)> && requires(T a) {
   typename T::word_t;
   typename T::state_t;
   { a.state() } -> std::same_as<typename T::state_t&>;
   { a._cursor() } -> std::same_as<size_t&>;
   { a.byte_rate() } -> std::same_as<size_t>;
};

template <typename T>
concept SpongeLikeWithTrivialPermute = SpongeLike<T> && requires(T a) {
   { a.permute() } -> std::same_as<void>;
};

/**
* Represents the bounds of partial byte-oriented data within a word of
* the sponge state. Downstream algorithms can use this to conveniently
* modify the passed in partial state word with data written or read
* from an input or output byte buffer.
*/
template <SpongeLike SpongeT>
class PartialWordBounds final {
   public:
      size_t offset;  // NOLINT(*-non-private-member-*)
      size_t length;  // NOLINT(*-non-private-member-*)

   private:
      using word_t = typename SpongeT::word_t;
      constexpr static auto word_bytes = SpongeT::word_bytes;

   public:
      /**
      * Reads '.length' bytes from the provided slicer and places them
      * within a word at the specified '.offset' in little-endian order.
      */
      word_t read_from(BufferSlicer& slicer) const {
         std::array<uint8_t, word_bytes> partial_word_bytes{};
         slicer.copy_into(std::span{partial_word_bytes}.subspan(offset, length));
         return load_le(partial_word_bytes);
      }

      /**
      * Writes '.length' bytes from the provided word at the specified
      * '.offset' into the provided stuffer in little-endian order.
      */
      void write_into(BufferStuffer& stuffer, word_t partial_word) const {
         const auto partial_word_bytes = store_le(partial_word);
         stuffer.append(std::span{partial_word_bytes}.subspan(offset, length));
      }

      /**
      * Assigns the bits in 'partial_input_word' to their corresponding
      * bits in 'state_word' at the specified '.offset' and '.length'
      * while leaving all other bits in 'state_word' unchanged.
      */
      word_t masked_assignment(word_t state_word, word_t partial_input_word) const {
         BOTAN_DEBUG_ASSERT(length > 0);
         const auto mask = ((word_t(0) - 1) >> ((word_bytes - length) * 8)) << (offset * 8);
         return (state_word & ~mask) | (partial_input_word & mask);
      }
};

/**
* A drop-in replacement for `PartialWordBounds` that is optimized for
* handling full words where no masking or offsetting is necessary.
*/
template <SpongeLike SpongeT>
class FullWordBounds final {
   private:
      using word_t = typename SpongeT::word_t;
      constexpr static auto word_bytes = SpongeT::word_bytes;

   public:
      word_t read_from(BufferSlicer& slicer) const { return load_le(slicer.take<word_bytes>()); }

      void write_into(BufferStuffer& stuffer, word_t full_word) const { stuffer.append(store_le(full_word)); }

      word_t masked_assignment(word_t /*unused*/, word_t full_input_word) const { return full_input_word; }
};

template <typename T>
concept PermutationFn = std::invocable<T> || std::same_as<T, void()>;

template <typename T, typename SpongeT, typename ModifierT>
concept BaseModifierFn = requires(T fn, typename SpongeT::word_t word, ModifierT bounds) {
   { std::invoke(fn, word, bounds) } -> std::same_as<typename SpongeT::word_t>;
};

template <typename T, typename SpongeT>
concept ModifierFn =
   BaseModifierFn<T, SpongeT, FullWordBounds<SpongeT>> || BaseModifierFn<T, SpongeT, PartialWordBounds<SpongeT>>;

}  // namespace detail

/**
* Performs the core processing loop for ingesting or extracting data into/from
* the sponge state in a byte-oriented manner for the given number of
* @p bytes_to_process. The provided @p word_modifier_fn is called for each
* (partial) word of the sponge state that needs to be modified or read.
*
* The processing loop ensures efficient handling of unaligned input and output
* data. For that, it calls the provided permutation function either with an
* instance of `PartialWordBounds` or `FullWordBounds`. Hence @p word_modifier_fn
* must be able to handle both types of bounds and should use their respective
* methods to read from or write into input or output buffers.
*
* @param sponge the sponge instance to process data into or from
* @param bytes_to_process the number of sponge state bytes to traverse
* @param permutation_fn a function that performs the sponge's permutation
* @param modifier_fn a function that modifies the sponge state words
*/
template <detail::SpongeLike SpongeT>
BOTAN_FORCE_INLINE void process_bytes_in_sponge(SpongeT& sponge,
                                                size_t bytes_to_process,
                                                const detail::PermutationFn auto& permutation_fn,
                                                const detail::ModifierFn<SpongeT> auto& modifier_fn) {
   if(bytes_to_process == 0) {
      return;
   }

   constexpr auto word_bytes = SpongeT::word_bytes;
   const auto byte_rate = sponge.byte_rate();
   auto& S = sponge.state();
   auto& cursor = sponge._cursor();

   // If necessary, try to get aligned with the sponge state's words array
   const auto bytes_out_of_word_alignment = static_cast<size_t>(cursor % word_bytes);
   if(bytes_out_of_word_alignment > 0) {
      const auto bytes_until_word_alignment = word_bytes - bytes_out_of_word_alignment;
      const auto bytes_from_input = std::min(bytes_to_process, bytes_until_word_alignment);
      BOTAN_DEBUG_ASSERT(bytes_from_input < word_bytes);

      S[cursor / word_bytes] = modifier_fn(S[cursor / word_bytes],
                                           detail::PartialWordBounds<SpongeT>{
                                              .offset = bytes_out_of_word_alignment,
                                              .length = bytes_from_input,
                                           });
      cursor += bytes_from_input;
      bytes_to_process -= bytes_from_input;

      if(cursor == byte_rate) {
         permutation_fn();
         cursor = 0;
      }
   }

   // If we didn't exhaust the bytes to process for this invocation, we should
   // be word-aligned with the sponge state now
   BOTAN_DEBUG_ASSERT(bytes_to_process == 0 || cursor % word_bytes == 0);

   // Block-wise incorporation of the input data into the sponge state until
   // all input bytes are processed
   while(bytes_to_process >= word_bytes) {
      // Process full words until we either run out of data or reach the
      // end of the current sponge state block
      while(bytes_to_process >= word_bytes && cursor < byte_rate) {
         S[cursor / word_bytes] = modifier_fn(S[cursor / word_bytes], detail::FullWordBounds<SpongeT>{});
         cursor += word_bytes;
         bytes_to_process -= word_bytes;
      }

      if(cursor == byte_rate) {
         permutation_fn();
         cursor = 0;
      }
   }

   // Process the remaining bytes that don't fill an entire word.
   // Therefore, leaving the sponge state in an unaligned state that won't
   // need another permutation until the next call to process().
   BOTAN_DEBUG_ASSERT(bytes_to_process < word_bytes && cursor < byte_rate);
   if(bytes_to_process > 0) {
      S[cursor / word_bytes] = modifier_fn(S[cursor / word_bytes],
                                           detail::PartialWordBounds<SpongeT>{
                                              .offset = 0,
                                              .length = bytes_to_process,
                                           });
      cursor += bytes_to_process;
   }
}

template <detail::SpongeLikeWithTrivialPermute SpongeT>
inline void process_bytes_in_sponge(SpongeT& sponge,
                                    size_t bytes_to_process,
                                    const detail::ModifierFn<SpongeT> auto& modifier_fn) {
   process_bytes_in_sponge(
      sponge, bytes_to_process, [&sponge] { sponge.permute(); }, modifier_fn);
}

/**
* Absorbs @p input data into the @p sponge state.
*
* @param sponge The sponge state to absorb data into.
* @param input The input data to absorb.
* @param permutation_fn The function to call for the sponge's permutation.
*/
template <detail::SpongeLike SpongeT>
inline void absorb_into_sponge(SpongeT& sponge,
                               std::span<const uint8_t> input,
                               const detail::PermutationFn auto& permutation_fn) {
   using word_t = typename SpongeT::word_t;

   BufferSlicer input_slicer(input);
   process_bytes_in_sponge(sponge, input.size(), permutation_fn, [&](word_t state_word, auto bounds) {
      return state_word ^ bounds.read_from(input_slicer);
   });
   BOTAN_ASSERT_NOMSG(input_slicer.empty());
}

inline void absorb_into_sponge(detail::SpongeLikeWithTrivialPermute auto& sponge, std::span<const uint8_t> input) {
   absorb_into_sponge(sponge, input, [&sponge] { sponge.permute(); });
}

/**
* Squeezes @p output data from the @p sponge state.
*
* @param sponge The sponge state to squeeze data from.
* @param output The output buffer to write the squeezed data into.
* @param permutation_fn The function to call for the sponge's permutation.
*/
template <detail::SpongeLike SpongeT>
inline void squeeze_from_sponge(SpongeT& sponge,
                                std::span<uint8_t> output,
                                const detail::PermutationFn auto& permutation_fn) {
   using word_t = typename SpongeT::word_t;

   BufferStuffer output_stuffer(output);
   process_bytes_in_sponge(sponge, output.size(), permutation_fn, [&](word_t state_word, auto bounds) {
      bounds.write_into(output_stuffer, state_word);
      return state_word;
   });
   BOTAN_ASSERT_NOMSG(output_stuffer.full());
}

inline void squeeze_from_sponge(detail::SpongeLikeWithTrivialPermute auto& sponge, std::span<uint8_t> output) {
   squeeze_from_sponge(sponge, output, [&sponge] { sponge.permute(); });
}

}  // namespace Botan

// TODO(Botan4): Move this to compiler.h (currently still a public header)

#if !defined(BOTAN_SCRUB_STACK_AFTER_RETURN)
   #if BOTAN_COMPILER_HAS_ATTRIBUTE(strub) && defined(BOTAN_USE_COMPILER_ASSISTED_STACK_SCRUBBING)
      /**
      * When a function definition is annotated with this macro, the compiler
      * generates a wrapper for the function's body to handle stack scrubbing
      * in the wrapper. In contrast to 'strub("at-calls")' this does not alter
      * the function's ABI.
      *
      * It is okay to use this annotation on C++ method definitions (in *.cpp),
      * even if the function is a public API.
      *
      * Currently this is supported on GCC 14+ only
      * See: https://gcc.gnu.org/onlinedocs/gcc-14.2.0/gcc/Common-Type-Attributes.html#index-strub-type-attribute
      */
      #define BOTAN_SCRUB_STACK_AFTER_RETURN BOTAN_COMPILER_ATTRIBUTE(strub("internal"))
   #else
      #define BOTAN_SCRUB_STACK_AFTER_RETURN
   #endif
#endif

namespace Botan {

/**
 * Reduce the values of @p keys into an accumulator initialized with @p acc using
 * the reducer function @p reducer.
 *
 * The @p reducer is a function taking the accumulator and a single key to return the
 * new accumulator. Keys are consecutively reduced into the accumulator.
 *
 * @return the accumulator containing the reduction of @p keys
 */
template <typename RetT, typename KeyT, typename ReducerT>
RetT reduce(const std::vector<KeyT>& keys, RetT acc, ReducerT reducer)
   requires std::invocable<ReducerT&, RetT, const KeyT&> &&
            std::convertible_to<std::invoke_result_t<ReducerT&, RetT, const KeyT&>, RetT>
{
   for(const KeyT& key : keys) {
      acc = reducer(std::move(acc), key);
   }
   return acc;
}

/**
* Existence check for values
*/
template <typename T, typename V>
bool value_exists(const std::vector<T>& vec, const V& val) {
   for(const auto& elem : vec) {
      if(elem == val) {
         return true;
      }
   }
   return false;
}

template <typename T, typename Pred>
void map_remove_if(Pred pred, T& assoc) {
   auto i = assoc.begin();
   while(i != assoc.end()) {
      if(pred(i->first)) {
         assoc.erase(i++);
      } else {
         i++;
      }
   }
}

template <typename... Alts, typename... Ts>
constexpr bool holds_any_of(const std::variant<Ts...>& v) noexcept {
   return (std::holds_alternative<Alts>(v) || ...);
}

template <typename GeneralVariantT, typename SpecialT>
constexpr bool is_generalizable_to(const SpecialT& /*unnamed*/) noexcept {
   return std::is_constructible_v<GeneralVariantT, SpecialT>;
}

template <typename GeneralVariantT, typename... SpecialTs>
constexpr bool is_generalizable_to(const std::variant<SpecialTs...>& /*unnamed*/) noexcept {
   return (std::is_constructible_v<GeneralVariantT, SpecialTs> && ...);
}

/**
 * @brief Converts a given variant into another variant-ish whose type states
 *        are a super set of the given variant.
 *
 * This is useful to convert restricted variant types into more general
 * variants types.
 */
template <typename GeneralVariantT, typename SpecialT>
constexpr GeneralVariantT generalize_to(SpecialT&& specific)
   requires(std::is_constructible_v<GeneralVariantT, std::decay_t<SpecialT>>)
{
   return std::forward<SpecialT>(specific);
}

/**
 * @brief Converts a given variant into another variant-ish whose type states
 *        are a super set of the given variant.
 *
 * This is useful to convert restricted variant types into more general
 * variants types.
 */
template <typename GeneralVariantT, typename... SpecialTs>
constexpr GeneralVariantT generalize_to(std::variant<SpecialTs...> specific) {
   static_assert(
      is_generalizable_to<GeneralVariantT>(specific),
      "Desired general type must be implicitly constructible by all types of the specialized std::variant<>");
   return std::visit([](auto s) -> GeneralVariantT { return s; }, std::move(specific));
}

/**
 * @brief Converts a given variant into another variant whose type states
 *        are a subset of the given variant.
 *
 * @returns a variant of type SpecificVariantT if the given variant holds a
 *          type in SpecificVariantT, std::nullopt otherwise.
 */
template <typename SpecificVariantT, typename GeneralVariantT>
constexpr std::optional<SpecificVariantT> specialize_to(GeneralVariantT&& v) {
   return std::visit(
      []<typename AlternativeT>(AlternativeT&& obj) -> std::optional<SpecificVariantT> {
         if constexpr(std::is_constructible_v<SpecificVariantT, AlternativeT>) {
            return std::forward<AlternativeT>(obj);
         } else {
            return std::nullopt;
         }
      },
      std::forward<GeneralVariantT>(v));
}

// This is a helper utility to emulate pattern matching with std::visit.
// See https://en.cppreference.com/w/cpp/utility/variant/visit for more info.
template <class... Ts>
struct overloaded : Ts... {
      using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// TODO: C++23: replace with std::to_underlying
template <typename T>
   requires std::is_enum_v<T>
auto to_underlying(T e) noexcept {
   return static_cast<std::underlying_type_t<T>>(e);
}

// TODO: C++23 - use std::out_ptr
template <typename T>
[[nodiscard]] constexpr auto out_ptr(T& outptr) noexcept {
   class out_ptr_t {
      public:
         constexpr ~out_ptr_t() noexcept {
            m_ptr.reset(m_rawptr);
            m_rawptr = nullptr;
         }

         constexpr explicit out_ptr_t(T& outptr) noexcept : m_ptr(outptr), m_rawptr(nullptr) {}

         out_ptr_t(const out_ptr_t&) = delete;
         out_ptr_t(out_ptr_t&&) = delete;
         out_ptr_t& operator=(const out_ptr_t&) = delete;
         out_ptr_t& operator=(out_ptr_t&&) = delete;

         // NOLINTNEXTLINE(*-explicit-conversions) - Implicit by design for C API interop
         [[nodiscard]] constexpr operator typename T::element_type **() && noexcept { return &m_rawptr; }

      private:
         T& m_ptr;
         typename T::element_type* m_rawptr;
   };

   return out_ptr_t{outptr};
}

}  // namespace Botan


#if defined(BOTAN_HAS_STREAM_CIPHER)
#endif

namespace Botan {

#if defined(BOTAN_HAS_STREAM_CIPHER)

class Stream_Cipher_Mode final : public Cipher_Mode {
   public:
      /**
      * @param cipher underlying stream cipher
      */
      explicit Stream_Cipher_Mode(std::unique_ptr<StreamCipher> cipher) : m_cipher(std::move(cipher)) {}

      size_t output_length(size_t input_length) const override { return input_length; }

      size_t update_granularity() const override { return 1; }

      size_t ideal_granularity() const override {
         const size_t buf_size = m_cipher->buffer_size();
         BOTAN_ASSERT_NOMSG(buf_size > 0);
         if(buf_size >= 256) {
            return buf_size;
         }
         return buf_size * (256 / buf_size);
      }

      size_t minimum_final_size() const override { return 0; }

      size_t default_nonce_length() const override { return 0; }

      bool valid_nonce_length(size_t nonce_len) const override { return m_cipher->valid_iv_length(nonce_len); }

      Key_Length_Specification key_spec() const override { return m_cipher->key_spec(); }

      std::string name() const override { return m_cipher->name(); }

      void clear() override {
         m_cipher->clear();
         reset();
      }

      void reset() override { /* no msg state */
      }

      bool has_keying_material() const override { return m_cipher->has_keying_material(); }

   private:
      void start_msg(const uint8_t nonce[], size_t nonce_len) override {
         if(nonce_len > 0) {
            m_cipher->set_iv(nonce, nonce_len);
         }
      }

      size_t process_msg(uint8_t buf[], size_t sz) override {
         m_cipher->cipher1(buf, sz);
         return sz;
      }

      void finish_msg(secure_vector<uint8_t>& buf, size_t offset) override { return update(buf, offset); }

      void key_schedule(std::span<const uint8_t> key) override { m_cipher->set_key(key); }

      std::unique_ptr<StreamCipher> m_cipher;
};

#endif

}  // namespace Botan


namespace Botan {

/**
* Streebog (GOST R 34.11-2012)
* RFC 6986
*/
class Streebog final : public HashFunction {
   public:
      size_t output_length() const override { return m_output_bits / 8; }

      std::unique_ptr<HashFunction> new_object() const override { return std::make_unique<Streebog>(m_output_bits); }

      void clear() override;
      std::string name() const override;

      size_t hash_block_size() const override { return 64; }

      std::unique_ptr<HashFunction> copy_state() const override;

      explicit Streebog(size_t output_bits);

   protected:
      void add_data(std::span<const uint8_t> input) override;
      void final_result(std::span<uint8_t> out) override;

      void compress(const uint8_t input[], bool lastblock = false);

      void compress_64(const uint64_t input[], bool lastblock = false);

   private:
      const size_t m_output_bits;
      uint64_t m_count;
      AlignmentBuffer<uint8_t, 64> m_buffer;
      secure_vector<uint64_t> m_h;
      secure_vector<uint64_t> m_S;
};

}  // namespace Botan

#if defined(BOTAN_HAS_OS_UTILS)
#endif

namespace Botan {

template <typename F>
uint64_t measure_cost(uint64_t trial_msec, F func) {
#if defined(BOTAN_HAS_OS_UTILS)
   const uint64_t trial_nsec = trial_msec * 1000000;

   uint64_t total_nsec = 0;
   uint64_t trials = 0;

   auto trial_start = OS::get_system_timestamp_ns();

   for(;;) {
      const auto start = OS::get_system_timestamp_ns();
      func();
      const auto end = OS::get_system_timestamp_ns();

      if(end >= start) {
         total_nsec += (end - start);
         trials += 1;

         if((end - trial_start) >= trial_nsec) {
            return (total_nsec / trials);
         }
      }
   }

#else
   BOTAN_UNUSED(trial_msec, func);
   throw Not_Implemented("No system clock available");
#endif
}

}  // namespace Botan

namespace Botan {

/**
* Twofish, an AES finalist
*/
class Twofish final : public Block_Cipher_Fixed_Params<16, 16, 32, 8> {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string provider() const override;

      std::string name() const override { return "Twofish"; }

      std::unique_ptr<BlockCipher> new_object() const override { return std::make_unique<Twofish>(); }

      size_t parallelism() const override;

      bool has_keying_material() const override;

   private:
      void key_schedule(std::span<const uint8_t> key) override;

#if defined(BOTAN_HAS_TWOFISH_AVX512)
      void avx512_encrypt_16(const uint8_t in[16 * 16], uint8_t out[16 * 16]) const;
      void avx512_decrypt_16(const uint8_t in[16 * 16], uint8_t out[16 * 16]) const;
#endif

      secure_vector<uint32_t> m_SB;
      secure_vector<uint32_t> m_RK;

      secure_vector<uint8_t> m_QS;  // Sboxes without MDS applied, only used for AVX-512
};

}  // namespace Botan
/* NOLINTBEGIN(*-macro-usage) */

#define BOTAN_FULL_VERSION_STRING "Botan 3.12.0 (release, dated 20260506, revision git:45d6f286c320b2f2efd5373d195ec88c367e3071)"

#define BOTAN_SHORT_VERSION_STRING "3.12.0"

#define BOTAN_VC_REVISION "git:45d6f286c320b2f2efd5373d195ec88c367e3071"


/* NOLINTEND(*-macro-usage) */

namespace Botan {

/**
* Whirlpool
*/
class Whirlpool final : public HashFunction {
   public:
      using digest_type = secure_vector<uint64_t>;

      static constexpr MD_Endian byte_endianness = MD_Endian::Big;
      static constexpr MD_Endian bit_endianness = MD_Endian::Big;
      static constexpr size_t block_bytes = 64;
      static constexpr size_t output_bytes = 64;
      static constexpr size_t ctr_bytes = 32;

      static void compress_n(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
      static void init(digest_type& digest);

#if defined(BOTAN_HAS_WHIRLPOOL_AVX512)
      static void compress_n_avx512(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
#endif

#if defined(BOTAN_HAS_WHIRLPOOL_AVX2)
      static void compress_n_avx2(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
#endif

   public:
      std::string name() const override { return "Whirlpool"; }

      size_t output_length() const override { return output_bytes; }

      size_t hash_block_size() const override { return block_bytes; }

      std::unique_ptr<HashFunction> new_object() const override;

      std::unique_ptr<HashFunction> copy_state() const override;

      std::string provider() const override;

      void clear() override { m_md.clear(); }

   private:
      void add_data(std::span<const uint8_t> input) override;

      void final_result(std::span<uint8_t> output) override;

   private:
      MerkleDamgard_Hash<Whirlpool> m_md;
};

}  // namespace Botan

namespace Botan {

/**
* IEEE P1619 XTS Mode
*/
class XTS_Mode : public Cipher_Mode {
   public:
      std::string name() const final;

      size_t update_granularity() const final;

      size_t ideal_granularity() const final;

      size_t minimum_final_size() const final;

      Key_Length_Specification key_spec() const final;

      size_t default_nonce_length() const final;

      bool valid_nonce_length(size_t n) const final;

      void clear() final;

      void reset() final;

      bool has_keying_material() const final;

   protected:
      explicit XTS_Mode(std::unique_ptr<BlockCipher> cipher);

      const uint8_t* tweak() const { return m_tweak.data(); }

      bool tweak_set() const { return !m_tweak.empty(); }

      size_t tweak_blocks() const { return m_tweak_blocks; }

      const BlockCipher& cipher() const { return *m_cipher; }

      void update_tweak(size_t consumed);

      size_t cipher_block_size() const { return m_cipher_block_size; }

   private:
      void start_msg(const uint8_t nonce[], size_t nonce_len) override;
      void key_schedule(std::span<const uint8_t> key) override;

      /*
      * Tweak block update step for XTS
      *
      * Assumes tweak is BS * n bytes long.
      *
      * Assumes that each block of tweak is already set to the successive doublings
      * of the block prior.
      */
      static void update_tweak_block(uint8_t tweak[], size_t BS, size_t blocks_in_tweak);

#if defined(BOTAN_HAS_MODE_XTS_AVX512_CLMUL)
      static void update_tweak_block_avx512_clmul(uint8_t tweak[], size_t BS, size_t blocks_in_tweak);
#endif

      std::unique_ptr<BlockCipher> m_cipher;
      std::unique_ptr<BlockCipher> m_tweak_cipher;
      secure_vector<uint8_t> m_tweak;
      const size_t m_cipher_block_size;
      const size_t m_cipher_parallelism;
      const size_t m_tweak_blocks;
};

/**
* IEEE P1619 XTS Encryption
*/
class XTS_Encryption final : public XTS_Mode {
   public:
      /**
      * @param cipher underlying block cipher
      */
      explicit XTS_Encryption(std::unique_ptr<BlockCipher> cipher) : XTS_Mode(std::move(cipher)) {}

      size_t output_length(size_t input_length) const override;

   private:
      size_t process_msg(uint8_t buf[], size_t size) override;
      void finish_msg(secure_vector<uint8_t>& final_block, size_t offset = 0) override;
};

/**
* IEEE P1619 XTS Decryption
*/
class XTS_Decryption final : public XTS_Mode {
   public:
      /**
      * @param cipher underlying block cipher
      */
      explicit XTS_Decryption(std::unique_ptr<BlockCipher> cipher) : XTS_Mode(std::move(cipher)) {}

      size_t output_length(size_t input_length) const override;

   private:
      size_t process_msg(uint8_t buf[], size_t size) override;
      void finish_msg(secure_vector<uint8_t>& final_block, size_t offset = 0) override;
};

}  // namespace Botan
/*
* (C) 1999-2010,2015,2017,2018,2020 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_CPUID)
#endif

#if defined(BOTAN_HAS_AES_POWER8) || defined(BOTAN_HAS_AES_ARMV8) || defined(BOTAN_HAS_AES_NI)
   #define BOTAN_HAS_HW_AES_SUPPORT
#endif

#if defined(BOTAN_HAS_HW_AES_SUPPORT)
   #include <bit>
#endif

namespace Botan {

/*
* One of three AES implementation strategies are used to get a constant time
* implementation which is immune to common cache/timing based side channels:
*
* - If AES hardware support is available (AES-NI, POWER8, Aarch64) use that
*
* - If 128-bit SIMD with byte shuffles are available (SSSE3, NEON, or Altivec),
*   use the vperm technique published by Mike Hamburg at CHES 2009.
*
* - If no hardware or SIMD support, fall back to a constant time bitsliced
*   implementation. This uses 32-bit words resulting in 2 blocks being processed
*   in parallel. Moving to 4 blocks (with 64-bit words) would approximately
*   double performance on 64-bit CPUs. Likewise moving to 128 bit SIMD would
*   again approximately double performance vs 64-bit. However the assumption is
*   that most 64-bit CPUs either have hardware AES or SIMD shuffle support and
*   that the majority of users falling back to this code will be 32-bit cores.
*   If this assumption proves to be unsound, the bitsliced code can easily be
*   extended to operate on either 32 or 64 bit words depending on the native
*   wordsize of the target processor.
*
* Useful references
*
* - "Accelerating AES with Vector Permute Instructions" Mike Hamburg
*   https://www.shiftleft.org/papers/vector_aes/vector_aes.pdf
*
* - "Faster and Timing-Attack Resistant AES-GCM" Käsper and Schwabe
*   https://eprint.iacr.org/2009/129.pdf
*
* - "A new combinational logic minimization technique with applications to cryptology."
*   Boyar and Peralta https://eprint.iacr.org/2009/191.pdf
*
* - "A depth-16 circuit for the AES S-box" Boyar and Peralta
*    https://eprint.iacr.org/2011/332.pdf
*
* - "A Very Compact S-box for AES" Canright
*   https://www.iacr.org/archive/ches2005/032.pdf
*   https://core.ac.uk/download/pdf/36694529.pdf (extended)
*/

namespace {

/*
This is an AES sbox circuit which can execute in bitsliced mode up to 32x in
parallel.

The circuit is from the "Circuit Minimization Team" group
http://www.cs.yale.edu/homes/peralta/CircuitStuff/CMT.html
http://www.cs.yale.edu/homes/peralta/CircuitStuff/SLP_AES_113.txt

This circuit has size 113 and depth 27. In software it is much faster than
circuits which are considered faster for hardware purposes (where circuit depth
is the critical constraint), because unlike in hardware, on common CPUs we can
only execute - at best - 3 or 4 logic operations per cycle. So a smaller circuit
is superior. On an x86-64 machine this circuit is about 15% faster than the
circuit of size 128 and depth 16 given in "A depth-16 circuit for the AES S-box".

Another circuit for AES Sbox of size 102 and depth 24 is describted in "New
Circuit Minimization Techniques for Smaller and Faster AES SBoxes"
[https://eprint.iacr.org/2019/802] however it relies on "non-standard" gates
like MUX, NOR, NAND, etc and so in practice in bitsliced software, its size is
actually a bit larger than this circuit, as few CPUs have such instructions and
otherwise they must be emulated using a sequence of available bit operations.
*/
void AES_SBOX(uint32_t V[8]) {
   const uint32_t U0 = V[0];
   const uint32_t U1 = V[1];
   const uint32_t U2 = V[2];
   const uint32_t U3 = V[3];
   const uint32_t U4 = V[4];
   const uint32_t U5 = V[5];
   const uint32_t U6 = V[6];
   const uint32_t U7 = V[7];

   const uint32_t y14 = U3 ^ U5;
   const uint32_t y13 = U0 ^ U6;
   const uint32_t y9 = U0 ^ U3;
   const uint32_t y8 = U0 ^ U5;
   const uint32_t t0 = U1 ^ U2;
   const uint32_t y1 = t0 ^ U7;
   const uint32_t y4 = y1 ^ U3;
   const uint32_t y12 = y13 ^ y14;
   const uint32_t y2 = y1 ^ U0;
   const uint32_t y5 = y1 ^ U6;
   const uint32_t y3 = y5 ^ y8;
   const uint32_t t1 = U4 ^ y12;
   const uint32_t y15 = t1 ^ U5;
   const uint32_t y20 = t1 ^ U1;
   const uint32_t y6 = y15 ^ U7;
   const uint32_t y10 = y15 ^ t0;
   const uint32_t y11 = y20 ^ y9;
   const uint32_t y7 = U7 ^ y11;
   const uint32_t y17 = y10 ^ y11;
   const uint32_t y19 = y10 ^ y8;
   const uint32_t y16 = t0 ^ y11;
   const uint32_t y21 = y13 ^ y16;
   const uint32_t y18 = U0 ^ y16;
   const uint32_t t2 = y12 & y15;
   const uint32_t t3 = y3 & y6;
   const uint32_t t4 = t3 ^ t2;
   const uint32_t t5 = y4 & U7;
   const uint32_t t6 = t5 ^ t2;
   const uint32_t t7 = y13 & y16;
   const uint32_t t8 = y5 & y1;
   const uint32_t t9 = t8 ^ t7;
   const uint32_t t10 = y2 & y7;
   const uint32_t t11 = t10 ^ t7;
   const uint32_t t12 = y9 & y11;
   const uint32_t t13 = y14 & y17;
   const uint32_t t14 = t13 ^ t12;
   const uint32_t t15 = y8 & y10;
   const uint32_t t16 = t15 ^ t12;
   const uint32_t t17 = t4 ^ y20;
   const uint32_t t18 = t6 ^ t16;
   const uint32_t t19 = t9 ^ t14;
   const uint32_t t20 = t11 ^ t16;
   const uint32_t t21 = t17 ^ t14;
   const uint32_t t22 = t18 ^ y19;
   const uint32_t t23 = t19 ^ y21;
   const uint32_t t24 = t20 ^ y18;
   const uint32_t t25 = t21 ^ t22;
   const uint32_t t26 = t21 & t23;
   const uint32_t t27 = t24 ^ t26;
   const uint32_t t28 = t25 & t27;
   const uint32_t t29 = t28 ^ t22;
   const uint32_t t30 = t23 ^ t24;
   const uint32_t t31 = t22 ^ t26;
   const uint32_t t32 = t31 & t30;
   const uint32_t t33 = t32 ^ t24;
   const uint32_t t34 = t23 ^ t33;
   const uint32_t t35 = t27 ^ t33;
   const uint32_t t36 = t24 & t35;
   const uint32_t t37 = t36 ^ t34;
   const uint32_t t38 = t27 ^ t36;
   const uint32_t t39 = t29 & t38;
   const uint32_t t40 = t25 ^ t39;
   const uint32_t t41 = t40 ^ t37;
   const uint32_t t42 = t29 ^ t33;
   const uint32_t t43 = t29 ^ t40;
   const uint32_t t44 = t33 ^ t37;
   const uint32_t t45 = t42 ^ t41;
   const uint32_t z0 = t44 & y15;
   const uint32_t z1 = t37 & y6;
   const uint32_t z2 = t33 & U7;
   const uint32_t z3 = t43 & y16;
   const uint32_t z4 = t40 & y1;
   const uint32_t z5 = t29 & y7;
   const uint32_t z6 = t42 & y11;
   const uint32_t z7 = t45 & y17;
   const uint32_t z8 = t41 & y10;
   const uint32_t z9 = t44 & y12;
   const uint32_t z10 = t37 & y3;
   const uint32_t z11 = t33 & y4;
   const uint32_t z12 = t43 & y13;
   const uint32_t z13 = t40 & y5;
   const uint32_t z14 = t29 & y2;
   const uint32_t z15 = t42 & y9;
   const uint32_t z16 = t45 & y14;
   const uint32_t z17 = t41 & y8;
   const uint32_t tc1 = z15 ^ z16;
   const uint32_t tc2 = z10 ^ tc1;
   const uint32_t tc3 = z9 ^ tc2;
   const uint32_t tc4 = z0 ^ z2;
   const uint32_t tc5 = z1 ^ z0;
   const uint32_t tc6 = z3 ^ z4;
   const uint32_t tc7 = z12 ^ tc4;
   const uint32_t tc8 = z7 ^ tc6;
   const uint32_t tc9 = z8 ^ tc7;
   const uint32_t tc10 = tc8 ^ tc9;
   const uint32_t tc11 = tc6 ^ tc5;
   const uint32_t tc12 = z3 ^ z5;
   const uint32_t tc13 = z13 ^ tc1;
   const uint32_t tc14 = tc4 ^ tc12;
   const uint32_t S3 = tc3 ^ tc11;
   const uint32_t tc16 = z6 ^ tc8;
   const uint32_t tc17 = z14 ^ tc10;
   const uint32_t tc18 = ~tc13 ^ tc14;
   const uint32_t S7 = z12 ^ tc18;
   const uint32_t tc20 = z15 ^ tc16;
   const uint32_t tc21 = tc2 ^ z11;
   const uint32_t S0 = tc3 ^ tc16;
   const uint32_t S6 = tc10 ^ tc18;
   const uint32_t S4 = tc14 ^ S3;
   const uint32_t S1 = ~(S3 ^ tc16);
   const uint32_t tc26 = tc17 ^ tc20;
   const uint32_t S2 = ~(tc26 ^ z17);
   const uint32_t S5 = tc21 ^ tc17;

   V[0] = S0;
   V[1] = S1;
   V[2] = S2;
   V[3] = S3;
   V[4] = S4;
   V[5] = S5;
   V[6] = S6;
   V[7] = S7;
}

/*
A circuit for inverse AES Sbox of size 121 and depth 21 from
http://www.cs.yale.edu/homes/peralta/CircuitStuff/CMT.html
http://www.cs.yale.edu/homes/peralta/CircuitStuff/Sinv.txt
*/
void AES_INV_SBOX(uint32_t V[8]) {
   const uint32_t U0 = V[0];
   const uint32_t U1 = V[1];
   const uint32_t U2 = V[2];
   const uint32_t U3 = V[3];
   const uint32_t U4 = V[4];
   const uint32_t U5 = V[5];
   const uint32_t U6 = V[6];
   const uint32_t U7 = V[7];

   const uint32_t Y0 = U0 ^ U3;
   const uint32_t Y2 = ~(U1 ^ U3);
   const uint32_t Y4 = U0 ^ Y2;
   const uint32_t RTL0 = U6 ^ U7;
   const uint32_t Y1 = Y2 ^ RTL0;
   const uint32_t Y7 = ~(U2 ^ Y1);
   const uint32_t RTL1 = U3 ^ U4;
   const uint32_t Y6 = ~(U7 ^ RTL1);
   const uint32_t Y3 = Y1 ^ RTL1;
   const uint32_t RTL2 = ~(U0 ^ U2);
   const uint32_t Y5 = U5 ^ RTL2;
   const uint32_t sa1 = Y0 ^ Y2;
   const uint32_t sa0 = Y1 ^ Y3;
   const uint32_t sb1 = Y4 ^ Y6;
   const uint32_t sb0 = Y5 ^ Y7;
   const uint32_t ah = Y0 ^ Y1;
   const uint32_t al = Y2 ^ Y3;
   const uint32_t aa = sa0 ^ sa1;
   const uint32_t bh = Y4 ^ Y5;
   const uint32_t bl = Y6 ^ Y7;
   const uint32_t bb = sb0 ^ sb1;
   const uint32_t ab20 = sa0 ^ sb0;
   const uint32_t ab22 = al ^ bl;
   const uint32_t ab23 = Y3 ^ Y7;
   const uint32_t ab21 = sa1 ^ sb1;
   const uint32_t abcd1 = ah & bh;
   const uint32_t rr1 = Y0 & Y4;
   const uint32_t ph11 = ab20 ^ abcd1;
   const uint32_t t01 = Y1 & Y5;
   const uint32_t ph01 = t01 ^ abcd1;
   const uint32_t abcd2 = al & bl;
   const uint32_t r1 = Y2 & Y6;
   const uint32_t pl11 = ab22 ^ abcd2;
   const uint32_t r2 = Y3 & Y7;
   const uint32_t pl01 = r2 ^ abcd2;
   const uint32_t r3 = sa0 & sb0;
   const uint32_t vr1 = aa & bb;
   const uint32_t pr1 = vr1 ^ r3;
   const uint32_t wr1 = sa1 & sb1;
   const uint32_t qr1 = wr1 ^ r3;
   const uint32_t ab0 = ph11 ^ rr1;
   const uint32_t ab1 = ph01 ^ ab21;
   const uint32_t ab2 = pl11 ^ r1;
   const uint32_t ab3 = pl01 ^ qr1;
   const uint32_t cp1 = ab0 ^ pr1;
   const uint32_t cp2 = ab1 ^ qr1;
   const uint32_t cp3 = ab2 ^ pr1;
   const uint32_t cp4 = ab3 ^ ab23;
   const uint32_t tinv1 = cp3 ^ cp4;
   const uint32_t tinv2 = cp3 & cp1;
   const uint32_t tinv3 = cp2 ^ tinv2;
   const uint32_t tinv4 = cp1 ^ cp2;
   const uint32_t tinv5 = cp4 ^ tinv2;
   const uint32_t tinv6 = tinv5 & tinv4;
   const uint32_t tinv7 = tinv3 & tinv1;
   const uint32_t d2 = cp4 ^ tinv7;
   const uint32_t d0 = cp2 ^ tinv6;
   const uint32_t tinv8 = cp1 & cp4;
   const uint32_t tinv9 = tinv4 & tinv8;
   const uint32_t tinv10 = tinv4 ^ tinv2;
   const uint32_t d1 = tinv9 ^ tinv10;
   const uint32_t tinv11 = cp2 & cp3;
   const uint32_t tinv12 = tinv1 & tinv11;
   const uint32_t tinv13 = tinv1 ^ tinv2;
   const uint32_t d3 = tinv12 ^ tinv13;
   const uint32_t sd1 = d1 ^ d3;
   const uint32_t sd0 = d0 ^ d2;
   const uint32_t dl = d0 ^ d1;  // NOLINT(misc-confusable-identifiers)
   const uint32_t dh = d2 ^ d3;
   const uint32_t dd = sd0 ^ sd1;
   const uint32_t abcd3 = dh & bh;
   const uint32_t rr2 = d3 & Y4;
   const uint32_t t02 = d2 & Y5;
   const uint32_t abcd4 = dl & bl;
   const uint32_t r4 = d1 & Y6;
   const uint32_t r5 = d0 & Y7;
   const uint32_t r6 = sd0 & sb0;
   const uint32_t vr2 = dd & bb;
   const uint32_t wr2 = sd1 & sb1;
   const uint32_t abcd5 = dh & ah;
   const uint32_t r7 = d3 & Y0;
   const uint32_t r8 = d2 & Y1;
   const uint32_t abcd6 = dl & al;
   const uint32_t r9 = d1 & Y2;
   const uint32_t r10 = d0 & Y3;
   const uint32_t r11 = sd0 & sa0;
   const uint32_t vr3 = dd & aa;
   const uint32_t wr3 = sd1 & sa1;
   const uint32_t ph12 = rr2 ^ abcd3;
   const uint32_t ph02 = t02 ^ abcd3;
   const uint32_t pl12 = r4 ^ abcd4;
   const uint32_t pl02 = r5 ^ abcd4;
   const uint32_t pr2 = vr2 ^ r6;
   const uint32_t qr2 = wr2 ^ r6;
   const uint32_t p0 = ph12 ^ pr2;
   const uint32_t p1 = ph02 ^ qr2;
   const uint32_t p2 = pl12 ^ pr2;
   const uint32_t p3 = pl02 ^ qr2;
   const uint32_t ph13 = r7 ^ abcd5;
   const uint32_t ph03 = r8 ^ abcd5;
   const uint32_t pl13 = r9 ^ abcd6;
   const uint32_t pl03 = r10 ^ abcd6;
   const uint32_t pr3 = vr3 ^ r11;
   const uint32_t qr3 = wr3 ^ r11;
   const uint32_t p4 = ph13 ^ pr3;
   const uint32_t S7 = ph03 ^ qr3;
   const uint32_t p6 = pl13 ^ pr3;
   const uint32_t p7 = pl03 ^ qr3;
   const uint32_t S3 = p1 ^ p6;
   const uint32_t S6 = p2 ^ p6;
   const uint32_t S0 = p3 ^ p6;
   const uint32_t X11 = p0 ^ p2;
   const uint32_t S5 = S0 ^ X11;
   const uint32_t X13 = p4 ^ p7;
   const uint32_t X14 = X11 ^ X13;
   const uint32_t S1 = S3 ^ X14;
   const uint32_t X16 = p1 ^ S7;
   const uint32_t S2 = X14 ^ X16;
   const uint32_t X18 = p0 ^ p4;
   const uint32_t X19 = S5 ^ X16;
   const uint32_t S4 = X18 ^ X19;

   V[0] = S0;
   V[1] = S1;
   V[2] = S2;
   V[3] = S3;
   V[4] = S4;
   V[5] = S5;
   V[6] = S6;
   V[7] = S7;
}

inline void bit_transpose(uint32_t B[8]) {
   swap_bits<uint32_t>(B[1], B[0], 0x55555555, 1);
   swap_bits<uint32_t>(B[3], B[2], 0x55555555, 1);
   swap_bits<uint32_t>(B[5], B[4], 0x55555555, 1);
   swap_bits<uint32_t>(B[7], B[6], 0x55555555, 1);

   swap_bits<uint32_t>(B[2], B[0], 0x33333333, 2);
   swap_bits<uint32_t>(B[3], B[1], 0x33333333, 2);
   swap_bits<uint32_t>(B[6], B[4], 0x33333333, 2);
   swap_bits<uint32_t>(B[7], B[5], 0x33333333, 2);

   swap_bits<uint32_t>(B[4], B[0], 0x0F0F0F0F, 4);
   swap_bits<uint32_t>(B[5], B[1], 0x0F0F0F0F, 4);
   swap_bits<uint32_t>(B[6], B[2], 0x0F0F0F0F, 4);
   swap_bits<uint32_t>(B[7], B[3], 0x0F0F0F0F, 4);
}

inline void ks_expand(uint32_t B[8], const uint32_t K[], size_t r) {
   /*
   This is bit_transpose of K[r..r+4] || K[r..r+4], we can save some computation
   due to knowing the first and second halves are the same data.
   */
   for(size_t i = 0; i != 4; ++i) {
      B[i] = K[r + i];
   }

   swap_bits<uint32_t>(B[1], B[0], 0x55555555, 1);
   swap_bits<uint32_t>(B[3], B[2], 0x55555555, 1);

   swap_bits<uint32_t>(B[2], B[0], 0x33333333, 2);
   swap_bits<uint32_t>(B[3], B[1], 0x33333333, 2);

   B[4] = B[0];
   B[5] = B[1];
   B[6] = B[2];
   B[7] = B[3];

   swap_bits<uint32_t>(B[4], B[0], 0x0F0F0F0F, 4);
   swap_bits<uint32_t>(B[5], B[1], 0x0F0F0F0F, 4);
   swap_bits<uint32_t>(B[6], B[2], 0x0F0F0F0F, 4);
   swap_bits<uint32_t>(B[7], B[3], 0x0F0F0F0F, 4);
}

inline void shift_rows(uint32_t B[8]) {
   // 3 0 1 2 7 4 5 6 10 11 8 9 14 15 12 13 17 18 19 16 21 22 23 20 24 25 26 27 28 29 30 31
   if constexpr(HasNative64BitRegisters) {
      for(size_t i = 0; i != 8; i += 2) {
         uint64_t x = (static_cast<uint64_t>(B[i]) << 32) | B[i + 1];
         x = bit_permute_step<uint64_t>(x, 0x0022331100223311, 2);
         x = bit_permute_step<uint64_t>(x, 0x0055005500550055, 1);
         B[i] = static_cast<uint32_t>(x >> 32);
         B[i + 1] = static_cast<uint32_t>(x);
      }
   } else {
      for(size_t i = 0; i != 8; ++i) {
         uint32_t x = B[i];
         x = bit_permute_step<uint32_t>(x, 0x00223311, 2);
         x = bit_permute_step<uint32_t>(x, 0x00550055, 1);
         B[i] = x;
      }
   }
}

inline void inv_shift_rows(uint32_t B[8]) {
   // Inverse of shift_rows, just inverting the steps

   if constexpr(HasNative64BitRegisters) {
      for(size_t i = 0; i != 8; i += 2) {
         uint64_t x = (static_cast<uint64_t>(B[i]) << 32) | B[i + 1];
         x = bit_permute_step<uint64_t>(x, 0x0055005500550055, 1);
         x = bit_permute_step<uint64_t>(x, 0x0022331100223311, 2);
         B[i] = static_cast<uint32_t>(x >> 32);
         B[i + 1] = static_cast<uint32_t>(x);
      }
   } else {
      for(size_t i = 0; i != 8; ++i) {
         uint32_t x = B[i];
         x = bit_permute_step<uint32_t>(x, 0x00550055, 1);
         x = bit_permute_step<uint32_t>(x, 0x00223311, 2);
         B[i] = x;
      }
   }
}

inline void mix_columns(uint32_t B[8]) {
   // carry high bits in B[0] to positions in 0x1b == 0b11011
   const uint32_t X2[8] = {
      B[1],
      B[2],
      B[3],
      B[4] ^ B[0],
      B[5] ^ B[0],
      B[6],
      B[7] ^ B[0],
      B[0],
   };

   for(size_t i = 0; i != 8; i++) {
      const uint32_t X3 = B[i] ^ X2[i];
      B[i] = X2[i] ^ rotr<8>(B[i]) ^ rotr<16>(B[i]) ^ rotr<24>(X3);
   }
}

void inv_mix_columns(uint32_t B[8]) {
   /*
   OpenSSL's bsaes implementation credits Jussi Kivilinna with the lovely
   matrix decomposition

   | 0e 0b 0d 09 |   | 02 03 01 01 |   | 05 00 04 00 |
   | 09 0e 0b 0d | = | 01 02 03 01 | x | 00 05 00 04 |
   | 0d 09 0e 0b |   | 01 01 02 03 |   | 04 00 05 00 |
   | 0b 0d 09 0e |   | 03 01 01 02 |   | 00 04 00 05 |

   Notice the first component is simply the MixColumns matrix. So we can
   multiply first by (05,00,04,00) then perform MixColumns to get the equivalent
   of InvMixColumn.
   */
   const uint32_t X4[8] = {
      B[2],
      B[3],
      B[4] ^ B[0],
      B[5] ^ B[0] ^ B[1],
      B[6] ^ B[1],
      B[7] ^ B[0],
      B[0] ^ B[1],
      B[1],
   };

   for(size_t i = 0; i != 8; i++) {
      const uint32_t X5 = X4[i] ^ B[i];
      B[i] = X5 ^ rotr<16>(X4[i]);
   }

   mix_columns(B);
}

/*
* AES Encryption
*/
void aes_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks, const secure_vector<uint32_t>& EK) {
   BOTAN_ASSERT(EK.size() == 44 || EK.size() == 52 || EK.size() == 60, "Key was set");

   const size_t rounds = (EK.size() - 4) / 4;

   uint32_t KS[13 * 8] = {0};  // actual maximum is (rounds - 1) * 8
   for(size_t i = 0; i < rounds - 1; i += 1) {
      ks_expand(&KS[8 * i], EK.data(), 4 * i + 4);
   }

   const size_t BLOCK_SIZE = 16;
   const size_t BITSLICED_BLOCKS = 8 * sizeof(uint32_t) / BLOCK_SIZE;

   while(blocks > 0) {
      const size_t this_loop = std::min(blocks, BITSLICED_BLOCKS);

      uint32_t B[8] = {0};

      load_be(B, in, this_loop * 4);

      CT::poison(B, 8);

      for(size_t i = 0; i != 8; ++i) {
         B[i] ^= EK[i % 4];
      }

      bit_transpose(B);

      for(size_t r = 0; r != rounds - 1; ++r) {
         AES_SBOX(B);
         shift_rows(B);
         mix_columns(B);

         for(size_t i = 0; i != 8; ++i) {
            B[i] ^= KS[8 * r + i];
         }
      }

      // Final round:
      AES_SBOX(B);
      shift_rows(B);
      bit_transpose(B);

      for(size_t i = 0; i != 8; ++i) {
         B[i] ^= EK[4 * rounds + i % 4];
      }

      CT::unpoison(B, 8);

      copy_out_be(std::span(out, this_loop * 4 * sizeof(uint32_t)), B);

      in += this_loop * BLOCK_SIZE;
      out += this_loop * BLOCK_SIZE;
      blocks -= this_loop;
   }
}

/*
* AES Decryption
*/
void aes_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks, const secure_vector<uint32_t>& DK) {
   BOTAN_ASSERT(DK.size() == 44 || DK.size() == 52 || DK.size() == 60, "Key was set");

   const size_t rounds = (DK.size() - 4) / 4;

   uint32_t KS[13 * 8] = {0};  // actual maximum is (rounds - 1) * 8
   for(size_t i = 0; i < rounds - 1; i += 1) {
      ks_expand(&KS[8 * i], DK.data(), 4 * i + 4);
   }

   const size_t BLOCK_SIZE = 16;
   const size_t BITSLICED_BLOCKS = 8 * sizeof(uint32_t) / BLOCK_SIZE;

   while(blocks > 0) {
      const size_t this_loop = std::min(blocks, BITSLICED_BLOCKS);

      uint32_t B[8] = {0};

      load_be(B, in, this_loop * 4);

      CT::poison(B, 8);

      for(size_t i = 0; i != 8; ++i) {
         B[i] ^= DK[i % 4];
      }

      bit_transpose(B);

      for(size_t r = 0; r != rounds - 1; ++r) {
         AES_INV_SBOX(B);
         inv_shift_rows(B);
         inv_mix_columns(B);

         for(size_t i = 0; i != 8; ++i) {
            B[i] ^= KS[8 * r + i];
         }
      }

      // Final round:
      AES_INV_SBOX(B);
      inv_shift_rows(B);
      bit_transpose(B);

      for(size_t i = 0; i != 8; ++i) {
         B[i] ^= DK[4 * rounds + i % 4];
      }

      CT::unpoison(B, 8);

      copy_out_be(std::span(out, this_loop * 4 * sizeof(uint32_t)), B);

      in += this_loop * BLOCK_SIZE;
      out += this_loop * BLOCK_SIZE;
      blocks -= this_loop;
   }
}

inline uint32_t xtime32(uint32_t s) {
   const uint32_t lo_bit = 0x01010101;
   const uint32_t mask = 0x7F7F7F7F;
   const uint32_t poly = 0x1B;

   return ((s & mask) << 1) ^ (((s >> 7) & lo_bit) * poly);
}

inline uint32_t InvMixColumn(uint32_t s1) {
   const uint32_t s2 = xtime32(s1);
   const uint32_t s4 = xtime32(s2);
   const uint32_t s8 = xtime32(s4);
   const uint32_t s9 = s8 ^ s1;
   const uint32_t s11 = s9 ^ s2;
   const uint32_t s13 = s9 ^ s4;
   const uint32_t s14 = s8 ^ s4 ^ s2;

   return s14 ^ rotr<8>(s9) ^ rotr<16>(s13) ^ rotr<24>(s11);
}

void InvMixColumn_x4(uint32_t x[4]) {
   x[0] = InvMixColumn(x[0]);
   x[1] = InvMixColumn(x[1]);
   x[2] = InvMixColumn(x[2]);
   x[3] = InvMixColumn(x[3]);
}

uint32_t SE_word(uint32_t x) {
   uint32_t I[8] = {0};

   for(size_t i = 0; i != 8; ++i) {
      I[i] = (x >> (7 - i)) & 0x01010101;
   }

   AES_SBOX(I);

   x = 0;

   for(size_t i = 0; i != 8; ++i) {
      x |= ((I[i] & 0x01010101) << (7 - i));
   }

   return x;
}

void aes_key_schedule(const uint8_t key[],
                      size_t length,
                      secure_vector<uint32_t>& EK,
                      secure_vector<uint32_t>& DK,
                      bool bswap_keys = false) {
   static const uint32_t RC[10] = {0x01000000,
                                   0x02000000,
                                   0x04000000,
                                   0x08000000,
                                   0x10000000,
                                   0x20000000,
                                   0x40000000,
                                   0x80000000,
                                   0x1B000000,
                                   0x36000000};

   const size_t X = length / 4;

   // Can't happen, but make static analyzers happy
   BOTAN_ASSERT_NOMSG(X == 4 || X == 6 || X == 8);

   const size_t rounds = (length / 4) + 6;

   // Help the optimizer
   BOTAN_ASSERT_NOMSG(rounds == 10 || rounds == 12 || rounds == 14);

   CT::poison(key, length);

   const size_t KS_len = length + 28;
   EK.resize(KS_len);
   DK.resize(KS_len);

   for(size_t i = 0; i != X; ++i) {
      EK[i] = load_be<uint32_t>(key, i);
   }

   for(size_t i = X; i < 4 * (rounds + 1); i += X) {
      EK[i] = EK[i - X] ^ RC[(i - X) / X] ^ rotl<8>(SE_word(EK[i - 1]));

      for(size_t j = 1; j != X && (i + j) < EK.size(); ++j) {
         EK[i + j] = EK[i + j - X];

         if(X == 8 && j == 4) {
            EK[i + j] ^= SE_word(EK[i + j - 1]);
         } else {
            EK[i + j] ^= EK[i + j - 1];
         }
      }
   }

   for(size_t i = 0; i != 4 * (rounds + 1); i += 4) {
      DK[i] = EK[4 * rounds - i];
      DK[i + 1] = EK[4 * rounds - i + 1];
      DK[i + 2] = EK[4 * rounds - i + 2];
      DK[i + 3] = EK[4 * rounds - i + 3];
   }

   for(size_t i = 4; i != 4 * rounds; i += 4) {
      InvMixColumn_x4(&DK[i]);
   }

   if(bswap_keys) {
      // HW AES on little endian needs the subkeys to be byte reversed
      for(size_t i = 0; i != KS_len; ++i) {
         EK[i] = reverse_bytes(EK[i]);
         DK[i] = reverse_bytes(DK[i]);
      }
   }

   CT::unpoison(EK.data(), EK.size());
   CT::unpoison(DK.data(), DK.size());
   CT::unpoison(key, length);
}

size_t aes_parallelism() {
#if defined(BOTAN_HAS_AES_VAES)
   if(CPUID::has(CPUID::Feature::AVX2_AES)) {
      return 8;  // pipelined
   }
#endif

#if defined(BOTAN_HAS_HW_AES_SUPPORT)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      return 4;  // pipelined
   }
#endif

#if defined(BOTAN_HAS_AES_VPERM)
   if(CPUID::has(CPUID::Feature::SIMD_4X32)) {
      return 2;  // pipelined
   }
#endif

   // bitsliced:
   return 2;
}

std::string aes_provider() {
#if defined(BOTAN_HAS_AES_VAES)
   if(auto feat = CPUID::check(CPUID::Feature::AVX2_AES)) {
      return *feat;
   }
#endif

#if defined(BOTAN_HAS_HW_AES_SUPPORT)
   if(auto feat = CPUID::check(CPUID::Feature::HW_AES)) {
      return *feat;
   }
#endif

#if defined(BOTAN_HAS_AES_VPERM)
   if(auto feat = CPUID::check(CPUID::Feature::SIMD_4X32)) {
      return *feat;
   }
#endif

   return "base";
}

}  // namespace

std::string AES_128::provider() const {
   return aes_provider();
}

std::string AES_192::provider() const {
   return aes_provider();
}

std::string AES_256::provider() const {
   return aes_provider();
}

size_t AES_128::parallelism() const {
   return aes_parallelism();
}

size_t AES_192::parallelism() const {
   return aes_parallelism();
}

size_t AES_256::parallelism() const {
   return aes_parallelism();
}

bool AES_128::has_keying_material() const {
   return !m_EK.empty();
}

bool AES_192::has_keying_material() const {
   return !m_EK.empty();
}

bool AES_256::has_keying_material() const {
   return !m_EK.empty();
}

void AES_128::encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();

#if defined(BOTAN_HAS_AES_VAES)
   if(CPUID::has(CPUID::Feature::AVX2_AES)) {
      return x86_vaes_encrypt_n(in, out, blocks);
   }
#endif

#if defined(BOTAN_HAS_HW_AES_SUPPORT)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      return hw_aes_encrypt_n(in, out, blocks);
   }
#endif

#if defined(BOTAN_HAS_AES_VPERM)
   if(CPUID::has(CPUID::Feature::SIMD_4X32)) {
      return vperm_encrypt_n(in, out, blocks);
   }
#endif

   aes_encrypt_n(in, out, blocks, m_EK);
}

void AES_128::decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();

#if defined(BOTAN_HAS_AES_VAES)
   if(CPUID::has(CPUID::Feature::AVX2_AES)) {
      return x86_vaes_decrypt_n(in, out, blocks);
   }
#endif

#if defined(BOTAN_HAS_HW_AES_SUPPORT)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      return hw_aes_decrypt_n(in, out, blocks);
   }
#endif

#if defined(BOTAN_HAS_AES_VPERM)
   if(CPUID::has(CPUID::Feature::SIMD_4X32)) {
      return vperm_decrypt_n(in, out, blocks);
   }
#endif

   aes_decrypt_n(in, out, blocks, m_DK);
}

void AES_128::key_schedule(std::span<const uint8_t> key) {
#if defined(BOTAN_HAS_AES_NI)
   if(CPUID::has(CPUID::Feature::AESNI)) {
      return aesni_key_schedule(key.data(), key.size());
   }
#endif

#if defined(BOTAN_HAS_AES_VAES)
   if(CPUID::has(CPUID::Feature::AVX2_AES)) {
      return aes_key_schedule(key.data(), key.size(), m_EK, m_DK, true);
   }
#endif

#if defined(BOTAN_HAS_HW_AES_SUPPORT)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      constexpr bool is_little_endian = std::endian::native == std::endian::little;
      return aes_key_schedule(key.data(), key.size(), m_EK, m_DK, is_little_endian);
   }
#endif

#if defined(BOTAN_HAS_AES_VPERM)
   if(CPUID::has(CPUID::Feature::SIMD_4X32)) {
      return vperm_key_schedule(key.data(), key.size());
   }
#endif

   aes_key_schedule(key.data(), key.size(), m_EK, m_DK);
}

void AES_128::clear() {
   zap(m_EK);
   zap(m_DK);
}

void AES_192::encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();

#if defined(BOTAN_HAS_AES_VAES)
   if(CPUID::has(CPUID::Feature::AVX2_AES)) {
      return x86_vaes_encrypt_n(in, out, blocks);
   }
#endif

#if defined(BOTAN_HAS_HW_AES_SUPPORT)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      return hw_aes_encrypt_n(in, out, blocks);
   }
#endif

#if defined(BOTAN_HAS_AES_VPERM)
   if(CPUID::has(CPUID::Feature::SIMD_4X32)) {
      return vperm_encrypt_n(in, out, blocks);
   }
#endif

   aes_encrypt_n(in, out, blocks, m_EK);
}

void AES_192::decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();

#if defined(BOTAN_HAS_AES_VAES)
   if(CPUID::has(CPUID::Feature::AVX2_AES)) {
      return x86_vaes_decrypt_n(in, out, blocks);
   }
#endif

#if defined(BOTAN_HAS_HW_AES_SUPPORT)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      return hw_aes_decrypt_n(in, out, blocks);
   }
#endif

#if defined(BOTAN_HAS_AES_VPERM)
   if(CPUID::has(CPUID::Feature::SIMD_4X32)) {
      return vperm_decrypt_n(in, out, blocks);
   }
#endif

   aes_decrypt_n(in, out, blocks, m_DK);
}

void AES_192::key_schedule(std::span<const uint8_t> key) {
#if defined(BOTAN_HAS_AES_NI)
   if(CPUID::has(CPUID::Feature::AESNI)) {
      return aesni_key_schedule(key.data(), key.size());
   }
#endif

#if defined(BOTAN_HAS_AES_VAES)
   if(CPUID::has(CPUID::Feature::AVX2_AES)) {
      return aes_key_schedule(key.data(), key.size(), m_EK, m_DK, true);
   }
#endif

#if defined(BOTAN_HAS_HW_AES_SUPPORT)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      constexpr bool is_little_endian = std::endian::native == std::endian::little;
      return aes_key_schedule(key.data(), key.size(), m_EK, m_DK, is_little_endian);
   }
#endif

#if defined(BOTAN_HAS_AES_VPERM)
   if(CPUID::has(CPUID::Feature::SIMD_4X32)) {
      return vperm_key_schedule(key.data(), key.size());
   }
#endif

   aes_key_schedule(key.data(), key.size(), m_EK, m_DK);
}

void AES_192::clear() {
   zap(m_EK);
   zap(m_DK);
}

void AES_256::encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();

#if defined(BOTAN_HAS_AES_VAES)
   if(CPUID::has(CPUID::Feature::AVX2_AES)) {
      return x86_vaes_encrypt_n(in, out, blocks);
   }
#endif

#if defined(BOTAN_HAS_HW_AES_SUPPORT)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      return hw_aes_encrypt_n(in, out, blocks);
   }
#endif

#if defined(BOTAN_HAS_AES_VPERM)
   if(CPUID::has(CPUID::Feature::SIMD_4X32)) {
      return vperm_encrypt_n(in, out, blocks);
   }
#endif

   aes_encrypt_n(in, out, blocks, m_EK);
}

void AES_256::decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();

#if defined(BOTAN_HAS_AES_VAES)
   if(CPUID::has(CPUID::Feature::AVX2_AES)) {
      return x86_vaes_decrypt_n(in, out, blocks);
   }
#endif

#if defined(BOTAN_HAS_HW_AES_SUPPORT)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      return hw_aes_decrypt_n(in, out, blocks);
   }
#endif

#if defined(BOTAN_HAS_AES_VPERM)
   if(CPUID::has(CPUID::Feature::SIMD_4X32)) {
      return vperm_decrypt_n(in, out, blocks);
   }
#endif

   aes_decrypt_n(in, out, blocks, m_DK);
}

void AES_256::key_schedule(std::span<const uint8_t> key) {
#if defined(BOTAN_HAS_AES_NI)
   if(CPUID::has(CPUID::Feature::AESNI)) {
      return aesni_key_schedule(key.data(), key.size());
   }
#endif

#if defined(BOTAN_HAS_AES_VAES)
   if(CPUID::has(CPUID::Feature::AVX2_AES)) {
      return aes_key_schedule(key.data(), key.size(), m_EK, m_DK, true);
   }
#endif

#if defined(BOTAN_HAS_HW_AES_SUPPORT)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      constexpr bool is_little_endian = std::endian::native == std::endian::little;
      return aes_key_schedule(key.data(), key.size(), m_EK, m_DK, is_little_endian);
   }
#endif

#if defined(BOTAN_HAS_AES_VPERM)
   if(CPUID::has(CPUID::Feature::SIMD_4X32)) {
      return vperm_key_schedule(key.data(), key.size());
   }
#endif

   aes_key_schedule(key.data(), key.size(), m_EK, m_DK);
}

void AES_256::clear() {
   zap(m_EK);
   zap(m_DK);
}

}  // namespace Botan
/*
* AES using ARMv8
* Contributed by Jeffrey Walton
*
* Further changes
* (C) 2017,2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <arm_neon.h>

namespace Botan {

namespace AES_AARCH64 {

namespace {

BOTAN_FORCE_INLINE BOTAN_FN_ISA_AES void enc(uint8x16_t& B, uint8x16_t K) {
   B = vaesmcq_u8(vaeseq_u8(B, K));
}

BOTAN_FORCE_INLINE BOTAN_FN_ISA_AES void enc4(
   uint8x16_t& B0, uint8x16_t& B1, uint8x16_t& B2, uint8x16_t& B3, uint8x16_t K) {
   B0 = vaesmcq_u8(vaeseq_u8(B0, K));
   B1 = vaesmcq_u8(vaeseq_u8(B1, K));
   B2 = vaesmcq_u8(vaeseq_u8(B2, K));
   B3 = vaesmcq_u8(vaeseq_u8(B3, K));
}

BOTAN_FORCE_INLINE BOTAN_FN_ISA_AES void enc_last(uint8x16_t& B, uint8x16_t K, uint8x16_t K2) {
   B = veorq_u8(vaeseq_u8(B, K), K2);
}

BOTAN_FORCE_INLINE BOTAN_FN_ISA_AES void enc4_last(
   uint8x16_t& B0, uint8x16_t& B1, uint8x16_t& B2, uint8x16_t& B3, uint8x16_t K, uint8x16_t K2) {
   B0 = veorq_u8(vaeseq_u8(B0, K), K2);
   B1 = veorq_u8(vaeseq_u8(B1, K), K2);
   B2 = veorq_u8(vaeseq_u8(B2, K), K2);
   B3 = veorq_u8(vaeseq_u8(B3, K), K2);
}

BOTAN_FORCE_INLINE BOTAN_FN_ISA_AES void dec(uint8x16_t& B, uint8x16_t K) {
   B = vaesimcq_u8(vaesdq_u8(B, K));
}

BOTAN_FORCE_INLINE BOTAN_FN_ISA_AES void dec4(
   uint8x16_t& B0, uint8x16_t& B1, uint8x16_t& B2, uint8x16_t& B3, uint8x16_t K) {
   B0 = vaesimcq_u8(vaesdq_u8(B0, K));
   B1 = vaesimcq_u8(vaesdq_u8(B1, K));
   B2 = vaesimcq_u8(vaesdq_u8(B2, K));
   B3 = vaesimcq_u8(vaesdq_u8(B3, K));
}

BOTAN_FORCE_INLINE BOTAN_FN_ISA_AES void dec_last(uint8x16_t& B, uint8x16_t K, uint8x16_t K2) {
   B = veorq_u8(vaesdq_u8(B, K), K2);
}

BOTAN_FORCE_INLINE BOTAN_FN_ISA_AES void dec4_last(
   uint8x16_t& B0, uint8x16_t& B1, uint8x16_t& B2, uint8x16_t& B3, uint8x16_t K, uint8x16_t K2) {
   B0 = veorq_u8(vaesdq_u8(B0, K), K2);
   B1 = veorq_u8(vaesdq_u8(B1, K), K2);
   B2 = veorq_u8(vaesdq_u8(B2, K), K2);
   B3 = veorq_u8(vaesdq_u8(B3, K), K2);
}

}  // namespace

}  // namespace AES_AARCH64

/*
* AES-128 Encryption
*/
BOTAN_FN_ISA_AES void AES_128::hw_aes_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   const uint8_t* skey = reinterpret_cast<const uint8_t*>(m_EK.data());

   const uint8x16_t K0 = vld1q_u8(skey + 0 * 16);
   const uint8x16_t K1 = vld1q_u8(skey + 1 * 16);
   const uint8x16_t K2 = vld1q_u8(skey + 2 * 16);
   const uint8x16_t K3 = vld1q_u8(skey + 3 * 16);
   const uint8x16_t K4 = vld1q_u8(skey + 4 * 16);
   const uint8x16_t K5 = vld1q_u8(skey + 5 * 16);
   const uint8x16_t K6 = vld1q_u8(skey + 6 * 16);
   const uint8x16_t K7 = vld1q_u8(skey + 7 * 16);
   const uint8x16_t K8 = vld1q_u8(skey + 8 * 16);
   const uint8x16_t K9 = vld1q_u8(skey + 9 * 16);
   const uint8x16_t K10 = vld1q_u8(skey + 10 * 16);

   using namespace AES_AARCH64;

   while(blocks >= 4) {
      uint8x16_t B0 = vld1q_u8(in);
      uint8x16_t B1 = vld1q_u8(in + 16);
      uint8x16_t B2 = vld1q_u8(in + 32);
      uint8x16_t B3 = vld1q_u8(in + 48);

      enc4(B0, B1, B2, B3, K0);
      enc4(B0, B1, B2, B3, K1);
      enc4(B0, B1, B2, B3, K2);
      enc4(B0, B1, B2, B3, K3);
      enc4(B0, B1, B2, B3, K4);
      enc4(B0, B1, B2, B3, K5);
      enc4(B0, B1, B2, B3, K6);
      enc4(B0, B1, B2, B3, K7);
      enc4(B0, B1, B2, B3, K8);
      enc4_last(B0, B1, B2, B3, K9, K10);

      vst1q_u8(out, B0);
      vst1q_u8(out + 16, B1);
      vst1q_u8(out + 32, B2);
      vst1q_u8(out + 48, B3);

      in += 16 * 4;
      out += 16 * 4;
      blocks -= 4;
   }

   for(size_t i = 0; i != blocks; ++i) {
      uint8x16_t B = vld1q_u8(in + 16 * i);
      enc(B, K0);
      enc(B, K1);
      enc(B, K2);
      enc(B, K3);
      enc(B, K4);
      enc(B, K5);
      enc(B, K6);
      enc(B, K7);
      enc(B, K8);
      enc_last(B, K9, K10);
      vst1q_u8(out + 16 * i, B);
   }
}

/*
* AES-128 Decryption
*/
BOTAN_FN_ISA_AES void AES_128::hw_aes_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   const uint8_t* skey = reinterpret_cast<const uint8_t*>(m_DK.data());

   const uint8x16_t K0 = vld1q_u8(skey + 0 * 16);
   const uint8x16_t K1 = vld1q_u8(skey + 1 * 16);
   const uint8x16_t K2 = vld1q_u8(skey + 2 * 16);
   const uint8x16_t K3 = vld1q_u8(skey + 3 * 16);
   const uint8x16_t K4 = vld1q_u8(skey + 4 * 16);
   const uint8x16_t K5 = vld1q_u8(skey + 5 * 16);
   const uint8x16_t K6 = vld1q_u8(skey + 6 * 16);
   const uint8x16_t K7 = vld1q_u8(skey + 7 * 16);
   const uint8x16_t K8 = vld1q_u8(skey + 8 * 16);
   const uint8x16_t K9 = vld1q_u8(skey + 9 * 16);
   const uint8x16_t K10 = vld1q_u8(skey + 10 * 16);

   using namespace AES_AARCH64;

   while(blocks >= 4) {
      uint8x16_t B0 = vld1q_u8(in);
      uint8x16_t B1 = vld1q_u8(in + 16);
      uint8x16_t B2 = vld1q_u8(in + 32);
      uint8x16_t B3 = vld1q_u8(in + 48);

      dec4(B0, B1, B2, B3, K0);
      dec4(B0, B1, B2, B3, K1);
      dec4(B0, B1, B2, B3, K2);
      dec4(B0, B1, B2, B3, K3);
      dec4(B0, B1, B2, B3, K4);
      dec4(B0, B1, B2, B3, K5);
      dec4(B0, B1, B2, B3, K6);
      dec4(B0, B1, B2, B3, K7);
      dec4(B0, B1, B2, B3, K8);
      dec4_last(B0, B1, B2, B3, K9, K10);

      vst1q_u8(out, B0);
      vst1q_u8(out + 16, B1);
      vst1q_u8(out + 32, B2);
      vst1q_u8(out + 48, B3);

      in += 16 * 4;
      out += 16 * 4;
      blocks -= 4;
   }

   for(size_t i = 0; i != blocks; ++i) {
      uint8x16_t B = vld1q_u8(in + 16 * i);
      dec(B, K0);
      dec(B, K1);
      dec(B, K2);
      dec(B, K3);
      dec(B, K4);
      dec(B, K5);
      dec(B, K6);
      dec(B, K7);
      dec(B, K8);
      dec_last(B, K9, K10);
      vst1q_u8(out + 16 * i, B);
   }
}

/*
* AES-192 Encryption
*/
BOTAN_FN_ISA_AES void AES_192::hw_aes_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   const uint8_t* skey = reinterpret_cast<const uint8_t*>(m_EK.data());

   const uint8x16_t K0 = vld1q_u8(skey + 0 * 16);
   const uint8x16_t K1 = vld1q_u8(skey + 1 * 16);
   const uint8x16_t K2 = vld1q_u8(skey + 2 * 16);
   const uint8x16_t K3 = vld1q_u8(skey + 3 * 16);
   const uint8x16_t K4 = vld1q_u8(skey + 4 * 16);
   const uint8x16_t K5 = vld1q_u8(skey + 5 * 16);
   const uint8x16_t K6 = vld1q_u8(skey + 6 * 16);
   const uint8x16_t K7 = vld1q_u8(skey + 7 * 16);
   const uint8x16_t K8 = vld1q_u8(skey + 8 * 16);
   const uint8x16_t K9 = vld1q_u8(skey + 9 * 16);
   const uint8x16_t K10 = vld1q_u8(skey + 10 * 16);
   const uint8x16_t K11 = vld1q_u8(skey + 11 * 16);
   const uint8x16_t K12 = vld1q_u8(skey + 12 * 16);

   using namespace AES_AARCH64;

   while(blocks >= 4) {
      uint8x16_t B0 = vld1q_u8(in);
      uint8x16_t B1 = vld1q_u8(in + 16);
      uint8x16_t B2 = vld1q_u8(in + 32);
      uint8x16_t B3 = vld1q_u8(in + 48);

      enc4(B0, B1, B2, B3, K0);
      enc4(B0, B1, B2, B3, K1);
      enc4(B0, B1, B2, B3, K2);
      enc4(B0, B1, B2, B3, K3);
      enc4(B0, B1, B2, B3, K4);
      enc4(B0, B1, B2, B3, K5);
      enc4(B0, B1, B2, B3, K6);
      enc4(B0, B1, B2, B3, K7);
      enc4(B0, B1, B2, B3, K8);
      enc4(B0, B1, B2, B3, K9);
      enc4(B0, B1, B2, B3, K10);
      enc4_last(B0, B1, B2, B3, K11, K12);

      vst1q_u8(out, B0);
      vst1q_u8(out + 16, B1);
      vst1q_u8(out + 32, B2);
      vst1q_u8(out + 48, B3);

      in += 16 * 4;
      out += 16 * 4;
      blocks -= 4;
   }

   for(size_t i = 0; i != blocks; ++i) {
      uint8x16_t B = vld1q_u8(in + 16 * i);
      enc(B, K0);
      enc(B, K1);
      enc(B, K2);
      enc(B, K3);
      enc(B, K4);
      enc(B, K5);
      enc(B, K6);
      enc(B, K7);
      enc(B, K8);
      enc(B, K9);
      enc(B, K10);
      enc_last(B, K11, K12);
      vst1q_u8(out + 16 * i, B);
   }
}

/*
* AES-192 Decryption
*/
BOTAN_FN_ISA_AES void AES_192::hw_aes_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   const uint8_t* skey = reinterpret_cast<const uint8_t*>(m_DK.data());

   const uint8x16_t K0 = vld1q_u8(skey + 0 * 16);
   const uint8x16_t K1 = vld1q_u8(skey + 1 * 16);
   const uint8x16_t K2 = vld1q_u8(skey + 2 * 16);
   const uint8x16_t K3 = vld1q_u8(skey + 3 * 16);
   const uint8x16_t K4 = vld1q_u8(skey + 4 * 16);
   const uint8x16_t K5 = vld1q_u8(skey + 5 * 16);
   const uint8x16_t K6 = vld1q_u8(skey + 6 * 16);
   const uint8x16_t K7 = vld1q_u8(skey + 7 * 16);
   const uint8x16_t K8 = vld1q_u8(skey + 8 * 16);
   const uint8x16_t K9 = vld1q_u8(skey + 9 * 16);
   const uint8x16_t K10 = vld1q_u8(skey + 10 * 16);
   const uint8x16_t K11 = vld1q_u8(skey + 11 * 16);
   const uint8x16_t K12 = vld1q_u8(skey + 12 * 16);

   using namespace AES_AARCH64;

   while(blocks >= 4) {
      uint8x16_t B0 = vld1q_u8(in);
      uint8x16_t B1 = vld1q_u8(in + 16);
      uint8x16_t B2 = vld1q_u8(in + 32);
      uint8x16_t B3 = vld1q_u8(in + 48);

      dec4(B0, B1, B2, B3, K0);
      dec4(B0, B1, B2, B3, K1);
      dec4(B0, B1, B2, B3, K2);
      dec4(B0, B1, B2, B3, K3);
      dec4(B0, B1, B2, B3, K4);
      dec4(B0, B1, B2, B3, K5);
      dec4(B0, B1, B2, B3, K6);
      dec4(B0, B1, B2, B3, K7);
      dec4(B0, B1, B2, B3, K8);
      dec4(B0, B1, B2, B3, K9);
      dec4(B0, B1, B2, B3, K10);
      dec4_last(B0, B1, B2, B3, K11, K12);

      vst1q_u8(out, B0);
      vst1q_u8(out + 16, B1);
      vst1q_u8(out + 32, B2);
      vst1q_u8(out + 48, B3);

      in += 16 * 4;
      out += 16 * 4;
      blocks -= 4;
   }

   for(size_t i = 0; i != blocks; ++i) {
      uint8x16_t B = vld1q_u8(in + 16 * i);
      dec(B, K0);
      dec(B, K1);
      dec(B, K2);
      dec(B, K3);
      dec(B, K4);
      dec(B, K5);
      dec(B, K6);
      dec(B, K7);
      dec(B, K8);
      dec(B, K9);
      dec(B, K10);
      dec_last(B, K11, K12);
      vst1q_u8(out + 16 * i, B);
   }
}

/*
* AES-256 Encryption
*/
BOTAN_FN_ISA_AES void AES_256::hw_aes_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   const uint8_t* skey = reinterpret_cast<const uint8_t*>(m_EK.data());

   const uint8x16_t K0 = vld1q_u8(skey + 0 * 16);
   const uint8x16_t K1 = vld1q_u8(skey + 1 * 16);
   const uint8x16_t K2 = vld1q_u8(skey + 2 * 16);
   const uint8x16_t K3 = vld1q_u8(skey + 3 * 16);
   const uint8x16_t K4 = vld1q_u8(skey + 4 * 16);
   const uint8x16_t K5 = vld1q_u8(skey + 5 * 16);
   const uint8x16_t K6 = vld1q_u8(skey + 6 * 16);
   const uint8x16_t K7 = vld1q_u8(skey + 7 * 16);
   const uint8x16_t K8 = vld1q_u8(skey + 8 * 16);
   const uint8x16_t K9 = vld1q_u8(skey + 9 * 16);
   const uint8x16_t K10 = vld1q_u8(skey + 10 * 16);
   const uint8x16_t K11 = vld1q_u8(skey + 11 * 16);
   const uint8x16_t K12 = vld1q_u8(skey + 12 * 16);
   const uint8x16_t K13 = vld1q_u8(skey + 13 * 16);
   const uint8x16_t K14 = vld1q_u8(skey + 14 * 16);

   using namespace AES_AARCH64;

   using namespace AES_AARCH64;

   while(blocks >= 4) {
      uint8x16_t B0 = vld1q_u8(in);
      uint8x16_t B1 = vld1q_u8(in + 16);
      uint8x16_t B2 = vld1q_u8(in + 32);
      uint8x16_t B3 = vld1q_u8(in + 48);

      enc4(B0, B1, B2, B3, K0);
      enc4(B0, B1, B2, B3, K1);
      enc4(B0, B1, B2, B3, K2);
      enc4(B0, B1, B2, B3, K3);
      enc4(B0, B1, B2, B3, K4);
      enc4(B0, B1, B2, B3, K5);
      enc4(B0, B1, B2, B3, K6);
      enc4(B0, B1, B2, B3, K7);
      enc4(B0, B1, B2, B3, K8);
      enc4(B0, B1, B2, B3, K9);
      enc4(B0, B1, B2, B3, K10);
      enc4(B0, B1, B2, B3, K11);
      enc4(B0, B1, B2, B3, K12);
      enc4_last(B0, B1, B2, B3, K13, K14);

      vst1q_u8(out, B0);
      vst1q_u8(out + 16, B1);
      vst1q_u8(out + 32, B2);
      vst1q_u8(out + 48, B3);

      in += 16 * 4;
      out += 16 * 4;
      blocks -= 4;
   }

   for(size_t i = 0; i != blocks; ++i) {
      uint8x16_t B = vld1q_u8(in + 16 * i);
      enc(B, K0);
      enc(B, K1);
      enc(B, K2);
      enc(B, K3);
      enc(B, K4);
      enc(B, K5);
      enc(B, K6);
      enc(B, K7);
      enc(B, K8);
      enc(B, K9);
      enc(B, K10);
      enc(B, K11);
      enc(B, K12);
      enc_last(B, K13, K14);
      vst1q_u8(out + 16 * i, B);
   }
}

/*
* AES-256 Decryption
*/
BOTAN_FN_ISA_AES void AES_256::hw_aes_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   const uint8_t* skey = reinterpret_cast<const uint8_t*>(m_DK.data());

   const uint8x16_t K0 = vld1q_u8(skey + 0 * 16);
   const uint8x16_t K1 = vld1q_u8(skey + 1 * 16);
   const uint8x16_t K2 = vld1q_u8(skey + 2 * 16);
   const uint8x16_t K3 = vld1q_u8(skey + 3 * 16);
   const uint8x16_t K4 = vld1q_u8(skey + 4 * 16);
   const uint8x16_t K5 = vld1q_u8(skey + 5 * 16);
   const uint8x16_t K6 = vld1q_u8(skey + 6 * 16);
   const uint8x16_t K7 = vld1q_u8(skey + 7 * 16);
   const uint8x16_t K8 = vld1q_u8(skey + 8 * 16);
   const uint8x16_t K9 = vld1q_u8(skey + 9 * 16);
   const uint8x16_t K10 = vld1q_u8(skey + 10 * 16);
   const uint8x16_t K11 = vld1q_u8(skey + 11 * 16);
   const uint8x16_t K12 = vld1q_u8(skey + 12 * 16);
   const uint8x16_t K13 = vld1q_u8(skey + 13 * 16);
   const uint8x16_t K14 = vld1q_u8(skey + 14 * 16);

   using namespace AES_AARCH64;

   while(blocks >= 4) {
      uint8x16_t B0 = vld1q_u8(in);
      uint8x16_t B1 = vld1q_u8(in + 16);
      uint8x16_t B2 = vld1q_u8(in + 32);
      uint8x16_t B3 = vld1q_u8(in + 48);

      dec4(B0, B1, B2, B3, K0);
      dec4(B0, B1, B2, B3, K1);
      dec4(B0, B1, B2, B3, K2);
      dec4(B0, B1, B2, B3, K3);
      dec4(B0, B1, B2, B3, K4);
      dec4(B0, B1, B2, B3, K5);
      dec4(B0, B1, B2, B3, K6);
      dec4(B0, B1, B2, B3, K7);
      dec4(B0, B1, B2, B3, K8);
      dec4(B0, B1, B2, B3, K9);
      dec4(B0, B1, B2, B3, K10);
      dec4(B0, B1, B2, B3, K11);
      dec4(B0, B1, B2, B3, K12);
      dec4_last(B0, B1, B2, B3, K13, K14);

      vst1q_u8(out, B0);
      vst1q_u8(out + 16, B1);
      vst1q_u8(out + 32, B2);
      vst1q_u8(out + 48, B3);

      in += 16 * 4;
      out += 16 * 4;
      blocks -= 4;
   }

   for(size_t i = 0; i != blocks; ++i) {
      uint8x16_t B = vld1q_u8(in + 16 * i);
      dec(B, K0);
      dec(B, K1);
      dec(B, K2);
      dec(B, K3);
      dec(B, K4);
      dec(B, K5);
      dec(B, K6);
      dec(B, K7);
      dec(B, K8);
      dec(B, K9);
      dec(B, K10);
      dec(B, K11);
      dec(B, K12);
      dec_last(B, K13, K14);
      vst1q_u8(out + 16 * i, B);
   }
}

}  // namespace Botan
/**
* (C) 2018,2019,2022 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <limits>

#if defined(BOTAN_HAS_THREAD_UTILS)
#endif

#if defined(BOTAN_HAS_CPUID)
#endif

namespace Botan {

namespace {

const size_t SYNC_POINTS = 4;

void argon2_H0(uint8_t H0[64],
               HashFunction& blake2b,
               size_t output_len,
               const char* password,
               size_t password_len,
               const uint8_t salt[],
               size_t salt_len,
               const uint8_t key[],
               size_t key_len,
               const uint8_t ad[],
               size_t ad_len,
               size_t y,
               size_t p,
               size_t M,
               size_t t) {
   const uint8_t v = 19;  // Argon2 version code

   blake2b.update_le(static_cast<uint32_t>(p));
   blake2b.update_le(static_cast<uint32_t>(output_len));
   blake2b.update_le(static_cast<uint32_t>(M));
   blake2b.update_le(static_cast<uint32_t>(t));
   blake2b.update_le(static_cast<uint32_t>(v));
   blake2b.update_le(static_cast<uint32_t>(y));

   blake2b.update_le(static_cast<uint32_t>(password_len));
   blake2b.update(as_span_of_bytes(password, password_len));

   blake2b.update_le(static_cast<uint32_t>(salt_len));
   blake2b.update(salt, salt_len);

   blake2b.update_le(static_cast<uint32_t>(key_len));
   blake2b.update(key, key_len);

   blake2b.update_le(static_cast<uint32_t>(ad_len));
   blake2b.update(ad, ad_len);

   blake2b.final(H0);
}

void extract_key(uint8_t output[], size_t output_len, const secure_vector<uint64_t>& B, size_t memory, size_t threads) {
   const size_t lanes = memory / threads;

   uint64_t sum[128] = {0};

   for(size_t lane = 0; lane != threads; ++lane) {
      const size_t start = 128 * (lane * lanes + lanes - 1);
      const size_t end = 128 * (lane * lanes + lanes);

      for(size_t j = start; j != end; ++j) {
         sum[j % 128] ^= B[j];
      }
   }

   if(output_len <= 64) {
      auto blake2b = HashFunction::create_or_throw(fmt("BLAKE2b({})", output_len * 8));
      blake2b->update_le(static_cast<uint32_t>(output_len));
      for(size_t i = 0; i != 128; ++i) {  // NOLINT(modernize-loop-convert)
         blake2b->update_le(sum[i]);
      }
      blake2b->final(output);
   } else {
      secure_vector<uint8_t> T(64);

      auto blake2b = HashFunction::create_or_throw("BLAKE2b(512)");
      blake2b->update_le(static_cast<uint32_t>(output_len));
      for(size_t i = 0; i != 128; ++i) {  // NOLINT(modernize-loop-convert)
         blake2b->update_le(sum[i]);
      }
      blake2b->final(std::span{T});

      while(output_len > 64) {
         copy_mem(output, T.data(), 32);
         output_len -= 32;
         output += 32;

         if(output_len > 64) {
            blake2b->update(T);
            blake2b->final(std::span{T});
         }
      }

      if(output_len == 64) {
         blake2b->update(T);
         blake2b->final(output);
      } else {
         auto blake2b_f = HashFunction::create_or_throw(fmt("BLAKE2b({})", output_len * 8));
         blake2b_f->update(T);
         blake2b_f->final(output);
      }
   }
}

void init_blocks(
   secure_vector<uint64_t>& B, HashFunction& blake2b, const uint8_t H0[64], size_t memory, size_t threads) {
   BOTAN_ASSERT_NOMSG(B.size() >= threads * 256);

   for(size_t i = 0; i != threads; ++i) {
      const size_t B_off = i * (memory / threads);

      BOTAN_ASSERT_NOMSG(B.size() >= 128 * (B_off + 2));

      for(size_t j = 0; j != 2; ++j) {
         uint8_t T[64] = {0};

         blake2b.update_le(static_cast<uint32_t>(1024));
         blake2b.update(H0, 64);
         blake2b.update_le(static_cast<uint32_t>(j));
         blake2b.update_le(static_cast<uint32_t>(i));
         blake2b.final(T);

         for(size_t k = 0; k != 30; ++k) {
            load_le(&B[128 * (B_off + j) + 4 * k], T, 32 / 8);
            blake2b.update(T, 64);
            blake2b.final(T);
         }

         load_le(&B[128 * (B_off + j) + 4 * 30], T, 64 / 8);
      }
   }
}

BOTAN_FORCE_INLINE void blamka_G(uint64_t& A, uint64_t& B, uint64_t& C, uint64_t& D) {
   A += B + (static_cast<uint64_t>(2) * static_cast<uint32_t>(A)) * static_cast<uint32_t>(B);
   D = rotr<32>(A ^ D);

   C += D + (static_cast<uint64_t>(2) * static_cast<uint32_t>(C)) * static_cast<uint32_t>(D);
   B = rotr<24>(B ^ C);

   A += B + (static_cast<uint64_t>(2) * static_cast<uint32_t>(A)) * static_cast<uint32_t>(B);
   D = rotr<16>(A ^ D);

   C += D + (static_cast<uint64_t>(2) * static_cast<uint32_t>(C)) * static_cast<uint32_t>(D);
   B = rotr<63>(B ^ C);
}

}  // namespace

void Argon2::blamka(uint64_t N[128], uint64_t T[128]) {
#if defined(BOTAN_HAS_ARGON2_AVX512)
   if(CPUID::has(CPUID::Feature::AVX512)) {
      return Argon2::blamka_avx512(N, T);
   }
#endif

#if defined(BOTAN_HAS_ARGON2_AVX2)
   if(CPUID::has(CPUID::Feature::AVX2)) {
      return Argon2::blamka_avx2(N, T);
   }
#endif

#if defined(BOTAN_HAS_ARGON2_SIMD64)
   if(CPUID::has(CPUID::Feature::SIMD_2X64)) {
      return Argon2::blamka_simd64(N, T);
   }
#endif

   copy_mem(T, N, 128);

   for(size_t i = 0; i != 128; i += 16) {
      blamka_G(T[i + 0], T[i + 4], T[i + 8], T[i + 12]);
      blamka_G(T[i + 1], T[i + 5], T[i + 9], T[i + 13]);
      blamka_G(T[i + 2], T[i + 6], T[i + 10], T[i + 14]);
      blamka_G(T[i + 3], T[i + 7], T[i + 11], T[i + 15]);

      blamka_G(T[i + 0], T[i + 5], T[i + 10], T[i + 15]);
      blamka_G(T[i + 1], T[i + 6], T[i + 11], T[i + 12]);
      blamka_G(T[i + 2], T[i + 7], T[i + 8], T[i + 13]);
      blamka_G(T[i + 3], T[i + 4], T[i + 9], T[i + 14]);
   }

   for(size_t i = 0; i != 128 / 8; i += 2) {
      blamka_G(T[i + 0], T[i + 32], T[i + 64], T[i + 96]);
      blamka_G(T[i + 1], T[i + 33], T[i + 65], T[i + 97]);
      blamka_G(T[i + 16], T[i + 48], T[i + 80], T[i + 112]);
      blamka_G(T[i + 17], T[i + 49], T[i + 81], T[i + 113]);

      blamka_G(T[i + 0], T[i + 33], T[i + 80], T[i + 113]);
      blamka_G(T[i + 1], T[i + 48], T[i + 81], T[i + 96]);
      blamka_G(T[i + 16], T[i + 49], T[i + 64], T[i + 97]);
      blamka_G(T[i + 17], T[i + 32], T[i + 65], T[i + 112]);
   }

   for(size_t i = 0; i != 128; ++i) {
      N[i] ^= T[i];
   }
}

namespace {

void gen_2i_addresses(uint64_t T[128],
                      uint64_t B[128],
                      size_t n,
                      size_t lane,
                      size_t slice,
                      size_t memory,
                      size_t time,
                      size_t mode,
                      size_t cnt) {
   clear_mem(B, 128);

   B[0] = n;
   B[1] = lane;
   B[2] = slice;
   B[3] = memory;
   B[4] = time;
   B[5] = mode;
   B[6] = cnt;

   for(size_t r = 0; r != 2; ++r) {
      Argon2::blamka(B, T);
   }
}

// Reduce random modulo Argon2 thread count (normally a power of 2)
inline size_t mod_threads(uint32_t random, size_t threads) {
   if(is_power_of_2(threads)) {
      return random & static_cast<uint32_t>(threads - 1);
   } else {
      return random % threads;
   }
}

// Reduce alpha modulo the lane length; always a multiple of 4 and commonly a power of 2
inline size_t mod_lanes(uint64_t alpha, size_t lanes) {
   if(is_power_of_2(lanes)) {
      return static_cast<size_t>(alpha & static_cast<uint64_t>(lanes - 1));
   } else {
      return alpha % lanes;
   }
}

uint32_t index_alpha(
   uint64_t random, size_t lanes, size_t segments, size_t threads, size_t n, size_t slice, size_t lane, size_t index) {
   size_t ref_lane = mod_threads(static_cast<uint32_t>(random >> 32), threads);

   if(n == 0 && slice == 0) {
      ref_lane = lane;
   }

   size_t m = 3 * segments;
   size_t s = ((slice + 1) % 4) * segments;

   if(lane == ref_lane) {
      m += index;
   }

   if(n == 0) {
      m = slice * segments;
      s = 0;
      if(slice == 0 || lane == ref_lane) {
         m += index;
      }
   }

   if(index == 0 || lane == ref_lane) {
      m -= 1;
   }

   uint64_t p = static_cast<uint32_t>(random);
   p = (p * p) >> 32;
   p = (p * m) >> 32;

   return static_cast<uint32_t>(ref_lane * lanes + mod_lanes(s + m - (p + 1), lanes));
}

void process_block(secure_vector<uint64_t>& B,
                   size_t n,
                   size_t slice,
                   size_t lane,
                   size_t lanes,
                   size_t segments,
                   size_t threads,
                   uint8_t mode,
                   size_t memory,
                   size_t time) {
   uint64_t T[128];
   size_t index = 0;
   if(n == 0 && slice == 0) {
      index = 2;
   }

   const bool use_2i = mode == 1 || (mode == 2 && n == 0 && slice < SYNC_POINTS / 2);

   uint64_t addresses[128];
   size_t address_counter = 1;

   if(use_2i) {
      gen_2i_addresses(T, addresses, n, lane, slice, memory, time, mode, address_counter);
   }

   while(index < segments) {
      const size_t offset = lane * lanes + slice * segments + index;

      size_t prev = offset - 1;
      if(index == 0 && slice == 0) {
         prev += lanes;
      }

      if(use_2i && index > 0 && index % 128 == 0) {
         address_counter += 1;
         gen_2i_addresses(T, addresses, n, lane, slice, memory, time, mode, address_counter);
      }

      const uint64_t random = use_2i ? addresses[index % 128] : B.at(128 * prev);
      const size_t new_offset = index_alpha(random, lanes, segments, threads, n, slice, lane, index);

      uint64_t N[128];
      for(size_t i = 0; i != 128; ++i) {
         N[i] = B[128 * prev + i] ^ B[128 * new_offset + i];
      }

      Argon2::blamka(N, T);

      for(size_t i = 0; i != 128; ++i) {
         B[128 * offset + i] ^= N[i];
      }

      index += 1;
   }
}

void process_blocks(secure_vector<uint64_t>& B, size_t t, size_t memory, size_t threads, uint8_t mode) {
   const size_t lanes = memory / threads;
   const size_t segments = lanes / SYNC_POINTS;

#if defined(BOTAN_HAS_THREAD_UTILS)
   if(threads > 1) {
      auto& thread_pool = Thread_Pool::global_instance();

      for(size_t n = 0; n != t; ++n) {
         for(size_t slice = 0; slice != SYNC_POINTS; ++slice) {
            std::vector<std::future<void>> fut_results;
            fut_results.reserve(threads);

            for(size_t lane = 0; lane != threads; ++lane) {
               fut_results.push_back(thread_pool.run(
                  process_block, std::ref(B), n, slice, lane, lanes, segments, threads, mode, memory, t));
            }

            for(auto& fut : fut_results) {
               fut.get();
            }
         }
      }

      return;
   }
#endif

   for(size_t n = 0; n != t; ++n) {
      for(size_t slice = 0; slice != SYNC_POINTS; ++slice) {
         for(size_t lane = 0; lane != threads; ++lane) {
            process_block(B, n, slice, lane, lanes, segments, threads, mode, memory, t);
         }
      }
   }
}

}  // namespace

void Argon2::argon2(uint8_t output[],
                    size_t output_len,
                    const char* password,
                    size_t password_len,
                    const uint8_t salt[],
                    size_t salt_len,
                    const uint8_t key[],
                    size_t key_len,
                    const uint8_t ad[],
                    size_t ad_len) const {
   BOTAN_ARG_CHECK(output_len >= 4 && output_len <= std::numeric_limits<uint32_t>::max(),
                   "Invalid Argon2 output length");
   BOTAN_ARG_CHECK(password_len <= std::numeric_limits<uint32_t>::max(), "Invalid Argon2 password length");
   BOTAN_ARG_CHECK(salt_len <= std::numeric_limits<uint32_t>::max(), "Invalid Argon2 salt length");
   BOTAN_ARG_CHECK(key_len <= std::numeric_limits<uint32_t>::max(), "Invalid Argon2 key length");
   BOTAN_ARG_CHECK(ad_len <= std::numeric_limits<uint32_t>::max(), "Invalid Argon2 ad length");

   auto blake2 = HashFunction::create_or_throw("BLAKE2b");

   uint8_t H0[64] = {0};
   argon2_H0(H0,
             *blake2,
             output_len,
             password,
             password_len,
             salt,
             salt_len,
             key,
             key_len,
             ad,
             ad_len,
             m_family,
             m_p,
             m_M,
             m_t);

   const size_t memory = (m_M / (SYNC_POINTS * m_p)) * (SYNC_POINTS * m_p);

   secure_vector<uint64_t> B(memory * 1024 / 8);

   init_blocks(B, *blake2, H0, memory, m_p);
   process_blocks(B, m_t, memory, m_p, m_family);

   clear_mem(output, output_len);
   extract_key(output, output_len, B, memory, m_p);
}

}  // namespace Botan
/**
* (C) 2019 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <algorithm>

namespace Botan {

Argon2::Argon2(uint8_t family, size_t M, size_t t, size_t p) : m_family(family), m_M(M), m_t(t), m_p(p) {
   BOTAN_ARG_CHECK(m_p >= 1 && m_p <= 128, "Invalid Argon2 threads parameter");
   BOTAN_ARG_CHECK(m_M >= 8 * m_p && m_M <= 8192 * 1024, "Invalid Argon2 M parameter");
   BOTAN_ARG_CHECK(m_t >= 1 && m_t <= std::numeric_limits<uint32_t>::max(), "Invalid Argon2 t parameter");
}

void Argon2::derive_key(uint8_t output[],
                        size_t output_len,
                        const char* password,
                        size_t password_len,
                        const uint8_t salt[],
                        size_t salt_len) const {
   argon2(output, output_len, password, password_len, salt, salt_len, nullptr, 0, nullptr, 0);
}

void Argon2::derive_key(uint8_t output[],
                        size_t output_len,
                        const char* password,
                        size_t password_len,
                        const uint8_t salt[],
                        size_t salt_len,
                        const uint8_t ad[],
                        size_t ad_len,
                        const uint8_t key[],
                        size_t key_len) const {
   argon2(output, output_len, password, password_len, salt, salt_len, key, key_len, ad, ad_len);
}

namespace {

std::string argon2_family_name(uint8_t f) {
   switch(f) {
      case 0:
         return "Argon2d";
      case 1:
         return "Argon2i";
      case 2:
         return "Argon2id";
      default:
         throw Invalid_Argument("Unknown Argon2 parameter");
   }
}

}  // namespace

std::string Argon2::to_string() const {
   return fmt("{}({},{},{})", argon2_family_name(m_family), m_M, m_t, m_p);
}

Argon2_Family::Argon2_Family(uint8_t family) : m_family(family) {
   if(m_family != 0 && m_family != 1 && m_family != 2) {
      throw Invalid_Argument("Unknown Argon2 family identifier");
   }
}

std::string Argon2_Family::name() const {
   return argon2_family_name(m_family);
}

std::unique_ptr<PasswordHash> Argon2_Family::tune_params(size_t /*output_length*/,
                                                         uint64_t desired_msec,
                                                         std::optional<size_t> max_memory,
                                                         uint64_t tune_msec) const {
   // If not set use 256 MB as default max
   const size_t max_kib = max_memory.value_or(256) * 1024;

   // Tune with a large memory otherwise we measure cache vs RAM speeds and underestimate
   // costs for larger params. Default is 36 MiB, or use 128 for long times.
   const size_t tune_M = (desired_msec >= 200 ? 128 : 36) * 1024;
   const size_t p = 1;
   size_t t = 1;

   size_t M = 4 * 1024;

   auto pwhash = this->from_params(tune_M, t, p);

   auto tune_fn = [&]() {
      uint8_t output[64] = {0};
      pwhash->derive_key(output, sizeof(output), "test", 4, nullptr, 0);
   };

   const uint64_t measured_time = measure_cost(tune_msec, tune_fn) / (tune_M / M);

   const uint64_t target_nsec = desired_msec * static_cast<uint64_t>(1000000);

   /*
   * Argon2 scaling rules:
   * k*M, k*t, k*p all increase cost by about k
   *
   * First preference is to increase M up to max allowed value.
   * Any remaining time budget is spent on increasing t.
   */

   uint64_t est_nsec = measured_time;

   if(est_nsec < target_nsec && M < max_kib) {
      const uint64_t desired_cost_increase = (target_nsec + est_nsec - 1) / est_nsec;
      const uint64_t mem_headroom = max_kib / M;

      const uint64_t M_mult = std::min(desired_cost_increase, mem_headroom);
      M *= static_cast<size_t>(M_mult);
      est_nsec *= M_mult;
   }

   if(est_nsec < target_nsec / 2) {
      const uint64_t desired_cost_increase = (target_nsec + est_nsec - 1) / est_nsec;
      t *= static_cast<size_t>(desired_cost_increase);
   }

   return this->from_params(M, t, p);
}

std::unique_ptr<PasswordHash> Argon2_Family::default_params() const {
   return this->from_params(128 * 1024, 1, 1);
}

std::unique_ptr<PasswordHash> Argon2_Family::from_iterations(size_t iter) const {
   /*
   These choices are arbitrary, but should not change in future
   releases since they will break applications expecting deterministic
   mapping from iteration count to params
   */
   const size_t M = iter;
   const size_t t = 1;
   const size_t p = 1;
   return this->from_params(M, t, p);
}

std::unique_ptr<PasswordHash> Argon2_Family::from_params(size_t M, size_t t, size_t p) const {
   return std::make_unique<Argon2>(m_family, M, t, p);
}

}  // namespace Botan
/*
* (C) 2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_ENTROPY_SOURCE)
#endif

#if defined(BOTAN_HAS_SYSTEM_RNG)
#endif

namespace Botan {

namespace {

std::unique_ptr<MessageAuthenticationCode> auto_rng_hmac() {
   const std::string possible_auto_rng_hmacs[] = {
      "HMAC(SHA-512)",
      "HMAC(SHA-256)",
   };

   for(const auto& hmac : possible_auto_rng_hmacs) {
      if(auto mac = MessageAuthenticationCode::create(hmac)) {
         return mac;
      }
   }

   // This shouldn't happen since this module has a dependency on sha2_32
   throw Internal_Error("AutoSeeded_RNG: No usable HMAC hash found");
}

}  // namespace

AutoSeeded_RNG::AutoSeeded_RNG(AutoSeeded_RNG&& other) noexcept = default;

AutoSeeded_RNG::~AutoSeeded_RNG() = default;

AutoSeeded_RNG::AutoSeeded_RNG(RandomNumberGenerator& underlying_rng, size_t reseed_interval) {
   m_rng = std::make_unique<HMAC_DRBG>(auto_rng_hmac(), underlying_rng, reseed_interval);

   force_reseed();
}

AutoSeeded_RNG::AutoSeeded_RNG(Entropy_Sources& entropy_sources, size_t reseed_interval) {
   m_rng = std::make_unique<HMAC_DRBG>(auto_rng_hmac(), entropy_sources, reseed_interval);

   force_reseed();
}

AutoSeeded_RNG::AutoSeeded_RNG(RandomNumberGenerator& underlying_rng,
                               Entropy_Sources& entropy_sources,
                               size_t reseed_interval) {
   m_rng = std::make_unique<HMAC_DRBG>(auto_rng_hmac(), underlying_rng, entropy_sources, reseed_interval);

   force_reseed();
}

AutoSeeded_RNG::AutoSeeded_RNG(size_t reseed_interval) {
#if defined(BOTAN_HAS_SYSTEM_RNG)
   m_rng = std::make_unique<HMAC_DRBG>(auto_rng_hmac(), system_rng(), reseed_interval);
#elif defined(BOTAN_HAS_ENTROPY_SOURCE)
   m_rng = std::make_unique<HMAC_DRBG>(auto_rng_hmac(), Entropy_Sources::global_sources(), reseed_interval);
#else
   BOTAN_UNUSED(reseed_interval);
   throw Not_Implemented("AutoSeeded_RNG default constructor not available due to no RNG or entropy sources");
#endif

   force_reseed();
}

void AutoSeeded_RNG::force_reseed() {
   m_rng->force_reseed();
   m_rng->next_byte();

   if(!m_rng->is_seeded()) {
      throw Internal_Error("AutoSeeded_RNG reseeding failed");
   }
}

bool AutoSeeded_RNG::is_seeded() const {
   return m_rng->is_seeded();
}

void AutoSeeded_RNG::clear() {
   m_rng->clear();
}

std::string AutoSeeded_RNG::name() const {
   return m_rng->name();
}

size_t AutoSeeded_RNG::reseed_from_sources(Entropy_Sources& srcs, size_t poll_bits) {
   return m_rng->reseed_from_sources(srcs, poll_bits);
}

void AutoSeeded_RNG::fill_bytes_with_input(std::span<uint8_t> out, std::span<const uint8_t> in) {
   if(in.empty()) {
      m_rng->randomize_with_ts_input(out);
   } else {
      m_rng->randomize_with_input(out, in);
   }
}

}  // namespace Botan
/*
* (C) 2019 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

void Buffered_Computation::update(std::string_view str) {
   add_data(as_span_of_bytes(str));
}

void Buffered_Computation::update_be(uint16_t val) {
   uint8_t inb[sizeof(val)];
   store_be(val, inb);
   add_data({inb, sizeof(inb)});
}

void Buffered_Computation::update_be(uint32_t val) {
   uint8_t inb[sizeof(val)];
   store_be(val, inb);
   add_data({inb, sizeof(inb)});
}

void Buffered_Computation::update_be(uint64_t val) {
   uint8_t inb[sizeof(val)];
   store_be(val, inb);
   add_data({inb, sizeof(inb)});
}

void Buffered_Computation::update_le(uint16_t val) {
   uint8_t inb[sizeof(val)];
   store_le(val, inb);
   add_data({inb, sizeof(inb)});
}

void Buffered_Computation::update_le(uint32_t val) {
   uint8_t inb[sizeof(val)];
   store_le(val, inb);
   add_data({inb, sizeof(inb)});
}

void Buffered_Computation::update_le(uint64_t val) {
   uint8_t inb[sizeof(val)];
   store_le(val, inb);
   add_data({inb, sizeof(inb)});
}

void Buffered_Computation::final(std::span<uint8_t> out) {
   BOTAN_ARG_CHECK(out.size() >= output_length(), "provided output buffer has insufficient capacity");
   final_result(out);
}

}  // namespace Botan
/*
* (C) 2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

void SymmetricAlgorithm::set_key(const OctetString& key) {
   set_key(std::span{key.begin(), key.length()});
}

void SymmetricAlgorithm::throw_key_not_set_error() const {
   throw Key_Not_Set(name());
}

void SymmetricAlgorithm::set_key(std::span<const uint8_t> key) {
   if(!valid_keylength(key.size())) {
      throw Invalid_Key_Length(name(), key.size());
   }
   key_schedule(key);
}

}  // namespace Botan
/*
* OctetString
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

/*
* Create an OctetString from RNG output
*/
OctetString::OctetString(RandomNumberGenerator& rng, size_t len) {
   rng.random_vec(m_data, len);
}

/*
* Create an OctetString from a hex string
*/
OctetString::OctetString(std::string_view hex_string) {
   if(!hex_string.empty()) {
      m_data.resize(1 + hex_string.length() / 2);
      m_data.resize(hex_decode(m_data.data(), hex_string));
   }
}

/*
* Create an OctetString from a byte string
*/
OctetString::OctetString(const uint8_t in[], size_t n) {
   m_data.assign(in, in + n);
}

namespace {

uint8_t odd_parity_of(uint8_t x) {
   uint8_t f = x | 0x01;
   f ^= (f >> 4);
   f ^= (f >> 2);
   f ^= (f >> 1);

   return (x & 0xFE) ^ (f & 0x01);
}

}  // namespace

/*
* Set the parity of each key byte to odd
*/
void OctetString::set_odd_parity() {
   for(auto& b : m_data) {
      b = odd_parity_of(b);
   }
}

/*
* Hex encode an OctetString
*/
std::string OctetString::to_string() const {
   return hex_encode(m_data.data(), m_data.size());
}

/*
* XOR Operation for OctetStrings
*/
OctetString& OctetString::operator^=(const OctetString& k) {
   if(&k == this) {
      zeroise(m_data);
      return (*this);
   }
   xor_buf(m_data.data(), k.begin(), std::min(length(), k.length()));
   return (*this);
}

/*
* Equality Operation for OctetStrings
*/
bool operator==(const OctetString& s1, const OctetString& s2) {
   return (s1.bits_of() == s2.bits_of());
}

/*
* Inequality Operation for OctetStrings
*/
bool operator!=(const OctetString& s1, const OctetString& s2) {
   return !(s1 == s2);
}

/*
* Append Operation for OctetStrings
*/
OctetString operator+(const OctetString& k1, const OctetString& k2) {
   secure_vector<uint8_t> out;
   out += k1.bits_of();
   out += k2.bits_of();
   return OctetString(out);
}

/*
* XOR Operation for OctetStrings
*/
OctetString operator^(const OctetString& k1, const OctetString& k2) {
   secure_vector<uint8_t> out(std::max(k1.length(), k2.length()));

   copy_mem(out.data(), k1.begin(), k1.length());
   xor_buf(out.data(), k2.begin(), k2.length());
   return OctetString(out);
}

}  // namespace Botan
/*
* BLAKE2b
* (C) 2016 cynecx
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <array>

namespace Botan {

namespace {

constexpr std::array<uint64_t, 8> blake2b_IV{0x6a09e667f3bcc908,
                                             0xbb67ae8584caa73b,
                                             0x3c6ef372fe94f82b,
                                             0xa54ff53a5f1d36f1,
                                             0x510e527fade682d1,
                                             0x9b05688c2b3e6c1f,
                                             0x1f83d9abfb41bd6b,
                                             0x5be0cd19137e2179};

}  // namespace

BLAKE2b::BLAKE2b(size_t output_bits) : m_output_bits(output_bits), m_H(blake2b_IV.size()), m_T(), m_F(), m_key_size(0) {
   if(output_bits == 0 || output_bits > 512 || output_bits % 8 != 0) {
      throw Invalid_Argument("Bad output bits size for BLAKE2b");
   }

   state_init();
}

void BLAKE2b::state_init() {
   copy_mem(m_H.data(), blake2b_IV.data(), blake2b_IV.size());
   m_H[0] ^= (0x01010000 | (static_cast<uint8_t>(m_key_size) << 8) | static_cast<uint8_t>(output_length()));
   m_T[0] = m_T[1] = 0;
   m_F = 0;

   m_buffer.clear();
   if(m_key_size > 0) {
      m_buffer.append(m_padded_key_buffer);
   }
}

namespace {

BOTAN_FORCE_INLINE void G(uint64_t& a, uint64_t& b, uint64_t& c, uint64_t& d, uint64_t M0, uint64_t M1) {
   a = a + b + M0;
   d = rotr<32>(d ^ a);
   c = c + d;
   b = rotr<24>(b ^ c);
   a = a + b + M1;
   d = rotr<16>(d ^ a);
   c = c + d;
   b = rotr<63>(b ^ c);
}

template <size_t i0,
          size_t i1,
          size_t i2,
          size_t i3,
          size_t i4,
          size_t i5,
          size_t i6,
          size_t i7,
          size_t i8,
          size_t i9,
          size_t iA,
          size_t iB,
          size_t iC,
          size_t iD,
          size_t iE,
          size_t iF>
BOTAN_FORCE_INLINE void ROUND(uint64_t* v, const uint64_t* M) {
   G(v[0], v[4], v[8], v[12], M[i0], M[i1]);
   G(v[1], v[5], v[9], v[13], M[i2], M[i3]);
   G(v[2], v[6], v[10], v[14], M[i4], M[i5]);
   G(v[3], v[7], v[11], v[15], M[i6], M[i7]);
   G(v[0], v[5], v[10], v[15], M[i8], M[i9]);
   G(v[1], v[6], v[11], v[12], M[iA], M[iB]);
   G(v[2], v[7], v[8], v[13], M[iC], M[iD]);
   G(v[3], v[4], v[9], v[14], M[iE], M[iF]);
}

}  // namespace

void BLAKE2b::compress(const uint8_t* input, size_t blocks, uint64_t increment) {
   for(size_t b = 0; b != blocks; ++b) {
      m_T[0] += increment;
      if(m_T[0] < increment) {
         m_T[1]++;
      }

      uint64_t M[16];
      uint64_t v[16];
      load_le(M, input, 16);

      input += BLAKE2B_BLOCKBYTES;

      for(size_t i = 0; i < 8; i++) {
         v[i] = m_H[i];
      }
      for(size_t i = 0; i != 8; ++i) {
         v[i + 8] = blake2b_IV[i];
      }

      v[12] ^= m_T[0];
      v[13] ^= m_T[1];
      v[14] ^= m_F;

      ROUND<0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15>(v, M);
      ROUND<14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3>(v, M);
      ROUND<11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4>(v, M);
      ROUND<7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8>(v, M);
      ROUND<9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13>(v, M);
      ROUND<2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9>(v, M);
      ROUND<12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11>(v, M);
      ROUND<13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10>(v, M);
      ROUND<6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5>(v, M);
      ROUND<10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0>(v, M);
      ROUND<0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15>(v, M);
      ROUND<14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3>(v, M);

      for(size_t i = 0; i < 8; i++) {
         m_H[i] ^= v[i] ^ v[i + 8];
      }
   }
}

void BLAKE2b::add_data(std::span<const uint8_t> input) {
   BufferSlicer in(input);

   while(!in.empty()) {
      if(const auto one_block = m_buffer.handle_unaligned_data(in)) {
         compress(one_block->data(), 1, BLAKE2B_BLOCKBYTES);
      }

      if(m_buffer.in_alignment()) {
         const auto [aligned_data, full_blocks] = m_buffer.aligned_data_to_process(in);
         if(full_blocks > 0) {
            compress(aligned_data.data(), full_blocks, BLAKE2B_BLOCKBYTES);
         }
      }
   }
}

void BLAKE2b::final_result(std::span<uint8_t> output) {
   const auto pos = m_buffer.elements_in_buffer();
   m_buffer.fill_up_with_zeros();

   m_F = 0xFFFFFFFFFFFFFFFF;
   compress(m_buffer.consume().data(), 1, pos);
   copy_out_le(output.first(output_length()), m_H);
   state_init();
}

Key_Length_Specification BLAKE2b::key_spec() const {
   return Key_Length_Specification(1, 64);
}

std::string BLAKE2b::name() const {
   return fmt("BLAKE2b({})", m_output_bits);
}

std::unique_ptr<HashFunction> BLAKE2b::new_object() const {
   return std::make_unique<BLAKE2b>(m_output_bits);
}

std::unique_ptr<HashFunction> BLAKE2b::copy_state() const {
   return std::make_unique<BLAKE2b>(*this);
}

bool BLAKE2b::has_keying_material() const {
   return m_key_size > 0;
}

void BLAKE2b::key_schedule(std::span<const uint8_t> key) {
   BOTAN_ASSERT_NOMSG(key.size() <= m_buffer.size());

   m_key_size = key.size();
   m_padded_key_buffer.resize(m_buffer.size());

   if(m_padded_key_buffer.size() > m_key_size) {
      const size_t padding = m_padded_key_buffer.size() - m_key_size;
      clear_mem(m_padded_key_buffer.data() + m_key_size, padding);
   }

   copy_mem(m_padded_key_buffer.data(), key.data(), key.size());
   state_init();
}

void BLAKE2b::clear() {
   zeroise(m_H);
   m_buffer.clear();
   zeroise(m_padded_key_buffer);
   m_key_size = 0;
   state_init();
}

}  // namespace Botan
/*
 * BLAKE2s
 * (C) 2023, 2025       Richard Huveneers
 * (C) 2025             Kagan Can Sit
 * (C) 2025             René Meusel, Rohde & Schwarz Cybersecurity
 *
 * Based on the RFC7693 reference implementation
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 */

namespace Botan {

namespace {

// Initialization Vector.
constexpr std::array<uint32_t, 8> blake2s_iv{
   0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A, 0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19};

// Mixing function G.
template <uint8_t a, uint8_t b, uint8_t c, uint8_t d>
   requires(a < 16 && b < 16 && c < 16 && d < 16)
constexpr void B2S_G(uint32_t x, uint32_t y, std::span<uint32_t, 16> v) {
   v[a] = v[a] + v[b] + x;
   v[d] = rotr<16>(v[d] ^ v[a]);
   v[c] = v[c] + v[d];
   v[b] = rotr<12>(v[b] ^ v[c]);
   v[a] = v[a] + v[b] + y;
   v[d] = rotr<8>(v[d] ^ v[a]);
   v[c] = v[c] + v[d];
   v[b] = rotr<7>(v[b] ^ v[c]);
}

}  // namespace

std::string BLAKE2s::name() const {
   return fmt("BLAKE2s({})", m_outlen << 3);
}

// BLAKE2s is specified as a message authentication code. For that, the
// key would need to be zero-padded and incorporated into the initial hash
// state. See RFC 7693 Section 3.3 and Appendix D.2 `blake2s_init()`.
void BLAKE2s::state_init(size_t outlen) {
   m_h = blake2s_iv;
   m_h[0] ^= 0x01010000 ^ outlen;

   m_bytes_processed = 0;
   m_outlen = outlen;
   m_buffer.clear();
}

// Compression function. "last" flag indicates last block.
void BLAKE2s::compress(bool last, std::span<const uint8_t> buf) {
   BOTAN_ASSERT_NOMSG(buf.size() == block_size);
   constexpr std::array<std::array<uint8_t, 16>, 10> sigma{{{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                                            {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
                                                            {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4},
                                                            {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
                                                            {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13},
                                                            {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9},
                                                            {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11},
                                                            {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10},
                                                            {6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5},
                                                            {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0}}};

   std::array<uint32_t, 16> v = concat(m_h, blake2s_iv);
   v[12] ^= static_cast<uint32_t>(m_bytes_processed);
   v[13] ^= static_cast<uint32_t>(m_bytes_processed >> 32);
   if(last) v[14] = ~v[14];

   const auto m = load_le<std::array<uint32_t, 16>>(buf);
   for(const auto& perm : sigma) {
      B2S_G<0, 4, 8, 12>(m[perm[0]], m[perm[1]], v);
      B2S_G<1, 5, 9, 13>(m[perm[2]], m[perm[3]], v);
      B2S_G<2, 6, 10, 14>(m[perm[4]], m[perm[5]], v);
      B2S_G<3, 7, 11, 15>(m[perm[6]], m[perm[7]], v);
      B2S_G<0, 5, 10, 15>(m[perm[8]], m[perm[9]], v);
      B2S_G<1, 6, 11, 12>(m[perm[10]], m[perm[11]], v);
      B2S_G<2, 7, 8, 13>(m[perm[12]], m[perm[13]], v);
      B2S_G<3, 4, 9, 14>(m[perm[14]], m[perm[15]], v);
   }
   for(size_t i = 0; i < 8; ++i) m_h[i] ^= v[i] ^ v[i + 8];
}

void BLAKE2s::clear() {
   state_init(m_outlen);
}

void BLAKE2s::add_data(std::span<const uint8_t> input) {
   BufferSlicer in(input);
   while(!in.empty()) {
      if(const auto one_block = m_buffer.handle_unaligned_data(in)) {
         m_bytes_processed += block_size;
         compress(false, *one_block);
      }
      if(m_buffer.in_alignment()) {
         while(const auto aligned_block = m_buffer.next_aligned_block_to_process(in)) {
            m_bytes_processed += block_size;
            compress(false, *aligned_block);
         }
      }
   }
}

void BLAKE2s::final_result(std::span<uint8_t> out) {
   m_bytes_processed += m_buffer.elements_in_buffer();
   m_buffer.fill_up_with_zeros();
   compress(true, m_buffer.consume());
   copy_out_le(out.first(output_length()), m_h);
   clear();
}

std::unique_ptr<HashFunction> BLAKE2s::copy_state() const {
   return std::make_unique<BLAKE2s>(*this);
}

BLAKE2s::BLAKE2s(size_t output_bits) {
   if(output_bits == 0 || output_bits > 256 || output_bits % 8 != 0) {
      throw Invalid_Argument("Bad output bits size for BLAKE2s");
   }
   state_init(output_bits >> 3);
}

BLAKE2s::~BLAKE2s() {
   secure_scrub_memory(m_h);
}

}  // namespace Botan
/*
* Block Ciphers
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <memory>
#include <utility>

#if defined(BOTAN_HAS_AES)
#endif

#if defined(BOTAN_HAS_ARIA)
#endif

#if defined(BOTAN_HAS_BLOWFISH)
#endif

#if defined(BOTAN_HAS_CAMELLIA)
#endif

#if defined(BOTAN_HAS_CAST_128)
#endif

#if defined(BOTAN_HAS_CASCADE)
#endif

#if defined(BOTAN_HAS_DES)
#endif

#if defined(BOTAN_HAS_GOST_28147_89)
#endif

#if defined(BOTAN_HAS_IDEA)
#endif

#if defined(BOTAN_HAS_KUZNYECHIK)
#endif

#if defined(BOTAN_HAS_LION)
#endif

#if defined(BOTAN_HAS_NOEKEON)
#endif

#if defined(BOTAN_HAS_SEED)
#endif

#if defined(BOTAN_HAS_SERPENT)
#endif

#if defined(BOTAN_HAS_SHACAL2)
#endif

#if defined(BOTAN_HAS_SM4)
#endif

#if defined(BOTAN_HAS_TWOFISH)
#endif

#if defined(BOTAN_HAS_THREEFISH_512)
#endif

#if defined(BOTAN_HAS_COMMONCRYPTO)
#endif

namespace Botan {

std::unique_ptr<BlockCipher> BlockCipher::create(std::string_view algo, std::string_view provider) {
#if defined(BOTAN_HAS_COMMONCRYPTO)
   if(provider.empty() || provider == "commoncrypto") {
      if(auto bc = make_commoncrypto_block_cipher(algo))
         return bc;

      if(!provider.empty())
         return nullptr;
   }
#endif

   // TODO: CryptoAPI
   // TODO: /dev/crypto

   // Only base providers from here on out
   if(provider.empty() == false && provider != "base") {
      return nullptr;
   }

#if defined(BOTAN_HAS_AES)
   if(algo == "AES-128") {
      return std::make_unique<AES_128>();
   }

   if(algo == "AES-192") {
      return std::make_unique<AES_192>();
   }

   if(algo == "AES-256") {
      return std::make_unique<AES_256>();
   }
#endif

#if defined(BOTAN_HAS_ARIA)
   if(algo == "ARIA-128") {
      return std::make_unique<ARIA_128>();
   }

   if(algo == "ARIA-192") {
      return std::make_unique<ARIA_192>();
   }

   if(algo == "ARIA-256") {
      return std::make_unique<ARIA_256>();
   }
#endif

#if defined(BOTAN_HAS_SERPENT)
   if(algo == "Serpent") {
      return std::make_unique<Serpent>();
   }
#endif

#if defined(BOTAN_HAS_SHACAL2)
   if(algo == "SHACAL2") {
      return std::make_unique<SHACAL2>();
   }
#endif

#if defined(BOTAN_HAS_TWOFISH)
   if(algo == "Twofish") {
      return std::make_unique<Twofish>();
   }
#endif

#if defined(BOTAN_HAS_THREEFISH_512)
   if(algo == "Threefish-512") {
      return std::make_unique<Threefish_512>();
   }
#endif

#if defined(BOTAN_HAS_BLOWFISH)
   if(algo == "Blowfish") {
      return std::make_unique<Blowfish>();
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA)
   if(algo == "Camellia-128") {
      return std::make_unique<Camellia_128>();
   }

   if(algo == "Camellia-192") {
      return std::make_unique<Camellia_192>();
   }

   if(algo == "Camellia-256") {
      return std::make_unique<Camellia_256>();
   }
#endif

#if defined(BOTAN_HAS_DES)
   if(algo == "DES") {
      return std::make_unique<DES>();
   }

   if(algo == "TripleDES" || algo == "3DES" || algo == "DES-EDE") {
      return std::make_unique<TripleDES>();
   }
#endif

#if defined(BOTAN_HAS_NOEKEON)
   if(algo == "Noekeon") {
      return std::make_unique<Noekeon>();
   }
#endif

#if defined(BOTAN_HAS_CAST_128)
   if(algo == "CAST-128" || algo == "CAST5") {
      return std::make_unique<CAST_128>();
   }
#endif

#if defined(BOTAN_HAS_IDEA)
   if(algo == "IDEA") {
      return std::make_unique<IDEA>();
   }
#endif

#if defined(BOTAN_HAS_KUZNYECHIK)
   if(algo == "Kuznyechik") {
      return std::make_unique<Kuznyechik>();
   }
#endif

#if defined(BOTAN_HAS_SEED)
   if(algo == "SEED") {
      return std::make_unique<SEED>();
   }
#endif

#if defined(BOTAN_HAS_SM4)
   if(algo == "SM4") {
      return std::make_unique<SM4>();
   }
#endif

   const SCAN_Name req(algo);

#if defined(BOTAN_HAS_GOST_28147_89)
   if(req.algo_name() == "GOST-28147-89") {
      return std::make_unique<GOST_28147_89>(req.arg(0, "R3411_94_TestParam"));
   }
#endif

#if defined(BOTAN_HAS_CASCADE)
   if(req.algo_name() == "Cascade" && req.arg_count() == 2) {
      auto c1 = BlockCipher::create(req.arg(0));
      auto c2 = BlockCipher::create(req.arg(1));

      if(c1 && c2) {
         return std::make_unique<Cascade_Cipher>(std::move(c1), std::move(c2));
      }
   }
#endif

#if defined(BOTAN_HAS_LION)
   if(req.algo_name() == "Lion" && req.arg_count_between(2, 3)) {
      auto hash = HashFunction::create(req.arg(0));
      auto stream = StreamCipher::create(req.arg(1));

      if(hash && stream) {
         const size_t block_size = req.arg_as_integer(2, 1024);
         return std::make_unique<Lion>(std::move(hash), std::move(stream), block_size);
      }
   }
#endif

   BOTAN_UNUSED(req);
   BOTAN_UNUSED(provider);

   return nullptr;
}

//static
std::unique_ptr<BlockCipher> BlockCipher::create_or_throw(std::string_view algo, std::string_view provider) {
   if(auto bc = BlockCipher::create(algo, provider)) {
      return bc;
   }
   throw Lookup_Error("Block cipher", algo, provider);
}

std::vector<std::string> BlockCipher::providers(std::string_view algo) {
   return probe_providers_of<BlockCipher>(algo, {"base", "commoncrypto"});
}

}  // namespace Botan
/*
* Camellia
* (C) 2012,2020 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_CPUID)
#endif

namespace Botan {

namespace {

namespace Camellia_F {

alignas(256) const uint8_t SBOX1[256] = {
   0x70, 0x82, 0x2C, 0xEC, 0xB3, 0x27, 0xC0, 0xE5, 0xE4, 0x85, 0x57, 0x35, 0xEA, 0x0C, 0xAE, 0x41, 0x23, 0xEF, 0x6B,
   0x93, 0x45, 0x19, 0xA5, 0x21, 0xED, 0x0E, 0x4F, 0x4E, 0x1D, 0x65, 0x92, 0xBD, 0x86, 0xB8, 0xAF, 0x8F, 0x7C, 0xEB,
   0x1F, 0xCE, 0x3E, 0x30, 0xDC, 0x5F, 0x5E, 0xC5, 0x0B, 0x1A, 0xA6, 0xE1, 0x39, 0xCA, 0xD5, 0x47, 0x5D, 0x3D, 0xD9,
   0x01, 0x5A, 0xD6, 0x51, 0x56, 0x6C, 0x4D, 0x8B, 0x0D, 0x9A, 0x66, 0xFB, 0xCC, 0xB0, 0x2D, 0x74, 0x12, 0x2B, 0x20,
   0xF0, 0xB1, 0x84, 0x99, 0xDF, 0x4C, 0xCB, 0xC2, 0x34, 0x7E, 0x76, 0x05, 0x6D, 0xB7, 0xA9, 0x31, 0xD1, 0x17, 0x04,
   0xD7, 0x14, 0x58, 0x3A, 0x61, 0xDE, 0x1B, 0x11, 0x1C, 0x32, 0x0F, 0x9C, 0x16, 0x53, 0x18, 0xF2, 0x22, 0xFE, 0x44,
   0xCF, 0xB2, 0xC3, 0xB5, 0x7A, 0x91, 0x24, 0x08, 0xE8, 0xA8, 0x60, 0xFC, 0x69, 0x50, 0xAA, 0xD0, 0xA0, 0x7D, 0xA1,
   0x89, 0x62, 0x97, 0x54, 0x5B, 0x1E, 0x95, 0xE0, 0xFF, 0x64, 0xD2, 0x10, 0xC4, 0x00, 0x48, 0xA3, 0xF7, 0x75, 0xDB,
   0x8A, 0x03, 0xE6, 0xDA, 0x09, 0x3F, 0xDD, 0x94, 0x87, 0x5C, 0x83, 0x02, 0xCD, 0x4A, 0x90, 0x33, 0x73, 0x67, 0xF6,
   0xF3, 0x9D, 0x7F, 0xBF, 0xE2, 0x52, 0x9B, 0xD8, 0x26, 0xC8, 0x37, 0xC6, 0x3B, 0x81, 0x96, 0x6F, 0x4B, 0x13, 0xBE,
   0x63, 0x2E, 0xE9, 0x79, 0xA7, 0x8C, 0x9F, 0x6E, 0xBC, 0x8E, 0x29, 0xF5, 0xF9, 0xB6, 0x2F, 0xFD, 0xB4, 0x59, 0x78,
   0x98, 0x06, 0x6A, 0xE7, 0x46, 0x71, 0xBA, 0xD4, 0x25, 0xAB, 0x42, 0x88, 0xA2, 0x8D, 0xFA, 0x72, 0x07, 0xB9, 0x55,
   0xF8, 0xEE, 0xAC, 0x0A, 0x36, 0x49, 0x2A, 0x68, 0x3C, 0x38, 0xF1, 0xA4, 0x40, 0x28, 0xD3, 0x7B, 0xBB, 0xC9, 0x43,
   0xC1, 0x15, 0xE3, 0xAD, 0xF4, 0x77, 0xC7, 0x80, 0x9E};

// SBOX2[x] = rotl<1>(SBOX1[x])
alignas(256) const uint8_t SBOX2[256] = {
   0xE0, 0x05, 0x58, 0xD9, 0x67, 0x4E, 0x81, 0xCB, 0xC9, 0x0B, 0xAE, 0x6A, 0xD5, 0x18, 0x5D, 0x82, 0x46, 0xDF, 0xD6,
   0x27, 0x8A, 0x32, 0x4B, 0x42, 0xDB, 0x1C, 0x9E, 0x9C, 0x3A, 0xCA, 0x25, 0x7B, 0x0D, 0x71, 0x5F, 0x1F, 0xF8, 0xD7,
   0x3E, 0x9D, 0x7C, 0x60, 0xB9, 0xBE, 0xBC, 0x8B, 0x16, 0x34, 0x4D, 0xC3, 0x72, 0x95, 0xAB, 0x8E, 0xBA, 0x7A, 0xB3,
   0x02, 0xB4, 0xAD, 0xA2, 0xAC, 0xD8, 0x9A, 0x17, 0x1A, 0x35, 0xCC, 0xF7, 0x99, 0x61, 0x5A, 0xE8, 0x24, 0x56, 0x40,
   0xE1, 0x63, 0x09, 0x33, 0xBF, 0x98, 0x97, 0x85, 0x68, 0xFC, 0xEC, 0x0A, 0xDA, 0x6F, 0x53, 0x62, 0xA3, 0x2E, 0x08,
   0xAF, 0x28, 0xB0, 0x74, 0xC2, 0xBD, 0x36, 0x22, 0x38, 0x64, 0x1E, 0x39, 0x2C, 0xA6, 0x30, 0xE5, 0x44, 0xFD, 0x88,
   0x9F, 0x65, 0x87, 0x6B, 0xF4, 0x23, 0x48, 0x10, 0xD1, 0x51, 0xC0, 0xF9, 0xD2, 0xA0, 0x55, 0xA1, 0x41, 0xFA, 0x43,
   0x13, 0xC4, 0x2F, 0xA8, 0xB6, 0x3C, 0x2B, 0xC1, 0xFF, 0xC8, 0xA5, 0x20, 0x89, 0x00, 0x90, 0x47, 0xEF, 0xEA, 0xB7,
   0x15, 0x06, 0xCD, 0xB5, 0x12, 0x7E, 0xBB, 0x29, 0x0F, 0xB8, 0x07, 0x04, 0x9B, 0x94, 0x21, 0x66, 0xE6, 0xCE, 0xED,
   0xE7, 0x3B, 0xFE, 0x7F, 0xC5, 0xA4, 0x37, 0xB1, 0x4C, 0x91, 0x6E, 0x8D, 0x76, 0x03, 0x2D, 0xDE, 0x96, 0x26, 0x7D,
   0xC6, 0x5C, 0xD3, 0xF2, 0x4F, 0x19, 0x3F, 0xDC, 0x79, 0x1D, 0x52, 0xEB, 0xF3, 0x6D, 0x5E, 0xFB, 0x69, 0xB2, 0xF0,
   0x31, 0x0C, 0xD4, 0xCF, 0x8C, 0xE2, 0x75, 0xA9, 0x4A, 0x57, 0x84, 0x11, 0x45, 0x1B, 0xF5, 0xE4, 0x0E, 0x73, 0xAA,
   0xF1, 0xDD, 0x59, 0x14, 0x6C, 0x92, 0x54, 0xD0, 0x78, 0x70, 0xE3, 0x49, 0x80, 0x50, 0xA7, 0xF6, 0x77, 0x93, 0x86,
   0x83, 0x2A, 0xC7, 0x5B, 0xE9, 0xEE, 0x8F, 0x01, 0x3D};

// SBOX3[x] = rotl<7>(SBOX1[x])
alignas(256) const uint8_t SBOX3[256] = {
   0x38, 0x41, 0x16, 0x76, 0xD9, 0x93, 0x60, 0xF2, 0x72, 0xC2, 0xAB, 0x9A, 0x75, 0x06, 0x57, 0xA0, 0x91, 0xF7, 0xB5,
   0xC9, 0xA2, 0x8C, 0xD2, 0x90, 0xF6, 0x07, 0xA7, 0x27, 0x8E, 0xB2, 0x49, 0xDE, 0x43, 0x5C, 0xD7, 0xC7, 0x3E, 0xF5,
   0x8F, 0x67, 0x1F, 0x18, 0x6E, 0xAF, 0x2F, 0xE2, 0x85, 0x0D, 0x53, 0xF0, 0x9C, 0x65, 0xEA, 0xA3, 0xAE, 0x9E, 0xEC,
   0x80, 0x2D, 0x6B, 0xA8, 0x2B, 0x36, 0xA6, 0xC5, 0x86, 0x4D, 0x33, 0xFD, 0x66, 0x58, 0x96, 0x3A, 0x09, 0x95, 0x10,
   0x78, 0xD8, 0x42, 0xCC, 0xEF, 0x26, 0xE5, 0x61, 0x1A, 0x3F, 0x3B, 0x82, 0xB6, 0xDB, 0xD4, 0x98, 0xE8, 0x8B, 0x02,
   0xEB, 0x0A, 0x2C, 0x1D, 0xB0, 0x6F, 0x8D, 0x88, 0x0E, 0x19, 0x87, 0x4E, 0x0B, 0xA9, 0x0C, 0x79, 0x11, 0x7F, 0x22,
   0xE7, 0x59, 0xE1, 0xDA, 0x3D, 0xC8, 0x12, 0x04, 0x74, 0x54, 0x30, 0x7E, 0xB4, 0x28, 0x55, 0x68, 0x50, 0xBE, 0xD0,
   0xC4, 0x31, 0xCB, 0x2A, 0xAD, 0x0F, 0xCA, 0x70, 0xFF, 0x32, 0x69, 0x08, 0x62, 0x00, 0x24, 0xD1, 0xFB, 0xBA, 0xED,
   0x45, 0x81, 0x73, 0x6D, 0x84, 0x9F, 0xEE, 0x4A, 0xC3, 0x2E, 0xC1, 0x01, 0xE6, 0x25, 0x48, 0x99, 0xB9, 0xB3, 0x7B,
   0xF9, 0xCE, 0xBF, 0xDF, 0x71, 0x29, 0xCD, 0x6C, 0x13, 0x64, 0x9B, 0x63, 0x9D, 0xC0, 0x4B, 0xB7, 0xA5, 0x89, 0x5F,
   0xB1, 0x17, 0xF4, 0xBC, 0xD3, 0x46, 0xCF, 0x37, 0x5E, 0x47, 0x94, 0xFA, 0xFC, 0x5B, 0x97, 0xFE, 0x5A, 0xAC, 0x3C,
   0x4C, 0x03, 0x35, 0xF3, 0x23, 0xB8, 0x5D, 0x6A, 0x92, 0xD5, 0x21, 0x44, 0x51, 0xC6, 0x7D, 0x39, 0x83, 0xDC, 0xAA,
   0x7C, 0x77, 0x56, 0x05, 0x1B, 0xA4, 0x15, 0x34, 0x1E, 0x1C, 0xF8, 0x52, 0x20, 0x14, 0xE9, 0xBD, 0xDD, 0xE4, 0xA1,
   0xE0, 0x8A, 0xF1, 0xD6, 0x7A, 0xBB, 0xE3, 0x40, 0x4F};

// SBOX4[x] = SBOX1[rotl<1>(x)]
alignas(256) const uint8_t SBOX4[256] = {
   0x70, 0x2C, 0xB3, 0xC0, 0xE4, 0x57, 0xEA, 0xAE, 0x23, 0x6B, 0x45, 0xA5, 0xED, 0x4F, 0x1D, 0x92, 0x86, 0xAF, 0x7C,
   0x1F, 0x3E, 0xDC, 0x5E, 0x0B, 0xA6, 0x39, 0xD5, 0x5D, 0xD9, 0x5A, 0x51, 0x6C, 0x8B, 0x9A, 0xFB, 0xB0, 0x74, 0x2B,
   0xF0, 0x84, 0xDF, 0xCB, 0x34, 0x76, 0x6D, 0xA9, 0xD1, 0x04, 0x14, 0x3A, 0xDE, 0x11, 0x32, 0x9C, 0x53, 0xF2, 0xFE,
   0xCF, 0xC3, 0x7A, 0x24, 0xE8, 0x60, 0x69, 0xAA, 0xA0, 0xA1, 0x62, 0x54, 0x1E, 0xE0, 0x64, 0x10, 0x00, 0xA3, 0x75,
   0x8A, 0xE6, 0x09, 0xDD, 0x87, 0x83, 0xCD, 0x90, 0x73, 0xF6, 0x9D, 0xBF, 0x52, 0xD8, 0xC8, 0xC6, 0x81, 0x6F, 0x13,
   0x63, 0xE9, 0xA7, 0x9F, 0xBC, 0x29, 0xF9, 0x2F, 0xB4, 0x78, 0x06, 0xE7, 0x71, 0xD4, 0xAB, 0x88, 0x8D, 0x72, 0xB9,
   0xF8, 0xAC, 0x36, 0x2A, 0x3C, 0xF1, 0x40, 0xD3, 0xBB, 0x43, 0x15, 0xAD, 0x77, 0x80, 0x82, 0xEC, 0x27, 0xE5, 0x85,
   0x35, 0x0C, 0x41, 0xEF, 0x93, 0x19, 0x21, 0x0E, 0x4E, 0x65, 0xBD, 0xB8, 0x8F, 0xEB, 0xCE, 0x30, 0x5F, 0xC5, 0x1A,
   0xE1, 0xCA, 0x47, 0x3D, 0x01, 0xD6, 0x56, 0x4D, 0x0D, 0x66, 0xCC, 0x2D, 0x12, 0x20, 0xB1, 0x99, 0x4C, 0xC2, 0x7E,
   0x05, 0xB7, 0x31, 0x17, 0xD7, 0x58, 0x61, 0x1B, 0x1C, 0x0F, 0x16, 0x18, 0x22, 0x44, 0xB2, 0xB5, 0x91, 0x08, 0xA8,
   0xFC, 0x50, 0xD0, 0x7D, 0x89, 0x97, 0x5B, 0x95, 0xFF, 0xD2, 0xC4, 0x48, 0xF7, 0xDB, 0x03, 0xDA, 0x3F, 0x94, 0x5C,
   0x02, 0x4A, 0x33, 0x67, 0xF3, 0x7F, 0xE2, 0x9B, 0x26, 0x37, 0x3B, 0x96, 0x4B, 0xBE, 0x2E, 0x79, 0x8C, 0x6E, 0x8E,
   0xF5, 0xB6, 0xFD, 0x59, 0x98, 0x6A, 0x46, 0xBA, 0x25, 0x42, 0xA2, 0xFA, 0x07, 0x55, 0xEE, 0x0A, 0x49, 0x68, 0x38,
   0xA4, 0x28, 0x7B, 0xC9, 0xC1, 0xE3, 0xF4, 0xC7, 0x9E};

uint64_t F(uint64_t v, uint64_t K) {
   const uint64_t M1 = 0x0101010001000001;
   const uint64_t M2 = 0x0001010101010000;
   const uint64_t M3 = 0x0100010100010100;
   const uint64_t M4 = 0x0101000100000101;
   const uint64_t M5 = 0x0001010100010101;
   const uint64_t M6 = 0x0100010101000101;
   const uint64_t M7 = 0x0101000101010001;
   const uint64_t M8 = 0x0101010001010100;

   const uint64_t x = v ^ K;

   const uint64_t Z1 = M1 * SBOX1[get_byte<0>(x)];
   const uint64_t Z2 = M2 * SBOX2[get_byte<1>(x)];
   const uint64_t Z3 = M3 * SBOX3[get_byte<2>(x)];
   const uint64_t Z4 = M4 * SBOX4[get_byte<3>(x)];
   const uint64_t Z5 = M5 * SBOX2[get_byte<4>(x)];
   const uint64_t Z6 = M6 * SBOX3[get_byte<5>(x)];
   const uint64_t Z7 = M7 * SBOX4[get_byte<6>(x)];
   const uint64_t Z8 = M8 * SBOX1[get_byte<7>(x)];

   return Z1 ^ Z2 ^ Z3 ^ Z4 ^ Z5 ^ Z6 ^ Z7 ^ Z8;
}

inline uint64_t FL(uint64_t v, uint64_t K) {
   uint32_t x1 = static_cast<uint32_t>(v >> 32);
   uint32_t x2 = static_cast<uint32_t>(v & 0xFFFFFFFF);

   const uint32_t k1 = static_cast<uint32_t>(K >> 32);
   const uint32_t k2 = static_cast<uint32_t>(K & 0xFFFFFFFF);

   x2 ^= rotl<1>(x1 & k1);
   x1 ^= (x2 | k2);

   return ((static_cast<uint64_t>(x1) << 32) | x2);
}

inline uint64_t FLINV(uint64_t v, uint64_t K) {
   uint32_t x1 = static_cast<uint32_t>(v >> 32);
   uint32_t x2 = static_cast<uint32_t>(v & 0xFFFFFFFF);

   const uint32_t k1 = static_cast<uint32_t>(K >> 32);
   const uint32_t k2 = static_cast<uint32_t>(K & 0xFFFFFFFF);

   x1 ^= (x2 | k2);
   x2 ^= rotl<1>(x1 & k1);

   return ((static_cast<uint64_t>(x1) << 32) | x2);
}

/*
* Camellia Encryption
*/
void encrypt(const uint8_t in[], uint8_t out[], size_t blocks, const secure_vector<uint64_t>& SK, size_t rounds) {
   prefetch_arrays(SBOX1, SBOX2, SBOX3, SBOX4);

   for(size_t i = 0; i < blocks; ++i) {
      uint64_t D1 = load_be<uint64_t>(in, 2 * i + 0);
      uint64_t D2 = load_be<uint64_t>(in, 2 * i + 1);

      const uint64_t* K = SK.data();

      D1 ^= *K++;
      D2 ^= *K++;

      D2 ^= F(D1, *K++);
      D1 ^= F(D2, *K++);

      for(size_t r = 1; r != rounds - 1; ++r) {
         if(r % 3 == 0) {
            D1 = FL(D1, *K++);
            D2 = FLINV(D2, *K++);
         }

         D2 ^= F(D1, *K++);
         D1 ^= F(D2, *K++);
      }

      D2 ^= F(D1, *K++);
      D1 ^= F(D2, *K++);

      D2 ^= *K++;
      D1 ^= *K++;

      store_be(out + 16 * i, D2, D1);
   }
}

/*
* Camellia Decryption
*/
void decrypt(const uint8_t in[], uint8_t out[], size_t blocks, const secure_vector<uint64_t>& SK, size_t rounds) {
   prefetch_arrays(SBOX1, SBOX2, SBOX3, SBOX4);

   for(size_t i = 0; i < blocks; ++i) {
      uint64_t D1 = load_be<uint64_t>(in, 2 * i + 0);
      uint64_t D2 = load_be<uint64_t>(in, 2 * i + 1);

      const uint64_t* K = &SK[SK.size() - 1];

      D2 ^= *K--;
      D1 ^= *K--;

      D2 ^= F(D1, *K--);
      D1 ^= F(D2, *K--);

      for(size_t r = 1; r != rounds - 1; ++r) {
         if(r % 3 == 0) {
            D1 = FL(D1, *K--);
            D2 = FLINV(D2, *K--);
         }

         D2 ^= F(D1, *K--);
         D1 ^= F(D2, *K--);
      }

      D2 ^= F(D1, *K--);
      D1 ^= F(D2, *K--);

      D1 ^= *K--;
      D2 ^= *K;

      store_be(out + 16 * i, D2, D1);
   }
}

inline uint64_t left_rot_hi(uint64_t h, uint64_t l, size_t shift) {
   if(shift >= 64) {
      shift -= 64;
   }
   return (h << shift) | (l >> (64 - shift));
}

inline uint64_t left_rot_lo(uint64_t h, uint64_t l, size_t shift) {
   if(shift >= 64) {
      shift -= 64;
   }
   return (h >> (64 - shift)) | (l << shift);
}

/*
* Camellia Key Schedule
*/
void key_schedule(secure_vector<uint64_t>& SK, std::span<const uint8_t> key) {
   const uint64_t Sigma1 = 0xA09E667F3BCC908B;
   const uint64_t Sigma2 = 0xB67AE8584CAA73B2;
   const uint64_t Sigma3 = 0xC6EF372FE94F82BE;
   const uint64_t Sigma4 = 0x54FF53A5F1D36F1C;
   const uint64_t Sigma5 = 0x10E527FADE682D1D;
   const uint64_t Sigma6 = 0xB05688C2B3E6C1FD;

   const uint64_t KL_H = load_be<uint64_t>(key.data(), 0);
   const uint64_t KL_L = load_be<uint64_t>(key.data(), 1);

   const uint64_t KR_H = (key.size() >= 24) ? load_be<uint64_t>(key.data(), 2) : 0;

   const uint64_t KR_L = [&]() -> uint64_t {
      if(key.size() == 32) {
         return load_be<uint64_t>(key.data(), 3);
      } else if(key.size() == 24) {
         return ~KR_H;
      } else {
         return 0;
      }
   }();

   uint64_t D1 = KL_H ^ KR_H;
   uint64_t D2 = KL_L ^ KR_L;
   D2 ^= F(D1, Sigma1);
   D1 ^= F(D2, Sigma2);
   D1 ^= KL_H;
   D2 ^= KL_L;
   D2 ^= F(D1, Sigma3);
   D1 ^= F(D2, Sigma4);

   const uint64_t KA_H = D1;
   const uint64_t KA_L = D2;

   D1 = KA_H ^ KR_H;
   D2 = KA_L ^ KR_L;
   D2 ^= F(D1, Sigma5);
   D1 ^= F(D2, Sigma6);

   const uint64_t KB_H = D1;
   const uint64_t KB_L = D2;

   if(key.size() == 16) {
      SK.resize(26);

      SK[0] = KL_H;
      SK[1] = KL_L;
      SK[2] = KA_H;
      SK[3] = KA_L;
      SK[4] = left_rot_hi(KL_H, KL_L, 15);
      SK[5] = left_rot_lo(KL_H, KL_L, 15);
      SK[6] = left_rot_hi(KA_H, KA_L, 15);
      SK[7] = left_rot_lo(KA_H, KA_L, 15);
      SK[8] = left_rot_hi(KA_H, KA_L, 30);
      SK[9] = left_rot_lo(KA_H, KA_L, 30);
      SK[10] = left_rot_hi(KL_H, KL_L, 45);
      SK[11] = left_rot_lo(KL_H, KL_L, 45);
      SK[12] = left_rot_hi(KA_H, KA_L, 45);
      SK[13] = left_rot_lo(KL_H, KL_L, 60);
      SK[14] = left_rot_hi(KA_H, KA_L, 60);
      SK[15] = left_rot_lo(KA_H, KA_L, 60);
      SK[16] = left_rot_lo(KL_H, KL_L, 77);
      SK[17] = left_rot_hi(KL_H, KL_L, 77);
      SK[18] = left_rot_lo(KL_H, KL_L, 94);
      SK[19] = left_rot_hi(KL_H, KL_L, 94);
      SK[20] = left_rot_lo(KA_H, KA_L, 94);
      SK[21] = left_rot_hi(KA_H, KA_L, 94);
      SK[22] = left_rot_lo(KL_H, KL_L, 111);
      SK[23] = left_rot_hi(KL_H, KL_L, 111);
      SK[24] = left_rot_lo(KA_H, KA_L, 111);
      SK[25] = left_rot_hi(KA_H, KA_L, 111);
   } else {
      SK.resize(34);

      SK[0] = KL_H;
      SK[1] = KL_L;
      SK[2] = KB_H;
      SK[3] = KB_L;

      SK[4] = left_rot_hi(KR_H, KR_L, 15);
      SK[5] = left_rot_lo(KR_H, KR_L, 15);
      SK[6] = left_rot_hi(KA_H, KA_L, 15);
      SK[7] = left_rot_lo(KA_H, KA_L, 15);

      SK[8] = left_rot_hi(KR_H, KR_L, 30);
      SK[9] = left_rot_lo(KR_H, KR_L, 30);
      SK[10] = left_rot_hi(KB_H, KB_L, 30);
      SK[11] = left_rot_lo(KB_H, KB_L, 30);

      SK[12] = left_rot_hi(KL_H, KL_L, 45);
      SK[13] = left_rot_lo(KL_H, KL_L, 45);
      SK[14] = left_rot_hi(KA_H, KA_L, 45);
      SK[15] = left_rot_lo(KA_H, KA_L, 45);

      SK[16] = left_rot_hi(KL_H, KL_L, 60);
      SK[17] = left_rot_lo(KL_H, KL_L, 60);
      SK[18] = left_rot_hi(KR_H, KR_L, 60);
      SK[19] = left_rot_lo(KR_H, KR_L, 60);
      SK[20] = left_rot_hi(KB_H, KB_L, 60);
      SK[21] = left_rot_lo(KB_H, KB_L, 60);

      SK[22] = left_rot_lo(KL_H, KL_L, 77);
      SK[23] = left_rot_hi(KL_H, KL_L, 77);
      SK[24] = left_rot_lo(KA_H, KA_L, 77);
      SK[25] = left_rot_hi(KA_H, KA_L, 77);

      SK[26] = left_rot_lo(KR_H, KR_L, 94);
      SK[27] = left_rot_hi(KR_H, KR_L, 94);
      SK[28] = left_rot_lo(KA_H, KA_L, 94);
      SK[29] = left_rot_hi(KA_H, KA_L, 94);
      SK[30] = left_rot_lo(KL_H, KL_L, 111);
      SK[31] = left_rot_hi(KL_H, KL_L, 111);
      SK[32] = left_rot_lo(KB_H, KB_L, 111);
      SK[33] = left_rot_hi(KB_H, KB_L, 111);
   }
}

std::string provider() {
#if defined(BOTAN_HAS_CAMELLIA_AVX512_GFNI)
   if(auto feat = CPUID::check(CPUID::Feature::AVX512, CPUID::Feature::GFNI)) {
      return *feat;
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_AVX2_GFNI)
   if(auto feat = CPUID::check(CPUID::Feature::GFNI)) {
      return *feat;
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_HWAES)
   if(auto feat = CPUID::check(CPUID::Feature::HW_AES)) {
      return *feat;
   }
#endif

   return "base";
}

size_t parallelism() {
#if defined(BOTAN_HAS_CAMELLIA_AVX512_GFNI)
   if(CPUID::has(CPUID::Feature::AVX512, CPUID::Feature::GFNI)) {
      return 16;
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_AVX2_GFNI)
   if(CPUID::has(CPUID::Feature::GFNI)) {
      return 4;
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_HWAES)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      return 2;
   }
#endif

   return 1;
}

}  // namespace Camellia_F

}  // namespace

void Camellia_128::encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();

#if defined(BOTAN_HAS_CAMELLIA_AVX512_GFNI)
   if(CPUID::has(CPUID::Feature::AVX512, CPUID::Feature::GFNI)) {
      return avx512_gfni_encrypt(in, out, blocks, m_SK);
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_AVX2_GFNI)
   if(CPUID::has(CPUID::Feature::GFNI)) {
      return avx2_gfni_encrypt(in, out, blocks, m_SK);
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_HWAES)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      return hwaes_encrypt(in, out, blocks, m_SK);
   }
#endif

   Camellia_F::encrypt(in, out, blocks, m_SK, 9);
}

void Camellia_192::encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();

#if defined(BOTAN_HAS_CAMELLIA_AVX512_GFNI)
   if(CPUID::has(CPUID::Feature::AVX512, CPUID::Feature::GFNI)) {
      return avx512_gfni_encrypt(in, out, blocks, m_SK);
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_AVX2_GFNI)
   if(CPUID::has(CPUID::Feature::GFNI)) {
      return avx2_gfni_encrypt(in, out, blocks, m_SK);
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_HWAES)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      return hwaes_encrypt(in, out, blocks, m_SK);
   }
#endif

   Camellia_F::encrypt(in, out, blocks, m_SK, 12);
}

void Camellia_256::encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();

#if defined(BOTAN_HAS_CAMELLIA_AVX512_GFNI)
   if(CPUID::has(CPUID::Feature::AVX512, CPUID::Feature::GFNI)) {
      return avx512_gfni_encrypt(in, out, blocks, m_SK);
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_AVX2_GFNI)
   if(CPUID::has(CPUID::Feature::GFNI)) {
      return avx2_gfni_encrypt(in, out, blocks, m_SK);
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_HWAES)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      return hwaes_encrypt(in, out, blocks, m_SK);
   }
#endif

   Camellia_F::encrypt(in, out, blocks, m_SK, 12);
}

void Camellia_128::decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();

#if defined(BOTAN_HAS_CAMELLIA_AVX512_GFNI)
   if(CPUID::has(CPUID::Feature::AVX512, CPUID::Feature::GFNI)) {
      return avx512_gfni_decrypt(in, out, blocks, m_SK);
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_AVX2_GFNI)
   if(CPUID::has(CPUID::Feature::GFNI)) {
      return avx2_gfni_decrypt(in, out, blocks, m_SK);
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_HWAES)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      return hwaes_decrypt(in, out, blocks, m_SK);
   }
#endif

   Camellia_F::decrypt(in, out, blocks, m_SK, 9);
}

void Camellia_192::decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();

#if defined(BOTAN_HAS_CAMELLIA_AVX512_GFNI)
   if(CPUID::has(CPUID::Feature::AVX512, CPUID::Feature::GFNI)) {
      return avx512_gfni_decrypt(in, out, blocks, m_SK);
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_AVX2_GFNI)
   if(CPUID::has(CPUID::Feature::GFNI)) {
      return avx2_gfni_decrypt(in, out, blocks, m_SK);
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_HWAES)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      return hwaes_decrypt(in, out, blocks, m_SK);
   }
#endif

   Camellia_F::decrypt(in, out, blocks, m_SK, 12);
}

void Camellia_256::decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();

#if defined(BOTAN_HAS_CAMELLIA_AVX512_GFNI)
   if(CPUID::has(CPUID::Feature::AVX512, CPUID::Feature::GFNI)) {
      return avx512_gfni_decrypt(in, out, blocks, m_SK);
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_AVX2_GFNI)
   if(CPUID::has(CPUID::Feature::GFNI)) {
      return avx2_gfni_decrypt(in, out, blocks, m_SK);
   }
#endif

#if defined(BOTAN_HAS_CAMELLIA_HWAES)
   if(CPUID::has(CPUID::Feature::HW_AES)) {
      return hwaes_decrypt(in, out, blocks, m_SK);
   }
#endif

   Camellia_F::decrypt(in, out, blocks, m_SK, 12);
}

bool Camellia_128::has_keying_material() const {
   return !m_SK.empty();
}

bool Camellia_192::has_keying_material() const {
   return !m_SK.empty();
}

bool Camellia_256::has_keying_material() const {
   return !m_SK.empty();
}

void Camellia_128::key_schedule(std::span<const uint8_t> key) {
   Camellia_F::key_schedule(m_SK, key);
}

void Camellia_192::key_schedule(std::span<const uint8_t> key) {
   Camellia_F::key_schedule(m_SK, key);
}

void Camellia_256::key_schedule(std::span<const uint8_t> key) {
   Camellia_F::key_schedule(m_SK, key);
}

void Camellia_128::clear() {
   zap(m_SK);
}

void Camellia_192::clear() {
   zap(m_SK);
}

void Camellia_256::clear() {
   zap(m_SK);
}

std::string Camellia_128::provider() const {
   return Camellia_F::provider();
}

std::string Camellia_192::provider() const {
   return Camellia_F::provider();
}

std::string Camellia_256::provider() const {
   return Camellia_F::provider();
}

size_t Camellia_128::parallelism() const {
   return Camellia_F::parallelism();
}

size_t Camellia_192::parallelism() const {
   return Camellia_F::parallelism();
}

size_t Camellia_256::parallelism() const {
   return Camellia_F::parallelism();
}

}  // namespace Botan
/*
* Block Cipher Cascade
* (C) 2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <numeric>

namespace Botan {

void Cascade_Cipher::encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   const size_t c1_blocks = blocks * (block_size() / m_cipher1->block_size());
   const size_t c2_blocks = blocks * (block_size() / m_cipher2->block_size());

   m_cipher1->encrypt_n(in, out, c1_blocks);
   m_cipher2->encrypt_n(out, out, c2_blocks);
}

void Cascade_Cipher::decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   const size_t c1_blocks = blocks * (block_size() / m_cipher1->block_size());
   const size_t c2_blocks = blocks * (block_size() / m_cipher2->block_size());

   m_cipher2->decrypt_n(in, out, c2_blocks);
   m_cipher1->decrypt_n(out, out, c1_blocks);
}

void Cascade_Cipher::key_schedule(std::span<const uint8_t> key) {
   BufferSlicer keys(key);

   m_cipher1->set_key(keys.take(m_cipher1->maximum_keylength()));
   m_cipher2->set_key(keys.take(m_cipher2->maximum_keylength()));
}

void Cascade_Cipher::clear() {
   m_cipher1->clear();
   m_cipher2->clear();
}

std::string Cascade_Cipher::name() const {
   return fmt("Cascade({},{})", m_cipher1->name(), m_cipher2->name());
}

bool Cascade_Cipher::has_keying_material() const {
   return m_cipher1->has_keying_material() && m_cipher2->has_keying_material();
}

std::unique_ptr<BlockCipher> Cascade_Cipher::new_object() const {
   return std::make_unique<Cascade_Cipher>(m_cipher1->new_object(), m_cipher2->new_object());
}

Cascade_Cipher::Cascade_Cipher(std::unique_ptr<BlockCipher> cipher1, std::unique_ptr<BlockCipher> cipher2) :
      m_cipher1(std::move(cipher1)),
      m_cipher2(std::move(cipher2)),
      m_block_size(std::lcm(m_cipher1->block_size(), m_cipher2->block_size())) {
   BOTAN_ASSERT(m_block_size % m_cipher1->block_size() == 0 && m_block_size % m_cipher2->block_size() == 0,
                "Combined block size is a multiple of each ciphers block");
}

}  // namespace Botan
/*
* Runtime CPU detection
* (C) 2009,2010,2013,2017,2023 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_OS_UTILS)
#endif

namespace Botan {

#if !defined(BOTAN_HAS_CPUID_DETECTION)
uint32_t CPUFeature::as_u32() const {
   throw Invalid_State("CPUFeature invalid bit");
}

std::optional<CPUFeature> CPUFeature::from_string(std::string_view) {
   return {};
}

std::string CPUFeature::to_string() const {
   throw Invalid_State("CPUFeature invalid bit");
}
#endif

//static
std::string CPUID::to_string() {
   std::vector<std::string> flags;

   const uint32_t bitset = state().bitset();

   for(size_t i = 0; i != 32; ++i) {
      const uint32_t b = static_cast<uint32_t>(1) << i;
      if((bitset & b) == b) {
         // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
         flags.push_back(CPUFeature(static_cast<CPUFeature::Bit>(b)).to_string());
      }
   }

   return string_join(flags, ' ');
}

//static
void CPUID::initialize() {
   state() = CPUID_Data();
}

#if defined(BOTAN_HAS_CPUID_DETECTION)

namespace {

uint32_t cleared_cpuid_bits() {
   uint32_t cleared = 0;

   #if defined(BOTAN_HAS_OS_UTILS)
   std::string clear_cpuid_env;
   if(OS::read_env_variable(clear_cpuid_env, "BOTAN_CLEAR_CPUID")) {
      for(const auto& cpuid : split_on(clear_cpuid_env, ',')) {
         if(auto bit = CPUID::bit_from_string(cpuid)) {
            cleared |= bit->as_u32();
         }
      }
   }
   #endif

   return cleared;
}

}  // namespace

#endif

CPUID::CPUID_Data::CPUID_Data() {
   // NOLINTBEGIN(*-prefer-member-initializer)
#if defined(BOTAN_HAS_CPUID_DETECTION)
   m_processor_features = detect_cpu_features(~cleared_cpuid_bits());
#else
   m_processor_features = 0;
#endif
   // NOLINTEND(*-prefer-member-initializer)
}

std::optional<CPUFeature> CPUID::bit_from_string(std::string_view tok) {
   return CPUFeature::from_string(tok);
}

}  // namespace Botan
/*
* Runtime CPU detection for Aarch64
* (C) 2009,2010,2013,2017,2020,2024 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <optional>

#if defined(BOTAN_HAS_OS_UTILS)
#endif

#if defined(BOTAN_TARGET_OS_HAS_SYSCTLBYNAME)
   #include <sys/sysctl.h>
   #include <sys/types.h>
#endif

namespace Botan {

namespace {

std::optional<uint32_t> aarch64_feat_via_auxval(uint32_t allowed) {
#if defined(BOTAN_HAS_OS_UTILS)

   if(auto auxval = OS::get_auxval_hwcap()) {
      uint32_t feat = 0;

      /*
      * On systems with getauxval these bits should normally be defined
      * in bits/auxv.h but some buggy? glibc installs seem to miss them.
      * These following values are all fixed, for the Linux ELF format,
      * so we just hardcode them in ARM_hwcap_bit enum.
      */
      enum class ARM_hwcap_bit : uint64_t /* NOLINT(*-enum-size) */ {
         NEON_bit = (1 << 1),
         AES_bit = (1 << 3),
         PMULL_bit = (1 << 4),
         SHA1_bit = (1 << 5),
         SHA2_bit = (1 << 6),
         SHA3_bit = (1 << 17),
         SM3_bit = (1 << 18),
         SM4_bit = (1 << 19),
         SHA2_512_bit = (1 << 21),
         SVE_bit = (1 << 22),
      };

      const auto hwcap = auxval->first;

      feat |= CPUID::if_set(hwcap, ARM_hwcap_bit::NEON_bit, CPUFeature::Bit::NEON, allowed);

      if((feat & CPUFeature::Bit::NEON) == CPUFeature::Bit::NEON) {
         feat |= CPUID::if_set(hwcap, ARM_hwcap_bit::AES_bit, CPUFeature::Bit::AES, allowed);
         feat |= CPUID::if_set(hwcap, ARM_hwcap_bit::PMULL_bit, CPUFeature::Bit::PMULL, allowed);
         feat |= CPUID::if_set(hwcap, ARM_hwcap_bit::SHA1_bit, CPUFeature::Bit::SHA1, allowed);
         feat |= CPUID::if_set(hwcap, ARM_hwcap_bit::SHA2_bit, CPUFeature::Bit::SHA2, allowed);
         feat |= CPUID::if_set(hwcap, ARM_hwcap_bit::SHA3_bit, CPUFeature::Bit::SHA3, allowed);
         feat |= CPUID::if_set(hwcap, ARM_hwcap_bit::SM3_bit, CPUFeature::Bit::SM3, allowed);
         feat |= CPUID::if_set(hwcap, ARM_hwcap_bit::SM4_bit, CPUFeature::Bit::SM4, allowed);
         feat |= CPUID::if_set(hwcap, ARM_hwcap_bit::SHA2_512_bit, CPUFeature::Bit::SHA2_512, allowed);
         feat |= CPUID::if_set(hwcap, ARM_hwcap_bit::SVE_bit, CPUFeature::Bit::SVE, allowed);
      }

      return feat;
   }
#else
   BOTAN_UNUSED(allowed);
#endif

   return {};
}

std::optional<uint32_t> aarch64_feat_using_mac_api(uint32_t allowed) {
#if defined(BOTAN_TARGET_OS_IS_IOS) || defined(BOTAN_TARGET_OS_IS_MACOS)
   uint32_t feat = 0;

   auto sysctlbyname_has_feature = [](const char* feature_name) -> bool {
      unsigned int feature;
      size_t size = sizeof(feature);
      ::sysctlbyname(feature_name, &feature, &size, nullptr, 0);
      return (feature == 1);
   };

   // All 64-bit Apple ARM chips have NEON, AES, and SHA support
   feat |= CPUFeature::Bit::NEON & allowed;
   if((feat & CPUFeature::Bit::NEON) == CPUFeature::Bit::NEON) {
      feat |= CPUFeature::Bit::AES & allowed;
      feat |= CPUFeature::Bit::PMULL & allowed;
      feat |= CPUFeature::Bit::SHA1 & allowed;
      feat |= CPUFeature::Bit::SHA2 & allowed;

      if(sysctlbyname_has_feature("hw.optional.armv8_2_sha3")) {
         feat |= CPUFeature::Bit::SHA3 & allowed;
      }
      if(sysctlbyname_has_feature("hw.optional.armv8_2_sha512")) {
         feat |= CPUFeature::Bit::SHA2_512 & allowed;
      }
   }

   return feat;
#else
   BOTAN_UNUSED(allowed);
   return {};
#endif
}

std::optional<uint32_t> aarch64_feat_using_instr_probe(uint32_t allowed) {
#if defined(BOTAN_USE_GCC_INLINE_ASM) && defined(BOTAN_HAS_OS_UTILS)

   // NOLINTBEGIN(*-no-assembler)

   /*
   No getauxval API available, fall back on probe functions.
   NEON registers v0-v7 are caller saved in Aarch64
   */

   auto neon_probe = []() noexcept -> int {
      asm("and v0.16b, v0.16b, v0.16b");
      return 1;
   };
   auto aes_probe = []() noexcept -> int {
      asm(".word 0x4e284800");
      return 1;
   };
   auto pmull_probe = []() noexcept -> int {
      asm(".word 0x0ee0e000");
      return 1;
   };
   auto sha1_probe = []() noexcept -> int {
      asm(".word 0x5e280800");
      return 1;
   };
   auto sha2_probe = []() noexcept -> int {
      asm(".word 0x5e282800");
      return 1;
   };
   auto sha512_probe = []() noexcept -> int {
      asm(".long 0xcec08000");
      return 1;
   };

   // NOLINTEND(*-no-assembler)

   uint32_t feat = 0;
   if((allowed & CPUFeature::Bit::NEON) == CPUFeature::Bit::NEON) {
      if(OS::run_cpu_instruction_probe(neon_probe) == 1) {
         feat |= CPUFeature::Bit::NEON;

         if(OS::run_cpu_instruction_probe(aes_probe) == 1) {
            feat |= CPUFeature::Bit::AES & allowed;
         }
         if(OS::run_cpu_instruction_probe(pmull_probe) == 1) {
            feat |= CPUFeature::Bit::PMULL & allowed;
         }
         if(OS::run_cpu_instruction_probe(sha1_probe) == 1) {
            feat |= CPUFeature::Bit::SHA1 & allowed;
         }
         if(OS::run_cpu_instruction_probe(sha2_probe) == 1) {
            feat |= CPUFeature::Bit::SHA2 & allowed;
         }
         if(OS::run_cpu_instruction_probe(sha512_probe) == 1) {
            feat |= CPUFeature::Bit::SHA2_512 & allowed;
         }
      }
   }

   return feat;
#else
   BOTAN_UNUSED(allowed);
   return {};
#endif
}

}  // namespace

uint32_t CPUID::CPUID_Data::detect_cpu_features(uint32_t allowed) {
   if(auto feat_aux = aarch64_feat_via_auxval(allowed)) {
      return feat_aux.value();
   } else if(auto feat_mac = aarch64_feat_using_mac_api(allowed)) {
      return feat_mac.value();
   } else if(auto feat_instr = aarch64_feat_using_instr_probe(allowed)) {
      return feat_instr.value();
   } else {
      return 0;
   }
}

}  // namespace Botan
/**
* (C) 2025 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

std::string CPUFeature::to_string() const {
   switch(m_bit) {
      case CPUFeature::Bit::NEON:
         return "neon";
      case CPUFeature::Bit::SVE:
         return "sve";
      case CPUFeature::Bit::SHA1:
         return "armv8sha1";
      case CPUFeature::Bit::SHA2:
         return "armv8sha2";
      case CPUFeature::Bit::AES:
         return "armv8aes";
      case CPUFeature::Bit::PMULL:
         return "armv8pmull";
      case CPUFeature::Bit::SHA3:
         return "armv8sha3";
      case CPUFeature::Bit::SHA2_512:
         return "armv8sha2_512";
      case CPUFeature::Bit::SM3:
         return "armv8sm3";
      case CPUFeature::Bit::SM4:
         return "armv8sm4";
   }
   throw Invalid_State("CPUFeature invalid bit");
}

//static
std::optional<CPUFeature> CPUFeature::from_string(std::string_view tok) {
   // TODO(Botan4) remove the "arm_xxx" strings here
   if(tok == "neon" || tok == "simd") {
      return CPUFeature::Bit::NEON;
   } else if(tok == "sve" || tok == "arm_sve") {
      return CPUFeature::Bit::SVE;
   } else if(tok == "armv8sha1" || tok == "arm_sha1") {
      return CPUFeature::Bit::SHA1;
   } else if(tok == "armv8sha2" || tok == "arm_sha2") {
      return CPUFeature::Bit::SHA2;
   } else if(tok == "armv8aes" || tok == "arm_aes") {
      return CPUFeature::Bit::AES;
   } else if(tok == "armv8pmull" || tok == "arm_pmull") {
      return CPUFeature::Bit::PMULL;
   } else if(tok == "armv8sha3" || tok == "arm_sha3") {
      return CPUFeature::Bit::SHA3;
   } else if(tok == "armv8sha2_512" || tok == "arm_sha2_512") {
      return CPUFeature::Bit::SHA2_512;
   } else if(tok == "armv8sm3" || tok == "arm_sm3") {
      return CPUFeature::Bit::SM3;
   } else if(tok == "armv8sm4" || tok == "arm_sm4") {
      return CPUFeature::Bit::SM4;
   } else {
      return {};
   }
}

}  // namespace Botan
/*
* Hash Functions
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_ADLER32)
#endif

#if defined(BOTAN_HAS_ASCON_HASH256)
#endif

#if defined(BOTAN_HAS_CRC24)
#endif

#if defined(BOTAN_HAS_CRC32)
#endif

#if defined(BOTAN_HAS_GOST_34_11)
#endif

#if defined(BOTAN_HAS_KECCAK)
#endif

#if defined(BOTAN_HAS_MD4)
#endif

#if defined(BOTAN_HAS_MD5)
#endif

#if defined(BOTAN_HAS_RIPEMD_160)
#endif

#if defined(BOTAN_HAS_SHA1)
#endif

#if defined(BOTAN_HAS_SHA2_32)
#endif

#if defined(BOTAN_HAS_SHA2_64)
#endif

#if defined(BOTAN_HAS_SHA3)
#endif

#if defined(BOTAN_HAS_SHAKE)
#endif

#if defined(BOTAN_HAS_SKEIN_512)
#endif

#if defined(BOTAN_HAS_STREEBOG)
#endif

#if defined(BOTAN_HAS_SM3)
#endif

#if defined(BOTAN_HAS_WHIRLPOOL)
#endif

#if defined(BOTAN_HAS_PARALLEL_HASH)
#endif

#if defined(BOTAN_HAS_TRUNCATED_HASH)
#endif

#if defined(BOTAN_HAS_COMB4P)
#endif

#if defined(BOTAN_HAS_BLAKE2B)
#endif

#if defined(BOTAN_HAS_BLAKE2S)
#endif

#if defined(BOTAN_HAS_COMMONCRYPTO)
#endif

namespace Botan {

std::unique_ptr<HashFunction> HashFunction::create(std::string_view algo_spec, std::string_view provider) {
#if defined(BOTAN_HAS_COMMONCRYPTO)
   if(provider.empty() || provider == "commoncrypto") {
      if(auto hash = make_commoncrypto_hash(algo_spec))
         return hash;

      if(!provider.empty())
         return nullptr;
   }
#endif

   if(provider.empty() == false && provider != "base") {
      return nullptr;  // unknown provider
   }

#if defined(BOTAN_HAS_SHA1)
   if(algo_spec == "SHA-1") {
      return std::make_unique<SHA_1>();
   }
#endif

#if defined(BOTAN_HAS_SHA2_32)
   if(algo_spec == "SHA-224") {
      return std::make_unique<SHA_224>();
   }

   if(algo_spec == "SHA-256") {
      return std::make_unique<SHA_256>();
   }
#endif

#if defined(BOTAN_HAS_SHA2_64)
   if(algo_spec == "SHA-384") {
      return std::make_unique<SHA_384>();
   }

   if(algo_spec == "SHA-512") {
      return std::make_unique<SHA_512>();
   }

   if(algo_spec == "SHA-512-256") {
      return std::make_unique<SHA_512_256>();
   }
#endif

#if defined(BOTAN_HAS_RIPEMD_160)
   if(algo_spec == "RIPEMD-160") {
      return std::make_unique<RIPEMD_160>();
   }
#endif

#if defined(BOTAN_HAS_WHIRLPOOL)
   if(algo_spec == "Whirlpool") {
      return std::make_unique<Whirlpool>();
   }
#endif

#if defined(BOTAN_HAS_MD5)
   if(algo_spec == "MD5") {
      return std::make_unique<MD5>();
   }
#endif

#if defined(BOTAN_HAS_MD4)
   if(algo_spec == "MD4") {
      return std::make_unique<MD4>();
   }
#endif

#if defined(BOTAN_HAS_GOST_34_11)
   if(algo_spec == "GOST-R-34.11-94" || algo_spec == "GOST-34.11") {
      return std::make_unique<GOST_34_11>();
   }
#endif

#if defined(BOTAN_HAS_ADLER32)
   if(algo_spec == "Adler32") {
      return std::make_unique<Adler32>();
   }
#endif

#if defined(BOTAN_HAS_ASCON_HASH256)
   if(algo_spec == "Ascon-Hash256") {
      return std::make_unique<Ascon_Hash256>();
   }
#endif

#if defined(BOTAN_HAS_CRC24)
   if(algo_spec == "CRC24") {
      return std::make_unique<CRC24>();
   }
#endif

#if defined(BOTAN_HAS_CRC32)
   if(algo_spec == "CRC32") {
      return std::make_unique<CRC32>();
   }
#endif

#if defined(BOTAN_HAS_STREEBOG)
   if(algo_spec == "Streebog-256") {
      return std::make_unique<Streebog>(256);
   }
   if(algo_spec == "Streebog-512") {
      return std::make_unique<Streebog>(512);
   }
#endif

#if defined(BOTAN_HAS_SM3)
   if(algo_spec == "SM3") {
      return std::make_unique<SM3>();
   }
#endif

   const SCAN_Name req(algo_spec);

#if defined(BOTAN_HAS_SKEIN_512)
   if(req.algo_name() == "Skein-512") {
      return std::make_unique<Skein_512>(req.arg_as_integer(0, 512), req.arg(1, ""));
   }
#endif

#if defined(BOTAN_HAS_BLAKE2B)
   if(req.algo_name() == "Blake2b" || req.algo_name() == "BLAKE2b") {
      return std::make_unique<BLAKE2b>(req.arg_as_integer(0, 512));
   }
#endif

#if defined(BOTAN_HAS_BLAKE2S)
   if(req.algo_name() == "Blake2s" || req.algo_name() == "BLAKE2s") {
      return std::make_unique<BLAKE2s>(req.arg_as_integer(0, 256));
   }
#endif

#if defined(BOTAN_HAS_KECCAK)
   if(req.algo_name() == "Keccak-1600") {
      return std::make_unique<Keccak_1600>(req.arg_as_integer(0, 512));
   }
#endif

#if defined(BOTAN_HAS_SHA3)
   if(req.algo_name() == "SHA-3") {
      return std::make_unique<SHA_3>(req.arg_as_integer(0, 512));
   }
#endif

#if defined(BOTAN_HAS_SHAKE)
   if(req.algo_name() == "SHAKE-128" && req.arg_count() == 1) {
      return std::make_unique<SHAKE_128>(req.arg_as_integer(0));
   }
   if(req.algo_name() == "SHAKE-256" && req.arg_count() == 1) {
      return std::make_unique<SHAKE_256>(req.arg_as_integer(0));
   }
#endif

#if defined(BOTAN_HAS_PARALLEL_HASH)
   if(req.algo_name() == "Parallel") {
      std::vector<std::unique_ptr<HashFunction>> hashes;

      for(size_t i = 0; i != req.arg_count(); ++i) {
         auto h = HashFunction::create(req.arg(i));
         if(!h) {
            return nullptr;
         }
         hashes.push_back(std::move(h));
      }

      return std::make_unique<Parallel>(hashes);
   }
#endif

#if defined(BOTAN_HAS_TRUNCATED_HASH)
   if(req.algo_name() == "Truncated" && req.arg_count() == 2) {
      auto hash = HashFunction::create(req.arg(0));
      if(!hash) {
         return nullptr;
      }

      return std::make_unique<Truncated_Hash>(std::move(hash), req.arg_as_integer(1));
   }
#endif

#if defined(BOTAN_HAS_COMB4P)
   if(req.algo_name() == "Comb4P" && req.arg_count() == 2) {
      auto h1 = HashFunction::create(req.arg(0));
      auto h2 = HashFunction::create(req.arg(1));

      if(h1 && h2) {
         return std::make_unique<Comb4P>(std::move(h1), std::move(h2));
      }
   }
#endif

   return nullptr;
}

//static
std::unique_ptr<HashFunction> HashFunction::create_or_throw(std::string_view algo, std::string_view provider) {
   if(auto hash = HashFunction::create(algo, provider)) {
      return hash;
   }
   throw Lookup_Error("Hash", algo, provider);
}

std::vector<std::string> HashFunction::providers(std::string_view algo_spec) {
   return probe_providers_of<HashFunction>(algo_spec, {"base", "commoncrypto"});
}

}  // namespace Botan
/*
* Hex Encoding and Decoding
* (C) 2010,2020 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

namespace {

uint16_t hex_encode_2nibble(uint8_t n8, bool uppercase) {
   // Offset for upper or lower case 'a' resp
   const uint16_t a_mask = uppercase ? 0x0707 : 0x2727;

   const uint16_t n = (static_cast<uint16_t>(n8 & 0xF0) << 4) | (n8 & 0x0F);
   // n >= 10? If so add offset
   const uint16_t diff = swar_lt<uint16_t>(0x0909, n) & a_mask;
   // Can't overflow between bytes, so don't need explicit SWAR addition:
   return n + 0x3030 + diff;
}

}  // namespace

void hex_encode(char output[], const uint8_t input[], size_t input_length, bool uppercase) {
   for(size_t i = 0; i != input_length; ++i) {
      const uint16_t h = hex_encode_2nibble(input[i], uppercase);
      output[2 * i] = get_byte<0>(h);
      output[2 * i + 1] = get_byte<1>(h);
   }
}

std::string hex_encode(const uint8_t input[], size_t input_length, bool uppercase) {
   std::string output(2 * input_length, 0);

   if(input_length > 0) {
      hex_encode(&output.front(), input, input_length, uppercase);
   }

   return output;
}

namespace {

uint8_t hex_char_to_bin(char input) {
   // Starts of valid value ranges (v_lo) and their lengths (v_range)
   constexpr uint64_t v_lo = make_uint64(0, '0', 'a', 'A', ' ', '\n', '\t', '\r');
   constexpr uint64_t v_range = make_uint64(0, 10, 6, 6, 1, 1, 1, 1);

   const uint8_t x = static_cast<uint8_t>(input);
   const uint64_t x8 = x * 0x0101010101010101;

   const uint64_t v_mask = swar_in_range<uint64_t>(x8, v_lo, v_range) ^ 0x8000000000000000;

   // This is the offset added to x to get the value we need
   const uint64_t val_v = 0xd0a9c960767773 ^ static_cast<uint64_t>(0xFF - x) << 56;

   return x + static_cast<uint8_t>(val_v >> (8 * index_of_first_set_byte(v_mask)));
}

}  // namespace

size_t hex_decode(uint8_t output[], const char input[], size_t input_length, size_t& input_consumed, bool ignore_ws) {
   uint8_t* out_ptr = output;
   bool top_nibble = true;

   clear_mem(output, input_length / 2);

   for(size_t i = 0; i != input_length; ++i) {
      const uint8_t bin = hex_char_to_bin(input[i]);

      if(bin >= 0x10) {
         if(bin == 0x80 && ignore_ws) {
            continue;
         }

         throw Invalid_Argument(fmt("hex_decode: invalid character '{}'", format_char_for_display(input[i])));
      }

      if(top_nibble) {
         *out_ptr |= bin << 4;
      } else {
         *out_ptr |= bin;
      }

      top_nibble = !top_nibble;
      if(top_nibble) {
         ++out_ptr;
      }
   }

   input_consumed = input_length;
   const size_t written = (out_ptr - output);

   /*
   * We only got half of a uint8_t at the end; zap the half-written
   * output and mark it as unread
   */
   if(!top_nibble) {
      *out_ptr = 0;
      input_consumed -= 1;
   }

   return written;
}

size_t hex_decode(uint8_t output[], const char input[], size_t input_length, bool ignore_ws) {
   size_t consumed = 0;
   const size_t written = hex_decode(output, input, input_length, consumed, ignore_ws);

   if(consumed != input_length) {
      throw Invalid_Argument("hex_decode: input did not have full bytes");
   }

   return written;
}

size_t hex_decode(uint8_t output[], std::string_view input, bool ignore_ws) {
   return hex_decode(output, input.data(), input.length(), ignore_ws);
}

size_t hex_decode(std::span<uint8_t> output, std::string_view input, bool ignore_ws) {
   return hex_decode(output.data(), input.data(), input.length(), ignore_ws);
}

secure_vector<uint8_t> hex_decode_locked(const char input[], size_t input_length, bool ignore_ws) {
   secure_vector<uint8_t> bin(1 + input_length / 2);

   const size_t written = hex_decode(bin.data(), input, input_length, ignore_ws);

   bin.resize(written);
   return bin;
}

secure_vector<uint8_t> hex_decode_locked(std::string_view input, bool ignore_ws) {
   return hex_decode_locked(input.data(), input.size(), ignore_ws);
}

std::vector<uint8_t> hex_decode(const char input[], size_t input_length, bool ignore_ws) {
   std::vector<uint8_t> bin(1 + input_length / 2);

   const size_t written = hex_decode(bin.data(), input, input_length, ignore_ws);

   bin.resize(written);
   return bin;
}

std::vector<uint8_t> hex_decode(std::string_view input, bool ignore_ws) {
   return hex_decode(input.data(), input.size(), ignore_ws);
}

}  // namespace Botan
/*
* HMAC
* (C) 1999-2007,2014,2020 Jack Lloyd
*     2007 Yves Jerschow
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

/*
* Update a HMAC Calculation
*/
void HMAC::add_data(std::span<const uint8_t> input) {
   assert_key_material_set();
   m_hash->update(input);
}

/*
* Finalize a HMAC Calculation
*/
void HMAC::final_result(std::span<uint8_t> mac) {
   assert_key_material_set();
   m_hash->final(mac);
   m_hash->update(m_okey);
   m_hash->update(mac.first(m_hash_output_length));
   m_hash->final(mac);
   m_hash->update(m_ikey);
}

Key_Length_Specification HMAC::key_spec() const {
   // Support very long lengths for things like PBKDF2 and the TLS PRF
   return Key_Length_Specification(0, 8192);
}

size_t HMAC::output_length() const {
   return m_hash_output_length;
}

bool HMAC::has_keying_material() const {
   return !m_okey.empty();
}

/*
* HMAC Key Schedule
*/
void HMAC::key_schedule(std::span<const uint8_t> key) {
   const uint8_t ipad = 0x36;
   const uint8_t opad = 0x5C;

   m_hash->clear();

   m_ikey.resize(m_hash_block_size);
   m_okey.resize(m_hash_block_size);

   clear_mem(m_ikey.data(), m_ikey.size());
   clear_mem(m_okey.data(), m_okey.size());

   /*
   * Sometimes the HMAC key length itself is sensitive, as with PBKDF2 where it
   * reveals the length of the passphrase. Make some attempt to hide this to
   * side channels. Clearly if the secret is longer than the block size then the
   * branch to hash first reveals that. In addition, counting the number of
   * compression functions executed reveals the size at the granularity of the
   * hash function's block size.
   *
   * The greater concern is for smaller keys; being able to detect when a
   * passphrase is say 4 bytes may assist choosing weaker targets. Even though
   * the loop bounds are constant, we can only actually read key[0..length] so
   * it doesn't seem possible to make this computation truly constant time.
   *
   * We don't mind leaking if the length is exactly zero since that's
   * trivial to simply check.
   */

   if(key.size() > m_hash_block_size) {
      m_hash->update(key);
      m_hash->final(m_ikey.data());
   } else if(key.size() >= 20) {
      // For long keys we just leak the length either it is a cryptovariable
      // or a long enough password that just the length is not a useful signal
      copy_mem(std::span{m_ikey}.first(key.size()), key);
   } else if(!key.empty()) {
      for(size_t i = 0, i_mod_length = 0; i != m_hash_block_size; ++i) {
         /*
         access key[i % length] but avoiding division due to variable
         time computation on some processors.
         */
         auto needs_reduction = CT::Mask<size_t>::is_lte(key.size(), i_mod_length);
         i_mod_length = needs_reduction.select(0, i_mod_length);
         const uint8_t kb = key[i_mod_length];

         auto in_range = CT::Mask<size_t>::is_lt(i, key.size());
         m_ikey[i] = static_cast<uint8_t>(in_range.if_set_return(kb));
         i_mod_length += 1;
      }
   }

   for(size_t i = 0; i != m_hash_block_size; ++i) {
      m_ikey[i] ^= ipad;
      m_okey[i] = m_ikey[i] ^ ipad ^ opad;
   }

   m_hash->update(m_ikey);
}

/*
* Clear memory of sensitive data
*/
void HMAC::clear() {
   m_hash->clear();
   zap(m_ikey);
   zap(m_okey);
}

/*
* Return the name of this type
*/
std::string HMAC::name() const {
   return fmt("HMAC({})", m_hash->name());
}

/*
* Return a new_object of this object
*/
std::unique_ptr<MessageAuthenticationCode> HMAC::new_object() const {
   return std::make_unique<HMAC>(m_hash->new_object());
}

/*
* HMAC Constructor
*/
HMAC::HMAC(std::unique_ptr<HashFunction> hash) :
      m_hash(std::move(hash)),
      m_hash_output_length(m_hash->output_length()),
      m_hash_block_size(m_hash->hash_block_size()) {
   BOTAN_ARG_CHECK(m_hash_block_size >= m_hash_output_length, "HMAC is not compatible with this hash function");
}

}  // namespace Botan
/*
* HMAC_DRBG
* (C) 2014,2015,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

namespace {

size_t hmac_drbg_security_level(size_t mac_output_length) {
   // security strength of the hash function
   // for pre-image resistance (see NIST SP 800-57)
   // SHA-1: 128 bits
   // SHA-224, SHA-512/224: 192 bits,
   // SHA-256, SHA-512/256, SHA-384, SHA-512: >= 256 bits
   // NIST SP 800-90A only supports up to 256 bits though

   if(mac_output_length < 20) {
      throw Invalid_Argument(fmt("HMAC_DRBG MAC output length {} is too small", mac_output_length));
   }

   if(mac_output_length < 32) {
      return (mac_output_length - 4) * 8;
   } else {
      return 32 * 8;
   }
}

void check_limits(size_t reseed_interval, size_t max_number_of_bytes_per_request) {
   // SP800-90A permits up to 2^48, but it is not usable on 32 bit
   // platforms, so we only allow up to 2^24, which is still reasonably high
   if(reseed_interval == 0 || reseed_interval > static_cast<size_t>(1) << 24) {
      throw Invalid_Argument("Invalid value for reseed_interval");
   }

   if(max_number_of_bytes_per_request == 0 || max_number_of_bytes_per_request > 64 * 1024) {
      throw Invalid_Argument("Invalid value for max_number_of_bytes_per_request");
   }
}

template <typename T>
std::unique_ptr<T> check_not_null(std::unique_ptr<T> obj) {
   BOTAN_ARG_CHECK(obj != nullptr, "Argument must not be null");
   return obj;
}

}  // namespace

HMAC_DRBG::~HMAC_DRBG() = default;

HMAC_DRBG::HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf,
                     RandomNumberGenerator& underlying_rng,
                     size_t reseed_interval,
                     size_t max_number_of_bytes_per_request) :
      Stateful_RNG(underlying_rng, reseed_interval),
      m_mac(check_not_null(std::move(prf))),
      m_max_number_of_bytes_per_request(max_number_of_bytes_per_request),
      m_security_level(hmac_drbg_security_level(m_mac->output_length())) {
   check_limits(reseed_interval, max_number_of_bytes_per_request);

   clear();
}

HMAC_DRBG::HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf,
                     RandomNumberGenerator& underlying_rng,
                     Entropy_Sources& entropy_sources,
                     size_t reseed_interval,
                     size_t max_number_of_bytes_per_request) :
      Stateful_RNG(underlying_rng, entropy_sources, reseed_interval),
      m_mac(check_not_null(std::move(prf))),
      m_max_number_of_bytes_per_request(max_number_of_bytes_per_request),
      m_security_level(hmac_drbg_security_level(m_mac->output_length())) {
   check_limits(reseed_interval, max_number_of_bytes_per_request);

   clear();
}

HMAC_DRBG::HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf,
                     Entropy_Sources& entropy_sources,
                     size_t reseed_interval,
                     size_t max_number_of_bytes_per_request) :
      Stateful_RNG(entropy_sources, reseed_interval),
      m_mac(check_not_null(std::move(prf))),
      m_max_number_of_bytes_per_request(max_number_of_bytes_per_request),
      m_security_level(hmac_drbg_security_level(m_mac->output_length())) {
   check_limits(reseed_interval, max_number_of_bytes_per_request);

   clear();
}

HMAC_DRBG::HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf) :
      m_mac(check_not_null(std::move(prf))),
      m_max_number_of_bytes_per_request(64 * 1024),
      m_security_level(hmac_drbg_security_level(m_mac->output_length())) {
   clear();
}

HMAC_DRBG::HMAC_DRBG(std::string_view hmac_hash) :
      m_mac(MessageAuthenticationCode::create_or_throw(fmt("HMAC({})", hmac_hash))),
      m_max_number_of_bytes_per_request(64 * 1024),
      m_security_level(hmac_drbg_security_level(m_mac->output_length())) {
   clear();
}

void HMAC_DRBG::clear_state() {
   if(m_V.empty()) {
      const size_t output_length = m_mac->output_length();
      m_V.resize(output_length);
      m_T.resize(output_length);
   }

   std::fill(m_V.begin(), m_V.end(), 0x01);
   m_mac->set_key(std::vector<uint8_t>(m_V.size(), 0x00));
}

std::string HMAC_DRBG::name() const {
   return fmt("HMAC_DRBG({})", m_mac->name());
}

/*
* HMAC_DRBG generation
* See NIST SP800-90A section 10.1.2.5
*/
void HMAC_DRBG::generate_output(std::span<uint8_t> output, std::span<const uint8_t> input) {
   // This is an internal function, callers should have validated this beforehand
   BOTAN_ASSERT_NOMSG(!output.empty());

   if(!input.empty()) {
      update(input);
   }

   while(!output.empty()) {
      const size_t to_copy = std::min(output.size(), m_V.size());
      m_mac->update(m_V);
      m_mac->final(m_V);
      copy_mem(output.data(), m_V.data(), to_copy);

      output = output.subspan(to_copy);
   }

   update(input);
}

/*
* Reset V and the mac key with new values
* See NIST SP800-90A section 10.1.2.2
*/
void HMAC_DRBG::update(std::span<const uint8_t> input) {
   m_mac->update(m_V);
   m_mac->update(0x00);
   if(!input.empty()) {
      m_mac->update(input);
   }
   m_mac->final(m_T);
   m_mac->set_key(m_T);

   m_mac->update(m_V);
   m_mac->final(m_V);

   if(!input.empty()) {
      m_mac->update(m_V);
      m_mac->update(0x01);
      m_mac->update(input);
      m_mac->final(m_T);
      m_mac->set_key(m_T);

      m_mac->update(m_V);
      m_mac->final(m_V);
   }
}

size_t HMAC_DRBG::security_level() const {
   return m_security_level;
}
}  // namespace Botan
/*
 * Helper functions to implement Keccak-derived functions from NIST SP.800-185
 * (C) 2023 Jack Lloyd
 * (C) 2023 René Meusel - Rohde & Schwarz Cybersecurity
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 */




namespace Botan {

namespace {

size_t int_encoding_size(uint64_t x) {
   BOTAN_ASSERT_NOMSG(x < std::numeric_limits<uint64_t>::max());
   return ceil_tobytes(std::max(uint8_t(1), ceil_log2(x + 1)));
}

uint8_t encode(std::span<uint8_t> out, uint64_t x) {
   const auto bytes_needed = int_encoding_size(x);
   BOTAN_ASSERT_NOMSG(sizeof(x) >= bytes_needed);
   BOTAN_ASSERT_NOMSG(out.size() >= bytes_needed);

   const size_t leading_zeros = sizeof(x) - bytes_needed;

   std::array<uint8_t, sizeof(x)> bigendian_x{};
   store_be(x, bigendian_x.data());

   std::copy(bigendian_x.begin() + leading_zeros, bigendian_x.end(), out.begin());

   return static_cast<uint8_t>(bytes_needed);
}

}  // namespace

std::span<const uint8_t> keccak_int_left_encode(std::span<uint8_t> out, size_t x) {
   BOTAN_ASSERT_NOMSG(!out.empty());
   out[0] = encode(out.last(out.size() - 1), x);
   return out.first(out[0] + 1 /* the length tag */);
}

std::span<const uint8_t> keccak_int_right_encode(std::span<uint8_t> out, size_t x) {
   const auto bytes_needed = encode(out, x);
   BOTAN_ASSERT_NOMSG(out.size() >= bytes_needed + size_t(1));
   out[bytes_needed] = bytes_needed;
   return out.first(bytes_needed + 1 /* the length tag */);
}

size_t keccak_int_encoding_size(size_t x) {
   return int_encoding_size(x) + 1 /* the length tag */;
}

}  // namespace Botan
/*
* Keccak Permutation
* (C) 2010,2016 Jack Lloyd
* (C) 2023 Falko Strenzke
* (C) 2023,2025 René Meusel - Rohde & Schwarz Cybersecurity
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_CPUID)
#endif

namespace Botan {

std::string Keccak_Permutation::provider() const {
#if defined(BOTAN_HAS_KECCAK_PERM_AVX512)
   if(auto feat = CPUID::check(CPUID::Feature::AVX512)) {
      return *feat;
   }
#endif

#if defined(BOTAN_HAS_KECCAK_PERM_BMI2)
   if(auto feat = CPUID::check(CPUID::Feature::BMI)) {
      return *feat;
   }
#endif

   return "base";
}

void Keccak_Permutation::clear() {
   state() = {};
   reset_cursor();
}

void Keccak_Permutation::absorb(std::span<const uint8_t> input) {
   absorb_into_sponge(*this, input);
}

void Keccak_Permutation::squeeze(std::span<uint8_t> output) {
   squeeze_from_sponge(*this, output);
}

void Keccak_Permutation::finish() {
   // The padding for Keccak[c]-based functions spans the entire remaining
   // byterate until the next permute() call. At most that could be an entire
   // byterate. First are a few bits of "custom" padding defined by the using
   // function (e.g. SHA-3 uses "01"), then the remaining space is filled with
   // "pad10*1" (see NIST FIPS 202 Section 5.1) followed by a final permute().

   auto& S = state();

   // Apply the custom padding + the left-most 1-bit of "pad10*1" to the current
   // (partial) word of the sponge state

   const uint64_t start_of_padding = (m_padding.padding | uint64_t(1) << m_padding.bit_len);
   S[cursor() / word_bytes] ^= start_of_padding << (8 * (cursor() % word_bytes));

   // XOR'ing the 0-bits of "pad10*1" into the state is a NOOP

   // If the custom padding + the left-most 1-bit of "pad10*1" had resulted in a
   // byte-aligned "partial padding", the final 1-bit of of "pad10*1" could
   // potentially override parts of the already-appended "start_of_padding".
   // In case we ever introduce a Keccak-based function with such a need, we
   // have to modify this padding algorithm.
   BOTAN_DEBUG_ASSERT(m_padding.bit_len % 8 != 7);

   // Append the final bit of "pad10*1" into the last word of the input range
   S[(byte_rate() / word_bytes) - 1] ^= uint64_t(0x8000000000000000);

   // Perform the final permutation and reset the state cursor
   permute();
   reset_cursor();

   BOTAN_DEBUG_ASSERT(cursor() == 0);
}

void Keccak_Permutation::permute() {
#if defined(BOTAN_HAS_KECCAK_PERM_AVX512)
   if(CPUID::has(CPUID::Feature::AVX512)) {
      return permute_avx512();
   }
#endif

#if defined(BOTAN_HAS_KECCAK_PERM_BMI2)
   if(CPUID::has(CPUID::Feature::BMI)) {
      return permute_bmi2();
   }
#endif

   static const uint64_t RC[24] = {0x0000000000000001, 0x0000000000008082, 0x800000000000808A, 0x8000000080008000,
                                   0x000000000000808B, 0x0000000080000001, 0x8000000080008081, 0x8000000000008009,
                                   0x000000000000008A, 0x0000000000000088, 0x0000000080008009, 0x000000008000000A,
                                   0x000000008000808B, 0x800000000000008B, 0x8000000000008089, 0x8000000000008003,
                                   0x8000000000008002, 0x8000000000000080, 0x000000000000800A, 0x800000008000000A,
                                   0x8000000080008081, 0x8000000000008080, 0x0000000080000001, 0x8000000080008008};

   uint64_t T[25];

   for(size_t i = 0; i != 24; i += 2) {
      Keccak_Permutation_round(T, state().data(), RC[i + 0]);
      Keccak_Permutation_round(state().data(), T, RC[i + 1]);
   }
}

}  // namespace Botan
/*
* GOST R 34.12-2015: Block Cipher "Kuznyechik" (RFC 7801)
* (C) 2023 Richard Huveneers
*     2024 Jack Lloyd
*
* This code is written by kerukuro for cppcrypto library (http://cppcrypto.sourceforge.net/)
* and released into public domain.
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

namespace {

namespace Kuznyechik_F {

alignas(256) const constexpr uint8_t S[256] = {
   252, 238, 221, 17,  207, 110, 49,  22,  251, 196, 250, 218, 35,  197, 4,   77,  233, 119, 240, 219, 147, 46,
   153, 186, 23,  54,  241, 187, 20,  205, 95,  193, 249, 24,  101, 90,  226, 92,  239, 33,  129, 28,  60,  66,
   139, 1,   142, 79,  5,   132, 2,   174, 227, 106, 143, 160, 6,   11,  237, 152, 127, 212, 211, 31,  235, 52,
   44,  81,  234, 200, 72,  171, 242, 42,  104, 162, 253, 58,  206, 204, 181, 112, 14,  86,  8,   12,  118, 18,
   191, 114, 19,  71,  156, 183, 93,  135, 21,  161, 150, 41,  16,  123, 154, 199, 243, 145, 120, 111, 157, 158,
   178, 177, 50,  117, 25,  61,  255, 53,  138, 126, 109, 84,  198, 128, 195, 189, 13,  87,  223, 245, 36,  169,
   62,  168, 67,  201, 215, 121, 214, 246, 124, 34,  185, 3,   224, 15,  236, 222, 122, 148, 176, 188, 220, 232,
   40,  80,  78,  51,  10,  74,  167, 151, 96,  115, 30,  0,   98,  68,  26,  184, 56,  130, 100, 159, 38,  65,
   173, 69,  70,  146, 39,  94,  85,  47,  140, 163, 165, 125, 105, 213, 149, 59,  7,   88,  179, 64,  134, 172,
   29,  247, 48,  55,  107, 228, 136, 217, 231, 137, 225, 27,  131, 73,  76,  63,  248, 254, 141, 83,  170, 144,
   202, 216, 133, 97,  32,  113, 103, 164, 45,  43,  9,   91,  203, 155, 37,  208, 190, 229, 108, 82,  89,  166,
   116, 210, 230, 244, 180, 192, 209, 102, 175, 194, 57,  75,  99,  182};

alignas(256) const constexpr uint8_t IS[256] = {
   165, 45,  50,  143, 14,  48,  56,  192, 84,  230, 158, 57,  85,  126, 82,  145, 100, 3,   87,  90,  28,  96,
   7,   24,  33,  114, 168, 209, 41,  198, 164, 63,  224, 39,  141, 12,  130, 234, 174, 180, 154, 99,  73,  229,
   66,  228, 21,  183, 200, 6,   112, 157, 65,  117, 25,  201, 170, 252, 77,  191, 42,  115, 132, 213, 195, 175,
   43,  134, 167, 177, 178, 91,  70,  211, 159, 253, 212, 15,  156, 47,  155, 67,  239, 217, 121, 182, 83,  127,
   193, 240, 35,  231, 37,  94,  181, 30,  162, 223, 166, 254, 172, 34,  249, 226, 74,  188, 53,  202, 238, 120,
   5,   107, 81,  225, 89,  163, 242, 113, 86,  17,  106, 137, 148, 101, 140, 187, 119, 60,  123, 40,  171, 210,
   49,  222, 196, 95,  204, 207, 118, 44,  184, 216, 46,  54,  219, 105, 179, 20,  149, 190, 98,  161, 59,  22,
   102, 233, 92,  108, 109, 173, 55,  97,  75,  185, 227, 186, 241, 160, 133, 131, 218, 71,  197, 176, 51,  250,
   150, 111, 110, 194, 246, 80,  255, 93,  169, 142, 23,  27,  151, 125, 236, 88,  247, 31,  251, 124, 9,   13,
   122, 103, 69,  135, 220, 232, 79,  29,  78,  4,   235, 248, 243, 62,  61,  189, 138, 136, 221, 205, 11,  19,
   152, 2,   147, 128, 144, 208, 36,  52,  203, 237, 244, 206, 153, 16,  68,  64,  146, 58,  1,   38,  18,  26,
   72,  104, 245, 129, 139, 199, 214, 32,  10,  8,   0,   76,  215, 116};

namespace Kuznyechik_T {

const constexpr uint8_t LINEAR[16] = {
   0x94, 0x20, 0x85, 0x10, 0xC2, 0xC0, 0x01, 0xFB, 0x01, 0xC0, 0xC2, 0x10, 0x85, 0x20, 0x94, 0x01};

consteval std::array<uint8_t, 256> L_table(bool forward) noexcept {
   std::array<uint8_t, 256> L = {};

   for(size_t i = 0; i != 16; ++i) {
      L[i] = LINEAR[i];
      if(i > 0) {
         L[17 * i - 1] = 1;
      }
   }

   if(!forward) {
      // Reverse L
      for(size_t i = 0; i != 128; ++i) {
         std::swap(L[i], L[255 - i]);
      }
   }

   auto sqr_matrix = [](std::span<const uint8_t, 256> mat) {
      std::array<uint8_t, 256> res = {};
      for(size_t i = 0; i != 16; ++i) {
         for(size_t j = 0; j != 16; ++j) {
            for(size_t k = 0; k != 16; ++k) {
               res[16 * i + j] ^= poly_mul<0xC3>(mat[16 * i + k], mat[16 * k + j]);
            }
         }
      }
      return res;
   };

   for(size_t i = 0; i != 4; ++i) {
      L = sqr_matrix(L);
   }

   return L;
}

consteval std::array<uint64_t, 16 * 256 * 2> T_table(std::span<const uint8_t> L,
                                                     std::span<const uint8_t, 256> SB) noexcept {
   std::array<uint64_t, 16 * 256 * 2> T = {};

   for(size_t i = 0; i != 16; ++i) {
      uint64_t L_stride_0 = 0;
      uint64_t L_stride_1 = 0;
      for(size_t j = 0; j != 8; ++j) {
         L_stride_0 |= static_cast<uint64_t>(L[i + 16 * j]) << (8 * (j % 8));
         L_stride_1 |= static_cast<uint64_t>(L[i + 16 * (j + 8)]) << (8 * (j % 8));
      }

      for(size_t j = 0; j != 256; ++j) {
         const uint8_t Sj = SB[j];
         T[512 * i + 2 * j] = poly_mul<0xC3>(L_stride_0, Sj);
         T[512 * i + 2 * j + 1] = poly_mul<0xC3>(L_stride_1, Sj);
      }
   }

   return T;
}

}  // namespace Kuznyechik_T

// TODO(Botan4) this indirection with L/IL is required to work around a problem
// with Clang 19, where suddenly T_table became too much for it to handle as constexpr.
// Check if it's possible to remove this.
constexpr auto L = Kuznyechik_T::L_table(true);
constexpr auto IL = Kuznyechik_T::L_table(false);
const constinit auto T = Kuznyechik_T::T_table(L, S);
const constinit auto IT = Kuznyechik_T::T_table(IL, IS);

const uint64_t C[32][2] = {{0xb87a486c7276a26e, 0x019484dd10bd275d}, {0xb3f490d8e4ec87dc, 0x02ebcb7920b94eba},
                           {0x0b8ed8b4969a25b2, 0x037f4fa4300469e7}, {0xa52be3730b1bcd7b, 0x041555f240b19cb7},
                           {0x1d51ab1f796d6f15, 0x0581d12f500cbbea}, {0x16df73abeff74aa7, 0x06fe9e8b6008d20d},
                           {0xaea53bc79d81e8c9, 0x076a1a5670b5f550}, {0x895605e6163659f6, 0x082aaa2780a1fbad},
                           {0x312c4d8a6440fb98, 0x09be2efa901cdcf0}, {0x3aa2953ef2dade2a, 0x0ac1615ea018b517},
                           {0x82d8dd5280ac7c44, 0x0b55e583b0a5924a}, {0x2c7de6951d2d948d, 0x0c3fffd5c010671a},
                           {0x9407aef96f5b36e3, 0x0dab7b08d0ad4047}, {0x9f89764df9c11351, 0x0ed434ace0a929a0},
                           {0x27f33e218bb7b13f, 0x0f40b071f0140efd}, {0xd1ac0a0f2c6cb22f, 0x1054974ec3813599},
                           {0x69d642635e1a1041, 0x11c01393d33c12c4}, {0x62589ad7c88035f3, 0x12bf5c37e3387b23},
                           {0xda22d2bbbaf6979d, 0x132bd8eaf3855c7e}, {0x7487e97c27777f54, 0x1441c2bc8330a92e},
                           {0xccfda1105501dd3a, 0x15d54661938d8e73}, {0xc77379a4c39bf888, 0x16aa09c5a389e794},
                           {0x7f0931c8b1ed5ae6, 0x173e8d18b334c0c9}, {0x58fa0fe93a5aebd9, 0x187e3d694320ce34},
                           {0xe0804785482c49b7, 0x19eab9b4539de969}, {0xeb0e9f31deb66c05, 0x1a95f6106399808e},
                           {0x5374d75dacc0ce6b, 0x1b0172cd7324a7d3}, {0xfdd1ec9a314126a2, 0x1c6b689b03915283},
                           {0x45aba4f6433784cc, 0x1dffec46132c75de}, {0x4e257c42d5ada17e, 0x1e80a3e223281c39},
                           {0xf65f342ea7db0310, 0x1f14273f33953b64}, {0x619b141e58d8a75e, 0x20a8ed9c45c16af1}};

inline void LS(uint64_t& x1, uint64_t& x2) {
   uint64_t t1 = 0;
   uint64_t t2 = 0;
   for(size_t i = 0; i != 16; ++i) {
      const uint8_t x = get_byte_var(7 - (i % 8), (i < 8) ? x1 : x2);
      t1 ^= T[512 * i + 2 * x + 0];
      t2 ^= T[512 * i + 2 * x + 1];
   }

   x1 = t1;
   x2 = t2;
}

inline void ILS(uint64_t& x1, uint64_t& x2) {
   uint64_t t1 = 0;
   uint64_t t2 = 0;
   for(size_t i = 0; i != 16; ++i) {
      const uint8_t x = get_byte_var(7 - (i % 8), (i < 8) ? x1 : x2);
      t1 ^= IT[512 * i + 2 * x + 0];
      t2 ^= IT[512 * i + 2 * x + 1];
   }
   x1 = t1;
   x2 = t2;
}

inline void ILSS(uint64_t& x1, uint64_t& x2) {
   uint64_t t1 = 0;
   uint64_t t2 = 0;
   for(size_t i = 0; i != 16; ++i) {
      const uint8_t x = S[get_byte_var(7 - (i % 8), (i < 8) ? x1 : x2)];
      t1 ^= IT[512 * i + 2 * x + 0];
      t2 ^= IT[512 * i + 2 * x + 1];
   }
   x1 = t1;
   x2 = t2;
}

inline uint64_t ISI(uint64_t val) {
   uint64_t out = 0;
   for(size_t i = 0; i != 8; ++i) {
      out <<= 8;
      out |= IS[get_byte_var(i, val)];
   }
   return out;
}

}  // namespace Kuznyechik_F

}  // namespace

void Kuznyechik::clear() {
   zap(m_rke);
   zap(m_rkd);
}

bool Kuznyechik::has_keying_material() const {
   return !m_rke.empty();
}

void Kuznyechik::key_schedule(std::span<const uint8_t> key) {
   using namespace Kuznyechik_F;

   BOTAN_ASSERT_NOMSG(key.size() == 32);

   uint64_t k0 = load_le<uint64_t>(key.data(), 0);
   uint64_t k1 = load_le<uint64_t>(key.data(), 1);
   uint64_t k2 = load_le<uint64_t>(key.data(), 2);
   uint64_t k3 = load_le<uint64_t>(key.data(), 3);

   m_rke.resize(20);

   m_rke[0] = k0;
   m_rke[1] = k1;
   m_rke[2] = k2;
   m_rke[3] = k3;

   for(size_t i = 0; i != 4; ++i) {
      for(size_t r = 0; r != 8; r += 2) {
         uint64_t t0 = k0 ^ C[8 * i + r][0];
         uint64_t t1 = k1 ^ C[8 * i + r][1];
         const uint64_t t2 = k0;
         const uint64_t t3 = k1;
         LS(t0, t1);
         t0 ^= k2;
         t1 ^= k3;

         k0 = t0 ^ C[8 * i + r + 1][0];
         k1 = t1 ^ C[8 * i + r + 1][1];
         k2 = t0;
         k3 = t1;
         LS(k0, k1);
         k0 ^= t2;
         k1 ^= t3;
      }

      m_rke[4 * (i + 1) + 0] = k0;
      m_rke[4 * (i + 1) + 1] = k1;
      m_rke[4 * (i + 1) + 2] = k2;
      m_rke[4 * (i + 1) + 3] = k3;
   }

   m_rkd.resize(20);

   for(size_t i = 0; i != 10; i++) {
      uint64_t t0 = m_rke[2 * i + 0];
      uint64_t t1 = m_rke[2 * i + 1];

      if(i > 0) {
         Kuznyechik_F::ILSS(t0, t1);
      }

      const size_t dest = 9 - i;

      m_rkd[2 * dest + 0] = t0;
      m_rkd[2 * dest + 1] = t1;
   }
}

void Kuznyechik::encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();
   while(blocks > 0) {
      uint64_t x1 = load_le<uint64_t>(in, 0);
      uint64_t x2 = load_le<uint64_t>(in, 1);

      x1 ^= m_rke[0];
      x2 ^= m_rke[1];
      Kuznyechik_F::LS(x1, x2);

      x1 ^= m_rke[2];
      x2 ^= m_rke[3];
      Kuznyechik_F::LS(x1, x2);

      x1 ^= m_rke[4];
      x2 ^= m_rke[5];
      Kuznyechik_F::LS(x1, x2);

      x1 ^= m_rke[6];
      x2 ^= m_rke[7];
      Kuznyechik_F::LS(x1, x2);

      x1 ^= m_rke[8];
      x2 ^= m_rke[9];
      Kuznyechik_F::LS(x1, x2);

      x1 ^= m_rke[10];
      x2 ^= m_rke[11];
      Kuznyechik_F::LS(x1, x2);

      x1 ^= m_rke[12];
      x2 ^= m_rke[13];
      Kuznyechik_F::LS(x1, x2);

      x1 ^= m_rke[14];
      x2 ^= m_rke[15];
      Kuznyechik_F::LS(x1, x2);

      x1 ^= m_rke[16];
      x2 ^= m_rke[17];
      Kuznyechik_F::LS(x1, x2);

      x1 ^= m_rke[18];
      x2 ^= m_rke[19];

      store_le(out, x1, x2);

      in += 16;
      out += 16;
      blocks--;
   }
}

void Kuznyechik::decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();
   while(blocks > 0) {
      uint64_t x1 = load_le<uint64_t>(in, 0);
      uint64_t x2 = load_le<uint64_t>(in, 1);

      Kuznyechik_F::ILSS(x1, x2);

      x1 ^= m_rkd[0];
      x2 ^= m_rkd[1];
      Kuznyechik_F::ILS(x1, x2);

      x1 ^= m_rkd[2];
      x2 ^= m_rkd[3];
      Kuznyechik_F::ILS(x1, x2);

      x1 ^= m_rkd[4];
      x2 ^= m_rkd[5];
      Kuznyechik_F::ILS(x1, x2);

      x1 ^= m_rkd[6];
      x2 ^= m_rkd[7];
      Kuznyechik_F::ILS(x1, x2);

      x1 ^= m_rkd[8];
      x2 ^= m_rkd[9];
      Kuznyechik_F::ILS(x1, x2);

      x1 ^= m_rkd[10];
      x2 ^= m_rkd[11];
      Kuznyechik_F::ILS(x1, x2);

      x1 ^= m_rkd[12];
      x2 ^= m_rkd[13];
      Kuznyechik_F::ILS(x1, x2);

      x1 ^= m_rkd[14];
      x2 ^= m_rkd[15];
      Kuznyechik_F::ILS(x1, x2);

      x1 ^= m_rkd[16];
      x2 ^= m_rkd[17];
      x1 = Kuznyechik_F::ISI(x1);
      x2 = Kuznyechik_F::ISI(x2);

      x1 ^= m_rkd[18];
      x2 ^= m_rkd[19];

      store_le(out, x1, x2);

      in += 16;
      out += 16;
      blocks--;
   }
}

}  // namespace Botan
/*
* Message Authentication Code base class
* (C) 1999-2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_CMAC)
#endif

#if defined(BOTAN_HAS_GMAC)
#endif

#if defined(BOTAN_HAS_HMAC)
#endif

#if defined(BOTAN_HAS_POLY1305)
#endif

#if defined(BOTAN_HAS_SIPHASH)
#endif

#if defined(BOTAN_HAS_ANSI_X919_MAC)
#endif

#if defined(BOTAN_HAS_BLAKE2BMAC)
#endif

#if defined(BOTAN_HAS_KMAC)
#endif

namespace Botan {

std::unique_ptr<MessageAuthenticationCode> MessageAuthenticationCode::create(std::string_view algo_spec,
                                                                             std::string_view provider) {
   const SCAN_Name req(algo_spec);

#if defined(BOTAN_HAS_BLAKE2BMAC)
   if(req.algo_name() == "Blake2b" || req.algo_name() == "BLAKE2b") {
      return std::make_unique<BLAKE2bMAC>(req.arg_as_integer(0, 512));
   }
#endif

#if defined(BOTAN_HAS_GMAC)
   if(req.algo_name() == "GMAC" && req.arg_count() == 1) {
      if(provider.empty() || provider == "base") {
         if(auto bc = BlockCipher::create(req.arg(0))) {
            return std::make_unique<GMAC>(std::move(bc));
         }
      }
   }
#endif

#if defined(BOTAN_HAS_HMAC)
   if(req.algo_name() == "HMAC" && req.arg_count() == 1) {
      if(provider.empty() || provider == "base") {
         if(auto hash = HashFunction::create(req.arg(0))) {
            return std::make_unique<HMAC>(std::move(hash));
         }
      }
   }
#endif

#if defined(BOTAN_HAS_POLY1305)
   if(req.algo_name() == "Poly1305" && req.arg_count() == 0) {
      if(provider.empty() || provider == "base") {
         return std::make_unique<Poly1305>();
      }
   }
#endif

#if defined(BOTAN_HAS_SIPHASH)
   if(req.algo_name() == "SipHash") {
      if(provider.empty() || provider == "base") {
         return std::make_unique<SipHash>(req.arg_as_integer(0, 2), req.arg_as_integer(1, 4));
      }
   }
#endif

#if defined(BOTAN_HAS_CMAC)
   if((req.algo_name() == "CMAC" || req.algo_name() == "OMAC") && req.arg_count() == 1) {
      if(provider.empty() || provider == "base") {
         if(auto bc = BlockCipher::create(req.arg(0))) {
            return std::make_unique<CMAC>(std::move(bc));
         }
      }
   }
#endif

#if defined(BOTAN_HAS_ANSI_X919_MAC)
   if(req.algo_name() == "X9.19-MAC") {
      if(provider.empty() || provider == "base") {
         return std::make_unique<ANSI_X919_MAC>();
      }
   }
#endif

#if defined(BOTAN_HAS_KMAC)
   if(req.algo_name() == "KMAC-128") {
      if(provider.empty() || provider == "base") {
         if(req.arg_count() != 1) {
            throw Invalid_Argument(
               "invalid algorithm specification for KMAC-128: need exactly one argument for output bit length");
         }
         return std::make_unique<KMAC128>(req.arg_as_integer(0));
      }
   }

   if(req.algo_name() == "KMAC-256") {
      if(provider.empty() || provider == "base") {
         if(req.arg_count() != 1) {
            throw Invalid_Argument(
               "invalid algorithm specification for KMAC-256: need exactly one argument for output bit length");
         }
         return std::make_unique<KMAC256>(req.arg_as_integer(0));
      }
   }
#endif

   BOTAN_UNUSED(req);
   BOTAN_UNUSED(provider);

   return nullptr;
}

std::vector<std::string> MessageAuthenticationCode::providers(std::string_view algo_spec) {
   return probe_providers_of<MessageAuthenticationCode>(algo_spec);
}

//static
std::unique_ptr<MessageAuthenticationCode> MessageAuthenticationCode::create_or_throw(std::string_view algo,
                                                                                      std::string_view provider) {
   if(auto mac = MessageAuthenticationCode::create(algo, provider)) {
      return mac;
   }
   throw Lookup_Error("MAC", algo, provider);
}

void MessageAuthenticationCode::start_msg(std::span<const uint8_t> nonce) {
   BOTAN_UNUSED(nonce);
   if(!nonce.empty()) {
      throw Invalid_IV_Length(name(), nonce.size());
   }
}

/*
* Default (deterministic) MAC verification operation
*/
bool MessageAuthenticationCode::verify_mac_result(std::span<const uint8_t> mac) {
   secure_vector<uint8_t> our_mac = final();

   if(our_mac.size() != mac.size()) {
      return false;
   }

   return CT::is_equal(our_mac.data(), mac.data(), mac.size()).as_bool();
}

}  // namespace Botan
/*
* Cipher Modes
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <sstream>

#if defined(BOTAN_HAS_BLOCK_CIPHER)
#endif

#if defined(BOTAN_HAS_AEAD_MODES)
#endif

#if defined(BOTAN_HAS_MODE_CBC)
#endif

#if defined(BOTAN_HAS_MODE_CFB)
#endif

#if defined(BOTAN_HAS_MODE_XTS)
#endif

#if defined(BOTAN_HAS_COMMONCRYPTO)
#endif

namespace Botan {

std::unique_ptr<Cipher_Mode> Cipher_Mode::create_or_throw(std::string_view algo,
                                                          Cipher_Dir direction,
                                                          std::string_view provider) {
   if(auto mode = Cipher_Mode::create(algo, direction, provider)) {
      return mode;
   }

   throw Lookup_Error("Cipher mode", algo, provider);
}

std::unique_ptr<Cipher_Mode> Cipher_Mode::create(std::string_view algo,
                                                 Cipher_Dir direction,
                                                 std::string_view provider) {
#if defined(BOTAN_HAS_COMMONCRYPTO)
   if(provider.empty() || provider == "commoncrypto") {
      if(auto cm = make_commoncrypto_cipher_mode(algo, direction))
         return cm;

      if(!provider.empty())
         return nullptr;
   }
#endif

   if(provider != "base" && !provider.empty()) {
      return nullptr;
   }

#if defined(BOTAN_HAS_STREAM_CIPHER)
   if(auto sc = StreamCipher::create(algo)) {
      return std::make_unique<Stream_Cipher_Mode>(std::move(sc));
   }
#endif

#if defined(BOTAN_HAS_AEAD_MODES)
   if(auto aead = AEAD_Mode::create(algo, direction)) {
      return aead;
   }
#endif

   if(algo.find('/') != std::string::npos) {
      const std::vector<std::string> algo_parts = split_on(algo, '/');
      const std::string_view cipher_name = algo_parts[0];
      const std::vector<std::string> mode_info = parse_algorithm_name(algo_parts[1]);

      if(mode_info.empty()) {
         return std::unique_ptr<Cipher_Mode>();
      }

      std::ostringstream mode_name;

      mode_name << mode_info[0] << '(' << cipher_name;
      for(size_t i = 1; i < mode_info.size(); ++i) {
         mode_name << ',' << mode_info[i];
      }
      for(size_t i = 2; i < algo_parts.size(); ++i) {
         mode_name << ',' << algo_parts[i];
      }
      mode_name << ')';

      return Cipher_Mode::create(mode_name.str(), direction, provider);
   }

#if defined(BOTAN_HAS_BLOCK_CIPHER)

   const SCAN_Name spec(algo);

   if(spec.arg_count() == 0) {
      return std::unique_ptr<Cipher_Mode>();
   }

   auto bc = BlockCipher::create(spec.arg(0), provider);

   if(!bc) {
      return std::unique_ptr<Cipher_Mode>();
   }

   #if defined(BOTAN_HAS_MODE_CBC)
   if(spec.algo_name() == "CBC") {
      const std::string padding = spec.arg(1, "PKCS7");

      if(padding == "CTS") {
         if(direction == Cipher_Dir::Encryption) {
            return std::make_unique<CTS_Encryption>(std::move(bc));
         } else {
            return std::make_unique<CTS_Decryption>(std::move(bc));
         }
      } else {
         auto pad = BlockCipherModePaddingMethod::create(padding);

         if(pad) {
            if(direction == Cipher_Dir::Encryption) {
               return std::make_unique<CBC_Encryption>(std::move(bc), std::move(pad));
            } else {
               return std::make_unique<CBC_Decryption>(std::move(bc), std::move(pad));
            }
         }
      }
   }
   #endif

   #if defined(BOTAN_HAS_MODE_XTS)
   if(spec.algo_name() == "XTS") {
      if(direction == Cipher_Dir::Encryption) {
         return std::make_unique<XTS_Encryption>(std::move(bc));
      } else {
         return std::make_unique<XTS_Decryption>(std::move(bc));
      }
   }
   #endif

   #if defined(BOTAN_HAS_MODE_CFB)
   if(spec.algo_name() == "CFB") {
      const size_t feedback_bits = spec.arg_as_integer(1, 8 * bc->block_size());
      if(direction == Cipher_Dir::Encryption) {
         return std::make_unique<CFB_Encryption>(std::move(bc), feedback_bits);
      } else {
         return std::make_unique<CFB_Decryption>(std::move(bc), feedback_bits);
      }
   }
   #endif

#endif

   return std::unique_ptr<Cipher_Mode>();
}

//static
std::vector<std::string> Cipher_Mode::providers(std::string_view algo_spec) {
   const std::vector<std::string>& possible = {"base", "commoncrypto"};
   std::vector<std::string> providers;
   for(auto&& prov : possible) {
      auto mode = Cipher_Mode::create(algo_spec, Cipher_Dir::Encryption, prov);
      if(mode) {
         providers.push_back(prov);  // available
      }
   }
   return providers;
}

}  // namespace Botan
/*
* OS and machine specific utility functions
* (C) 2015,2016,2017,2018 Jack Lloyd
* (C) 2016 Daniel Neus
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_CPUID)
#endif

#include <chrono>
#include <cstdlib>
#include <iomanip>

#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   #include <errno.h>
   #include <pthread.h>
   #include <setjmp.h>
   #include <signal.h>
   #include <sys/mman.h>
   #include <sys/resource.h>
   #include <sys/types.h>
   #include <termios.h>
   #include <unistd.h>
   #undef B0
#endif

#if defined(BOTAN_TARGET_OS_IS_EMSCRIPTEN)
   #include <emscripten/emscripten.h>
#endif

#if defined(BOTAN_TARGET_OS_HAS_GETAUXVAL) || defined(BOTAN_TARGET_OS_HAS_ELF_AUX_INFO)
   #include <sys/auxv.h>
#endif

#if defined(BOTAN_TARGET_OS_HAS_WIN32)
   #define NOMINMAX 1
   #define _WINSOCKAPI_  // stop windows.h including winsock.h
   #include <windows.h>
   #if defined(BOTAN_BUILD_COMPILER_IS_MSVC)
      #include <libloaderapi.h>
      #include <stringapiset.h>
   #endif
#endif

#if defined(BOTAN_TARGET_OS_IS_IOS) || defined(BOTAN_TARGET_OS_IS_MACOS)
   #include <mach/vm_statistics.h>
   #include <sys/sysctl.h>
   #include <sys/types.h>
#endif

#if defined(BOTAN_TARGET_OS_HAS_PRCTL)
   #include <sys/prctl.h>
#endif

#if defined(BOTAN_TARGET_OS_IS_FREEBSD) || defined(BOTAN_TARGET_OS_IS_OPENBSD) || defined(BOTAN_TARGET_OS_IS_DRAGONFLY)
   #include <pthread_np.h>
#endif

#if defined(BOTAN_TARGET_OS_IS_HAIKU)
   #include <kernel/OS.h>
#endif

namespace Botan {

uint32_t OS::get_process_id() {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   return ::getpid();
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
   return ::GetCurrentProcessId();
#elif defined(BOTAN_TARGET_OS_IS_LLVM) || defined(BOTAN_TARGET_OS_IS_NONE)
   return 0;  // truly no meaningful value
#else
   #error "Missing get_process_id"
#endif
}

namespace {

#if defined(BOTAN_TARGET_OS_HAS_GETAUXVAL) || defined(BOTAN_TARGET_OS_HAS_ELF_AUX_INFO)
   #define BOTAN_TARGET_HAS_AUXVAL_INTERFACE
#endif

std::optional<unsigned long> auxval_hwcap() {
#if defined(AT_HWCAP)
   return AT_HWCAP;
#elif defined(BOTAN_TARGET_HAS_AUXVAL_INTERFACE)
   // If the value is not defined in a header we can see,
   // but auxval is supported, return the Linux/Android value
   return 16;
#else
   return {};
#endif
}

std::optional<unsigned long> auxval_hwcap2() {
#if defined(AT_HWCAP2)
   return AT_HWCAP2;
#elif defined(BOTAN_TARGET_HAS_AUXVAL_INTERFACE)
   // If the value is not defined in a header we can see,
   // but auxval is supported, return the Linux/Android value
   return 26;
#else
   return {};
#endif
}

std::optional<unsigned long> get_auxval(std::optional<unsigned long> id) {
   if(id) {
#if defined(BOTAN_TARGET_OS_HAS_GETAUXVAL)
      return ::getauxval(*id);
#elif defined(BOTAN_TARGET_OS_HAS_ELF_AUX_INFO)
      unsigned long auxinfo = 0;
      if(::elf_aux_info(static_cast<int>(*id), &auxinfo, sizeof(auxinfo)) == 0) {
         return auxinfo;
      }
#endif
   }

   return {};
}

}  // namespace

std::optional<std::pair<unsigned long, unsigned long>> OS::get_auxval_hwcap() {
   if(const auto hwcap = get_auxval(auxval_hwcap())) {
      // If hwcap worked/was valid, we don't require hwcap2 to also
      // succeed but instead will return zeros if it failed.
      auto hwcap2 = get_auxval(auxval_hwcap2()).value_or(0);
      return std::make_pair(*hwcap, hwcap2);
   } else {
      return {};
   }
}

namespace {

/**
* Test if we are currently running with elevated permissions
* eg setuid, setgid, or with POSIX caps set.
*/
bool running_in_privileged_state() {
#if defined(AT_SECURE)
   if(auto at_secure = get_auxval(AT_SECURE)) {
      return at_secure != 0;
   }
#endif

#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   return (::getuid() != ::geteuid()) || (::getgid() != ::getegid());
#else
   return false;
#endif
}

}  // namespace

uint64_t OS::get_cpu_cycle_counter() {
   uint64_t rtc = 0;

#if defined(BOTAN_TARGET_OS_HAS_WIN32)
   LARGE_INTEGER tv;
   ::QueryPerformanceCounter(&tv);
   rtc = tv.QuadPart;

#elif defined(BOTAN_USE_GCC_INLINE_ASM)

   // NOLINTBEGIN(*-no-assembler)

   #if defined(BOTAN_TARGET_ARCH_IS_X86_64)

   uint32_t rtc_low = 0;   // NOLINT(*-const-correctness) clang-tidy doesn't understand inline asm
   uint32_t rtc_high = 0;  // NOLINT(*-const-correctness) clang-tidy doesn't understand inline asm
   asm volatile("rdtsc" : "=d"(rtc_high), "=a"(rtc_low));
   rtc = (static_cast<uint64_t>(rtc_high) << 32) | rtc_low;

   #elif defined(BOTAN_TARGET_ARCH_IS_X86_FAMILY) && defined(BOTAN_HAS_CPUID)

   if(CPUID::has(CPUID::Feature::RDTSC)) {
      uint32_t rtc_low = 0;
      uint32_t rtc_high = 0;
      asm volatile("rdtsc" : "=d"(rtc_high), "=a"(rtc_low));
      rtc = (static_cast<uint64_t>(rtc_high) << 32) | rtc_low;
   }

   #elif defined(BOTAN_TARGET_ARCH_IS_PPC64)

   for(;;) {
      uint32_t rtc_low = 0;
      uint32_t rtc_high = 0;
      uint32_t rtc_high2 = 0;
      asm volatile("mftbu %0" : "=r"(rtc_high));
      asm volatile("mftb %0" : "=r"(rtc_low));
      asm volatile("mftbu %0" : "=r"(rtc_high2));

      if(rtc_high == rtc_high2) {
         rtc = (static_cast<uint64_t>(rtc_high) << 32) | rtc_low;
         break;
      }
   }

   #elif defined(BOTAN_TARGET_ARCH_IS_ALPHA)
   asm volatile("rpcc %0" : "=r"(rtc));

   #elif defined(BOTAN_TARGET_ARCH_IS_SPARC64) && !defined(BOTAN_TARGET_OS_IS_OPENBSD)
   // OpenBSD does not trap access to the %tick register so we avoid it there
   asm volatile("rd %%tick, %0" : "=r"(rtc));

   #elif defined(BOTAN_TARGET_ARCH_IS_IA64)
   asm volatile("mov %0=ar.itc" : "=r"(rtc));

   #elif defined(BOTAN_TARGET_ARCH_IS_S390X)
   asm volatile("stck 0(%0)" : : "a"(&rtc) : "memory", "cc");

   #elif defined(BOTAN_TARGET_ARCH_IS_HPPA)
   asm volatile("mfctl 16,%0" : "=r"(rtc));  // 64-bit only?

   #else
      //#warning "OS::get_cpu_cycle_counter not implemented"
   #endif

   // NOLINTEND(*-no-assembler)

#endif

   return rtc;
}

size_t OS::get_cpu_available() {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)

   #if defined(_SC_NPROCESSORS_ONLN)
   const long cpu_online = ::sysconf(_SC_NPROCESSORS_ONLN);
   if(cpu_online > 0) {
      return static_cast<size_t>(cpu_online);
   }
   #endif

   #if defined(_SC_NPROCESSORS_CONF)
   const long cpu_conf = ::sysconf(_SC_NPROCESSORS_CONF);
   if(cpu_conf > 0) {
      return static_cast<size_t>(cpu_conf);
   }
   #endif

#endif

#if defined(BOTAN_TARGET_OS_HAS_THREADS)
   // hardware_concurrency is allowed to return 0 if the value is not
   // well defined or not computable.
   const size_t hw_concur = std::thread::hardware_concurrency();

   if(hw_concur > 0) {
      return hw_concur;
   }
#endif

   return 1;
}

uint64_t OS::get_high_resolution_clock() {
   if(const uint64_t cpu_clock = OS::get_cpu_cycle_counter()) {
      return cpu_clock;
   }

#if defined(BOTAN_TARGET_OS_IS_EMSCRIPTEN)
   return emscripten_get_now();
#endif

   /*
   If we got here either we either don't have an asm instruction
   above, or (for x86) RDTSC is not available at runtime. Try some
   clock_gettimes and return the first one that works, or otherwise
   fall back to std::chrono.
   */

#if defined(BOTAN_TARGET_OS_HAS_CLOCK_GETTIME)

   // The ordering here is somewhat arbitrary...
   const clockid_t clock_types[] = {
   #if defined(CLOCK_MONOTONIC_HR)
      CLOCK_MONOTONIC_HR,
   #endif
   #if defined(CLOCK_MONOTONIC_RAW)
      CLOCK_MONOTONIC_RAW,
   #endif
   #if defined(CLOCK_MONOTONIC)
      CLOCK_MONOTONIC,
   #endif
   #if defined(CLOCK_PROCESS_CPUTIME_ID)
      CLOCK_PROCESS_CPUTIME_ID,
   #endif
   #if defined(CLOCK_THREAD_CPUTIME_ID)
      CLOCK_THREAD_CPUTIME_ID,
   #endif
   };

   for(const clockid_t clock : clock_types) {
      struct timespec ts {};

      if(::clock_gettime(clock, &ts) == 0) {
         return (static_cast<uint64_t>(ts.tv_sec) * 1000000000) + static_cast<uint64_t>(ts.tv_nsec);
      }
   }
#endif

#if defined(BOTAN_TARGET_OS_HAS_SYSTEM_CLOCK)
   // Plain C++11 fallback
   auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
   return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
#else
   return 0;
#endif
}

uint64_t OS::get_system_timestamp_ns() {
#if defined(BOTAN_TARGET_OS_HAS_CLOCK_GETTIME)
   struct timespec ts {};

   if(::clock_gettime(CLOCK_REALTIME, &ts) == 0) {
      return (static_cast<uint64_t>(ts.tv_sec) * 1000000000) + static_cast<uint64_t>(ts.tv_nsec);
   }
#endif

#if defined(BOTAN_TARGET_OS_HAS_SYSTEM_CLOCK)
   auto now = std::chrono::system_clock::now().time_since_epoch();
   return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
#else
   throw Not_Implemented("OS::get_system_timestamp_ns this system does not support a clock");
#endif
}

std::string OS::format_time(time_t time, const std::string& format) {
   std::tm tm{};

#if defined(BOTAN_TARGET_OS_HAS_WIN32)
   if(::localtime_s(&tm, &time) != 0) {
      throw Encoding_Error("Could not convert time_t to localtime");
   }
#elif defined(BOTAN_TARGET_OS_HAS_POSIX1)
   if(::localtime_r(&time, &tm) == nullptr) {
      throw Encoding_Error("Could not convert time_t to localtime");
   }
#else
   if(auto tmp = std::localtime(&time)) {
      tm = *tmp;
   } else {
      throw Encoding_Error("Could not convert time_t to localtime");
   }
#endif

   std::ostringstream oss;
   oss << std::put_time(&tm, format.c_str());
   return oss.str();
}

size_t OS::system_page_size() {
   const size_t default_page_size = 4096;

#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   const long p = ::sysconf(_SC_PAGESIZE);
   if(p > 1) {
      return static_cast<size_t>(p);
   } else {
      return default_page_size;
   }
#elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
   BOTAN_UNUSED(default_page_size);
   SYSTEM_INFO sys_info;
   ::GetSystemInfo(&sys_info);
   return sys_info.dwPageSize;
#else
   return default_page_size;
#endif
}

size_t OS::get_memory_locking_limit() {
   /*
   * Linux defaults to only 64 KiB of mlockable memory per process (too small)
   * but BSDs offer a small fraction of total RAM (more than we need). Bound the
   * total mlock size to 512 KiB which is enough to run the entire test suite
   * without spilling to non-mlock memory (and thus presumably also enough for
   * many useful programs), but small enough that we should not cause problems
   * even if many processes are mlocking on the same machine.
   */
   const size_t max_locked_kb = 512;

   /*
   * If RLIMIT_MEMLOCK is not defined, likely the OS does not support
   * unprivileged mlock calls.
   */
#if defined(RLIMIT_MEMLOCK) && defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)
   const size_t mlock_requested =
      std::min<size_t>(read_env_variable_sz("BOTAN_MLOCK_POOL_SIZE", max_locked_kb), max_locked_kb);

   if(mlock_requested > 0) {
      struct ::rlimit limits {};

      ::getrlimit(RLIMIT_MEMLOCK, &limits);

      if(limits.rlim_cur < limits.rlim_max) {
         limits.rlim_cur = limits.rlim_max;
         ::setrlimit(RLIMIT_MEMLOCK, &limits);
         ::getrlimit(RLIMIT_MEMLOCK, &limits);
      }

      return std::min<size_t>(limits.rlim_cur, mlock_requested * 1024);
   }

#elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
   const size_t mlock_requested =
      std::min<size_t>(read_env_variable_sz("BOTAN_MLOCK_POOL_SIZE", max_locked_kb), max_locked_kb);

   SIZE_T working_min = 0, working_max = 0;
   if(!::GetProcessWorkingSetSize(::GetCurrentProcess(), &working_min, &working_max)) {
      return 0;
   }

   // According to Microsoft MSDN:
   // The maximum number of pages that a process can lock is equal to the number of pages in its minimum working set minus a small overhead
   // In the book "Windows Internals Part 2": the maximum lockable pages are minimum working set size - 8 pages
   // But the information in the book seems to be inaccurate/outdated
   // I've tested this on Windows 8.1 x64, Windows 10 x64 and Windows 7 x86
   // On all three OS the value is 11 instead of 8
   const size_t overhead = OS::system_page_size() * 11;
   if(working_min > overhead) {
      const size_t lockable_bytes = working_min - overhead;
      return std::min<size_t>(lockable_bytes, mlock_requested * 1024);
   }
#else
   // Not supported on this platform
   BOTAN_UNUSED(max_locked_kb);
#endif

   return 0;
}

bool OS::read_env_variable(std::string& value_out, std::string_view name_view) {
   value_out = "";

   if(running_in_privileged_state()) {
      return false;
   }

#if defined(BOTAN_TARGET_OS_HAS_WIN32) && \
   (defined(BOTAN_BUILD_COMPILER_IS_MSVC) || defined(BOTAN_BUILD_COMPILER_IS_CLANGCL))
   const std::string name(name_view);
   char val[128] = {0};
   size_t req_size = 0;
   if(getenv_s(&req_size, val, sizeof(val), name.c_str()) == 0) {
      // Microsoft's implementation always writes a terminating \0,
      // and includes it in the reported length of the environment variable
      // if a value exists.
      if(req_size > 0 && val[req_size - 1] == '\0') {
         value_out = std::string(val);
      } else {
         value_out = std::string(val, req_size);
      }
      return true;
   }
#else
   const std::string name(name_view);
   if(const char* val = std::getenv(name.c_str())) {
      value_out = val;
      return true;
   }
#endif

   return false;
}

size_t OS::read_env_variable_sz(std::string_view name, size_t def) {
   std::string value;
   if(read_env_variable(value, name) && !value.empty()) {
      try {
         const size_t val = std::stoul(value, nullptr);
         return val;
      } catch(std::exception&) { /* ignore it */
      }
   }

   return def;
}

#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)

namespace {

int get_locked_fd() {
   #if defined(BOTAN_TARGET_OS_IS_IOS) || defined(BOTAN_TARGET_OS_IS_MACOS)
   // On Darwin, tagging anonymous pages allows vmmap to track these.
   // Allowed from 240 to 255 for userland applications
   static constexpr int default_locked_fd = 255;
   int locked_fd = default_locked_fd;

   if(size_t locked_fdl = OS::read_env_variable_sz("BOTAN_LOCKED_FD", default_locked_fd)) {
      if(locked_fdl < 240 || locked_fdl > 255) {
         locked_fdl = default_locked_fd;
      }
      locked_fd = static_cast<int>(locked_fdl);
   }
   return VM_MAKE_TAG(locked_fd);
   #else
   return -1;
   #endif
}

int mmap_flags() {
   int flags = MAP_PRIVATE;

   #if defined(MAP_ANONYMOUS)
   flags |= MAP_ANONYMOUS;
   #elif defined(MAP_ANON)
   flags |= MAP_ANON;
   #endif

   #if defined(MAP_CONCEAL)
   flags |= MAP_CONCEAL;
   #elif defined(MAP_NOCORE)
   flags |= MAP_NOCORE;
   #endif

   return flags;
}

int mmap_prot() {
   int prot = PROT_READ | PROT_WRITE;  // NOLINT(*-const-correctness)

   #if defined(PROT_MAX)
   prot |= PROT_MAX(prot);
   #endif

   return prot;
}

}  // namespace

#endif

std::vector<void*> OS::allocate_locked_pages(size_t count) {
   std::vector<void*> result;

#if(defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)) || \
   defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)

   result.reserve(count);

   const size_t page_size = OS::system_page_size();

   #if defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)
   static const int locked_fd = get_locked_fd();
   #endif

   for(size_t i = 0; i != count; ++i) {
      void* ptr = nullptr;

   #if defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)
      ptr = ::mmap(nullptr,
                   3 * page_size,
                   mmap_prot(),
                   mmap_flags(),
                   /*fd=*/locked_fd,
                   /*offset=*/0);

      if(ptr == MAP_FAILED) {
         continue;
      }

      // lock the data page
      if(::mlock(static_cast<uint8_t*>(ptr) + page_size, page_size) != 0) {
         ::munmap(ptr, 3 * page_size);
         continue;
      }

      #if defined(MADV_DONTDUMP)
      // we ignore errors here, as DONTDUMP is just a bonus
      ::madvise(static_cast<uint8_t*>(ptr) + page_size, page_size, MADV_DONTDUMP);
      #endif

   #elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
      ptr = ::VirtualAlloc(nullptr, 3 * page_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

      if(ptr == nullptr)
         continue;

      if(::VirtualLock(static_cast<uint8_t*>(ptr) + page_size, page_size) == 0) {
         ::VirtualFree(ptr, 0, MEM_RELEASE);
         continue;
      }
   #endif

      std::memset(ptr, 0, 3 * page_size);  // zero data page and both guard pages

      // Attempts to name the data page
      page_named(ptr, 3 * page_size);
      // Make guard page preceding the data page
      page_prohibit_access(static_cast<uint8_t*>(ptr));
      // Make guard page following the data page
      page_prohibit_access(static_cast<uint8_t*>(ptr) + 2 * page_size);

      result.push_back(static_cast<uint8_t*>(ptr) + page_size);
   }
#else
   BOTAN_UNUSED(count);
#endif

   return result;
}

void OS::page_allow_access(void* page) {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   const size_t page_size = OS::system_page_size();
   ::mprotect(page, page_size, PROT_READ | PROT_WRITE);
#elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
   const size_t page_size = OS::system_page_size();
   DWORD old_perms = 0;
   ::VirtualProtect(page, page_size, PAGE_READWRITE, &old_perms);
   BOTAN_UNUSED(old_perms);
#else
   BOTAN_UNUSED(page);
#endif
}

void OS::page_prohibit_access(void* page) {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   const size_t page_size = OS::system_page_size();
   ::mprotect(page, page_size, PROT_NONE);
#elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
   const size_t page_size = OS::system_page_size();
   DWORD old_perms = 0;
   ::VirtualProtect(page, page_size, PAGE_NOACCESS, &old_perms);
   BOTAN_UNUSED(old_perms);
#else
   BOTAN_UNUSED(page);
#endif
}

void OS::free_locked_pages(const std::vector<void*>& pages) {
   const size_t page_size = OS::system_page_size();

   for(void* ptr : pages) {
      secure_scrub_memory(ptr, page_size);

      // ptr points to the data page, guard pages are before and after
      page_allow_access(static_cast<uint8_t*>(ptr) - page_size);
      page_allow_access(static_cast<uint8_t*>(ptr) + page_size);

#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)
      ::munlock(ptr, page_size);
      ::munmap(static_cast<uint8_t*>(ptr) - page_size, 3 * page_size);
#elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
      ::VirtualUnlock(ptr, page_size);
      ::VirtualFree(static_cast<uint8_t*>(ptr) - page_size, 0, MEM_RELEASE);
#endif
   }
}

void OS::page_named(void* page, size_t size) {
#if defined(BOTAN_TARGET_OS_HAS_PRCTL) && defined(PR_SET_VMA) && defined(PR_SET_VMA_ANON_NAME)
   static constexpr char name[] = "Botan mlock pool";
   // NOLINTNEXTLINE(*-vararg)
   const int r = prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, reinterpret_cast<uintptr_t>(page), size, name);
   BOTAN_UNUSED(r);
#else
   BOTAN_UNUSED(page, size);
#endif
}

#if defined(BOTAN_TARGET_OS_HAS_THREADS)
void OS::set_thread_name(std::thread& thread, const std::string& name) {
   #if defined(BOTAN_TARGET_OS_IS_LINUX) || defined(BOTAN_TARGET_OS_IS_FREEBSD) || defined(BOTAN_TARGET_OS_IS_DRAGONFLY)
   static_cast<void>(pthread_setname_np(thread.native_handle(), name.c_str()));
   #elif defined(BOTAN_TARGET_OS_IS_OPENBSD)
   static_cast<void>(pthread_set_name_np(thread.native_handle(), name.c_str()));
   #elif defined(BOTAN_TARGET_OS_IS_NETBSD)
   static_cast<void>(pthread_setname_np(thread.native_handle(), "%s", const_cast<char*>(name.c_str())));
   #elif defined(BOTAN_TARGET_OS_HAS_WIN32) && defined(_LIBCPP_HAS_THREAD_API_PTHREAD)
   static_cast<void>(pthread_setname_np(thread.native_handle(), name.c_str()));
   #elif defined(BOTAN_TARGET_OS_HAS_WIN32) && defined(BOTAN_BUILD_COMPILER_IS_MSVC)
   typedef HRESULT(WINAPI * std_proc)(HANDLE, PCWSTR);
   HMODULE kern = GetModuleHandleA("KernelBase.dll");
   std_proc set_thread_name = reinterpret_cast<std_proc>(GetProcAddress(kern, "SetThreadDescription"));
   if(set_thread_name) {
      std::wstring w;
      auto sz = MultiByteToWideChar(CP_UTF8, 0, name.data(), -1, nullptr, 0);
      if(sz > 0) {
         w.resize(sz);
         if(MultiByteToWideChar(CP_UTF8, 0, name.data(), -1, &w[0], sz) > 0) {
            (void)set_thread_name(thread.native_handle(), w.c_str());
         }
      }
   }
   #elif defined(BOTAN_TARGET_OS_IF_HAIKU)
   auto thread_id = get_pthread_thread_id(thread.native_handle());
   static_cast<void>(rename_thread(thread_id, name.c_str()));
   #else
   // TODO other possible oses ?
   // macOs does not seem to allow to name threads other than the current one.
   BOTAN_UNUSED(thread, name);
   #endif
}
#endif

#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && !defined(BOTAN_TARGET_OS_IS_EMSCRIPTEN)

namespace {

// NOLINTNEXTLINE(*-avoid-non-const-global-variables)
::sigjmp_buf g_sigill_jmp_buf;

void botan_sigill_handler(int /*unused*/) {
   siglongjmp(g_sigill_jmp_buf, /*non-zero return value*/ 1);
}

}  // namespace

#endif

int OS::run_cpu_instruction_probe(const std::function<int()>& probe_fn) {
   volatile int probe_result = -3;

#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && !defined(BOTAN_TARGET_OS_IS_EMSCRIPTEN)
   struct sigaction old_sigaction {};

   struct sigaction sigaction {};

   sigaction.sa_handler = botan_sigill_handler;
   sigemptyset(&sigaction.sa_mask);
   sigaction.sa_flags = 0;

   int rc = ::sigaction(SIGILL, &sigaction, &old_sigaction);

   if(rc != 0) {
      throw System_Error("run_cpu_instruction_probe sigaction failed", errno);
   }

   rc = sigsetjmp(g_sigill_jmp_buf, /*save sigs*/ 1);

   if(rc == 0) {
      // first call to sigsetjmp
      probe_result = probe_fn();
   } else if(rc == 1) {
      // non-local return from siglongjmp in signal handler: return error
      probe_result = -1;
   }

   // Restore old SIGILL handler, if any
   rc = ::sigaction(SIGILL, &old_sigaction, nullptr);
   if(rc != 0) {
      throw System_Error("run_cpu_instruction_probe sigaction restore failed", errno);
   }

#else
   BOTAN_UNUSED(probe_fn);
#endif

   return probe_result;
}

std::unique_ptr<OS::Echo_Suppression> OS::suppress_echo_on_terminal() {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   class POSIX_Echo_Suppression final : public Echo_Suppression {
      public:
         POSIX_Echo_Suppression() : m_stdin_fd(fileno(stdin)), m_old_termios{} {
            if(::tcgetattr(m_stdin_fd, &m_old_termios) != 0) {
               throw System_Error("Getting terminal status failed", errno);
            }

            struct termios noecho_flags = m_old_termios;
            noecho_flags.c_lflag &= ~ECHO;
            noecho_flags.c_lflag |= ECHONL;

            if(::tcsetattr(m_stdin_fd, TCSANOW, &noecho_flags) != 0) {
               throw System_Error("Clearing terminal echo bit failed", errno);
            }
         }

         void reenable_echo() override {
            if(m_stdin_fd > 0) {
               if(::tcsetattr(m_stdin_fd, TCSANOW, &m_old_termios) != 0) {
                  throw System_Error("Restoring terminal echo bit failed", errno);
               }
               m_stdin_fd = -1;
            }
         }

         ~POSIX_Echo_Suppression() override {
            try {
               reenable_echo();
            } catch(...) {}
         }

         POSIX_Echo_Suppression(const POSIX_Echo_Suppression& other) = delete;
         POSIX_Echo_Suppression(POSIX_Echo_Suppression&& other) = delete;
         POSIX_Echo_Suppression& operator=(const POSIX_Echo_Suppression& other) = delete;
         POSIX_Echo_Suppression& operator=(POSIX_Echo_Suppression&& other) = delete;

      private:
         int m_stdin_fd;
         struct termios m_old_termios;
   };

   return std::make_unique<POSIX_Echo_Suppression>();

#elif defined(BOTAN_TARGET_OS_HAS_WIN32)

   class Win32_Echo_Suppression final : public Echo_Suppression {
      public:
         Win32_Echo_Suppression() {
            m_input_handle = ::GetStdHandle(STD_INPUT_HANDLE);
            if(::GetConsoleMode(m_input_handle, &m_console_state) == 0)
               throw System_Error("Getting console mode failed", ::GetLastError());

            DWORD new_mode = ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT;
            if(::SetConsoleMode(m_input_handle, new_mode) == 0)
               throw System_Error("Setting console mode failed", ::GetLastError());
         }

         void reenable_echo() override {
            if(m_input_handle != INVALID_HANDLE_VALUE) {
               if(::SetConsoleMode(m_input_handle, m_console_state) == 0)
                  throw System_Error("Setting console mode failed", ::GetLastError());
               m_input_handle = INVALID_HANDLE_VALUE;
            }
         }

         ~Win32_Echo_Suppression() override {
            try {
               reenable_echo();
            } catch(...) {}
         }

         Win32_Echo_Suppression(const Win32_Echo_Suppression& other) = delete;
         Win32_Echo_Suppression(Win32_Echo_Suppression&& other) = delete;
         Win32_Echo_Suppression& operator=(const Win32_Echo_Suppression& other) = delete;
         Win32_Echo_Suppression& operator=(Win32_Echo_Suppression&& other) = delete;

      private:
         HANDLE m_input_handle;
         DWORD m_console_state;
   };

   return std::make_unique<Win32_Echo_Suppression>();

#else

   // Not supported on this platform, return null
   return nullptr;
#endif
}

}  // namespace Botan
/*
* PBKDF
* (C) 2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_PBKDF2)
#endif

#if defined(BOTAN_HAS_PGP_S2K)
#endif

namespace Botan {

std::unique_ptr<PBKDF> PBKDF::create(std::string_view algo_spec, std::string_view provider) {
   const SCAN_Name req(algo_spec);

#if defined(BOTAN_HAS_PBKDF2)
   if(req.algo_name() == "PBKDF2") {
      if(provider.empty() || provider == "base") {
         if(auto mac = MessageAuthenticationCode::create("HMAC(" + req.arg(0) + ")")) {
            return std::make_unique<PKCS5_PBKDF2>(std::move(mac));
         }

         if(auto mac = MessageAuthenticationCode::create(req.arg(0))) {
            return std::make_unique<PKCS5_PBKDF2>(std::move(mac));
         }
      }

      return nullptr;
   }
#endif

#if defined(BOTAN_HAS_PGP_S2K)
   if(req.algo_name() == "OpenPGP-S2K" && req.arg_count() == 1) {
      if(auto hash = HashFunction::create(req.arg(0))) {
         return std::make_unique<OpenPGP_S2K>(std::move(hash));
      }
   }
#endif

   BOTAN_UNUSED(req, provider);

   return nullptr;
}

//static
std::unique_ptr<PBKDF> PBKDF::create_or_throw(std::string_view algo, std::string_view provider) {
   if(auto pbkdf = PBKDF::create(algo, provider)) {
      return pbkdf;
   }
   throw Lookup_Error("PBKDF", algo, provider);
}

std::vector<std::string> PBKDF::providers(std::string_view algo_spec) {
   return probe_providers_of<PBKDF>(algo_spec);
}

void PBKDF::pbkdf_timed(uint8_t out[],
                        size_t out_len,
                        std::string_view passphrase,
                        const uint8_t salt[],
                        size_t salt_len,
                        std::chrono::milliseconds msec,
                        size_t& iterations) const {
   iterations = pbkdf(out, out_len, passphrase, salt, salt_len, 0, msec);
}

void PBKDF::pbkdf_iterations(uint8_t out[],
                             size_t out_len,
                             std::string_view passphrase,
                             const uint8_t salt[],
                             size_t salt_len,
                             size_t iterations) const {
   if(iterations == 0) {
      throw Invalid_Argument(name() + ": Invalid iteration count");
   }

   const size_t iterations_run =
      pbkdf(out, out_len, passphrase, salt, salt_len, iterations, std::chrono::milliseconds(0));
   BOTAN_ASSERT_EQUAL(iterations, iterations_run, "Expected PBKDF iterations");
}

secure_vector<uint8_t> PBKDF::pbkdf_iterations(
   size_t out_len, std::string_view passphrase, const uint8_t salt[], size_t salt_len, size_t iterations) const {
   secure_vector<uint8_t> out(out_len);
   pbkdf_iterations(out.data(), out_len, passphrase, salt, salt_len, iterations);
   return out;
}

secure_vector<uint8_t> PBKDF::pbkdf_timed(size_t out_len,
                                          std::string_view passphrase,
                                          const uint8_t salt[],
                                          size_t salt_len,
                                          std::chrono::milliseconds msec,
                                          size_t& iterations) const {
   secure_vector<uint8_t> out(out_len);
   pbkdf_timed(out.data(), out_len, passphrase, salt, salt_len, msec, iterations);
   return out;
}

}  // namespace Botan
/*
* (C) 2018 Ribose Inc
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_PBKDF2)
#endif

#if defined(BOTAN_HAS_PGP_S2K)
#endif

#if defined(BOTAN_HAS_SCRYPT)
#endif

#if defined(BOTAN_HAS_ARGON2)
#endif

#if defined(BOTAN_HAS_PBKDF_BCRYPT)
#endif

namespace Botan {

void PasswordHash::derive_key(uint8_t out[],
                              size_t out_len,
                              const char* password,
                              size_t password_len,
                              const uint8_t salt[],
                              size_t salt_len,
                              const uint8_t ad[],
                              size_t ad_len,
                              const uint8_t key[],
                              size_t key_len) const {
   BOTAN_UNUSED(ad, key);

   if(ad_len == 0 && key_len == 0) {
      return this->derive_key(out, out_len, password, password_len, salt, salt_len);
   } else {
      throw Not_Implemented("PasswordHash " + this->to_string() + " does not support AD or key");
   }
}

std::unique_ptr<PasswordHashFamily> PasswordHashFamily::create(std::string_view algo_spec, std::string_view provider) {
   const SCAN_Name req(algo_spec);

#if defined(BOTAN_HAS_PBKDF2)
   if(req.algo_name() == "PBKDF2") {
      if(provider.empty() || provider == "base") {
         if(auto mac = MessageAuthenticationCode::create("HMAC(" + req.arg(0) + ")")) {
            return std::make_unique<PBKDF2_Family>(std::move(mac));
         }

         if(auto mac = MessageAuthenticationCode::create(req.arg(0))) {
            return std::make_unique<PBKDF2_Family>(std::move(mac));
         }
      }

      return nullptr;
   }
#endif

#if defined(BOTAN_HAS_SCRYPT)
   if(req.algo_name() == "Scrypt") {
      return std::make_unique<Scrypt_Family>();
   }
#endif

#if defined(BOTAN_HAS_ARGON2)
   if(req.algo_name() == "Argon2d") {
      return std::make_unique<Argon2_Family>(static_cast<uint8_t>(0));
   } else if(req.algo_name() == "Argon2i") {
      return std::make_unique<Argon2_Family>(static_cast<uint8_t>(1));
   } else if(req.algo_name() == "Argon2id") {
      return std::make_unique<Argon2_Family>(static_cast<uint8_t>(2));
   }
#endif

#if defined(BOTAN_HAS_PBKDF_BCRYPT)
   if(req.algo_name() == "Bcrypt-PBKDF") {
      return std::make_unique<Bcrypt_PBKDF_Family>();
   }
#endif

#if defined(BOTAN_HAS_PGP_S2K)
   if(req.algo_name() == "OpenPGP-S2K" && req.arg_count() == 1) {
      if(auto hash = HashFunction::create(req.arg(0))) {
         return std::make_unique<RFC4880_S2K_Family>(std::move(hash));
      }
   }
#endif

   BOTAN_UNUSED(req);
   BOTAN_UNUSED(provider);

   return nullptr;
}

//static
std::unique_ptr<PasswordHashFamily> PasswordHashFamily::create_or_throw(std::string_view algo,
                                                                        std::string_view provider) {
   if(auto pbkdf = PasswordHashFamily::create(algo, provider)) {
      return pbkdf;
   }
   throw Lookup_Error("PasswordHashFamily", algo, provider);
}

std::vector<std::string> PasswordHashFamily::providers(std::string_view algo_spec) {
   return probe_providers_of<PasswordHashFamily>(algo_spec);
}

}  // namespace Botan
/*
* PBKDF2
* (C) 1999-2007 Jack Lloyd
* (C) 2018 Ribose Inc
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

namespace {

void pbkdf2_set_key(MessageAuthenticationCode& prf, const char* password, size_t password_len) {
   try {
      prf.set_key(as_span_of_bytes(password, password_len));
   } catch(Invalid_Key_Length&) {
      throw Invalid_Argument("PBKDF2 cannot accept passphrase of the given size");
   }
}

size_t tune_pbkdf2(MessageAuthenticationCode& prf,
                   size_t output_length,
                   uint64_t desired_msec,
                   uint64_t tuning_msec = 10) {
   if(output_length == 0) {
      output_length = 1;
   }

   const size_t prf_sz = prf.output_length();
   BOTAN_ASSERT_NOMSG(prf_sz > 0);
   const secure_vector<uint8_t> U(prf_sz);

   const size_t trial_iterations = 2000;

   // Short output ensures we only need a single PBKDF2 block

   prf.set_key(nullptr, 0);

   const uint64_t duration_nsec = measure_cost(tuning_msec, [&]() {
      uint8_t out[12] = {0};
      uint8_t salt[12] = {0};
      pbkdf2(prf, out, sizeof(out), salt, sizeof(salt), trial_iterations);
   });

   const uint64_t desired_nsec = desired_msec * 1000000;

   if(duration_nsec > desired_nsec) {
      return trial_iterations;
   }

   const size_t blocks_needed = (output_length + prf_sz - 1) / prf_sz;

   const size_t multiplier = static_cast<size_t>(desired_nsec / duration_nsec / blocks_needed);

   if(multiplier == 0) {
      return trial_iterations;
   } else {
      return trial_iterations * multiplier;
   }
}

}  // namespace

size_t pbkdf2(MessageAuthenticationCode& prf,
              uint8_t out[],
              size_t out_len,
              std::string_view password,
              const uint8_t salt[],
              size_t salt_len,
              size_t iterations,
              std::chrono::milliseconds msec) {
   if(iterations == 0) {
      iterations = tune_pbkdf2(prf, out_len, msec.count());
   }

   const PBKDF2 pbkdf2(prf, iterations);

   pbkdf2.derive_key(out, out_len, password.data(), password.size(), salt, salt_len);

   return iterations;
}

void pbkdf2(MessageAuthenticationCode& prf,
            uint8_t out[],
            size_t out_len,
            const uint8_t salt[],
            size_t salt_len,
            size_t iterations) {
   if(iterations == 0) {
      throw Invalid_Argument("PBKDF2: Invalid iteration count");
   }

   clear_mem(out, out_len);

   if(out_len == 0) {
      return;
   }

   const size_t prf_sz = prf.output_length();
   BOTAN_ASSERT_NOMSG(prf_sz > 0);

   // RFC 2898 Section 5.2: derived key length limited to (2^32 - 1) * hLen
   const auto blocks_required = ceil_division<uint64_t>(out_len, prf_sz);
   BOTAN_ARG_CHECK(blocks_required <= 0xFFFFFFFE, "PBKDF2 maximum output length exceeded");

   secure_vector<uint8_t> U(prf_sz);

   uint32_t counter = 1;
   while(out_len > 0) {
      const size_t prf_output = std::min<size_t>(prf_sz, out_len);

      prf.update(salt, salt_len);
      prf.update_be(counter++);
      prf.final(U.data());

      xor_buf(out, U.data(), prf_output);

      for(size_t i = 1; i != iterations; ++i) {
         prf.update(U);
         prf.final(U.data());
         xor_buf(out, U.data(), prf_output);
      }

      out_len -= prf_output;
      out += prf_output;
   }
}

// PBKDF interface
size_t PKCS5_PBKDF2::pbkdf(uint8_t key[],
                           size_t key_len,
                           std::string_view password,
                           const uint8_t salt[],
                           size_t salt_len,
                           size_t iterations,
                           std::chrono::milliseconds msec) const {
   if(iterations == 0) {
      iterations = tune_pbkdf2(*m_mac, key_len, msec.count());
   }

   const PBKDF2 pbkdf2(*m_mac, iterations);

   pbkdf2.derive_key(key, key_len, password.data(), password.size(), salt, salt_len);

   return iterations;
}

std::string PKCS5_PBKDF2::name() const {
   return fmt("PBKDF2({})", m_mac->name());
}

std::unique_ptr<PBKDF> PKCS5_PBKDF2::new_object() const {
   return std::make_unique<PKCS5_PBKDF2>(m_mac->new_object());
}

// PasswordHash interface

PBKDF2::PBKDF2(const MessageAuthenticationCode& prf, size_t olen, std::chrono::milliseconds msec) :
      m_prf(prf.new_object()), m_iterations(tune_pbkdf2(*m_prf, olen, msec.count())) {}

std::string PBKDF2::to_string() const {
   return fmt("PBKDF2({},{})", m_prf->name(), m_iterations);
}

void PBKDF2::derive_key(uint8_t out[],
                        size_t out_len,
                        const char* password,
                        const size_t password_len,
                        const uint8_t salt[],
                        size_t salt_len) const {
   pbkdf2_set_key(*m_prf, password, password_len);
   pbkdf2(*m_prf, out, out_len, salt, salt_len, m_iterations);
}

std::string PBKDF2_Family::name() const {
   return fmt("PBKDF2({})", m_prf->name());
}

std::unique_ptr<PasswordHash> PBKDF2_Family::tune_params(size_t output_len,
                                                         uint64_t desired_runtime_msec,
                                                         std::optional<size_t> /*max_memory*/,
                                                         uint64_t tune_msec) const {
   auto iterations = tune_pbkdf2(*m_prf, output_len, desired_runtime_msec, tune_msec);
   return std::make_unique<PBKDF2>(*m_prf, iterations);
}

std::unique_ptr<PasswordHash> PBKDF2_Family::default_params() const {
   return std::make_unique<PBKDF2>(*m_prf, 150000);
}

std::unique_ptr<PasswordHash> PBKDF2_Family::from_params(size_t iter, size_t /*i2*/, size_t /*i3*/) const {
   return std::make_unique<PBKDF2>(*m_prf, iter);
}

std::unique_ptr<PasswordHash> PBKDF2_Family::from_iterations(size_t iter) const {
   return std::make_unique<PBKDF2>(*m_prf, iter);
}

}  // namespace Botan
/*
* (C) 2017,2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

namespace {

/*
* The minimum weight irreducible binary polynomial of size n
*
* See "Table of Low-Weight Binary Irreducible Polynomials"
* by Gadiel Seroussi, HP Labs Tech Report HPL-98-135
* https://shiftleft.com/mirrors/www.hpl.hp.com/techreports/98/HPL-98-135.pdf
*/
enum class MinWeightPolynomial : uint32_t {
   P64 = 0x1B,
   P128 = 0x87,
   P192 = 0x87,
   P256 = 0x425,
   P512 = 0x125,
   P1024 = 0x80043,
};

/**
* If the top bit of c is set, returns the carry (the polynomial)
*
* Otherwise returns zero.
*/
template <MinWeightPolynomial P>
inline uint64_t return_carry(uint64_t c) {
   return CT::Mask<uint64_t>::expand_top_bit(c).if_set_return(static_cast<uint64_t>(P));
}

template <size_t LIMBS, MinWeightPolynomial P>
void poly_double(uint8_t out[], const uint8_t in[]) {
   uint64_t W[LIMBS];
   load_be(W, in, LIMBS);

   const uint64_t carry = return_carry<P>(W[0]);

   if constexpr(LIMBS > 0) {
      for(size_t i = 0; i != LIMBS - 1; ++i) {
         W[i] = (W[i] << 1) ^ (W[i + 1] >> 63);
      }
   }

   W[LIMBS - 1] = (W[LIMBS - 1] << 1) ^ carry;

   copy_out_be(std::span(out, LIMBS * 8), W);
}

template <size_t LIMBS, MinWeightPolynomial P>
void poly_double_le(uint8_t out[], const uint8_t in[]) {
   uint64_t W[LIMBS];
   load_le(W, in, LIMBS);

   const uint64_t carry = return_carry<P>(W[LIMBS - 1]);

   if constexpr(LIMBS > 0) {
      for(size_t i = 0; i != LIMBS - 1; ++i) {
         W[LIMBS - 1 - i] = (W[LIMBS - 1 - i] << 1) ^ (W[LIMBS - 2 - i] >> 63);
      }
   }

   W[0] = (W[0] << 1) ^ carry;

   copy_out_le(std::span(out, LIMBS * 8), W);
}

}  // namespace

void poly_double_n(uint8_t out[], const uint8_t in[], size_t n) {
   switch(n) {
      case 8:
         return poly_double<1, MinWeightPolynomial::P64>(out, in);
      case 16:
         return poly_double<2, MinWeightPolynomial::P128>(out, in);
      case 24:
         return poly_double<3, MinWeightPolynomial::P192>(out, in);
      case 32:
         return poly_double<4, MinWeightPolynomial::P256>(out, in);
      case 64:
         return poly_double<8, MinWeightPolynomial::P512>(out, in);
      case 128:
         return poly_double<16, MinWeightPolynomial::P1024>(out, in);
      default:
         throw Invalid_Argument("Unsupported size for poly_double_n");
   }
}

void poly_double_n_le(uint8_t out[], const uint8_t in[], size_t n) {
   switch(n) {
      case 8:
         return poly_double_le<1, MinWeightPolynomial::P64>(out, in);
      case 16:
         return poly_double_le<2, MinWeightPolynomial::P128>(out, in);
      case 24:
         return poly_double_le<3, MinWeightPolynomial::P192>(out, in);
      case 32:
         return poly_double_le<4, MinWeightPolynomial::P256>(out, in);
      case 64:
         return poly_double_le<8, MinWeightPolynomial::P512>(out, in);
      case 128:
         return poly_double_le<16, MinWeightPolynomial::P1024>(out, in);
      default:
         throw Invalid_Argument("Unsupported size for poly_double_n_le");
   }
}

void xts_compute_tweak_block(uint8_t tweak[], size_t BS, size_t blocks_in_tweak) {
   BOTAN_ASSERT_NOMSG(blocks_in_tweak > 0);

   if(BS == 16) {
      constexpr size_t LIMBS = 2;

      uint64_t W[LIMBS];
      load_le(W, &tweak[0], LIMBS);

      for(size_t i = 1; i < blocks_in_tweak; ++i) {
         const uint64_t carry = return_carry<MinWeightPolynomial::P128>(W[1]);
         W[1] = (W[1] << 1) ^ (W[0] >> 63);
         W[0] = (W[0] << 1) ^ carry;
         copy_out_le(std::span(&tweak[i * BS], 2 * 8), W);
      }
   } else {
      for(size_t i = 1; i < blocks_in_tweak; ++i) {
         const uint8_t* prev = &tweak[(i - 1) * BS];
         uint8_t* cur = &tweak[i * BS];
         poly_double_n_le(cur, prev, BS);
      }
   }
}

}  // namespace Botan
/*
* (C) 2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_ENTROPY_SOURCE)
#endif

#if defined(BOTAN_HAS_SYSTEM_RNG)
#endif

#if defined(BOTAN_HAS_OS_UTILS)
#endif


namespace Botan {

void RandomNumberGenerator::randomize_with_ts_input(std::span<uint8_t> output) {
   if(this->accepts_input()) {
      std::array<uint8_t, 32> additional_input = {0};

#if defined(BOTAN_HAS_OS_UTILS)
      store_le(std::span{additional_input}.subspan<0, 8>(), OS::get_high_resolution_clock());
      store_le(std::span{additional_input}.subspan<8, 4>(), OS::get_process_id());
      constexpr size_t offset = 12;
#else
      constexpr size_t offset = 0;
#endif

#if defined(BOTAN_HAS_SYSTEM_RNG)
      system_rng().randomize(std::span{additional_input}.subspan<offset>());
#else
      BOTAN_UNUSED(offset);
#endif

      this->fill_bytes_with_input(output, additional_input);
   } else {
      this->fill_bytes_with_input(output, {});
   }
}

size_t RandomNumberGenerator::reseed_from_sources(Entropy_Sources& srcs, size_t poll_bits) {
   if(this->accepts_input()) {
#if defined(BOTAN_HAS_ENTROPY_SOURCE)
      return srcs.poll(*this, poll_bits);
#else
      BOTAN_UNUSED(srcs, poll_bits);
#endif
   }

   return 0;
}

void RandomNumberGenerator::reseed_from_rng(RandomNumberGenerator& rng, size_t poll_bits) {
   if(this->accepts_input()) {
      this->add_entropy(rng.random_vec(poll_bits / 8));
   }
}

void Null_RNG::fill_bytes_with_input(std::span<uint8_t> output, std::span<const uint8_t> /* ignored */) {
   // throw if caller tries to obtain random bytes
   if(!output.empty()) {
      throw PRNG_Unseeded("Null_RNG called");
   }
}

}  // namespace Botan
/*
* Serpent
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_CPUID)
#endif

namespace Botan {

/*
* Serpent Encryption
*/
void Serpent::encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   using namespace Botan::Serpent_F;

   assert_key_material_set();

#if defined(BOTAN_HAS_SERPENT_AVX512)
   if(CPUID::has(CPUID::Feature::AVX512)) {
      while(blocks >= 16) {
         avx512_encrypt_16(in, out);
         in += 16 * BLOCK_SIZE;
         out += 16 * BLOCK_SIZE;
         blocks -= 16;
      }
   }
#endif

#if defined(BOTAN_HAS_SERPENT_AVX2)
   if(CPUID::has(CPUID::Feature::AVX2)) {
      while(blocks >= 8) {
         avx2_encrypt_8(in, out);
         in += 8 * BLOCK_SIZE;
         out += 8 * BLOCK_SIZE;
         blocks -= 8;
      }
   }
#endif

#if defined(BOTAN_HAS_SERPENT_SIMD)
   if(CPUID::has(CPUID::Feature::SIMD_4X32)) {
      while(blocks >= 4) {
         simd_encrypt_4(in, out);
         in += 4 * BLOCK_SIZE;
         out += 4 * BLOCK_SIZE;
         blocks -= 4;
      }
   }
#endif

   const Key_Inserter key_xor(m_round_key.data());

   for(size_t i = 0; i < blocks; ++i) {
      uint32_t B0 = 0;
      uint32_t B1 = 0;
      uint32_t B2 = 0;
      uint32_t B3 = 0;
      load_le(in + 16 * i, B0, B1, B2, B3);

      key_xor(0, B0, B1, B2, B3);
      SBoxE0(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(1, B0, B1, B2, B3);
      SBoxE1(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(2, B0, B1, B2, B3);
      SBoxE2(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(3, B0, B1, B2, B3);
      SBoxE3(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(4, B0, B1, B2, B3);
      SBoxE4(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(5, B0, B1, B2, B3);
      SBoxE5(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(6, B0, B1, B2, B3);
      SBoxE6(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(7, B0, B1, B2, B3);
      SBoxE7(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(8, B0, B1, B2, B3);
      SBoxE0(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(9, B0, B1, B2, B3);
      SBoxE1(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(10, B0, B1, B2, B3);
      SBoxE2(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(11, B0, B1, B2, B3);
      SBoxE3(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(12, B0, B1, B2, B3);
      SBoxE4(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(13, B0, B1, B2, B3);
      SBoxE5(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(14, B0, B1, B2, B3);
      SBoxE6(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(15, B0, B1, B2, B3);
      SBoxE7(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(16, B0, B1, B2, B3);
      SBoxE0(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(17, B0, B1, B2, B3);
      SBoxE1(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(18, B0, B1, B2, B3);
      SBoxE2(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(19, B0, B1, B2, B3);
      SBoxE3(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(20, B0, B1, B2, B3);
      SBoxE4(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(21, B0, B1, B2, B3);
      SBoxE5(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(22, B0, B1, B2, B3);
      SBoxE6(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(23, B0, B1, B2, B3);
      SBoxE7(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(24, B0, B1, B2, B3);
      SBoxE0(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(25, B0, B1, B2, B3);
      SBoxE1(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(26, B0, B1, B2, B3);
      SBoxE2(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(27, B0, B1, B2, B3);
      SBoxE3(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(28, B0, B1, B2, B3);
      SBoxE4(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(29, B0, B1, B2, B3);
      SBoxE5(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(30, B0, B1, B2, B3);
      SBoxE6(B0, B1, B2, B3);
      transform(B0, B1, B2, B3);
      key_xor(31, B0, B1, B2, B3);
      SBoxE7(B0, B1, B2, B3);
      key_xor(32, B0, B1, B2, B3);

      store_le(out + 16 * i, B0, B1, B2, B3);
   }
}

/*
* Serpent Decryption
*/
void Serpent::decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   using namespace Botan::Serpent_F;

   assert_key_material_set();

#if defined(BOTAN_HAS_SERPENT_AVX512)
   if(CPUID::has(CPUID::Feature::AVX512)) {
      while(blocks >= 16) {
         avx512_decrypt_16(in, out);
         in += 16 * BLOCK_SIZE;
         out += 16 * BLOCK_SIZE;
         blocks -= 16;
      }
   }
#endif

#if defined(BOTAN_HAS_SERPENT_AVX2)
   if(CPUID::has(CPUID::Feature::AVX2)) {
      while(blocks >= 8) {
         avx2_decrypt_8(in, out);
         in += 8 * BLOCK_SIZE;
         out += 8 * BLOCK_SIZE;
         blocks -= 8;
      }
   }
#endif

#if defined(BOTAN_HAS_SERPENT_SIMD)
   if(CPUID::has(CPUID::Feature::SIMD_4X32)) {
      while(blocks >= 4) {
         simd_decrypt_4(in, out);
         in += 4 * BLOCK_SIZE;
         out += 4 * BLOCK_SIZE;
         blocks -= 4;
      }
   }
#endif

   const Key_Inserter key_xor(m_round_key.data());

   for(size_t i = 0; i < blocks; ++i) {
      uint32_t B0 = 0;
      uint32_t B1 = 0;
      uint32_t B2 = 0;
      uint32_t B3 = 0;
      load_le(in + 16 * i, B0, B1, B2, B3);

      key_xor(32, B0, B1, B2, B3);
      SBoxD7(B0, B1, B2, B3);
      key_xor(31, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD6(B0, B1, B2, B3);
      key_xor(30, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD5(B0, B1, B2, B3);
      key_xor(29, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD4(B0, B1, B2, B3);
      key_xor(28, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD3(B0, B1, B2, B3);
      key_xor(27, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD2(B0, B1, B2, B3);
      key_xor(26, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD1(B0, B1, B2, B3);
      key_xor(25, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD0(B0, B1, B2, B3);
      key_xor(24, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD7(B0, B1, B2, B3);
      key_xor(23, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD6(B0, B1, B2, B3);
      key_xor(22, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD5(B0, B1, B2, B3);
      key_xor(21, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD4(B0, B1, B2, B3);
      key_xor(20, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD3(B0, B1, B2, B3);
      key_xor(19, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD2(B0, B1, B2, B3);
      key_xor(18, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD1(B0, B1, B2, B3);
      key_xor(17, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD0(B0, B1, B2, B3);
      key_xor(16, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD7(B0, B1, B2, B3);
      key_xor(15, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD6(B0, B1, B2, B3);
      key_xor(14, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD5(B0, B1, B2, B3);
      key_xor(13, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD4(B0, B1, B2, B3);
      key_xor(12, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD3(B0, B1, B2, B3);
      key_xor(11, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD2(B0, B1, B2, B3);
      key_xor(10, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD1(B0, B1, B2, B3);
      key_xor(9, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD0(B0, B1, B2, B3);
      key_xor(8, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD7(B0, B1, B2, B3);
      key_xor(7, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD6(B0, B1, B2, B3);
      key_xor(6, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD5(B0, B1, B2, B3);
      key_xor(5, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD4(B0, B1, B2, B3);
      key_xor(4, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD3(B0, B1, B2, B3);
      key_xor(3, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD2(B0, B1, B2, B3);
      key_xor(2, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD1(B0, B1, B2, B3);
      key_xor(1, B0, B1, B2, B3);
      i_transform(B0, B1, B2, B3);
      SBoxD0(B0, B1, B2, B3);
      key_xor(0, B0, B1, B2, B3);

      store_le(out + 16 * i, B0, B1, B2, B3);
   }
}

bool Serpent::has_keying_material() const {
   return !m_round_key.empty();
}

/*
* Serpent Key Schedule
*/
void Serpent::key_schedule(std::span<const uint8_t> key) {
   using namespace Botan::Serpent_F;

   const uint32_t PHI = 0x9E3779B9;

   secure_vector<uint32_t> W(140);
   for(size_t i = 0; i != key.size() / 4; ++i) {
      W[i] = load_le<uint32_t>(key.data(), i);
   }

   W[key.size() / 4] |= uint32_t(1) << ((key.size() % 4) * 8);

   for(size_t i = 8; i != 140; ++i) {
      const uint32_t wi = W[i - 8] ^ W[i - 5] ^ W[i - 3] ^ W[i - 1] ^ PHI ^ uint32_t(i - 8);
      W[i] = rotl<11>(wi);
   }

   SBoxE0(W[20], W[21], W[22], W[23]);
   SBoxE0(W[52], W[53], W[54], W[55]);
   SBoxE0(W[84], W[85], W[86], W[87]);
   SBoxE0(W[116], W[117], W[118], W[119]);

   SBoxE1(W[16], W[17], W[18], W[19]);
   SBoxE1(W[48], W[49], W[50], W[51]);
   SBoxE1(W[80], W[81], W[82], W[83]);
   SBoxE1(W[112], W[113], W[114], W[115]);

   SBoxE2(W[12], W[13], W[14], W[15]);
   SBoxE2(W[44], W[45], W[46], W[47]);
   SBoxE2(W[76], W[77], W[78], W[79]);
   SBoxE2(W[108], W[109], W[110], W[111]);

   SBoxE3(W[8], W[9], W[10], W[11]);
   SBoxE3(W[40], W[41], W[42], W[43]);
   SBoxE3(W[72], W[73], W[74], W[75]);
   SBoxE3(W[104], W[105], W[106], W[107]);
   SBoxE3(W[136], W[137], W[138], W[139]);

   SBoxE4(W[36], W[37], W[38], W[39]);
   SBoxE4(W[68], W[69], W[70], W[71]);
   SBoxE4(W[100], W[101], W[102], W[103]);
   SBoxE4(W[132], W[133], W[134], W[135]);

   SBoxE5(W[32], W[33], W[34], W[35]);
   SBoxE5(W[64], W[65], W[66], W[67]);
   SBoxE5(W[96], W[97], W[98], W[99]);
   SBoxE5(W[128], W[129], W[130], W[131]);

   SBoxE6(W[28], W[29], W[30], W[31]);
   SBoxE6(W[60], W[61], W[62], W[63]);
   SBoxE6(W[92], W[93], W[94], W[95]);
   SBoxE6(W[124], W[125], W[126], W[127]);

   SBoxE7(W[24], W[25], W[26], W[27]);
   SBoxE7(W[56], W[57], W[58], W[59]);
   SBoxE7(W[88], W[89], W[90], W[91]);
   SBoxE7(W[120], W[121], W[122], W[123]);

   m_round_key.assign(W.begin() + 8, W.end());
}

void Serpent::clear() {
   zap(m_round_key);
}

std::string Serpent::provider() const {
#if defined(BOTAN_HAS_SERPENT_AVX512)
   if(auto feat = CPUID::check(CPUID::Feature::AVX512)) {
      return *feat;
   }
#endif

#if defined(BOTAN_HAS_SERPENT_AVX2)
   if(auto feat = CPUID::check(CPUID::Feature::AVX2)) {
      return *feat;
   }
#endif

#if defined(BOTAN_HAS_SERPENT_SIMD)
   if(auto feat = CPUID::check(CPUID::Feature::SIMD_4X32)) {
      return *feat;
   }
#endif

   return "base";
}

}  // namespace Botan
/*
* SHA-{224,256}
* (C) 1999-2010,2017 Jack Lloyd
*     2007 FlexSecure GmbH
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_CPUID)
#endif

namespace Botan {

namespace {

std::string sha256_provider() {
#if defined(BOTAN_HAS_SHA2_32_ARMV8)
   if(auto feat = CPUID::check(CPUID::Feature::SHA2)) {
      return *feat;
   }
#endif

#if defined(BOTAN_HAS_SHA2_32_X86)
   if(auto feat = CPUID::check(CPUID::Feature::SHA)) {
      return *feat;
   }
#endif

#if defined(BOTAN_HAS_SHA2_32_X86_AVX2)
   if(auto feat = CPUID::check(CPUID::Feature::AVX2, CPUID::Feature::BMI)) {
      return *feat;
   }
#endif

#if defined(BOTAN_HAS_SHA2_32_SIMD)
   if(auto feat = CPUID::check(CPUID::Feature::SIMD_4X32)) {
      return *feat;
   }
#endif

   return "base";
}

}  // namespace

/*
* SHA-224 / SHA-256 compression function
*/
void BOTAN_SCRUB_STACK_AFTER_RETURN SHA_256::compress_digest(digest_type& digest,
                                                             std::span<const uint8_t> input,
                                                             size_t blocks) {
#if defined(BOTAN_HAS_SHA2_32_X86)
   if(CPUID::has(CPUID::Feature::SHA)) {
      return SHA_256::compress_digest_x86(digest, input, blocks);
   }
#endif

#if defined(BOTAN_HAS_SHA2_32_ARMV8)
   if(CPUID::has(CPUID::Feature::SHA2)) {
      return SHA_256::compress_digest_armv8(digest, input, blocks);
   }
#endif

#if defined(BOTAN_HAS_SHA2_32_X86_AVX2)
   if(CPUID::has(CPUID::Feature::AVX2, CPUID::Feature::BMI)) {
      return SHA_256::compress_digest_x86_avx2(digest, input, blocks);
   }
#endif

#if defined(BOTAN_HAS_SHA2_32_SIMD)
   if(CPUID::has(CPUID::Feature::SIMD_4X32)) {
      return SHA_256::compress_digest_x86_simd(digest, input, blocks);
   }
#endif

   uint32_t A = digest[0];
   uint32_t B = digest[1];
   uint32_t C = digest[2];
   uint32_t D = digest[3];
   uint32_t E = digest[4];
   uint32_t F = digest[5];
   uint32_t G = digest[6];
   uint32_t H = digest[7];

   std::array<uint32_t, 16> W{};

   BufferSlicer in(input);

   for(size_t i = 0; i != blocks; ++i) {
      load_be(W, in.take<block_bytes>());

      // clang-format off

      SHA2_32_F(A, B, C, D, E, F, G, H, W[ 0], W[14], W[ 9], W[ 1], 0x428A2F98);
      SHA2_32_F(H, A, B, C, D, E, F, G, W[ 1], W[15], W[10], W[ 2], 0x71374491);
      SHA2_32_F(G, H, A, B, C, D, E, F, W[ 2], W[ 0], W[11], W[ 3], 0xB5C0FBCF);
      SHA2_32_F(F, G, H, A, B, C, D, E, W[ 3], W[ 1], W[12], W[ 4], 0xE9B5DBA5);
      SHA2_32_F(E, F, G, H, A, B, C, D, W[ 4], W[ 2], W[13], W[ 5], 0x3956C25B);
      SHA2_32_F(D, E, F, G, H, A, B, C, W[ 5], W[ 3], W[14], W[ 6], 0x59F111F1);
      SHA2_32_F(C, D, E, F, G, H, A, B, W[ 6], W[ 4], W[15], W[ 7], 0x923F82A4);
      SHA2_32_F(B, C, D, E, F, G, H, A, W[ 7], W[ 5], W[ 0], W[ 8], 0xAB1C5ED5);
      SHA2_32_F(A, B, C, D, E, F, G, H, W[ 8], W[ 6], W[ 1], W[ 9], 0xD807AA98);
      SHA2_32_F(H, A, B, C, D, E, F, G, W[ 9], W[ 7], W[ 2], W[10], 0x12835B01);
      SHA2_32_F(G, H, A, B, C, D, E, F, W[10], W[ 8], W[ 3], W[11], 0x243185BE);
      SHA2_32_F(F, G, H, A, B, C, D, E, W[11], W[ 9], W[ 4], W[12], 0x550C7DC3);
      SHA2_32_F(E, F, G, H, A, B, C, D, W[12], W[10], W[ 5], W[13], 0x72BE5D74);
      SHA2_32_F(D, E, F, G, H, A, B, C, W[13], W[11], W[ 6], W[14], 0x80DEB1FE);
      SHA2_32_F(C, D, E, F, G, H, A, B, W[14], W[12], W[ 7], W[15], 0x9BDC06A7);
      SHA2_32_F(B, C, D, E, F, G, H, A, W[15], W[13], W[ 8], W[ 0], 0xC19BF174);

      SHA2_32_F(A, B, C, D, E, F, G, H, W[ 0], W[14], W[ 9], W[ 1], 0xE49B69C1);
      SHA2_32_F(H, A, B, C, D, E, F, G, W[ 1], W[15], W[10], W[ 2], 0xEFBE4786);
      SHA2_32_F(G, H, A, B, C, D, E, F, W[ 2], W[ 0], W[11], W[ 3], 0x0FC19DC6);
      SHA2_32_F(F, G, H, A, B, C, D, E, W[ 3], W[ 1], W[12], W[ 4], 0x240CA1CC);
      SHA2_32_F(E, F, G, H, A, B, C, D, W[ 4], W[ 2], W[13], W[ 5], 0x2DE92C6F);
      SHA2_32_F(D, E, F, G, H, A, B, C, W[ 5], W[ 3], W[14], W[ 6], 0x4A7484AA);
      SHA2_32_F(C, D, E, F, G, H, A, B, W[ 6], W[ 4], W[15], W[ 7], 0x5CB0A9DC);
      SHA2_32_F(B, C, D, E, F, G, H, A, W[ 7], W[ 5], W[ 0], W[ 8], 0x76F988DA);
      SHA2_32_F(A, B, C, D, E, F, G, H, W[ 8], W[ 6], W[ 1], W[ 9], 0x983E5152);
      SHA2_32_F(H, A, B, C, D, E, F, G, W[ 9], W[ 7], W[ 2], W[10], 0xA831C66D);
      SHA2_32_F(G, H, A, B, C, D, E, F, W[10], W[ 8], W[ 3], W[11], 0xB00327C8);
      SHA2_32_F(F, G, H, A, B, C, D, E, W[11], W[ 9], W[ 4], W[12], 0xBF597FC7);
      SHA2_32_F(E, F, G, H, A, B, C, D, W[12], W[10], W[ 5], W[13], 0xC6E00BF3);
      SHA2_32_F(D, E, F, G, H, A, B, C, W[13], W[11], W[ 6], W[14], 0xD5A79147);
      SHA2_32_F(C, D, E, F, G, H, A, B, W[14], W[12], W[ 7], W[15], 0x06CA6351);
      SHA2_32_F(B, C, D, E, F, G, H, A, W[15], W[13], W[ 8], W[ 0], 0x14292967);

      SHA2_32_F(A, B, C, D, E, F, G, H, W[ 0], W[14], W[ 9], W[ 1], 0x27B70A85);
      SHA2_32_F(H, A, B, C, D, E, F, G, W[ 1], W[15], W[10], W[ 2], 0x2E1B2138);
      SHA2_32_F(G, H, A, B, C, D, E, F, W[ 2], W[ 0], W[11], W[ 3], 0x4D2C6DFC);
      SHA2_32_F(F, G, H, A, B, C, D, E, W[ 3], W[ 1], W[12], W[ 4], 0x53380D13);
      SHA2_32_F(E, F, G, H, A, B, C, D, W[ 4], W[ 2], W[13], W[ 5], 0x650A7354);
      SHA2_32_F(D, E, F, G, H, A, B, C, W[ 5], W[ 3], W[14], W[ 6], 0x766A0ABB);
      SHA2_32_F(C, D, E, F, G, H, A, B, W[ 6], W[ 4], W[15], W[ 7], 0x81C2C92E);
      SHA2_32_F(B, C, D, E, F, G, H, A, W[ 7], W[ 5], W[ 0], W[ 8], 0x92722C85);
      SHA2_32_F(A, B, C, D, E, F, G, H, W[ 8], W[ 6], W[ 1], W[ 9], 0xA2BFE8A1);
      SHA2_32_F(H, A, B, C, D, E, F, G, W[ 9], W[ 7], W[ 2], W[10], 0xA81A664B);
      SHA2_32_F(G, H, A, B, C, D, E, F, W[10], W[ 8], W[ 3], W[11], 0xC24B8B70);
      SHA2_32_F(F, G, H, A, B, C, D, E, W[11], W[ 9], W[ 4], W[12], 0xC76C51A3);
      SHA2_32_F(E, F, G, H, A, B, C, D, W[12], W[10], W[ 5], W[13], 0xD192E819);
      SHA2_32_F(D, E, F, G, H, A, B, C, W[13], W[11], W[ 6], W[14], 0xD6990624);
      SHA2_32_F(C, D, E, F, G, H, A, B, W[14], W[12], W[ 7], W[15], 0xF40E3585);
      SHA2_32_F(B, C, D, E, F, G, H, A, W[15], W[13], W[ 8], W[ 0], 0x106AA070);

      SHA2_32_F(A, B, C, D, E, F, G, H, W[ 0], W[14], W[ 9], W[ 1], 0x19A4C116);
      SHA2_32_F(H, A, B, C, D, E, F, G, W[ 1], W[15], W[10], W[ 2], 0x1E376C08);
      SHA2_32_F(G, H, A, B, C, D, E, F, W[ 2], W[ 0], W[11], W[ 3], 0x2748774C);
      SHA2_32_F(F, G, H, A, B, C, D, E, W[ 3], W[ 1], W[12], W[ 4], 0x34B0BCB5);
      SHA2_32_F(E, F, G, H, A, B, C, D, W[ 4], W[ 2], W[13], W[ 5], 0x391C0CB3);
      SHA2_32_F(D, E, F, G, H, A, B, C, W[ 5], W[ 3], W[14], W[ 6], 0x4ED8AA4A);
      SHA2_32_F(C, D, E, F, G, H, A, B, W[ 6], W[ 4], W[15], W[ 7], 0x5B9CCA4F);
      SHA2_32_F(B, C, D, E, F, G, H, A, W[ 7], W[ 5], W[ 0], W[ 8], 0x682E6FF3);
      SHA2_32_F(A, B, C, D, E, F, G, H, W[ 8], W[ 6], W[ 1], W[ 9], 0x748F82EE);
      SHA2_32_F(H, A, B, C, D, E, F, G, W[ 9], W[ 7], W[ 2], W[10], 0x78A5636F);
      SHA2_32_F(G, H, A, B, C, D, E, F, W[10], W[ 8], W[ 3], W[11], 0x84C87814);
      SHA2_32_F(F, G, H, A, B, C, D, E, W[11], W[ 9], W[ 4], W[12], 0x8CC70208);
      SHA2_32_F(E, F, G, H, A, B, C, D, W[12], W[10], W[ 5], W[13], 0x90BEFFFA);
      SHA2_32_F(D, E, F, G, H, A, B, C, W[13], W[11], W[ 6], W[14], 0xA4506CEB);
      SHA2_32_F(C, D, E, F, G, H, A, B, W[14], W[12], W[ 7], W[15], 0xBEF9A3F7);
      SHA2_32_F(B, C, D, E, F, G, H, A, W[15], W[13], W[ 8], W[ 0], 0xC67178F2);

      // clang-format on

      A = (digest[0] += A);
      B = (digest[1] += B);
      C = (digest[2] += C);
      D = (digest[3] += D);
      E = (digest[4] += E);
      F = (digest[5] += F);
      G = (digest[6] += G);
      H = (digest[7] += H);
   }
}

std::string SHA_224::provider() const {
   return sha256_provider();
}

void SHA_224::compress_n(digest_type& digest, std::span<const uint8_t> input, size_t blocks) {
   SHA_256::compress_digest(digest, input, blocks);
}

void SHA_224::init(digest_type& digest) {
   digest.assign({0xC1059ED8, 0x367CD507, 0x3070DD17, 0xF70E5939, 0xFFC00B31, 0x68581511, 0x64F98FA7, 0xBEFA4FA4});
}

std::unique_ptr<HashFunction> SHA_224::new_object() const {
   return std::make_unique<SHA_224>();
}

std::unique_ptr<HashFunction> SHA_224::copy_state() const {
   return std::make_unique<SHA_224>(*this);
}

void SHA_224::add_data(std::span<const uint8_t> input) {
   m_md.update(input);
}

void SHA_224::final_result(std::span<uint8_t> output) {
   m_md.final(output);
}

std::string SHA_256::provider() const {
   return sha256_provider();
}

void SHA_256::compress_n(digest_type& digest, std::span<const uint8_t> input, size_t blocks) {
   SHA_256::compress_digest(digest, input, blocks);
}

void SHA_256::init(digest_type& digest) {
   digest.assign({0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A, 0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19});
}

std::unique_ptr<HashFunction> SHA_256::new_object() const {
   return std::make_unique<SHA_256>();
}

std::unique_ptr<HashFunction> SHA_256::copy_state() const {
   return std::make_unique<SHA_256>(*this);
}

void SHA_256::add_data(std::span<const uint8_t> input) {
   m_md.update(input);
}

void SHA_256::final_result(std::span<uint8_t> output) {
   m_md.final(output);
}

}  // namespace Botan
/*
* SHA-256 using CPU instructions in ARMv8
*
* Contributed by Jeffrey Walton. Based on public domain code by
* Johannes Schneiders, Skip Hovsmith and Barry O'Rourke.
*
* Further changes (C) 2020,2025 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

namespace {

inline BOTAN_FN_ISA_SHA2 SIMD_4x32 aarch64_sha256_expand_w(const SIMD_4x32 w0,
                                                           const SIMD_4x32 w1,
                                                           const SIMD_4x32 w2,
                                                           const SIMD_4x32 w3) {
   return SIMD_4x32(vsha256su1q_u32(vsha256su0q_u32(w0.raw(), w1.raw()), w2.raw(), w3.raw()));
}

inline BOTAN_FN_ISA_SHA2 void aarch64_sha256_update(SIMD_4x32& s0,
                                                    SIMD_4x32& s1,
                                                    const SIMD_4x32 w,
                                                    const uint32_t K[4]) {
   auto w_k = w + SIMD_4x32::load_le(K);
   auto t = vsha256hq_u32(s0.raw(), s1.raw(), w_k.raw());
   s1 = SIMD_4x32(vsha256h2q_u32(s1.raw(), s0.raw(), w_k.raw()));
   s0 = SIMD_4x32(t);
}

}  // namespace

/*
* SHA-256 using CPU instructions in ARMv8
*/
//static
void BOTAN_FN_ISA_SHA2 BOTAN_SCRUB_STACK_AFTER_RETURN SHA_256::compress_digest_armv8(digest_type& digest,
                                                                                     std::span<const uint8_t> input8,
                                                                                     size_t blocks) {
   alignas(64) static const uint32_t K[] = {
      0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
      0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3, 0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
      0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
      0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
      0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13, 0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
      0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
      0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
      0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208, 0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2,
   };

   // Load initial values
   SIMD_4x32 s0 = SIMD_4x32::load_le(&digest[0]);  // NOLINT(*-container-data-pointer)
   SIMD_4x32 s1 = SIMD_4x32::load_le(&digest[4]);

   const uint32_t* input32 = reinterpret_cast<const uint32_t*>(input8.data());

   while(blocks > 0) {
      const auto s0_save = s0;
      const auto s1_save = s1;

      auto w0 = SIMD_4x32::load_be(input32);
      auto w1 = SIMD_4x32::load_be(input32 + 4);
      auto w2 = SIMD_4x32::load_be(input32 + 8);
      auto w3 = SIMD_4x32::load_be(input32 + 12);

      for(size_t r = 0; r != 48; r += 16) {
         aarch64_sha256_update(s0, s1, w0, &K[r]);
         w0 = aarch64_sha256_expand_w(w0, w1, w2, w3);

         aarch64_sha256_update(s0, s1, w1, &K[r + 4 * 1]);
         w1 = aarch64_sha256_expand_w(w1, w2, w3, w0);

         aarch64_sha256_update(s0, s1, w2, &K[r + 4 * 2]);
         w2 = aarch64_sha256_expand_w(w2, w3, w0, w1);

         aarch64_sha256_update(s0, s1, w3, &K[r + 4 * 3]);
         w3 = aarch64_sha256_expand_w(w3, w0, w1, w2);
      }

      aarch64_sha256_update(s0, s1, w0, &K[4 * 12]);
      aarch64_sha256_update(s0, s1, w1, &K[4 * 13]);
      aarch64_sha256_update(s0, s1, w2, &K[4 * 14]);
      aarch64_sha256_update(s0, s1, w3, &K[4 * 15]);

      s0 += s0_save;
      s1 += s1_save;

      input32 += 64 / 4;
      blocks--;
   }

   s0.store_le(&digest[0]);  // NOLINT(*-container-data-pointer)
   s1.store_le(&digest[4]);
}

}  // namespace Botan
/*
* SHA-{384,512}
* (C) 1999-2011,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_CPUID)
#endif

namespace Botan {

namespace {

std::string sha512_provider() {
#if defined(BOTAN_HAS_SHA2_64_X86)
   if(auto feat = CPUID::check(CPUID::Feature::SHA512)) {
      return *feat;
   }
#endif

#if defined(BOTAN_HAS_SHA2_64_ARMV8)
   if(auto feat = CPUID::check(CPUID::Feature::SHA2_512)) {
      return *feat;
   }
#endif

#if defined(BOTAN_HAS_SHA2_64_X86_AVX512)
   if(auto feat = CPUID::check(CPUID::Feature::AVX512, CPUID::Feature::BMI)) {
      return *feat;
   }
#endif

#if defined(BOTAN_HAS_SHA2_64_X86_AVX2)
   if(auto feat = CPUID::check(CPUID::Feature::AVX2, CPUID::Feature::BMI)) {
      return *feat;
   }
#endif

   return "base";
}

}  // namespace

/*
* SHA-{384,512} Compression Function
*/
//static
void SHA_512::compress_digest(digest_type& digest, std::span<const uint8_t> input, size_t blocks) {
#if defined(BOTAN_HAS_SHA2_64_X86)
   if(CPUID::has(CPUID::Feature::SHA512)) {
      return compress_digest_x86(digest, input, blocks);
   }
#endif

#if defined(BOTAN_HAS_SHA2_64_ARMV8)
   if(CPUID::has(CPUID::Feature::SHA2_512)) {
      return compress_digest_armv8(digest, input, blocks);
   }
#endif

#if defined(BOTAN_HAS_SHA2_64_X86_AVX512)
   if(CPUID::has(CPUID::Feature::AVX512, CPUID::Feature::BMI)) {
      return compress_digest_x86_avx512(digest, input, blocks);
   }
#endif

#if defined(BOTAN_HAS_SHA2_64_X86_AVX2)
   if(CPUID::has(CPUID::Feature::AVX2, CPUID::Feature::BMI)) {
      return compress_digest_x86_avx2(digest, input, blocks);
   }
#endif

   uint64_t A = digest[0];
   uint64_t B = digest[1];
   uint64_t C = digest[2];
   uint64_t D = digest[3];
   uint64_t E = digest[4];
   uint64_t F = digest[5];
   uint64_t G = digest[6];
   uint64_t H = digest[7];

   std::array<uint64_t, 16> W{};

   BufferSlicer in(input);

   for(size_t i = 0; i != blocks; ++i) {
      load_be(W, in.take<block_bytes>());

      // clang-format off

      SHA2_64_F(A, B, C, D, E, F, G, H, W[ 0], W[14], W[ 9], W[ 1], 0x428A2F98D728AE22);
      SHA2_64_F(H, A, B, C, D, E, F, G, W[ 1], W[15], W[10], W[ 2], 0x7137449123EF65CD);
      SHA2_64_F(G, H, A, B, C, D, E, F, W[ 2], W[ 0], W[11], W[ 3], 0xB5C0FBCFEC4D3B2F);
      SHA2_64_F(F, G, H, A, B, C, D, E, W[ 3], W[ 1], W[12], W[ 4], 0xE9B5DBA58189DBBC);
      SHA2_64_F(E, F, G, H, A, B, C, D, W[ 4], W[ 2], W[13], W[ 5], 0x3956C25BF348B538);
      SHA2_64_F(D, E, F, G, H, A, B, C, W[ 5], W[ 3], W[14], W[ 6], 0x59F111F1B605D019);
      SHA2_64_F(C, D, E, F, G, H, A, B, W[ 6], W[ 4], W[15], W[ 7], 0x923F82A4AF194F9B);
      SHA2_64_F(B, C, D, E, F, G, H, A, W[ 7], W[ 5], W[ 0], W[ 8], 0xAB1C5ED5DA6D8118);
      SHA2_64_F(A, B, C, D, E, F, G, H, W[ 8], W[ 6], W[ 1], W[ 9], 0xD807AA98A3030242);
      SHA2_64_F(H, A, B, C, D, E, F, G, W[ 9], W[ 7], W[ 2], W[10], 0x12835B0145706FBE);
      SHA2_64_F(G, H, A, B, C, D, E, F, W[10], W[ 8], W[ 3], W[11], 0x243185BE4EE4B28C);
      SHA2_64_F(F, G, H, A, B, C, D, E, W[11], W[ 9], W[ 4], W[12], 0x550C7DC3D5FFB4E2);
      SHA2_64_F(E, F, G, H, A, B, C, D, W[12], W[10], W[ 5], W[13], 0x72BE5D74F27B896F);
      SHA2_64_F(D, E, F, G, H, A, B, C, W[13], W[11], W[ 6], W[14], 0x80DEB1FE3B1696B1);
      SHA2_64_F(C, D, E, F, G, H, A, B, W[14], W[12], W[ 7], W[15], 0x9BDC06A725C71235);
      SHA2_64_F(B, C, D, E, F, G, H, A, W[15], W[13], W[ 8], W[ 0], 0xC19BF174CF692694);
      SHA2_64_F(A, B, C, D, E, F, G, H, W[ 0], W[14], W[ 9], W[ 1], 0xE49B69C19EF14AD2);
      SHA2_64_F(H, A, B, C, D, E, F, G, W[ 1], W[15], W[10], W[ 2], 0xEFBE4786384F25E3);
      SHA2_64_F(G, H, A, B, C, D, E, F, W[ 2], W[ 0], W[11], W[ 3], 0x0FC19DC68B8CD5B5);
      SHA2_64_F(F, G, H, A, B, C, D, E, W[ 3], W[ 1], W[12], W[ 4], 0x240CA1CC77AC9C65);
      SHA2_64_F(E, F, G, H, A, B, C, D, W[ 4], W[ 2], W[13], W[ 5], 0x2DE92C6F592B0275);
      SHA2_64_F(D, E, F, G, H, A, B, C, W[ 5], W[ 3], W[14], W[ 6], 0x4A7484AA6EA6E483);
      SHA2_64_F(C, D, E, F, G, H, A, B, W[ 6], W[ 4], W[15], W[ 7], 0x5CB0A9DCBD41FBD4);
      SHA2_64_F(B, C, D, E, F, G, H, A, W[ 7], W[ 5], W[ 0], W[ 8], 0x76F988DA831153B5);
      SHA2_64_F(A, B, C, D, E, F, G, H, W[ 8], W[ 6], W[ 1], W[ 9], 0x983E5152EE66DFAB);
      SHA2_64_F(H, A, B, C, D, E, F, G, W[ 9], W[ 7], W[ 2], W[10], 0xA831C66D2DB43210);
      SHA2_64_F(G, H, A, B, C, D, E, F, W[10], W[ 8], W[ 3], W[11], 0xB00327C898FB213F);
      SHA2_64_F(F, G, H, A, B, C, D, E, W[11], W[ 9], W[ 4], W[12], 0xBF597FC7BEEF0EE4);
      SHA2_64_F(E, F, G, H, A, B, C, D, W[12], W[10], W[ 5], W[13], 0xC6E00BF33DA88FC2);
      SHA2_64_F(D, E, F, G, H, A, B, C, W[13], W[11], W[ 6], W[14], 0xD5A79147930AA725);
      SHA2_64_F(C, D, E, F, G, H, A, B, W[14], W[12], W[ 7], W[15], 0x06CA6351E003826F);
      SHA2_64_F(B, C, D, E, F, G, H, A, W[15], W[13], W[ 8], W[ 0], 0x142929670A0E6E70);
      SHA2_64_F(A, B, C, D, E, F, G, H, W[ 0], W[14], W[ 9], W[ 1], 0x27B70A8546D22FFC);
      SHA2_64_F(H, A, B, C, D, E, F, G, W[ 1], W[15], W[10], W[ 2], 0x2E1B21385C26C926);
      SHA2_64_F(G, H, A, B, C, D, E, F, W[ 2], W[ 0], W[11], W[ 3], 0x4D2C6DFC5AC42AED);
      SHA2_64_F(F, G, H, A, B, C, D, E, W[ 3], W[ 1], W[12], W[ 4], 0x53380D139D95B3DF);
      SHA2_64_F(E, F, G, H, A, B, C, D, W[ 4], W[ 2], W[13], W[ 5], 0x650A73548BAF63DE);
      SHA2_64_F(D, E, F, G, H, A, B, C, W[ 5], W[ 3], W[14], W[ 6], 0x766A0ABB3C77B2A8);
      SHA2_64_F(C, D, E, F, G, H, A, B, W[ 6], W[ 4], W[15], W[ 7], 0x81C2C92E47EDAEE6);
      SHA2_64_F(B, C, D, E, F, G, H, A, W[ 7], W[ 5], W[ 0], W[ 8], 0x92722C851482353B);
      SHA2_64_F(A, B, C, D, E, F, G, H, W[ 8], W[ 6], W[ 1], W[ 9], 0xA2BFE8A14CF10364);
      SHA2_64_F(H, A, B, C, D, E, F, G, W[ 9], W[ 7], W[ 2], W[10], 0xA81A664BBC423001);
      SHA2_64_F(G, H, A, B, C, D, E, F, W[10], W[ 8], W[ 3], W[11], 0xC24B8B70D0F89791);
      SHA2_64_F(F, G, H, A, B, C, D, E, W[11], W[ 9], W[ 4], W[12], 0xC76C51A30654BE30);
      SHA2_64_F(E, F, G, H, A, B, C, D, W[12], W[10], W[ 5], W[13], 0xD192E819D6EF5218);
      SHA2_64_F(D, E, F, G, H, A, B, C, W[13], W[11], W[ 6], W[14], 0xD69906245565A910);
      SHA2_64_F(C, D, E, F, G, H, A, B, W[14], W[12], W[ 7], W[15], 0xF40E35855771202A);
      SHA2_64_F(B, C, D, E, F, G, H, A, W[15], W[13], W[ 8], W[ 0], 0x106AA07032BBD1B8);
      SHA2_64_F(A, B, C, D, E, F, G, H, W[ 0], W[14], W[ 9], W[ 1], 0x19A4C116B8D2D0C8);
      SHA2_64_F(H, A, B, C, D, E, F, G, W[ 1], W[15], W[10], W[ 2], 0x1E376C085141AB53);
      SHA2_64_F(G, H, A, B, C, D, E, F, W[ 2], W[ 0], W[11], W[ 3], 0x2748774CDF8EEB99);
      SHA2_64_F(F, G, H, A, B, C, D, E, W[ 3], W[ 1], W[12], W[ 4], 0x34B0BCB5E19B48A8);
      SHA2_64_F(E, F, G, H, A, B, C, D, W[ 4], W[ 2], W[13], W[ 5], 0x391C0CB3C5C95A63);
      SHA2_64_F(D, E, F, G, H, A, B, C, W[ 5], W[ 3], W[14], W[ 6], 0x4ED8AA4AE3418ACB);
      SHA2_64_F(C, D, E, F, G, H, A, B, W[ 6], W[ 4], W[15], W[ 7], 0x5B9CCA4F7763E373);
      SHA2_64_F(B, C, D, E, F, G, H, A, W[ 7], W[ 5], W[ 0], W[ 8], 0x682E6FF3D6B2B8A3);
      SHA2_64_F(A, B, C, D, E, F, G, H, W[ 8], W[ 6], W[ 1], W[ 9], 0x748F82EE5DEFB2FC);
      SHA2_64_F(H, A, B, C, D, E, F, G, W[ 9], W[ 7], W[ 2], W[10], 0x78A5636F43172F60);
      SHA2_64_F(G, H, A, B, C, D, E, F, W[10], W[ 8], W[ 3], W[11], 0x84C87814A1F0AB72);
      SHA2_64_F(F, G, H, A, B, C, D, E, W[11], W[ 9], W[ 4], W[12], 0x8CC702081A6439EC);
      SHA2_64_F(E, F, G, H, A, B, C, D, W[12], W[10], W[ 5], W[13], 0x90BEFFFA23631E28);
      SHA2_64_F(D, E, F, G, H, A, B, C, W[13], W[11], W[ 6], W[14], 0xA4506CEBDE82BDE9);
      SHA2_64_F(C, D, E, F, G, H, A, B, W[14], W[12], W[ 7], W[15], 0xBEF9A3F7B2C67915);
      SHA2_64_F(B, C, D, E, F, G, H, A, W[15], W[13], W[ 8], W[ 0], 0xC67178F2E372532B);
      SHA2_64_F(A, B, C, D, E, F, G, H, W[ 0], W[14], W[ 9], W[ 1], 0xCA273ECEEA26619C);
      SHA2_64_F(H, A, B, C, D, E, F, G, W[ 1], W[15], W[10], W[ 2], 0xD186B8C721C0C207);
      SHA2_64_F(G, H, A, B, C, D, E, F, W[ 2], W[ 0], W[11], W[ 3], 0xEADA7DD6CDE0EB1E);
      SHA2_64_F(F, G, H, A, B, C, D, E, W[ 3], W[ 1], W[12], W[ 4], 0xF57D4F7FEE6ED178);
      SHA2_64_F(E, F, G, H, A, B, C, D, W[ 4], W[ 2], W[13], W[ 5], 0x06F067AA72176FBA);
      SHA2_64_F(D, E, F, G, H, A, B, C, W[ 5], W[ 3], W[14], W[ 6], 0x0A637DC5A2C898A6);
      SHA2_64_F(C, D, E, F, G, H, A, B, W[ 6], W[ 4], W[15], W[ 7], 0x113F9804BEF90DAE);
      SHA2_64_F(B, C, D, E, F, G, H, A, W[ 7], W[ 5], W[ 0], W[ 8], 0x1B710B35131C471B);
      SHA2_64_F(A, B, C, D, E, F, G, H, W[ 8], W[ 6], W[ 1], W[ 9], 0x28DB77F523047D84);
      SHA2_64_F(H, A, B, C, D, E, F, G, W[ 9], W[ 7], W[ 2], W[10], 0x32CAAB7B40C72493);
      SHA2_64_F(G, H, A, B, C, D, E, F, W[10], W[ 8], W[ 3], W[11], 0x3C9EBE0A15C9BEBC);
      SHA2_64_F(F, G, H, A, B, C, D, E, W[11], W[ 9], W[ 4], W[12], 0x431D67C49C100D4C);
      SHA2_64_F(E, F, G, H, A, B, C, D, W[12], W[10], W[ 5], W[13], 0x4CC5D4BECB3E42B6);
      SHA2_64_F(D, E, F, G, H, A, B, C, W[13], W[11], W[ 6], W[14], 0x597F299CFC657E2A);
      SHA2_64_F(C, D, E, F, G, H, A, B, W[14], W[12], W[ 7], W[15], 0x5FCB6FAB3AD6FAEC);
      SHA2_64_F(B, C, D, E, F, G, H, A, W[15], W[13], W[ 8], W[ 0], 0x6C44198C4A475817);

      // clang-format on

      A = (digest[0] += A);
      B = (digest[1] += B);
      C = (digest[2] += C);
      D = (digest[3] += D);
      E = (digest[4] += E);
      F = (digest[5] += F);
      G = (digest[6] += G);
      H = (digest[7] += H);
   }
}

std::string SHA_512_256::provider() const {
   return sha512_provider();
}

std::string SHA_384::provider() const {
   return sha512_provider();
}

std::string SHA_512::provider() const {
   return sha512_provider();
}

void SHA_512_256::compress_n(digest_type& digest, std::span<const uint8_t> input, size_t blocks) {
   SHA_512::compress_digest(digest, input, blocks);
}

void SHA_384::compress_n(digest_type& digest, std::span<const uint8_t> input, size_t blocks) {
   SHA_512::compress_digest(digest, input, blocks);
}

void SHA_512::compress_n(digest_type& digest, std::span<const uint8_t> input, size_t blocks) {
   SHA_512::compress_digest(digest, input, blocks);
}

void SHA_512_256::init(digest_type& digest) {
   digest.assign({0x22312194FC2BF72C,
                  0x9F555FA3C84C64C2,
                  0x2393B86B6F53B151,
                  0x963877195940EABD,
                  0x96283EE2A88EFFE3,
                  0xBE5E1E2553863992,
                  0x2B0199FC2C85B8AA,
                  0x0EB72DDC81C52CA2});
}

void SHA_384::init(digest_type& digest) {
   digest.assign({0xCBBB9D5DC1059ED8,
                  0x629A292A367CD507,
                  0x9159015A3070DD17,
                  0x152FECD8F70E5939,
                  0x67332667FFC00B31,
                  0x8EB44A8768581511,
                  0xDB0C2E0D64F98FA7,
                  0x47B5481DBEFA4FA4});
}

void SHA_512::init(digest_type& digest) {
   digest.assign({0x6A09E667F3BCC908,
                  0xBB67AE8584CAA73B,
                  0x3C6EF372FE94F82B,
                  0xA54FF53A5F1D36F1,
                  0x510E527FADE682D1,
                  0x9B05688C2B3E6C1F,
                  0x1F83D9ABFB41BD6B,
                  0x5BE0CD19137E2179});
}

std::unique_ptr<HashFunction> SHA_384::new_object() const {
   return std::make_unique<SHA_384>();
}

std::unique_ptr<HashFunction> SHA_512::new_object() const {
   return std::make_unique<SHA_512>();
}

std::unique_ptr<HashFunction> SHA_512_256::new_object() const {
   return std::make_unique<SHA_512_256>();
}

std::unique_ptr<HashFunction> SHA_384::copy_state() const {
   return std::make_unique<SHA_384>(*this);
}

std::unique_ptr<HashFunction> SHA_512::copy_state() const {
   return std::make_unique<SHA_512>(*this);
}

std::unique_ptr<HashFunction> SHA_512_256::copy_state() const {
   return std::make_unique<SHA_512_256>(*this);
}

void SHA_384::add_data(std::span<const uint8_t> input) {
   m_md.update(input);
}

void SHA_512::add_data(std::span<const uint8_t> input) {
   m_md.update(input);
}

void SHA_512_256::add_data(std::span<const uint8_t> input) {
   m_md.update(input);
}

void SHA_384::final_result(std::span<uint8_t> output) {
   m_md.final(output);
}

void SHA_512::final_result(std::span<uint8_t> output) {
   m_md.final(output);
}

void SHA_512_256::final_result(std::span<uint8_t> output) {
   m_md.final(output);
}

}  // namespace Botan
/*
* SHA-512 using CPU instructions in ARMv8
*
* (C) 2023 René Fischer
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

/*
* SHA-512 using CPU instructions in ARMv8
*/
void BOTAN_FN_ISA_SHA512 SHA_512::compress_digest_armv8(digest_type& digest,
                                                        std::span<const uint8_t> input8,
                                                        size_t blocks) {
   alignas(128) static const uint64_t K[] = {
      0x428A2F98D728AE22, 0x7137449123EF65CD, 0xB5C0FBCFEC4D3B2F, 0xE9B5DBA58189DBBC, 0x3956C25BF348B538,
      0x59F111F1B605D019, 0x923F82A4AF194F9B, 0xAB1C5ED5DA6D8118, 0xD807AA98A3030242, 0x12835B0145706FBE,
      0x243185BE4EE4B28C, 0x550C7DC3D5FFB4E2, 0x72BE5D74F27B896F, 0x80DEB1FE3B1696B1, 0x9BDC06A725C71235,
      0xC19BF174CF692694, 0xE49B69C19EF14AD2, 0xEFBE4786384F25E3, 0x0FC19DC68B8CD5B5, 0x240CA1CC77AC9C65,
      0x2DE92C6F592B0275, 0x4A7484AA6EA6E483, 0x5CB0A9DCBD41FBD4, 0x76F988DA831153B5, 0x983E5152EE66DFAB,
      0xA831C66D2DB43210, 0xB00327C898FB213F, 0xBF597FC7BEEF0EE4, 0xC6E00BF33DA88FC2, 0xD5A79147930AA725,
      0x06CA6351E003826F, 0x142929670A0E6E70, 0x27B70A8546D22FFC, 0x2E1B21385C26C926, 0x4D2C6DFC5AC42AED,
      0x53380D139D95B3DF, 0x650A73548BAF63DE, 0x766A0ABB3C77B2A8, 0x81C2C92E47EDAEE6, 0x92722C851482353B,
      0xA2BFE8A14CF10364, 0xA81A664BBC423001, 0xC24B8B70D0F89791, 0xC76C51A30654BE30, 0xD192E819D6EF5218,
      0xD69906245565A910, 0xF40E35855771202A, 0x106AA07032BBD1B8, 0x19A4C116B8D2D0C8, 0x1E376C085141AB53,
      0x2748774CDF8EEB99, 0x34B0BCB5E19B48A8, 0x391C0CB3C5C95A63, 0x4ED8AA4AE3418ACB, 0x5B9CCA4F7763E373,
      0x682E6FF3D6B2B8A3, 0x748F82EE5DEFB2FC, 0x78A5636F43172F60, 0x84C87814A1F0AB72, 0x8CC702081A6439EC,
      0x90BEFFFA23631E28, 0xA4506CEBDE82BDE9, 0xBEF9A3F7B2C67915, 0xC67178F2E372532B, 0xCA273ECEEA26619C,
      0xD186B8C721C0C207, 0xEADA7DD6CDE0EB1E, 0xF57D4F7FEE6ED178, 0x06F067AA72176FBA, 0x0A637DC5A2C898A6,
      0x113F9804BEF90DAE, 0x1B710B35131C471B, 0x28DB77F523047D84, 0x32CAAB7B40C72493, 0x3C9EBE0A15C9BEBC,
      0x431D67C49C100D4C, 0x4CC5D4BECB3E42B6, 0x597F299CFC657E2A, 0x5FCB6FAB3AD6FAEC, 0x6C44198C4A475817};

   // Load initial values
   uint64x2_t STATE0 = vld1q_u64(&digest[0]);  // ab NOLINT(*-container-data-pointer)
   uint64x2_t STATE1 = vld1q_u64(&digest[2]);  // cd
   uint64x2_t STATE2 = vld1q_u64(&digest[4]);  // ef
   uint64x2_t STATE3 = vld1q_u64(&digest[6]);  // gh

   const uint64_t* input64 = reinterpret_cast<const uint64_t*>(input8.data());

   while(blocks > 0) {
      // Save current state
      const uint64x2_t AB_SAVE = STATE0;
      const uint64x2_t CD_SAVE = STATE1;
      const uint64x2_t EF_SAVE = STATE2;
      const uint64x2_t GH_SAVE = STATE3;

      uint64x2_t MSG0 = vld1q_u64(input64 + 0);
      uint64x2_t MSG1 = vld1q_u64(input64 + 2);
      uint64x2_t MSG2 = vld1q_u64(input64 + 4);
      uint64x2_t MSG3 = vld1q_u64(input64 + 6);
      uint64x2_t MSG4 = vld1q_u64(input64 + 8);
      uint64x2_t MSG5 = vld1q_u64(input64 + 10);
      uint64x2_t MSG6 = vld1q_u64(input64 + 12);
      uint64x2_t MSG7 = vld1q_u64(input64 + 14);

      MSG0 = vreinterpretq_u64_u8(vrev64q_u8(vreinterpretq_u8_u64(MSG0)));
      MSG1 = vreinterpretq_u64_u8(vrev64q_u8(vreinterpretq_u8_u64(MSG1)));
      MSG2 = vreinterpretq_u64_u8(vrev64q_u8(vreinterpretq_u8_u64(MSG2)));
      MSG3 = vreinterpretq_u64_u8(vrev64q_u8(vreinterpretq_u8_u64(MSG3)));
      MSG4 = vreinterpretq_u64_u8(vrev64q_u8(vreinterpretq_u8_u64(MSG4)));
      MSG5 = vreinterpretq_u64_u8(vrev64q_u8(vreinterpretq_u8_u64(MSG5)));
      MSG6 = vreinterpretq_u64_u8(vrev64q_u8(vreinterpretq_u8_u64(MSG6)));
      MSG7 = vreinterpretq_u64_u8(vrev64q_u8(vreinterpretq_u8_u64(MSG7)));

      uint64x2_t MSG_K;
      uint64x2_t TSTATE0;
      uint64x2_t TSTATE1;

      // Rounds 0-1
      MSG_K = vaddq_u64(MSG0, vld1q_u64(&K[2 * 0]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE3);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE2, STATE3, 1), vextq_u64(STATE1, STATE2, 1));
      STATE3 = vsha512h2q_u64(TSTATE1, STATE1, STATE0);
      STATE1 = vaddq_u64(STATE1, TSTATE1);
      MSG0 = vsha512su1q_u64(vsha512su0q_u64(MSG0, MSG1), MSG7, vextq_u64(MSG4, MSG5, 1));

      // Rounds 2-3
      MSG_K = vaddq_u64(MSG1, vld1q_u64(&K[2 * 1]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE2);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE1, STATE2, 1), vextq_u64(STATE0, STATE1, 1));
      STATE2 = vsha512h2q_u64(TSTATE1, STATE0, STATE3);
      STATE0 = vaddq_u64(STATE0, TSTATE1);
      MSG1 = vsha512su1q_u64(vsha512su0q_u64(MSG1, MSG2), MSG0, vextq_u64(MSG5, MSG6, 1));

      // Rounds 4-5
      MSG_K = vaddq_u64(MSG2, vld1q_u64(&K[2 * 2]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE1);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE0, STATE1, 1), vextq_u64(STATE3, STATE0, 1));
      STATE1 = vsha512h2q_u64(TSTATE1, STATE3, STATE2);
      STATE3 = vaddq_u64(STATE3, TSTATE1);
      MSG2 = vsha512su1q_u64(vsha512su0q_u64(MSG2, MSG3), MSG1, vextq_u64(MSG6, MSG7, 1));

      // Rounds 6-7
      MSG_K = vaddq_u64(MSG3, vld1q_u64(&K[2 * 3]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE0);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE3, STATE0, 1), vextq_u64(STATE2, STATE3, 1));
      STATE0 = vsha512h2q_u64(TSTATE1, STATE2, STATE1);
      STATE2 = vaddq_u64(STATE2, TSTATE1);
      MSG3 = vsha512su1q_u64(vsha512su0q_u64(MSG3, MSG4), MSG2, vextq_u64(MSG7, MSG0, 1));

      // Rounds 8-9
      MSG_K = vaddq_u64(MSG4, vld1q_u64(&K[2 * 4]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE3);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE2, STATE3, 1), vextq_u64(STATE1, STATE2, 1));
      STATE3 = vsha512h2q_u64(TSTATE1, STATE1, STATE0);
      STATE1 = vaddq_u64(STATE1, TSTATE1);
      MSG4 = vsha512su1q_u64(vsha512su0q_u64(MSG4, MSG5), MSG3, vextq_u64(MSG0, MSG1, 1));

      // Rounds 10-11
      MSG_K = vaddq_u64(MSG5, vld1q_u64(&K[2 * 5]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE2);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE1, STATE2, 1), vextq_u64(STATE0, STATE1, 1));
      STATE2 = vsha512h2q_u64(TSTATE1, STATE0, STATE3);
      STATE0 = vaddq_u64(STATE0, TSTATE1);
      MSG5 = vsha512su1q_u64(vsha512su0q_u64(MSG5, MSG6), MSG4, vextq_u64(MSG1, MSG2, 1));

      // Rounds 12-13
      MSG_K = vaddq_u64(MSG6, vld1q_u64(&K[2 * 6]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE1);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE0, STATE1, 1), vextq_u64(STATE3, STATE0, 1));
      STATE1 = vsha512h2q_u64(TSTATE1, STATE3, STATE2);
      STATE3 = vaddq_u64(STATE3, TSTATE1);
      MSG6 = vsha512su1q_u64(vsha512su0q_u64(MSG6, MSG7), MSG5, vextq_u64(MSG2, MSG3, 1));

      // Rounds 14-15
      MSG_K = vaddq_u64(MSG7, vld1q_u64(&K[2 * 7]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE0);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE3, STATE0, 1), vextq_u64(STATE2, STATE3, 1));
      STATE0 = vsha512h2q_u64(TSTATE1, STATE2, STATE1);
      STATE2 = vaddq_u64(STATE2, TSTATE1);
      MSG7 = vsha512su1q_u64(vsha512su0q_u64(MSG7, MSG0), MSG6, vextq_u64(MSG3, MSG4, 1));

      // Rounds 16-17
      MSG_K = vaddq_u64(MSG0, vld1q_u64(&K[2 * 8]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE3);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE2, STATE3, 1), vextq_u64(STATE1, STATE2, 1));
      STATE3 = vsha512h2q_u64(TSTATE1, STATE1, STATE0);
      STATE1 = vaddq_u64(STATE1, TSTATE1);
      MSG0 = vsha512su1q_u64(vsha512su0q_u64(MSG0, MSG1), MSG7, vextq_u64(MSG4, MSG5, 1));

      // Rounds 18-19
      MSG_K = vaddq_u64(MSG1, vld1q_u64(&K[2 * 9]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE2);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE1, STATE2, 1), vextq_u64(STATE0, STATE1, 1));
      STATE2 = vsha512h2q_u64(TSTATE1, STATE0, STATE3);
      STATE0 = vaddq_u64(STATE0, TSTATE1);
      MSG1 = vsha512su1q_u64(vsha512su0q_u64(MSG1, MSG2), MSG0, vextq_u64(MSG5, MSG6, 1));

      // Rounds 20-21
      MSG_K = vaddq_u64(MSG2, vld1q_u64(&K[2 * 10]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE1);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE0, STATE1, 1), vextq_u64(STATE3, STATE0, 1));
      STATE1 = vsha512h2q_u64(TSTATE1, STATE3, STATE2);
      STATE3 = vaddq_u64(STATE3, TSTATE1);
      MSG2 = vsha512su1q_u64(vsha512su0q_u64(MSG2, MSG3), MSG1, vextq_u64(MSG6, MSG7, 1));

      // Rounds 22-23
      MSG_K = vaddq_u64(MSG3, vld1q_u64(&K[2 * 11]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE0);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE3, STATE0, 1), vextq_u64(STATE2, STATE3, 1));
      STATE0 = vsha512h2q_u64(TSTATE1, STATE2, STATE1);
      STATE2 = vaddq_u64(STATE2, TSTATE1);
      MSG3 = vsha512su1q_u64(vsha512su0q_u64(MSG3, MSG4), MSG2, vextq_u64(MSG7, MSG0, 1));

      // Rounds 24-25
      MSG_K = vaddq_u64(MSG4, vld1q_u64(&K[2 * 12]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE3);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE2, STATE3, 1), vextq_u64(STATE1, STATE2, 1));
      STATE3 = vsha512h2q_u64(TSTATE1, STATE1, STATE0);
      STATE1 = vaddq_u64(STATE1, TSTATE1);
      MSG4 = vsha512su1q_u64(vsha512su0q_u64(MSG4, MSG5), MSG3, vextq_u64(MSG0, MSG1, 1));

      // Rounds 26-27
      MSG_K = vaddq_u64(MSG5, vld1q_u64(&K[2 * 13]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE2);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE1, STATE2, 1), vextq_u64(STATE0, STATE1, 1));
      STATE2 = vsha512h2q_u64(TSTATE1, STATE0, STATE3);
      STATE0 = vaddq_u64(STATE0, TSTATE1);
      MSG5 = vsha512su1q_u64(vsha512su0q_u64(MSG5, MSG6), MSG4, vextq_u64(MSG1, MSG2, 1));

      // Rounds 28-29
      MSG_K = vaddq_u64(MSG6, vld1q_u64(&K[2 * 14]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE1);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE0, STATE1, 1), vextq_u64(STATE3, STATE0, 1));
      STATE1 = vsha512h2q_u64(TSTATE1, STATE3, STATE2);
      STATE3 = vaddq_u64(STATE3, TSTATE1);
      MSG6 = vsha512su1q_u64(vsha512su0q_u64(MSG6, MSG7), MSG5, vextq_u64(MSG2, MSG3, 1));

      // Rounds 30-31
      MSG_K = vaddq_u64(MSG7, vld1q_u64(&K[2 * 15]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE0);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE3, STATE0, 1), vextq_u64(STATE2, STATE3, 1));
      STATE0 = vsha512h2q_u64(TSTATE1, STATE2, STATE1);
      STATE2 = vaddq_u64(STATE2, TSTATE1);
      MSG7 = vsha512su1q_u64(vsha512su0q_u64(MSG7, MSG0), MSG6, vextq_u64(MSG3, MSG4, 1));

      // Rounds 32-33
      MSG_K = vaddq_u64(MSG0, vld1q_u64(&K[2 * 16]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE3);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE2, STATE3, 1), vextq_u64(STATE1, STATE2, 1));
      STATE3 = vsha512h2q_u64(TSTATE1, STATE1, STATE0);
      STATE1 = vaddq_u64(STATE1, TSTATE1);
      MSG0 = vsha512su1q_u64(vsha512su0q_u64(MSG0, MSG1), MSG7, vextq_u64(MSG4, MSG5, 1));

      // Rounds 34-35
      MSG_K = vaddq_u64(MSG1, vld1q_u64(&K[2 * 17]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE2);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE1, STATE2, 1), vextq_u64(STATE0, STATE1, 1));
      STATE2 = vsha512h2q_u64(TSTATE1, STATE0, STATE3);
      STATE0 = vaddq_u64(STATE0, TSTATE1);
      MSG1 = vsha512su1q_u64(vsha512su0q_u64(MSG1, MSG2), MSG0, vextq_u64(MSG5, MSG6, 1));

      // Rounds 36-37
      MSG_K = vaddq_u64(MSG2, vld1q_u64(&K[2 * 18]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE1);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE0, STATE1, 1), vextq_u64(STATE3, STATE0, 1));
      STATE1 = vsha512h2q_u64(TSTATE1, STATE3, STATE2);
      STATE3 = vaddq_u64(STATE3, TSTATE1);
      MSG2 = vsha512su1q_u64(vsha512su0q_u64(MSG2, MSG3), MSG1, vextq_u64(MSG6, MSG7, 1));

      // Rounds 38-39
      MSG_K = vaddq_u64(MSG3, vld1q_u64(&K[2 * 19]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE0);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE3, STATE0, 1), vextq_u64(STATE2, STATE3, 1));
      STATE0 = vsha512h2q_u64(TSTATE1, STATE2, STATE1);
      STATE2 = vaddq_u64(STATE2, TSTATE1);
      MSG3 = vsha512su1q_u64(vsha512su0q_u64(MSG3, MSG4), MSG2, vextq_u64(MSG7, MSG0, 1));

      // Rounds 40-41
      MSG_K = vaddq_u64(MSG4, vld1q_u64(&K[2 * 20]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE3);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE2, STATE3, 1), vextq_u64(STATE1, STATE2, 1));
      STATE3 = vsha512h2q_u64(TSTATE1, STATE1, STATE0);
      STATE1 = vaddq_u64(STATE1, TSTATE1);
      MSG4 = vsha512su1q_u64(vsha512su0q_u64(MSG4, MSG5), MSG3, vextq_u64(MSG0, MSG1, 1));

      // Rounds 42-43
      MSG_K = vaddq_u64(MSG5, vld1q_u64(&K[2 * 21]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE2);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE1, STATE2, 1), vextq_u64(STATE0, STATE1, 1));
      STATE2 = vsha512h2q_u64(TSTATE1, STATE0, STATE3);
      STATE0 = vaddq_u64(STATE0, TSTATE1);
      MSG5 = vsha512su1q_u64(vsha512su0q_u64(MSG5, MSG6), MSG4, vextq_u64(MSG1, MSG2, 1));

      // Rounds 44-45
      MSG_K = vaddq_u64(MSG6, vld1q_u64(&K[2 * 22]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE1);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE0, STATE1, 1), vextq_u64(STATE3, STATE0, 1));
      STATE1 = vsha512h2q_u64(TSTATE1, STATE3, STATE2);
      STATE3 = vaddq_u64(STATE3, TSTATE1);
      MSG6 = vsha512su1q_u64(vsha512su0q_u64(MSG6, MSG7), MSG5, vextq_u64(MSG2, MSG3, 1));

      // Rounds 46-47
      MSG_K = vaddq_u64(MSG7, vld1q_u64(&K[2 * 23]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE0);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE3, STATE0, 1), vextq_u64(STATE2, STATE3, 1));
      STATE0 = vsha512h2q_u64(TSTATE1, STATE2, STATE1);
      STATE2 = vaddq_u64(STATE2, TSTATE1);
      MSG7 = vsha512su1q_u64(vsha512su0q_u64(MSG7, MSG0), MSG6, vextq_u64(MSG3, MSG4, 1));

      // Rounds 48-49
      MSG_K = vaddq_u64(MSG0, vld1q_u64(&K[2 * 24]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE3);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE2, STATE3, 1), vextq_u64(STATE1, STATE2, 1));
      STATE3 = vsha512h2q_u64(TSTATE1, STATE1, STATE0);
      STATE1 = vaddq_u64(STATE1, TSTATE1);
      MSG0 = vsha512su1q_u64(vsha512su0q_u64(MSG0, MSG1), MSG7, vextq_u64(MSG4, MSG5, 1));

      // Rounds 50-51
      MSG_K = vaddq_u64(MSG1, vld1q_u64(&K[2 * 25]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE2);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE1, STATE2, 1), vextq_u64(STATE0, STATE1, 1));
      STATE2 = vsha512h2q_u64(TSTATE1, STATE0, STATE3);
      STATE0 = vaddq_u64(STATE0, TSTATE1);
      MSG1 = vsha512su1q_u64(vsha512su0q_u64(MSG1, MSG2), MSG0, vextq_u64(MSG5, MSG6, 1));

      // Rounds 52-53
      MSG_K = vaddq_u64(MSG2, vld1q_u64(&K[2 * 26]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE1);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE0, STATE1, 1), vextq_u64(STATE3, STATE0, 1));
      STATE1 = vsha512h2q_u64(TSTATE1, STATE3, STATE2);
      STATE3 = vaddq_u64(STATE3, TSTATE1);
      MSG2 = vsha512su1q_u64(vsha512su0q_u64(MSG2, MSG3), MSG1, vextq_u64(MSG6, MSG7, 1));

      // Rounds 54-55
      MSG_K = vaddq_u64(MSG3, vld1q_u64(&K[2 * 27]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE0);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE3, STATE0, 1), vextq_u64(STATE2, STATE3, 1));
      STATE0 = vsha512h2q_u64(TSTATE1, STATE2, STATE1);
      STATE2 = vaddq_u64(STATE2, TSTATE1);
      MSG3 = vsha512su1q_u64(vsha512su0q_u64(MSG3, MSG4), MSG2, vextq_u64(MSG7, MSG0, 1));

      // Rounds 56-57
      MSG_K = vaddq_u64(MSG4, vld1q_u64(&K[2 * 28]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE3);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE2, STATE3, 1), vextq_u64(STATE1, STATE2, 1));
      STATE3 = vsha512h2q_u64(TSTATE1, STATE1, STATE0);
      STATE1 = vaddq_u64(STATE1, TSTATE1);
      MSG4 = vsha512su1q_u64(vsha512su0q_u64(MSG4, MSG5), MSG3, vextq_u64(MSG0, MSG1, 1));

      // Rounds 58-59
      MSG_K = vaddq_u64(MSG5, vld1q_u64(&K[2 * 29]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE2);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE1, STATE2, 1), vextq_u64(STATE0, STATE1, 1));
      STATE2 = vsha512h2q_u64(TSTATE1, STATE0, STATE3);
      STATE0 = vaddq_u64(STATE0, TSTATE1);
      MSG5 = vsha512su1q_u64(vsha512su0q_u64(MSG5, MSG6), MSG4, vextq_u64(MSG1, MSG2, 1));

      // Rounds 60-61
      MSG_K = vaddq_u64(MSG6, vld1q_u64(&K[2 * 30]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE1);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE0, STATE1, 1), vextq_u64(STATE3, STATE0, 1));
      STATE1 = vsha512h2q_u64(TSTATE1, STATE3, STATE2);
      STATE3 = vaddq_u64(STATE3, TSTATE1);
      MSG6 = vsha512su1q_u64(vsha512su0q_u64(MSG6, MSG7), MSG5, vextq_u64(MSG2, MSG3, 1));

      // Rounds 62-63
      MSG_K = vaddq_u64(MSG7, vld1q_u64(&K[2 * 31]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE0);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE3, STATE0, 1), vextq_u64(STATE2, STATE3, 1));
      STATE0 = vsha512h2q_u64(TSTATE1, STATE2, STATE1);
      STATE2 = vaddq_u64(STATE2, TSTATE1);
      MSG7 = vsha512su1q_u64(vsha512su0q_u64(MSG7, MSG0), MSG6, vextq_u64(MSG3, MSG4, 1));

      // Rounds 64-65
      MSG_K = vaddq_u64(MSG0, vld1q_u64(&K[2 * 32]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE3);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE2, STATE3, 1), vextq_u64(STATE1, STATE2, 1));
      STATE3 = vsha512h2q_u64(TSTATE1, STATE1, STATE0);
      STATE1 = vaddq_u64(STATE1, TSTATE1);

      // Rounds 66-67
      MSG_K = vaddq_u64(MSG1, vld1q_u64(&K[2 * 33]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE2);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE1, STATE2, 1), vextq_u64(STATE0, STATE1, 1));
      STATE2 = vsha512h2q_u64(TSTATE1, STATE0, STATE3);
      STATE0 = vaddq_u64(STATE0, TSTATE1);

      // Rounds 68-69
      MSG_K = vaddq_u64(MSG2, vld1q_u64(&K[2 * 34]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE1);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE0, STATE1, 1), vextq_u64(STATE3, STATE0, 1));
      STATE1 = vsha512h2q_u64(TSTATE1, STATE3, STATE2);
      STATE3 = vaddq_u64(STATE3, TSTATE1);

      // Rounds 70-71
      MSG_K = vaddq_u64(MSG3, vld1q_u64(&K[2 * 35]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE0);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE3, STATE0, 1), vextq_u64(STATE2, STATE3, 1));
      STATE0 = vsha512h2q_u64(TSTATE1, STATE2, STATE1);
      STATE2 = vaddq_u64(STATE2, TSTATE1);

      // Rounds 72-73
      MSG_K = vaddq_u64(MSG4, vld1q_u64(&K[2 * 36]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE3);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE2, STATE3, 1), vextq_u64(STATE1, STATE2, 1));
      STATE3 = vsha512h2q_u64(TSTATE1, STATE1, STATE0);
      STATE1 = vaddq_u64(STATE1, TSTATE1);

      // Rounds 74-75
      MSG_K = vaddq_u64(MSG5, vld1q_u64(&K[2 * 37]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE2);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE1, STATE2, 1), vextq_u64(STATE0, STATE1, 1));
      STATE2 = vsha512h2q_u64(TSTATE1, STATE0, STATE3);
      STATE0 = vaddq_u64(STATE0, TSTATE1);

      // Rounds 76-77
      MSG_K = vaddq_u64(MSG6, vld1q_u64(&K[2 * 38]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE1);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE0, STATE1, 1), vextq_u64(STATE3, STATE0, 1));
      STATE1 = vsha512h2q_u64(TSTATE1, STATE3, STATE2);
      STATE3 = vaddq_u64(STATE3, TSTATE1);

      // Rounds 78-79
      MSG_K = vaddq_u64(MSG7, vld1q_u64(&K[2 * 39]));
      TSTATE0 = vaddq_u64(vextq_u64(MSG_K, MSG_K, 1), STATE0);
      TSTATE1 = vsha512hq_u64(TSTATE0, vextq_u64(STATE3, STATE0, 1), vextq_u64(STATE2, STATE3, 1));
      STATE0 = vsha512h2q_u64(TSTATE1, STATE2, STATE1);
      STATE2 = vaddq_u64(STATE2, TSTATE1);

      // Add back to state
      STATE0 = vaddq_u64(STATE0, AB_SAVE);
      STATE1 = vaddq_u64(STATE1, CD_SAVE);
      STATE2 = vaddq_u64(STATE2, EF_SAVE);
      STATE3 = vaddq_u64(STATE3, GH_SAVE);

      input64 += 64 / 4;
      blocks--;
   }

   // Save state
   vst1q_u64(&digest[0], STATE0);  // NOLINT(*-container-data-pointer)
   vst1q_u64(&digest[2], STATE1);
   vst1q_u64(&digest[4], STATE2);
   vst1q_u64(&digest[6], STATE3);
}

}  // namespace Botan
/*
* SHA-3
* (C) 2010,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

SHA_3::SHA_3(size_t output_bits) :
      m_keccak({.capacity_bits = output_bits * 2, .padding = KeccakPadding::sha3()}), m_output_length(output_bits / 8) {
   // We only support the parameters for SHA-3 in this constructor

   if(output_bits != 224 && output_bits != 256 && output_bits != 384 && output_bits != 512) {
      throw Invalid_Argument(fmt("SHA_3: Invalid output length {}", output_bits));
   }
}

std::string SHA_3::name() const {
   return fmt("SHA-3({})", m_output_length * 8);
}

std::string SHA_3::provider() const {
   return m_keccak.provider();
}

std::unique_ptr<HashFunction> SHA_3::copy_state() const {
   return std::make_unique<SHA_3>(*this);
}

std::unique_ptr<HashFunction> SHA_3::new_object() const {
   return std::make_unique<SHA_3>(m_output_length * 8);
}

void SHA_3::clear() {
   m_keccak.clear();
}

void SHA_3::add_data(std::span<const uint8_t> input) {
   m_keccak.absorb(input);
}

void SHA_3::final_result(std::span<uint8_t> output) {
   m_keccak.finish();
   m_keccak.squeeze(output);
   m_keccak.clear();
}

}  // namespace Botan
/*
* (C) 2016,2020 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

void Stateful_RNG::clear() {
   const lock_guard_type<recursive_mutex_type> lock(m_mutex);
   m_reseed_counter = 0;
   m_last_pid = 0;
   clear_state();
}

void Stateful_RNG::force_reseed() {
   const lock_guard_type<recursive_mutex_type> lock(m_mutex);
   m_reseed_counter = 0;
}

bool Stateful_RNG::is_seeded() const {
   const lock_guard_type<recursive_mutex_type> lock(m_mutex);
   return m_reseed_counter > 0;
}

void Stateful_RNG::initialize_with(std::span<const uint8_t> input) {
   const lock_guard_type<recursive_mutex_type> lock(m_mutex);

   clear();
   add_entropy(input);
}

void Stateful_RNG::generate_batched_output(std::span<uint8_t> output, std::span<const uint8_t> input) {
   BOTAN_ASSERT_NOMSG(!output.empty());

   const size_t max_per_request = max_number_of_bytes_per_request();

   if(max_per_request == 0) {
      // no limit
      reseed_check();
      this->generate_output(output, input);
   } else {
      while(!output.empty()) {
         const size_t this_req = std::min(max_per_request, output.size());

         reseed_check();
         this->generate_output(output.subspan(0, this_req), input);

         // only include the input for the first iteration
         input = {};

         output = output.subspan(this_req);
      }
   }
}

void Stateful_RNG::fill_bytes_with_input(std::span<uint8_t> output, std::span<const uint8_t> input) {
   const lock_guard_type<recursive_mutex_type> lock(m_mutex);

   if(output.empty()) {
      // Special case for exclusively adding entropy to the stateful RNG.
      this->update(input);

      if(8 * input.size() >= security_level()) {
         reset_reseed_counter();
      }
   } else {
      generate_batched_output(output, input);
   }
}

size_t Stateful_RNG::reseed_from_sources(Entropy_Sources& srcs, size_t poll_bits) {
   const lock_guard_type<recursive_mutex_type> lock(m_mutex);

   const size_t bits_collected = RandomNumberGenerator::reseed_from_sources(srcs, poll_bits);

   if(bits_collected >= security_level()) {
      reset_reseed_counter();
   }

   return bits_collected;
}

void Stateful_RNG::reseed_from_rng(RandomNumberGenerator& rng, size_t poll_bits) {
   const lock_guard_type<recursive_mutex_type> lock(m_mutex);

   RandomNumberGenerator::reseed_from_rng(rng, poll_bits);

   if(poll_bits >= security_level()) {
      reset_reseed_counter();
   }
}

void Stateful_RNG::reset_reseed_counter() {
   // Lock is held whenever this function is called
   m_reseed_counter = 1;
}

void Stateful_RNG::reseed_check() {
   // Lock is held whenever this function is called

   const uint32_t cur_pid = OS::get_process_id();

   const bool fork_detected = (m_last_pid > 0) && (cur_pid != m_last_pid);

   if(is_seeded() == false || fork_detected || (m_reseed_interval > 0 && m_reseed_counter >= m_reseed_interval)) {
      m_reseed_counter = 0;
      m_last_pid = cur_pid;

      if(m_underlying_rng != nullptr) {
         reseed_from_rng(*m_underlying_rng, security_level());
      }

      if(m_entropy_sources != nullptr) {
         reseed_from_sources(*m_entropy_sources, security_level());
      }

      if(!is_seeded()) {
         if(fork_detected) {
            throw Invalid_State("Detected use of fork but cannot reseed DRBG");
         } else {
            throw PRNG_Unseeded(name());
         }
      }
   } else {
      BOTAN_ASSERT(m_reseed_counter != 0, "RNG is seeded");
      m_reseed_counter += 1;
   }
}

}  // namespace Botan
/*
* Streebog (GOST R 34.11-2012)
* (C) 2017 Ribose Inc.
* (C) 2018,2026 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <bit>

namespace Botan {

namespace {

// Build the combined T-tables at compile time
consteval std::array<std::array<uint64_t, 256>, 8> streebog_Ax_table() noexcept {
   // Streebog sbox (same as Kuznyechik's), RFC 6986 Section 6.2
   alignas(256) const constexpr uint8_t S[256] = {
      252, 238, 221, 17,  207, 110, 49,  22,  251, 196, 250, 218, 35,  197, 4,   77,  233, 119, 240, 219, 147, 46,
      153, 186, 23,  54,  241, 187, 20,  205, 95,  193, 249, 24,  101, 90,  226, 92,  239, 33,  129, 28,  60,  66,
      139, 1,   142, 79,  5,   132, 2,   174, 227, 106, 143, 160, 6,   11,  237, 152, 127, 212, 211, 31,  235, 52,
      44,  81,  234, 200, 72,  171, 242, 42,  104, 162, 253, 58,  206, 204, 181, 112, 14,  86,  8,   12,  118, 18,
      191, 114, 19,  71,  156, 183, 93,  135, 21,  161, 150, 41,  16,  123, 154, 199, 243, 145, 120, 111, 157, 158,
      178, 177, 50,  117, 25,  61,  255, 53,  138, 126, 109, 84,  198, 128, 195, 189, 13,  87,  223, 245, 36,  169,
      62,  168, 67,  201, 215, 121, 214, 246, 124, 34,  185, 3,   224, 15,  236, 222, 122, 148, 176, 188, 220, 232,
      40,  80,  78,  51,  10,  74,  167, 151, 96,  115, 30,  0,   98,  68,  26,  184, 56,  130, 100, 159, 38,  65,
      173, 69,  70,  146, 39,  94,  85,  47,  140, 163, 165, 125, 105, 213, 149, 59,  7,   88,  179, 64,  134, 172,
      29,  247, 48,  55,  107, 228, 136, 217, 231, 137, 225, 27,  131, 73,  76,  63,  248, 254, 141, 83,  170, 144,
      202, 216, 133, 97,  32,  113, 103, 164, 45,  43,  9,   91,  203, 155, 37,  208, 190, 229, 108, 82,  89,  166,
      116, 210, 230, 244, 180, 192, 209, 102, 175, 194, 57,  75,  99,  182,
   };

   // Columns of the 8x8 linear transformation matrix over GF(2^8)
   const constexpr uint64_t L[8] = {
      0x641c314b2b8ee083,
      0xa48b474f9ef5dc18,
      0xf97d86d98a327728,
      0x5b068c651810a89e,
      0x0321658cba93c138,
      0xaccc9ca9328a8950,
      0x46b60f011a83988e,
      0x83478b07b2468764,
   };

   std::array<std::array<uint64_t, 256>, 8> Ax = {};

   for(size_t j = 0; j != 8; ++j) {
      for(size_t x = 0; x != 256; ++x) {
         Ax[j][x] = poly_mul<0x1D>(L[j], S[x]);
      }
   }

   return Ax;
}

const constinit auto STREEBOG_Ax = streebog_Ax_table();

// Iteration constants C[1]..C[12] from GOST R 34.11-2012 (RFC 6986 Section 6.5)
// Word order matches the RFC (big-endian presentation); indexed with 7-j below
// clang-format off
const constexpr uint64_t STREEBOG_C[12][8] = {
   {0xb1085bda1ecadae9, 0xebcb2f81c0657c1f, 0x2f6a76432e45d016, 0x714eb88d7585c4fc,
    0x4b7ce09192676901, 0xa2422a08a460d315, 0x05767436cc744d23, 0xdd806559f2a64507},
   {0x6fa3b58aa99d2f1a, 0x4fe39d460f70b5d7, 0xf3feea720a232b98, 0x61d55e0f16b50131,
    0x9ab5176b12d69958, 0x5cb561c2db0aa7ca, 0x55dda21bd7cbcd56, 0xe679047021b19bb7},
   {0xf574dcac2bce2fc7, 0x0a39fc286a3d8435, 0x06f15e5f529c1f8b, 0xf2ea7514b1297b7b,
    0xd3e20fe490359eb1, 0xc1c93a376062db09, 0xc2b6f443867adb31, 0x991e96f50aba0ab2},
   {0xef1fdfb3e81566d2, 0xf948e1a05d71e4dd, 0x488e857e335c3c7d, 0x9d721cad685e353f,
    0xa9d72c82ed03d675, 0xd8b71333935203be, 0x3453eaa193e837f1, 0x220cbebc84e3d12e},
   {0x4bea6bacad474799, 0x9a3f410c6ca92363, 0x7f151c1f1686104a, 0x359e35d7800fffbd,
    0xbfcd1747253af5a3, 0xdfff00b723271a16, 0x7a56a27ea9ea63f5, 0x601758fd7c6cfe57},
   {0xae4faeae1d3ad3d9, 0x6fa4c33b7a3039c0, 0x2d66c4f95142a46c, 0x187f9ab49af08ec6,
    0xcffaa6b71c9ab7b4, 0x0af21f66c2bec6b6, 0xbf71c57236904f35, 0xfa68407a46647d6e},
   {0xf4c70e16eeaac5ec, 0x51ac86febf240954, 0x399ec6c7e6bf87c9, 0xd3473e33197a93c9,
    0x0992abc52d822c37, 0x06476983284a0504, 0x3517454ca23c4af3, 0x8886564d3a14d493},
   {0x9b1f5b424d93c9a7, 0x03e7aa020c6e4141, 0x4eb7f8719c36de1e, 0x89b4443b4ddbc49a,
    0xf4892bcb929b0690, 0x69d18d2bd1a5c42f, 0x36acc2355951a8d9, 0xa47f0dd4bf02e71e},
   {0x378f5a541631229b, 0x944c9ad8ec165fde, 0x3a7d3a1b25894224, 0x3cd955b7e00d0984,
    0x800a440bdbb2ceb1, 0x7b2b8a9aa6079c54, 0x0e38dc92cb1f2a60, 0x7261445183235adb},
   {0xabbedea680056f52, 0x382ae548b2e4f3f3, 0x8941e71cff8a78db, 0x1fffe18a1b336103,
    0x9fe76702af69334b, 0x7a1e6c303b7652f4, 0x3698fad1153bb6c3, 0x74b4c7fb98459ced},
   {0x7bcd9ed0efc889fb, 0x3002c6cd635afe94, 0xd8fa6bbbebab0761, 0x2001802114846679,
    0x8a1d71efea48b9ca, 0xefbacd1d7d476e98, 0xdea2594ac06fd85d, 0x6bcaa4cd81f32d1b},
   {0x378ee767f11631ba, 0xd21380b00449b17a, 0xcda43c32bcdf1d77, 0xf82012d430219f9b,
    0x5d80ef9d1891cc86, 0xe71da4aa88e12852, 0xfaf417d5d9b21b99, 0x48bc924af11bd720},
};

// clang-format on

inline uint64_t force_le(uint64_t x) {
   if constexpr(std::endian::native == std::endian::little) {
      return x;
   } else if constexpr(std::endian::native == std::endian::big) {
      return reverse_bytes(x);
   } else {
      store_le(x, reinterpret_cast<uint8_t*>(&x));
      return x;
   }
}

inline void lps(uint64_t block[8]) {
   const uint64_t block2[8] = {block[0], block[1], block[2], block[3], block[4], block[5], block[6], block[7]};
   const std::span<const uint8_t> r{reinterpret_cast<const uint8_t*>(block2), 64};

   for(int i = 0; i < 8; ++i) {
      block[i] = force_le(STREEBOG_Ax[0][r[i + 0 * 8]]) ^ force_le(STREEBOG_Ax[1][r[i + 1 * 8]]) ^
                 force_le(STREEBOG_Ax[2][r[i + 2 * 8]]) ^ force_le(STREEBOG_Ax[3][r[i + 3 * 8]]) ^
                 force_le(STREEBOG_Ax[4][r[i + 4 * 8]]) ^ force_le(STREEBOG_Ax[5][r[i + 5 * 8]]) ^
                 force_le(STREEBOG_Ax[6][r[i + 6 * 8]]) ^ force_le(STREEBOG_Ax[7][r[i + 7 * 8]]);
   }
}

}  //namespace

std::unique_ptr<HashFunction> Streebog::copy_state() const {
   return std::make_unique<Streebog>(*this);
}

Streebog::Streebog(size_t output_bits) : m_output_bits(output_bits), m_count(0), m_h(8), m_S(8) {
   if(output_bits != 256 && output_bits != 512) {
      throw Invalid_Argument(fmt("Streebog: Invalid output length {}", output_bits));
   }

   clear();
}

std::string Streebog::name() const {
   return fmt("Streebog-{}", m_output_bits);
}

/*
* Clear memory of sensitive data
*/
void Streebog::clear() {
   m_count = 0;
   m_buffer.clear();
   zeroise(m_S);

   const uint64_t fill = (m_output_bits == 512) ? 0 : 0x0101010101010101;
   std::fill(m_h.begin(), m_h.end(), fill);
}

/*
* Update the hash
*/
void Streebog::add_data(std::span<const uint8_t> input) {
   BufferSlicer in(input);

   while(!in.empty()) {
      if(const auto one_block = m_buffer.handle_unaligned_data(in)) {
         compress(one_block->data());
         m_count += 512;
      }

      if(m_buffer.in_alignment()) {
         while(const auto aligned_block = m_buffer.next_aligned_block_to_process(in)) {
            compress(aligned_block->data());
            m_count += 512;
         }
      }
   }
}

/*
* Finalize a hash
*/
void Streebog::final_result(std::span<uint8_t> output) {
   const auto pos = m_buffer.elements_in_buffer();

   const uint8_t padding = 0x01;
   m_buffer.append({&padding, 1});
   m_buffer.fill_up_with_zeros();

   compress(m_buffer.consume().data());
   m_count += pos * 8;

   m_buffer.fill_up_with_zeros();
   store_le(m_count, m_buffer.directly_modify_first(sizeof(m_count)).data());
   compress(m_buffer.consume().data(), true);

   compress_64(m_S.data(), true);

   const size_t offset = 8 - output_length() / 8;
   const size_t count = output_length() / sizeof(uint64_t);
   typecast_copy(output, std::span<const uint64_t>(&m_h[offset], count));
   clear();
}

void Streebog::compress(const uint8_t input[], bool last_block) {
   uint64_t M[8];
   typecast_copy(M, std::span<const uint8_t>(input, 64));
   compress_64(M, last_block);
}

void Streebog::compress_64(const uint64_t M[], bool last_block) {
   const uint64_t N = last_block ? 0 : force_le(m_count);

   uint64_t hN[8];
   uint64_t A[8];

   copy_mem(hN, m_h.data(), 8);
   hN[0] ^= N;
   lps(hN);

   copy_mem(A, hN, 8);

   for(size_t i = 0; i != 8; ++i) {
      hN[i] ^= M[i];
   }

   for(size_t i = 0; i < 12; ++i) {  // NOLINT(modernize-loop-convert)
      for(size_t j = 0; j != 8; ++j) {
         A[j] ^= force_le(STREEBOG_C[i][7 - j]);
      }
      lps(A);

      lps(hN);
      for(size_t j = 0; j != 8; ++j) {
         hN[j] ^= A[j];
      }
   }

   for(size_t i = 0; i != 8; ++i) {
      m_h[i] ^= hN[i] ^ M[i];
   }

   if(!last_block) {
      uint64_t carry = 0;
      for(int i = 0; i < 8; i++) {
         const uint64_t m = force_le(M[i]);
         const uint64_t hi = force_le(m_S[i]);
         const uint64_t t = hi + m + carry;

         m_S[i] = force_le(t);
         if(t != m) {
            carry = (t < m) ? 1 : 0;
         }
      }
   }
}

}  // namespace Botan
/*
* System RNG
* (C) 2014,2015,2017,2018,2022 Jack Lloyd
* (C) 2021 Tom Crowley
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_TARGET_OS_HAS_WIN32)
   #define NOMINMAX 1
   #define _WINSOCKAPI_  // stop windows.h including winsock.h
   #include <windows.h>
#endif

#if defined(BOTAN_TARGET_OS_HAS_RTLGENRANDOM)
#elif defined(BOTAN_TARGET_OS_HAS_CRYPTO_NG)
   #include <bcrypt.h>
   #include <windows.h>
#elif defined(BOTAN_TARGET_OS_HAS_CCRANDOM)
   #include <CommonCrypto/CommonRandom.h>
#elif defined(BOTAN_TARGET_OS_HAS_ARC4RANDOM)
   #include <stdlib.h>
#elif defined(BOTAN_TARGET_OS_HAS_GETRANDOM)
   #include <errno.h>
   #include <sys/random.h>
   #include <sys/syscall.h>
   #include <unistd.h>
#elif defined(BOTAN_TARGET_OS_HAS_DEV_RANDOM)
   #include <errno.h>
   #include <fcntl.h>
   #include <unistd.h>
#endif

namespace Botan {

namespace {

#if defined(BOTAN_TARGET_OS_HAS_RTLGENRANDOM)

class System_RNG_Impl final : public RandomNumberGenerator {
   public:
      System_RNG_Impl() : m_advapi("advapi32.dll") {
         // This throws if the function is not found
         m_rtlgenrandom = m_advapi.resolve<RtlGenRandom_fptr>("SystemFunction036");
      }

      System_RNG_Impl(const System_RNG_Impl& other) = delete;
      System_RNG_Impl(System_RNG_Impl&& other) = delete;
      System_RNG_Impl& operator=(const System_RNG_Impl& other) = delete;
      System_RNG_Impl& operator=(System_RNG_Impl&& other) = delete;

      bool is_seeded() const override { return true; }

      bool accepts_input() const override { return false; }

      void clear() override { /* not possible */
      }

      std::string name() const override { return "RtlGenRandom"; }

   private:
      void fill_bytes_with_input(std::span<uint8_t> output, std::span<const uint8_t> /* ignored */) override {
         const size_t limit = std::numeric_limits<ULONG>::max();

         uint8_t* pData = output.data();
         size_t bytesLeft = output.size();
         while(bytesLeft > 0) {
            const ULONG blockSize = static_cast<ULONG>(std::min(bytesLeft, limit));

            const bool success = m_rtlgenrandom(pData, blockSize) == TRUE;
            if(!success) {
               throw System_Error("RtlGenRandom failed");
            }

            BOTAN_ASSERT(bytesLeft >= blockSize, "Block is oversized");
            bytesLeft -= blockSize;
            pData += blockSize;
         }
      }

   private:
      using RtlGenRandom_fptr = BOOLEAN(NTAPI*)(PVOID, ULONG);

      Dynamically_Loaded_Library m_advapi;
      RtlGenRandom_fptr m_rtlgenrandom;
};

#elif defined(BOTAN_TARGET_OS_HAS_CRYPTO_NG)

class System_RNG_Impl final : public RandomNumberGenerator {
   public:
      System_RNG_Impl() {
         auto ret = ::BCryptOpenAlgorithmProvider(&m_prov, BCRYPT_RNG_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
         if(!BCRYPT_SUCCESS(ret)) {
            throw System_Error("System_RNG failed to acquire crypto provider", ret);
         }
      }

      System_RNG_Impl(const System_RNG_Impl& other) = delete;
      System_RNG_Impl(System_RNG_Impl&& other) = delete;
      System_RNG_Impl& operator=(const System_RNG_Impl& other) = delete;
      System_RNG_Impl& operator=(System_RNG_Impl&& other) = delete;

      ~System_RNG_Impl() override { ::BCryptCloseAlgorithmProvider(m_prov, 0); }

      bool is_seeded() const override { return true; }

      bool accepts_input() const override { return false; }

      void clear() override { /* not possible */
      }

      std::string name() const override { return "crypto_ng"; }

   private:
      void fill_bytes_with_input(std::span<uint8_t> output, std::span<const uint8_t> /* ignored */) override {
         /*
         There is a flag BCRYPT_RNG_USE_ENTROPY_IN_BUFFER to provide
         entropy inputs, but it is ignored in Windows 8 and later.
         */

         const size_t limit = std::numeric_limits<ULONG>::max();

         uint8_t* pData = output.data();
         size_t bytesLeft = output.size();
         while(bytesLeft > 0) {
            const ULONG blockSize = static_cast<ULONG>(std::min(bytesLeft, limit));

            auto ret = BCryptGenRandom(m_prov, static_cast<PUCHAR>(pData), blockSize, 0);
            if(!BCRYPT_SUCCESS(ret)) {
               throw System_Error("System_RNG call to BCryptGenRandom failed", ret);
            }

            BOTAN_ASSERT(bytesLeft >= blockSize, "Block is oversized");
            bytesLeft -= blockSize;
            pData += blockSize;
         }
      }

   private:
      BCRYPT_ALG_HANDLE m_prov;
};

#elif defined(BOTAN_TARGET_OS_HAS_CCRANDOM)

class System_RNG_Impl final : public RandomNumberGenerator {
   public:
      bool accepts_input() const override { return false; }

      bool is_seeded() const override { return true; }

      void clear() override { /* not possible */
      }

      std::string name() const override { return "CCRandomGenerateBytes"; }

   private:
      void fill_bytes_with_input(std::span<uint8_t> output, std::span<const uint8_t> /* ignored */) override {
         if(::CCRandomGenerateBytes(output.data(), output.size()) != kCCSuccess) {
            throw System_Error("System_RNG CCRandomGenerateBytes failed", errno);
         }
      }
};

#elif defined(BOTAN_TARGET_OS_HAS_ARC4RANDOM)

class System_RNG_Impl final : public RandomNumberGenerator {
   public:
      // No constructor or destructor needed as no userland state maintained

      bool accepts_input() const override { return false; }

      bool is_seeded() const override { return true; }

      void clear() override { /* not possible */
      }

      std::string name() const override { return "arc4random"; }

   private:
      void fill_bytes_with_input(std::span<uint8_t> output, std::span<const uint8_t> /* ignored */) override {
         // macOS 10.15 arc4random crashes if called with buf == nullptr && len == 0
         // however it uses ccrng_generate internally which returns a status, ignored
         // to respect arc4random "no-fail" interface contract
         if(!output.empty()) {
            ::arc4random_buf(output.data(), output.size());
         }
      }
};

#elif defined(BOTAN_TARGET_OS_HAS_GETRANDOM)

class System_RNG_Impl final : public RandomNumberGenerator {
   public:
      // No constructor or destructor needed as no userland state maintained

      bool accepts_input() const override { return false; }

      bool is_seeded() const override { return true; }

      void clear() override { /* not possible */
      }

      std::string name() const override { return "getrandom"; }

   private:
      void fill_bytes_with_input(std::span<uint8_t> output, std::span<const uint8_t> /* ignored */) override {
         const unsigned int flags = 0;

         uint8_t* buf = output.data();
         size_t len = output.size();
         while(len > 0) {
   #if defined(__GLIBC__) && __GLIBC__ == 2 && __GLIBC_MINOR__ < 25
            const ssize_t got = ::syscall(SYS_getrandom, buf, len, flags);
   #else
            const ssize_t got = ::getrandom(buf, len, flags);
   #endif

            if(got < 0) {
               if(errno == EINTR) {
                  continue;
               }
               throw System_Error("System_RNG getrandom failed", errno);
            }

            if(got == 0) {
               throw System_Error("System_RNG getrandom unexpectedly returned 0");
            }

            buf += got;
            len -= got;
         }
      }
};

#elif defined(BOTAN_TARGET_OS_HAS_DEV_RANDOM)

// Read a random device

class System_RNG_Impl final : public RandomNumberGenerator {
   public:
      System_RNG_Impl() {
   #ifndef O_NOCTTY
      #define O_NOCTTY 0
   #endif

         /*
         * First open /dev/random and read one byte. On old Linux kernels
         * this blocks the RNG until we have been actually seeded.
         */
         m_fd = ::open("/dev/random", O_RDONLY | O_NOCTTY);
         if(m_fd < 0)
            throw System_Error("System_RNG failed to open RNG device", errno);

         uint8_t b;
         const size_t got = ::read(m_fd, &b, 1);
         ::close(m_fd);

         if(got != 1)
            throw System_Error("System_RNG failed to read blocking RNG device");

         m_fd = ::open("/dev/urandom", O_RDWR | O_NOCTTY);

         if(m_fd >= 0) {
            m_writable = true;
         } else {
            /*
            Cannot open in read-write mode. Fall back to read-only,
            calls to add_entropy will fail, but randomize will work
            */
            m_fd = ::open("/dev/urandom", O_RDONLY | O_NOCTTY);
            m_writable = false;
         }

         if(m_fd < 0)
            throw System_Error("System_RNG failed to open RNG device", errno);
      }

      System_RNG_Impl(const System_RNG_Impl& other) = delete;
      System_RNG_Impl(System_RNG_Impl&& other) = delete;
      System_RNG_Impl& operator=(const System_RNG_Impl& other) = delete;
      System_RNG_Impl& operator=(System_RNG_Impl&& other) = delete;

      ~System_RNG_Impl() override {
         ::close(m_fd);
         m_fd = -1;
      }

      bool is_seeded() const override { return true; }

      bool accepts_input() const override { return m_writable; }

      void clear() override { /* not possible */
      }

      std::string name() const override { return "urandom"; }

   private:
      void fill_bytes_with_input(std::span<uint8_t> output, std::span<const uint8_t> /* ignored */) override;
      void maybe_write_entropy(std::span<const uint8_t> input);

   private:
      int m_fd;
      bool m_writable;
};

void System_RNG_Impl::fill_bytes_with_input(std::span<uint8_t> output, std::span<const uint8_t> input) {
   maybe_write_entropy(input);

   uint8_t* buf = output.data();
   size_t len = output.size();
   while(len) {
      ssize_t got = ::read(m_fd, buf, len);

      if(got < 0) {
         if(errno == EINTR)
            continue;
         throw System_Error("System_RNG read failed", errno);
      }
      if(got == 0)
         throw System_Error("System_RNG EOF on device");  // ?!?

      buf += got;
      len -= got;
   }
}

void System_RNG_Impl::maybe_write_entropy(std::span<const uint8_t> entropy_input) {
   if(!m_writable || entropy_input.empty())
      return;

   const uint8_t* input = entropy_input.data();
   size_t len = entropy_input.size();
   while(len) {
      ssize_t got = ::write(m_fd, input, len);

      if(got < 0) {
         if(errno == EINTR)
            continue;

         /*
         * This is seen on OS X CI, despite the fact that the man page
         * for macOS urandom explicitly states that writing to it is
         * supported, and write(2) does not document EPERM at all.
         * But in any case EPERM seems indicative of a policy decision
         * by the OS or sysadmin that additional entropy is not wanted
         * in the system pool, so we accept that and return here,
         * since there is no corrective action possible.
         *
         * In Linux EBADF or EPERM is returned if m_fd is not opened for
         * writing.
         */
         if(errno == EPERM || errno == EBADF)
            return;

         // maybe just ignore any failure here and return?
         throw System_Error("System_RNG write failed", errno);
      }

      input += got;
      len -= got;
   }
}

#endif

}  // namespace

RandomNumberGenerator& system_rng() {
   static System_RNG_Impl g_system_rng;
   return g_system_rng;
}

}  // namespace Botan
/*
* Twofish
* (C) 1999-2007,2017,2026 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_CPUID)
#endif

namespace Botan {

namespace {

namespace Twofish_KS {

// Twofish q-permutation derived from four 4-bit sboxes
// ("Twofish: A 128-Bit Block Cipher", section 4.3.5)
consteval std::array<uint8_t, 256> twofish_q_perm(std::array<uint8_t, 16> t0,
                                                  std::array<uint8_t, 16> t1,
                                                  std::array<uint8_t, 16> t2,
                                                  std::array<uint8_t, 16> t3) noexcept {
   std::array<uint8_t, 256> Q = {};
   for(size_t x = 0; x != 256; ++x) {
      const uint8_t a0 = static_cast<uint8_t>((x >> 4) & 0x0F);
      const uint8_t b0 = static_cast<uint8_t>(x & 0x0F);

      const uint8_t a1 = a0 ^ b0;
      const uint8_t b1 = a0 ^ ((b0 >> 1) | ((b0 & 1) << 3)) ^ ((8 * a0) & 0x0F);

      const uint8_t a2 = t0[a1];
      const uint8_t b2 = t1[b1];

      const uint8_t a3 = a2 ^ b2;
      const uint8_t b3 = a2 ^ ((b2 >> 1) | ((b2 & 1) << 3)) ^ ((8 * a2) & 0x0F);

      const uint8_t a4 = t2[a3];
      const uint8_t b4 = t3[b3];

      Q[x] = static_cast<uint8_t>((b4 << 4) | a4);
   }
   return Q;
}

// clang-format off
alignas(256) constexpr auto Q0 = twofish_q_perm(
   {8, 1, 7, 13, 6, 15, 3, 2, 0, 11, 5, 9, 14, 12, 10, 4},
   {14, 12, 11, 8, 1, 2, 3, 5, 15, 4, 10, 6, 7, 0, 9, 13},
   {11, 10, 5, 14, 6, 13, 9, 0, 12, 8, 15, 3, 2, 4, 7, 1},
   {13, 7, 15, 4, 1, 2, 6, 14, 9, 11, 3, 0, 8, 5, 12, 10});

alignas(256) constexpr auto Q1 = twofish_q_perm(
   {2, 8, 11, 13, 15, 7, 6, 14, 3, 1, 9, 4, 0, 10, 12, 5},
   {1, 14, 2, 11, 4, 12, 3, 7, 6, 13, 10, 5, 15, 9, 0, 8},
   {4, 12, 7, 5, 1, 6, 9, 10, 0, 14, 13, 8, 2, 11, 3, 15},
   {11, 9, 5, 1, 12, 3, 13, 14, 6, 4, 7, 15, 2, 0, 8, 10});

// clang-format on

/*
* MDS matrix multiplication (Twofish paper Section 4.2)
*
* MDS = [01, EF, 5B, 5B]
*       [5B, EF, EF, 01]
*       [EF, 5B, 01, EF]
*       [EF, 01, EF, 5B]
*
* The MDS coefficients are 01, 5B, and EF. These were chosen so that
*
*   5B = 1 + 1/x^2
*   EF = 1 + 1/x + 1/x^2
*
* in GF(2^8) mod x^8+x^6+x^5+x^3+1, where 1/x is computed by shifting
* right and conditionally XORing with 0xB4 (which is itself just the
* irreducible polynomial 0x169 shifted right by 1).
*
* This property of the MDS constants is described (briefly) in Section 7.3
* of the Twofish paper.
*/

inline uint8_t mds_div_x(uint8_t q) {
   return (q >> 1) ^ (CT::value_barrier<uint8_t>(q & 1) * 0xB4);
}

inline uint32_t mds0(uint8_t q) {
   const uint8_t q_div_x = mds_div_x(q);
   const uint8_t q5b = q ^ mds_div_x(q_div_x);
   const uint8_t qef = q5b ^ q_div_x;
   return make_uint32(qef, qef, q5b, q);
}

inline uint32_t mds1(uint8_t q) {
   const uint8_t q_div_x = mds_div_x(q);
   const uint8_t q5b = q ^ mds_div_x(q_div_x);
   const uint8_t qef = q5b ^ q_div_x;
   return make_uint32(q, q5b, qef, qef);
}

inline uint32_t mds2(uint8_t q) {
   const uint8_t q_div_x = mds_div_x(q);
   const uint8_t q5b = q ^ mds_div_x(q_div_x);
   const uint8_t qef = q5b ^ q_div_x;
   return make_uint32(qef, q, qef, q5b);
}

inline uint32_t mds3(uint8_t q) {
   const uint8_t q_div_x = mds_div_x(q);
   const uint8_t q5b = q ^ mds_div_x(q_div_x);
   const uint8_t qef = q5b ^ q_div_x;
   return make_uint32(q5b, qef, q, q5b);
}

// Constant-time GF(2^8) multiply in the RS field (irreducible polynomial 0x14D)
inline uint32_t gf_mul_rs32(uint32_t rs, uint8_t k) {
   constexpr uint32_t lo_bit = 0x01010101;
   constexpr uint32_t mask = 0x7F7F7F7F;
   constexpr uint32_t poly = 0x4D;

   uint32_t r = 0;
   for(size_t i = 0; i != 8; ++i) {
      const auto k_lo = CT::Mask<uint32_t>::expand(k & 1);
      r ^= k_lo.if_set_return(rs);
      rs = ((rs & mask) << 1) ^ (((rs >> 7) & lo_bit) * poly);
      k >>= 1;
   }
   return r;
}

}  // namespace Twofish_KS

inline void TF_E(
   uint32_t A, uint32_t B, uint32_t& C, uint32_t& D, uint32_t RK1, uint32_t RK2, const secure_vector<uint32_t>& SB) {
   uint32_t X = SB[get_byte<3>(A)] ^ SB[256 + get_byte<2>(A)] ^ SB[512 + get_byte<1>(A)] ^ SB[768 + get_byte<0>(A)];
   uint32_t Y = SB[get_byte<0>(B)] ^ SB[256 + get_byte<3>(B)] ^ SB[512 + get_byte<2>(B)] ^ SB[768 + get_byte<1>(B)];

   X += Y;
   Y += X;

   X += RK1;
   Y += RK2;

   C = rotr<1>(C ^ X);
   D = rotl<1>(D) ^ Y;
}

inline void TF_D(
   uint32_t A, uint32_t B, uint32_t& C, uint32_t& D, uint32_t RK1, uint32_t RK2, const secure_vector<uint32_t>& SB) {
   uint32_t X = SB[get_byte<3>(A)] ^ SB[256 + get_byte<2>(A)] ^ SB[512 + get_byte<1>(A)] ^ SB[768 + get_byte<0>(A)];
   uint32_t Y = SB[get_byte<0>(B)] ^ SB[256 + get_byte<3>(B)] ^ SB[512 + get_byte<2>(B)] ^ SB[768 + get_byte<1>(B)];

   X += Y;
   Y += X;

   X += RK1;
   Y += RK2;

   C = rotl<1>(C) ^ X;
   D = rotr<1>(D ^ Y);
}

}  // namespace

/*
* Twofish Encryption
*/
void Twofish::encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();

#if defined(BOTAN_HAS_TWOFISH_AVX512)
   if(!m_QS.empty()) {
      while(blocks >= 16) {
         avx512_encrypt_16(in, out);
         in += 16 * BLOCK_SIZE;
         out += 16 * BLOCK_SIZE;
         blocks -= 16;
      }
   }
#endif

   while(blocks >= 2) {
      uint32_t A0 = 0;
      uint32_t B0 = 0;
      uint32_t C0 = 0;
      uint32_t D0 = 0;
      uint32_t A1 = 0;
      uint32_t B1 = 0;
      uint32_t C1 = 0;
      uint32_t D1 = 0;
      load_le(in, A0, B0, C0, D0, A1, B1, C1, D1);

      A0 ^= m_RK[0];
      A1 ^= m_RK[0];
      B0 ^= m_RK[1];
      B1 ^= m_RK[1];
      C0 ^= m_RK[2];
      C1 ^= m_RK[2];
      D0 ^= m_RK[3];
      D1 ^= m_RK[3];

      for(size_t k = 8; k != 40; k += 4) {
         TF_E(A0, B0, C0, D0, m_RK[k + 0], m_RK[k + 1], m_SB);
         TF_E(A1, B1, C1, D1, m_RK[k + 0], m_RK[k + 1], m_SB);

         TF_E(C0, D0, A0, B0, m_RK[k + 2], m_RK[k + 3], m_SB);
         TF_E(C1, D1, A1, B1, m_RK[k + 2], m_RK[k + 3], m_SB);
      }

      C0 ^= m_RK[4];
      C1 ^= m_RK[4];
      D0 ^= m_RK[5];
      D1 ^= m_RK[5];
      A0 ^= m_RK[6];
      A1 ^= m_RK[6];
      B0 ^= m_RK[7];
      B1 ^= m_RK[7];

      store_le(out, C0, D0, A0, B0, C1, D1, A1, B1);

      blocks -= 2;
      out += 2 * BLOCK_SIZE;
      in += 2 * BLOCK_SIZE;
   }

   if(blocks > 0) {
      uint32_t A = 0;
      uint32_t B = 0;
      uint32_t C = 0;
      uint32_t D = 0;
      load_le(in, A, B, C, D);

      A ^= m_RK[0];
      B ^= m_RK[1];
      C ^= m_RK[2];
      D ^= m_RK[3];

      for(size_t k = 8; k != 40; k += 4) {
         TF_E(A, B, C, D, m_RK[k], m_RK[k + 1], m_SB);
         TF_E(C, D, A, B, m_RK[k + 2], m_RK[k + 3], m_SB);
      }

      C ^= m_RK[4];
      D ^= m_RK[5];
      A ^= m_RK[6];
      B ^= m_RK[7];

      store_le(out, C, D, A, B);
   }
}

/*
* Twofish Decryption
*/
void Twofish::decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const {
   assert_key_material_set();

#if defined(BOTAN_HAS_TWOFISH_AVX512)
   if(!m_QS.empty()) {
      while(blocks >= 16) {
         avx512_decrypt_16(in, out);
         in += 16 * BLOCK_SIZE;
         out += 16 * BLOCK_SIZE;
         blocks -= 16;
      }
   }
#endif

   while(blocks >= 2) {
      uint32_t A0 = 0;
      uint32_t B0 = 0;
      uint32_t C0 = 0;
      uint32_t D0 = 0;
      uint32_t A1 = 0;
      uint32_t B1 = 0;
      uint32_t C1 = 0;
      uint32_t D1 = 0;
      load_le(in, A0, B0, C0, D0, A1, B1, C1, D1);

      A0 ^= m_RK[4];
      A1 ^= m_RK[4];
      B0 ^= m_RK[5];
      B1 ^= m_RK[5];
      C0 ^= m_RK[6];
      C1 ^= m_RK[6];
      D0 ^= m_RK[7];
      D1 ^= m_RK[7];

      for(size_t k = 40; k != 8; k -= 4) {
         TF_D(A0, B0, C0, D0, m_RK[k - 2], m_RK[k - 1], m_SB);
         TF_D(A1, B1, C1, D1, m_RK[k - 2], m_RK[k - 1], m_SB);

         TF_D(C0, D0, A0, B0, m_RK[k - 4], m_RK[k - 3], m_SB);
         TF_D(C1, D1, A1, B1, m_RK[k - 4], m_RK[k - 3], m_SB);
      }

      C0 ^= m_RK[0];
      C1 ^= m_RK[0];
      D0 ^= m_RK[1];
      D1 ^= m_RK[1];
      A0 ^= m_RK[2];
      A1 ^= m_RK[2];
      B0 ^= m_RK[3];
      B1 ^= m_RK[3];

      store_le(out, C0, D0, A0, B0, C1, D1, A1, B1);

      blocks -= 2;
      out += 2 * BLOCK_SIZE;
      in += 2 * BLOCK_SIZE;
   }

   if(blocks > 0) {
      uint32_t A = 0;
      uint32_t B = 0;
      uint32_t C = 0;
      uint32_t D = 0;
      load_le(in, A, B, C, D);

      A ^= m_RK[4];
      B ^= m_RK[5];
      C ^= m_RK[6];
      D ^= m_RK[7];

      for(size_t k = 40; k != 8; k -= 4) {
         TF_D(A, B, C, D, m_RK[k - 2], m_RK[k - 1], m_SB);
         TF_D(C, D, A, B, m_RK[k - 4], m_RK[k - 3], m_SB);
      }

      C ^= m_RK[0];
      D ^= m_RK[1];
      A ^= m_RK[2];
      B ^= m_RK[3];

      store_le(out, C, D, A, B);
   }
}

bool Twofish::has_keying_material() const {
   return !m_SB.empty();
}

std::string Twofish::provider() const {
#if defined(BOTAN_HAS_TWOFISH_AVX512)
   if(CPUID::has(CPUID::Feature::AVX512, CPUID::Feature::GFNI)) {
      return "avx512";
   }
#endif
   return "base";
}

size_t Twofish::parallelism() const {
#if defined(BOTAN_HAS_TWOFISH_AVX512)
   if(CPUID::has(CPUID::Feature::AVX512, CPUID::Feature::GFNI)) {
      return 16;
   }
#endif
   return 1;
}

/*
* Twofish Key Schedule
*/
void Twofish::key_schedule(std::span<const uint8_t> key) {
   using namespace Twofish_KS;

   // Reed-Solomon matrix for key schedule (Twofish paper Section 4.3)
   // in column-major form

   // clang-format off
   constexpr uint32_t RS32[8] = {
      0x01A402A4,
      0xA456A155,
      0x5582FC87,
      0x87F3C15A,
      0x5A1E4758,
      0x58C6AEDB,
      0xDB683D9E,
      0x9EE51903
   };
   // clang-format on

   m_RK.resize(40);

   secure_vector<uint8_t> S(16);

   for(size_t i = 0; i != key.size(); ++i) {
      const uint8_t ki = key[i];
      const size_t s_off = 4 * (i / 8);

      const uint32_t p = gf_mul_rs32(RS32[i % 8], ki);

      S[s_off + 0] ^= get_byte<0>(p);
      S[s_off + 1] ^= get_byte<1>(p);
      S[s_off + 2] ^= get_byte<2>(p);
      S[s_off + 3] ^= get_byte<3>(p);
   }

   secure_vector<uint8_t> QS(1024);

   if(key.size() == 16) {
      for(size_t i = 0; i != 256; ++i) {
         QS[i] = Q1[Q0[Q0[i] ^ S[0]] ^ S[4]];
         QS[256 + i] = Q0[Q0[Q1[i] ^ S[1]] ^ S[5]];
         QS[512 + i] = Q1[Q1[Q0[i] ^ S[2]] ^ S[6]];
         QS[768 + i] = Q0[Q1[Q1[i] ^ S[3]] ^ S[7]];
      }

      for(size_t i = 0; i < 40; i += 2) {
         uint32_t X = mds0(Q1[Q0[Q0[i] ^ key[8]] ^ key[0]]) ^ mds1(Q0[Q0[Q1[i] ^ key[9]] ^ key[1]]) ^
                      mds2(Q1[Q1[Q0[i] ^ key[10]] ^ key[2]]) ^ mds3(Q0[Q1[Q1[i] ^ key[11]] ^ key[3]]);
         uint32_t Y = mds0(Q1[Q0[Q0[i + 1] ^ key[12]] ^ key[4]]) ^ mds1(Q0[Q0[Q1[i + 1] ^ key[13]] ^ key[5]]) ^
                      mds2(Q1[Q1[Q0[i + 1] ^ key[14]] ^ key[6]]) ^ mds3(Q0[Q1[Q1[i + 1] ^ key[15]] ^ key[7]]);
         Y = rotl<8>(Y);
         X += Y;
         Y += X;

         m_RK[i] = X;
         m_RK[i + 1] = rotl<9>(Y);
      }
   } else if(key.size() == 24) {
      for(size_t i = 0; i != 256; ++i) {
         QS[i] = Q1[Q0[Q0[Q1[i] ^ S[0]] ^ S[4]] ^ S[8]];
         QS[256 + i] = Q0[Q0[Q1[Q1[i] ^ S[1]] ^ S[5]] ^ S[9]];
         QS[512 + i] = Q1[Q1[Q0[Q0[i] ^ S[2]] ^ S[6]] ^ S[10]];
         QS[768 + i] = Q0[Q1[Q1[Q0[i] ^ S[3]] ^ S[7]] ^ S[11]];
      }

      for(size_t i = 0; i < 40; i += 2) {
         uint32_t X =
            mds0(Q1[Q0[Q0[Q1[i] ^ key[16]] ^ key[8]] ^ key[0]]) ^ mds1(Q0[Q0[Q1[Q1[i] ^ key[17]] ^ key[9]] ^ key[1]]) ^
            mds2(Q1[Q1[Q0[Q0[i] ^ key[18]] ^ key[10]] ^ key[2]]) ^ mds3(Q0[Q1[Q1[Q0[i] ^ key[19]] ^ key[11]] ^ key[3]]);
         uint32_t Y = mds0(Q1[Q0[Q0[Q1[i + 1] ^ key[20]] ^ key[12]] ^ key[4]]) ^
                      mds1(Q0[Q0[Q1[Q1[i + 1] ^ key[21]] ^ key[13]] ^ key[5]]) ^
                      mds2(Q1[Q1[Q0[Q0[i + 1] ^ key[22]] ^ key[14]] ^ key[6]]) ^
                      mds3(Q0[Q1[Q1[Q0[i + 1] ^ key[23]] ^ key[15]] ^ key[7]]);
         Y = rotl<8>(Y);
         X += Y;
         Y += X;

         m_RK[i] = X;
         m_RK[i + 1] = rotl<9>(Y);
      }
   } else if(key.size() == 32) {
      for(size_t i = 0; i != 256; ++i) {
         QS[i] = Q1[Q0[Q0[Q1[Q1[i] ^ S[0]] ^ S[4]] ^ S[8]] ^ S[12]];
         QS[256 + i] = Q0[Q0[Q1[Q1[Q0[i] ^ S[1]] ^ S[5]] ^ S[9]] ^ S[13]];
         QS[512 + i] = Q1[Q1[Q0[Q0[Q0[i] ^ S[2]] ^ S[6]] ^ S[10]] ^ S[14]];
         QS[768 + i] = Q0[Q1[Q1[Q0[Q1[i] ^ S[3]] ^ S[7]] ^ S[11]] ^ S[15]];
      }

      for(size_t i = 0; i < 40; i += 2) {
         uint32_t X = mds0(Q1[Q0[Q0[Q1[Q1[i] ^ key[24]] ^ key[16]] ^ key[8]] ^ key[0]]) ^
                      mds1(Q0[Q0[Q1[Q1[Q0[i] ^ key[25]] ^ key[17]] ^ key[9]] ^ key[1]]) ^
                      mds2(Q1[Q1[Q0[Q0[Q0[i] ^ key[26]] ^ key[18]] ^ key[10]] ^ key[2]]) ^
                      mds3(Q0[Q1[Q1[Q0[Q1[i] ^ key[27]] ^ key[19]] ^ key[11]] ^ key[3]]);
         uint32_t Y = mds0(Q1[Q0[Q0[Q1[Q1[i + 1] ^ key[28]] ^ key[20]] ^ key[12]] ^ key[4]]) ^
                      mds1(Q0[Q0[Q1[Q1[Q0[i + 1] ^ key[29]] ^ key[21]] ^ key[13]] ^ key[5]]) ^
                      mds2(Q1[Q1[Q0[Q0[Q0[i + 1] ^ key[30]] ^ key[22]] ^ key[14]] ^ key[6]]) ^
                      mds3(Q0[Q1[Q1[Q0[Q1[i + 1] ^ key[31]] ^ key[23]] ^ key[15]] ^ key[7]]);
         Y = rotl<8>(Y);
         X += Y;
         Y += X;

         m_RK[i] = X;
         m_RK[i + 1] = rotl<9>(Y);
      }
   }

   m_SB.resize(1024);
   for(size_t i = 0; i != 256; ++i) {
      m_SB[i] = mds0(QS[i]);
      m_SB[256 + i] = mds1(QS[256 + i]);
      m_SB[512 + i] = mds2(QS[512 + i]);
      m_SB[768 + i] = mds3(QS[768 + i]);
   }

#if defined(BOTAN_HAS_TWOFISH_AVX512)
   if(CPUID::has(CPUID::Feature::AVX512, CPUID::Feature::GFNI)) {
      m_QS = std::move(QS);
   }
#endif
}

/*
* Clear memory of sensitive data
*/
void Twofish::clear() {
   zap(m_SB);
   zap(m_RK);
   zap(m_QS);
}

}  // namespace Botan
/*
* (C) 2017,2023 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <new>

#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
#endif

namespace Botan {

BOTAN_MALLOC_FN void* allocate_memory(size_t elems, size_t elem_size) {
   if(elems == 0 || elem_size == 0) {
      return nullptr;
   }

   // Some calloc implementations do not check for overflow (?!?)
   if(!checked_mul(elems, elem_size).has_value()) {
      throw std::bad_alloc();
   }

#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
   // NOLINTNEXTLINE(*-const-correctness) bug in clang-tidy
   if(void* p = mlock_allocator::instance().allocate(elems, elem_size)) {
      return p;
   }
#endif

#if defined(BOTAN_TARGET_OS_HAS_ALLOC_CONCEAL)
   void* ptr = ::calloc_conceal(elems, elem_size);
#else
   // NOLINTNEXTLINE(*-const-correctness) bug in clang-tidy
   void* ptr = std::calloc(elems, elem_size);  // NOLINT(*-no-malloc,*-owning-memory)
#endif
   if(ptr == nullptr) {
      [[unlikely]] throw std::bad_alloc();
   }
   return ptr;
}

void deallocate_memory(void* p, size_t elems, size_t elem_size) {
   if(p == nullptr) {
      [[unlikely]] return;
   }

   secure_scrub_memory(p, elems * elem_size);

#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
   if(mlock_allocator::instance().deallocate(p, elems, elem_size)) {
      return;
   }
#endif

   std::free(p);  // NOLINT(*-no-malloc,*-owning-memory)
}

void initialize_allocator() {
#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
   mlock_allocator::instance();
#endif
}

}  // namespace Botan
/*
* Runtime assertion checking
* (C) 2010,2012,2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_TERMINATE_ON_ASSERTS)
   #include <iostream>
#endif

namespace Botan {

void throw_invalid_argument(const char* message, const char* func, const char* file) {
   throw Invalid_Argument(fmt("{} in {}:{}", message, func, file));
}

void throw_invalid_state(const char* expr, const char* func, const char* file) {
   throw Invalid_State(fmt("Invalid state: expr {} was false in {}:{}", expr, func, file));
}

// Declared in concepts.h
void ranges::memory_region_size_violation() {
   throw Invalid_Argument("Memory regions did not have expected byte lengths");
}

void assertion_failure(const char* expr_str, const char* assertion_made, const char* func, const char* file, int line) {
   std::ostringstream format;

   format << "False assertion ";

   if(assertion_made != nullptr && assertion_made[0] != 0) {
      format << "'" << assertion_made << "' (expression " << expr_str << ") ";
   } else {
      format << expr_str << " ";
   }

   if(func != nullptr) {
      format << "in " << func << " ";
   }

   format << "@" << file << ":" << line;

#if defined(BOTAN_TERMINATE_ON_ASSERTS)
   std::cerr << format.str() << '\n';
   std::abort();
#else
   throw Internal_Error(format.str());
#endif
}

void assert_unreachable(const char* file, int line) {
   const std::string msg = fmt("Codepath that was marked unreachable was reached @{}:{}", file, line);

#if defined(BOTAN_TERMINATE_ON_ASSERTS)
   std::cerr << msg << '\n';
   std::abort();
#else
   throw Internal_Error(msg);
#endif
}

}  // namespace Botan
/*
* Calendar Functions
* (C) 1999-2010,2017 Jack Lloyd
* (C) 2015 Simon Warta (Kullo GmbH)
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <ctime>

namespace Botan {

namespace {

// TODO replace this with https://howardhinnant.github.io/date_algorithms.html#civil_from_days
std::tm do_gmtime(std::time_t time_val) {
   std::tm tm{};

#if defined(BOTAN_TARGET_OS_HAS_WIN32)
   ::gmtime_s(&tm, &time_val);  // Windows
#elif defined(BOTAN_TARGET_OS_HAS_POSIX1)
   if(::gmtime_r(&time_val, &tm) == nullptr) {
      throw Encoding_Error("do_gmtime could not convert");
   }
#else
   std::tm* tm_p = std::gmtime(&time_val);
   if(tm_p == nullptr) {
      throw Encoding_Error("do_gmtime could not convert");
   }
   tm = *tm_p;
#endif

   return tm;
}

/*
Portable replacement for timegm, _mkgmtime, etc

Algorithm due to Howard Hinnant

See https://howardhinnant.github.io/date_algorithms.html#days_from_civil
for details and explanation. The code is slightly simplified by our assumption
that the date is at least 1970, which is sufficient for our purposes.
*/
uint64_t days_since_epoch(uint32_t year, uint32_t month, uint32_t day) {
   BOTAN_ARG_CHECK(year >= 1970, "Years before 1970 not supported");

   if(month <= 2) {
      year -= 1;
   }
   const uint32_t era = year / 400;
   const uint32_t yoe = year - era * 400;                                          // [0, 399]
   const uint32_t doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;  // [0, 365]
   const uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;                     // [0, 146096]
   return era * 146097 + doe - 719468;
}

}  // namespace

uint64_t calendar_point::seconds_since_epoch() const {
   return (days_since_epoch(year(), month(), day()) * 86400) + (hour() * 60 * 60) + (minutes() * 60) + seconds();
}

std::chrono::system_clock::time_point calendar_point::to_std_timepoint() const {
   const uint64_t seconds_64 = this->seconds_since_epoch();
   const time_t seconds_time_t = static_cast<time_t>(seconds_64);

   if(seconds_64 - seconds_time_t != 0) {
      throw Invalid_Argument("calendar_point::to_std_timepoint time_t overflow");
   }

   return std::chrono::system_clock::from_time_t(seconds_time_t);
}

std::string calendar_point::to_string() const {
   // desired format: <YYYY>-<MM>-<dd>T<HH>:<mm>:<ss>
   std::stringstream output;
   output << std::setfill('0') << std::setw(4) << year() << "-" << std::setw(2) << month() << "-" << std::setw(2)
          << day() << "T" << std::setw(2) << hour() << ":" << std::setw(2) << minutes() << ":" << std::setw(2)
          << seconds();
   return output.str();
}

calendar_point::calendar_point(const std::chrono::system_clock::time_point& time_point) {
   const std::tm tm = do_gmtime(std::chrono::system_clock::to_time_t(time_point));

   m_year = tm.tm_year + 1900;
   m_month = tm.tm_mon + 1;
   m_day = tm.tm_mday;
   m_hour = tm.tm_hour;
   m_minutes = tm.tm_min;
   m_seconds = tm.tm_sec;
}

}  // namespace Botan
/*
* Character Set Handling
* (C) 1999-2007,2021 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

namespace {

void append_utf8_for(std::string& s, uint32_t c) {
   if(c >= 0xD800 && c < 0xE000) {
      throw Decoding_Error("Invalid Unicode character");
   }

   if(c <= 0x7F) {
      const uint8_t b0 = static_cast<uint8_t>(c);
      s.push_back(static_cast<char>(b0));
   } else if(c <= 0x7FF) {
      const uint8_t b0 = 0xC0 | static_cast<uint8_t>(c >> 6);
      const uint8_t b1 = 0x80 | static_cast<uint8_t>(c & 0x3F);
      s.push_back(static_cast<char>(b0));
      s.push_back(static_cast<char>(b1));
   } else if(c <= 0xFFFF) {
      const uint8_t b0 = 0xE0 | static_cast<uint8_t>(c >> 12);
      const uint8_t b1 = 0x80 | static_cast<uint8_t>((c >> 6) & 0x3F);
      const uint8_t b2 = 0x80 | static_cast<uint8_t>(c & 0x3F);
      s.push_back(static_cast<char>(b0));
      s.push_back(static_cast<char>(b1));
      s.push_back(static_cast<char>(b2));
   } else if(c <= 0x10FFFF) {
      const uint8_t b0 = 0xF0 | static_cast<uint8_t>(c >> 18);
      const uint8_t b1 = 0x80 | static_cast<uint8_t>((c >> 12) & 0x3F);
      const uint8_t b2 = 0x80 | static_cast<uint8_t>((c >> 6) & 0x3F);
      const uint8_t b3 = 0x80 | static_cast<uint8_t>(c & 0x3F);
      s.push_back(static_cast<char>(b0));
      s.push_back(static_cast<char>(b1));
      s.push_back(static_cast<char>(b2));
      s.push_back(static_cast<char>(b3));
   } else {
      throw Decoding_Error("Invalid Unicode character");
   }
}

uint32_t next_utf8_codepoint(const std::string& utf8, size_t& pos) {
   auto read_continuation = [&]() -> uint32_t {
      if(pos >= utf8.size()) {
         throw Decoding_Error("Invalid UTF-8 sequence");
      }
      const uint8_t b = static_cast<uint8_t>(utf8[pos++]);
      if((b & 0xC0) != 0x80) {
         throw Decoding_Error("Invalid UTF-8 sequence");
      }
      return b & 0x3F;
   };

   const uint8_t lead = static_cast<uint8_t>(utf8[pos++]);
   uint32_t c = 0;

   if(lead <= 0x7F) {
      c = lead;
   } else if((lead & 0xE0) == 0xC0) {
      c = (lead & 0x1F) << 6;
      c |= read_continuation();
      if(c < 0x80) {
         throw Decoding_Error("Overlong UTF-8 sequence");
      }
   } else if((lead & 0xF0) == 0xE0) {
      c = (lead & 0x0F) << 12;
      c |= read_continuation() << 6;
      c |= read_continuation();
      if(c < 0x800) {
         throw Decoding_Error("Overlong UTF-8 sequence");
      }
   } else if((lead & 0xF8) == 0xF0) {
      c = (lead & 0x07) << 18;
      c |= read_continuation() << 12;
      c |= read_continuation() << 6;
      c |= read_continuation();
      if(c < 0x10000) {
         throw Decoding_Error("Overlong UTF-8 sequence");
      }
   } else {
      throw Decoding_Error("Invalid UTF-8 sequence");
   }

   if(c > 0x10FFFF) {
      throw Decoding_Error("UTF-8 sequence encodes value outside Unicode range");
   }
   if(c >= 0xD800 && c < 0xE000) {
      throw Decoding_Error("UTF-8 sequence encodes surrogate code point");
   }

   return c;
}

}  // namespace

bool is_valid_utf8(const std::string& utf8) {
   try {
      size_t pos = 0;
      while(pos < utf8.size()) {
         const uint32_t c = next_utf8_codepoint(utf8, pos);
         BOTAN_UNUSED(c);
      }
   } catch(Decoding_Error&) {
      return false;
   }
   return true;
}

std::string ucs2_to_utf8(const uint8_t ucs2[], size_t len) {
   if(len % 2 != 0) {
      throw Decoding_Error("Invalid length for UCS-2 string");
   }

   const size_t chars = len / 2;

   std::string s;
   for(size_t i = 0; i != chars; ++i) {
      const uint32_t c = load_be<uint16_t>(ucs2, i);
      append_utf8_for(s, c);
   }

   return s;
}

std::vector<uint8_t> utf8_to_ucs2(const std::string& utf8) {
   std::vector<uint8_t> out;
   out.reserve(utf8.size() * 2);

   size_t pos = 0;
   while(pos < utf8.size()) {
      const uint32_t c = next_utf8_codepoint(utf8, pos);
      if(c > 0xFFFF) {
         throw Decoding_Error("Cannot encode character in UCS-2");
      }
      const uint16_t val = static_cast<uint16_t>(c);
      out.push_back(get_byte<0>(val));
      out.push_back(get_byte<1>(val));
   }

   return out;
}

std::string ucs4_to_utf8(const uint8_t ucs4[], size_t len) {
   if(len % 4 != 0) {
      throw Decoding_Error("Invalid length for UCS-4 string");
   }

   const size_t chars = len / 4;

   std::string s;
   for(size_t i = 0; i != chars; ++i) {
      const uint32_t c = load_be<uint32_t>(ucs4, i);
      append_utf8_for(s, c);
   }

   return s;
}

std::vector<uint8_t> utf8_to_ucs4(const std::string& utf8) {
   std::vector<uint8_t> out;
   out.reserve(utf8.size() * 4);

   size_t pos = 0;
   while(pos < utf8.size()) {
      const uint32_t val = next_utf8_codepoint(utf8, pos);
      out.push_back(get_byte<0>(val));
      out.push_back(get_byte<1>(val));
      out.push_back(get_byte<2>(val));
      out.push_back(get_byte<3>(val));
   }

   return out;
}

/*
* Convert from ISO 8859-1 to UTF-8
*/
std::string latin1_to_utf8(const uint8_t chars[], size_t len) {
   std::string s;
   for(size_t i = 0; i != len; ++i) {
      const uint32_t c = static_cast<uint8_t>(chars[i]);
      append_utf8_for(s, c);
   }
   return s;
}

std::string format_char_for_display(char c) {
   std::ostringstream oss;

   oss << "'";

   if(c == '\t') {
      oss << "\\t";
   } else if(c == '\n') {
      oss << "\\n";
   } else if(c == '\r') {
      oss << "\\r";
   } else if(static_cast<unsigned char>(c) >= 128) {
      const unsigned char z = static_cast<unsigned char>(c);
      oss << "\\x" << std::hex << std::uppercase << static_cast<int>(z);
   } else {
      oss << c;
   }

   oss << "'";

   return oss.str();
}

}  // namespace Botan
/*
* (C) 2018,2021 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

CT::Option<size_t> CT::copy_output(CT::Choice accept,
                                   std::span<uint8_t> output,
                                   std::span<const uint8_t> input,
                                   size_t offset) {
   // This leaks information about the input length, but this happens
   // unavoidably since we are unable to ready any bytes besides those
   // in input[0..n]
   BOTAN_ARG_CHECK(output.size() >= input.size(), "Invalid span lengths");

   /*
   * We do not poison the input here because if we did we would have
   * to unpoison it at exit. We assume instead that callers have
   * already poisoned the input and will unpoison it at their own
   * time.
   */
   CT::poison(offset);

   /**
   * Zeroize the entire output buffer to get started
   */
   clear_mem(output);

   /*
   * If the offset is greater than input length, then the arguments are
   * invalid. Ideally we would throw an exception, but that leaks
   * information about the offset. Instead treat it as if the input
   * was invalid.
   */
   accept = accept && CT::Mask<size_t>::is_lte(offset, input.size()).as_choice();

   /*
   * If the input is invalid, then set offset == input_length
   */
   offset = CT::Mask<size_t>::from_choice(accept).select(offset, input.size());

   /*
   * Move the desired output bytes to the front using a slow (O^n)
   * but constant time loop that does not leak the value of the offset
   */
   for(size_t i = 0; i != input.size(); ++i) {
      /*
      * If bad_input was set then we modified offset to equal the input_length.
      * In that case, this_loop will be greater than input_length, and so is_eq
      * mask will always be false. As a result none of the input values will be
      * written to output.
      *
      * This is ignoring the possibility of integer overflow of offset + i. But
      * for this to happen the input would have to consume nearly the entire
      * address space.
      */
      const size_t this_loop = offset + i;

      /*
      start index from i rather than 0 since we know j must be >= i + offset
      to have any effect, and starting from i does not reveal information
      */
      for(size_t j = i; j != input.size(); ++j) {
         const uint8_t b = input[j];
         const auto is_eq = CT::Mask<size_t>::is_equal(j, this_loop);
         output[i] |= is_eq.if_set_return(b);
      }
   }

   // This will always be zero if the input was invalid
   const size_t output_bytes = input.size() - offset;

   CT::unpoison_all(output, output_bytes);

   return CT::Option<size_t>(output_bytes, accept);
}

size_t CT::count_leading_zero_bytes(std::span<const uint8_t> input) {
   size_t leading_zeros = 0;
   auto only_zeros = Mask<uint8_t>::set();
   for(const uint8_t b : input) {
      only_zeros &= CT::Mask<uint8_t>::is_zero(b);
      leading_zeros += only_zeros.if_set_return(1);
   }
   return leading_zeros;
}

secure_vector<uint8_t> CT::strip_leading_zeros(std::span<const uint8_t> input) {
   const size_t leading_zeros = CT::count_leading_zero_bytes(input);

   secure_vector<uint8_t> output(input.size());

   const auto written = CT::copy_output(CT::Choice::yes(), output, input, leading_zeros);

   /*
   This is potentially not const time, depending on how std::vector is
   implemented. But since we are always reducing length, it should
   just amount to setting the member var holding the length.
   */
   output.resize(written.value_or(0));

   return output;
}

}  // namespace Botan
/*
* DataSource
* (C) 1999-2007 Jack Lloyd
*     2005 Matthew Gregan
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <istream>

#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)
   #include <fstream>
#endif

namespace Botan {

/*
* Read a single byte from the DataSource
*/
size_t DataSource::read_byte(uint8_t& out) {
   return read(&out, 1);
}

/*
* Read a single byte from the DataSource
*/
std::optional<uint8_t> DataSource::read_byte() {
   uint8_t b = 0;
   if(this->read(&b, 1) == 1) {
      return b;
   } else {
      return {};
   }
}

/*
* Peek a single byte from the DataSource
*/
size_t DataSource::peek_byte(uint8_t& out) const {
   return peek(&out, 1, 0);
}

/*
* Discard the next N bytes of the data
*/
size_t DataSource::discard_next(size_t n) {
   uint8_t buf[64] = {0};
   size_t discarded = 0;

   while(n > 0) {
      const size_t got = this->read(buf, std::min(n, sizeof(buf)));
      discarded += got;
      n -= got;

      if(got == 0) {
         break;
      }
   }

   return discarded;
}

/*
* Read from a memory buffer
*/
size_t DataSource_Memory::read(uint8_t out[], size_t length) {
   const size_t got = std::min<size_t>(m_source.size() - m_offset, length);
   copy_mem(out, m_source.data() + m_offset, got);
   m_offset += got;
   return got;
}

bool DataSource_Memory::check_available(size_t n) {
   return (n <= (m_source.size() - m_offset));
}

/*
* Peek into a memory buffer
*/
size_t DataSource_Memory::peek(uint8_t out[], size_t length, size_t peek_offset) const {
   const size_t bytes_left = m_source.size() - m_offset;
   if(peek_offset >= bytes_left) {
      return 0;
   }

   const size_t got = std::min(bytes_left - peek_offset, length);
   copy_mem(out, &m_source[m_offset + peek_offset], got);
   return got;
}

/*
* Check if the memory buffer is empty
*/
bool DataSource_Memory::end_of_data() const {
   return (m_offset == m_source.size());
}

/*
* DataSource_Memory Constructor
*/
DataSource_Memory::DataSource_Memory(std::string_view in) : DataSource_Memory(as_span_of_bytes(in)) {}

/*
* Read from a stream
*/
size_t DataSource_Stream::read(uint8_t out[], size_t length) {
   m_source.read(cast_uint8_ptr_to_char(out), length);
   if(m_source.bad()) {
      throw Stream_IO_Error("DataSource_Stream::read: Source failure");
   }

   const size_t got = static_cast<size_t>(m_source.gcount());
   m_total_read += got;
   return got;
}

bool DataSource_Stream::check_available(size_t n) {
   const std::streampos orig_pos = m_source.tellg();
   m_source.seekg(0, std::ios::end);
   const size_t avail = static_cast<size_t>(m_source.tellg() - orig_pos);
   m_source.seekg(orig_pos);
   return (avail >= n);
}

/*
* Peek into a stream
*/
size_t DataSource_Stream::peek(uint8_t out[], size_t length, size_t offset) const {
   if(end_of_data()) {
      throw Invalid_State("DataSource_Stream: Cannot peek when out of data");
   }

   size_t got = 0;

   if(offset > 0) {
      m_source.seekg(offset, std::ios::cur);
      if(!m_source.good()) {
         m_source.clear();
         m_source.seekg(m_total_read, std::ios::beg);
         return 0;
      }
   }

   m_source.read(cast_uint8_ptr_to_char(out), length);
   if(m_source.bad()) {
      throw Stream_IO_Error("DataSource_Stream::peek: Source failure");
   }
   got = static_cast<size_t>(m_source.gcount());

   if(m_source.eof()) {
      m_source.clear();
   }
   m_source.seekg(m_total_read, std::ios::beg);

   return got;
}

/*
* Check if the stream is empty or in error
*/
bool DataSource_Stream::end_of_data() const {
   /*
   Peek to trigger EOF indicator if positioned at the end of the stream.
   Without this, good() returns true even when all data has been read.
   */
   m_source.peek();
   return (!m_source.good());
}

/*
* Return a human-readable ID for this stream
*/
std::string DataSource_Stream::id() const {
   return m_identifier;
}

#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)

/*
* DataSource_Stream Constructor
*/
DataSource_Stream::DataSource_Stream(std::string_view path, bool use_binary) :
      m_identifier(path),
      m_source_memory(std::make_unique<std::ifstream>(std::string(path), use_binary ? std::ios::binary : std::ios::in)),
      m_source(*m_source_memory),
      m_total_read(0) {
   if(!m_source.good()) {
      throw Stream_IO_Error(fmt("DataSource: Failure opening file '{}'", path));
   }
}

#endif

/*
* DataSource_Stream Constructor
*/
DataSource_Stream::DataSource_Stream(std::istream& in, std::string_view name) :
      m_identifier(name), m_source(in), m_total_read(0) {}

DataSource_Stream::~DataSource_Stream() = default;

}  // namespace Botan
/*
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

std::string to_string(ErrorType type) {
   switch(type) {
      case ErrorType::Unknown:
         return "Unknown";
      case ErrorType::SystemError:
         return "SystemError";
      case ErrorType::NotImplemented:
         return "NotImplemented";
      case ErrorType::OutOfMemory:
         return "OutOfMemory";
      case ErrorType::InternalError:
         return "InternalError";
      case ErrorType::IoError:
         return "IoError";
      case ErrorType::InvalidObjectState:
         return "InvalidObjectState";
      case ErrorType::KeyNotSet:
         return "KeyNotSet";
      case ErrorType::InvalidArgument:
         return "InvalidArgument";
      case ErrorType::InvalidKeyLength:
         return "InvalidKeyLength";
      case ErrorType::InvalidNonceLength:
         return "InvalidNonceLength";
      case ErrorType::LookupError:
         return "LookupError";
      case ErrorType::EncodingFailure:
         return "EncodingFailure";
      case ErrorType::DecodingFailure:
         return "DecodingFailure";
      case ErrorType::TLSError:
         return "TLSError";
      case ErrorType::HttpError:
         return "HttpError";
      case ErrorType::InvalidTag:
         return "InvalidTag";
      case ErrorType::RoughtimeError:
         return "RoughtimeError";
      case ErrorType::CommonCryptoError:
         return "CommonCryptoError";
      case ErrorType::Pkcs11Error:
         return "Pkcs11Error";
      case ErrorType::TPMError:
         return "TPMError";
      case ErrorType::DatabaseError:
         return "DatabaseError";
      case ErrorType::ZlibError:
         return "ZlibError";
      case ErrorType::Bzip2Error:
         return "Bzip2Error";
      case ErrorType::LzmaError:
         return "LzmaError";
   }

   // No default case in above switch so compiler warns
   return "Unrecognized Botan error";
}

Exception::Exception(std::string_view msg) : m_msg(msg) {}

Exception::Exception(std::string_view msg, const std::exception& e) : m_msg(fmt("{} failed with {}", msg, e.what())) {}

Exception::Exception(const char* prefix, std::string_view msg) : m_msg(fmt("{} {}", prefix, msg)) {}

Invalid_Argument::Invalid_Argument(std::string_view msg) : Exception(msg) {}

Invalid_Argument::Invalid_Argument(std::string_view msg, std::string_view where) :
      Exception(fmt("{} in {}", msg, where)) {}

Invalid_Argument::Invalid_Argument(std::string_view msg, const std::exception& e) : Exception(msg, e) {}

namespace {

std::string format_lookup_error(std::string_view type, std::string_view algo, std::string_view provider) {
   if(provider.empty()) {
      return fmt("Unavailable {} {}", type, algo);
   } else {
      return fmt("Unavailable {} {} for provider {}", type, algo, provider);
   }
}

}  // namespace

Lookup_Error::Lookup_Error(std::string_view type, std::string_view algo, std::string_view provider) :
      Exception(format_lookup_error(type, algo, provider)) {}

Internal_Error::Internal_Error(std::string_view err) : Exception("Internal error:", err) {}

Unknown_PK_Field_Name::Unknown_PK_Field_Name(std::string_view algo_name, std::string_view field_name) :
      Invalid_Argument(fmt("Unknown field '{}' for algorithm {}", field_name, algo_name)) {}

Invalid_Key_Length::Invalid_Key_Length(std::string_view name, size_t length) :
      Invalid_Argument(fmt("{} cannot accept a key of length {}", name, length)) {}

Invalid_IV_Length::Invalid_IV_Length(std::string_view mode, size_t bad_len) :
      Invalid_Argument(fmt("IV length {} is invalid for {}", bad_len, mode)) {}

Key_Not_Set::Key_Not_Set(std::string_view algo) : Invalid_State(fmt("Key not set in {}", algo)) {}

PRNG_Unseeded::PRNG_Unseeded(std::string_view algo) : Invalid_State(fmt("PRNG {} not seeded", algo)) {}

Algorithm_Not_Found::Algorithm_Not_Found(std::string_view name) :
      Lookup_Error(fmt("Could not find any algorithm named '{}'", name)) {}

Provider_Not_Found::Provider_Not_Found(std::string_view algo, std::string_view provider) :
      Lookup_Error(fmt("Could not find provider '{}' for algorithm '{}'", provider, algo)) {}

Invalid_Algorithm_Name::Invalid_Algorithm_Name(std::string_view name) :
      Invalid_Argument(fmt("Invalid algorithm name: '{}'", name)) {}

Encoding_Error::Encoding_Error(std::string_view name) : Exception("Encoding error:", name) {}

Decoding_Error::Decoding_Error(std::string_view name) : Exception(name) {}

Decoding_Error::Decoding_Error(std::string_view category, std::string_view err) :
      Exception(fmt("{}: {}", category, err)) {}

Decoding_Error::Decoding_Error(std::string_view msg, const std::exception& e) : Exception(msg, e) {}

Invalid_Authentication_Tag::Invalid_Authentication_Tag(std::string_view msg) :
      Exception("Invalid authentication tag:", msg) {}

Stream_IO_Error::Stream_IO_Error(std::string_view err) : Exception("I/O error:", err) {}

System_Error::System_Error(std::string_view msg, int err_code) :
      Exception(fmt("{} error code {}", msg, err_code)), m_error_code(err_code) {}

Not_Implemented::Not_Implemented(std::string_view err) : Exception("Not implemented", err) {}

}  // namespace Botan
/*
* (C) 2015,2017,2019 Jack Lloyd
* (C) 2015 Simon Warta (Kullo GmbH)
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <deque>

#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   #include <dirent.h>
   #include <functional>
   #include <sys/stat.h>
   #include <sys/types.h>
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
   #define NOMINMAX 1
   #define _WINSOCKAPI_  // stop windows.h including winsock.h
   #include <windows.h>
#endif

namespace Botan {

namespace {

#if defined(BOTAN_TARGET_OS_HAS_POSIX1)

std::vector<std::string> impl_readdir(std::string_view dir_path) {
   std::vector<std::string> out;
   std::deque<std::string> dir_list;
   dir_list.push_back(std::string(dir_path));

   while(!dir_list.empty()) {
      const std::string cur_path = dir_list[0];
      dir_list.pop_front();

      const std::unique_ptr<DIR, std::function<int(DIR*)>> dir(::opendir(cur_path.c_str()), ::closedir);

      if(dir) {
         while(struct dirent* dirent = ::readdir(dir.get())) {
            const std::string filename = dirent->d_name;
            if(filename == "." || filename == "..") {
               continue;
            }

            std::ostringstream full_path_sstr;
            full_path_sstr << cur_path << "/" << filename;
            const std::string full_path = full_path_sstr.str();

            struct stat stat_buf {};

            if(::stat(full_path.c_str(), &stat_buf) == -1) {
               continue;
            }

            if(S_ISDIR(stat_buf.st_mode)) {
               dir_list.push_back(full_path);
            } else if(S_ISREG(stat_buf.st_mode)) {
               out.push_back(full_path);
            }
         }
      }
   }

   return out;
}

#elif defined(BOTAN_TARGET_OS_HAS_WIN32)

std::vector<std::string> impl_win32(std::string_view dir_path) {
   std::vector<std::string> out;
   std::deque<std::string> dir_list;
   dir_list.push_back(std::string(dir_path));

   while(!dir_list.empty()) {
      const std::string cur_path = dir_list[0];
      dir_list.pop_front();

      WIN32_FIND_DATAA find_data;
      HANDLE dir = ::FindFirstFileA((cur_path + "/*").c_str(), &find_data);

      if(dir != INVALID_HANDLE_VALUE) {
         do {
            const std::string filename = find_data.cFileName;
            if(filename == "." || filename == "..")
               continue;
            const std::string full_path = cur_path + "/" + filename;

            if(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
               dir_list.push_back(full_path);
            } else {
               out.push_back(full_path);
            }
         } while(::FindNextFileA(dir, &find_data));
      }

      ::FindClose(dir);
   }

   return out;
}
#endif

}  // namespace

bool has_filesystem_impl() {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   return true;
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
   return true;
#else
   return false;
#endif
}

std::vector<std::string> get_files_recursive(std::string_view dir) {
   std::vector<std::string> files;

#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   files = impl_readdir(dir);
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
   files = impl_win32(dir);
#else
   BOTAN_UNUSED(dir);
   throw No_Filesystem_Access();
#endif

   std::sort(files.begin(), files.end());

   return files;
}

}  // namespace Botan
/*
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

uint8_t ct_compare_u8(const uint8_t x[], const uint8_t y[], size_t len) {
   return CT::is_equal(x, y, len).value();
}

bool constant_time_compare(std::span<const uint8_t> x, std::span<const uint8_t> y) {
   const auto min_size = CT::Mask<size_t>::is_lte(x.size(), y.size()).select(x.size(), y.size());
   const auto equal_size = CT::Mask<size_t>::is_equal(x.size(), y.size());
   const auto equal_content = CT::Mask<size_t>::expand(CT::is_equal(x.data(), y.data(), min_size));
   return (equal_content & equal_size).as_bool();
}

}  // namespace Botan
/*
* (C) 2025 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <cstring>

#if defined(BOTAN_TARGET_OS_HAS_EXPLICIT_BZERO)
   #include <string.h>
#endif

#if defined(BOTAN_TARGET_OS_HAS_RTLSECUREZEROMEMORY)
   #define NOMINMAX 1
   #define _WINSOCKAPI_  // stop windows.h including winsock.h
   #include <windows.h>
#endif

namespace Botan {

void secure_scrub_memory(void* ptr, size_t n) {
   return secure_zeroize_buffer(ptr, n);
}

void secure_zeroize_buffer(void* ptr, size_t n) {
   if(n == 0) {
      return;
   }

#if defined(BOTAN_TARGET_OS_HAS_RTLSECUREZEROMEMORY)
   ::RtlSecureZeroMemory(ptr, n);

#elif defined(BOTAN_TARGET_OS_HAS_EXPLICIT_BZERO)
   ::explicit_bzero(ptr, n);

#elif defined(BOTAN_TARGET_OS_HAS_EXPLICIT_MEMSET)
   (void)::explicit_memset(ptr, 0, n);

#else
   /*
   * Call memset through a static volatile pointer, which the compiler should
   * not elide. This construct should be safe in conforming compilers, but who
   * knows. This has been checked to generate the expected code, which saves the
   * memset address in the data segment and unconditionally loads and jumps to
   * that address, with the following targets:
   *
   * x86-64: Clang 19, GCC 6, 11, 13, 14
   * riscv64: GCC 14
   * aarch64: GCC 14
   * armv7: GCC 14
   *
   * Actually all of them generated the expected jump even without marking the
   * function pointer as volatile. However this seems worth including as an
   * additional precaution.
   */
   static void* (*const volatile memset_ptr)(void*, int, size_t) = std::memset;
   (memset_ptr)(ptr, 0, n);
#endif
}

}  // namespace Botan
/*
* Various string utils and parsing functions
* (C) 1999-2007,2013,2014,2015,2018 Jack Lloyd
* (C) 2015 Simon Warta (Kullo GmbH)
* (C) 2017 René Korthaus, Rohde & Schwarz Cybersecurity
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

uint16_t to_uint16(std::string_view str) {
   const uint32_t x = to_u32bit(str);

   if(x != static_cast<uint16_t>(x)) {
      throw Invalid_Argument("Integer value exceeds 16 bit range");
   }

   return static_cast<uint16_t>(x);
}

uint32_t to_u32bit(std::string_view str_view) {
   const std::string str(str_view);

   // std::stoul is not strict enough. Ensure that str is digit only [0-9]*
   for(const char chr : str) {
      if(chr < '0' || chr > '9') {
         throw Invalid_Argument("to_u32bit invalid decimal string '" + str + "'");
      }
   }

   const unsigned long int x = std::stoul(str);

   if constexpr(sizeof(unsigned long int) > 4) {
      // x might be uint64
      if(x > std::numeric_limits<uint32_t>::max()) {
         throw Invalid_Argument("Integer value of " + str + " exceeds 32 bit range");
      }
   }

   return static_cast<uint32_t>(x);
}

/*
* Parse a SCAN-style algorithm name
*/
std::vector<std::string> parse_algorithm_name(std::string_view scan_name) {
   if(scan_name.find('(') == std::string::npos && scan_name.find(')') == std::string::npos) {
      return {std::string(scan_name)};
   }

   std::string name(scan_name);
   std::string substring;
   std::vector<std::string> elems;
   size_t level = 0;

   elems.push_back(name.substr(0, name.find('(')));
   name = name.substr(name.find('('));

   for(auto i = name.begin(); i != name.end(); ++i) {
      const char c = *i;

      if(c == '(') {
         ++level;
      }
      if(c == ')') {
         if(level == 1 && i == name.end() - 1) {
            if(elems.size() == 1) {
               elems.push_back(substring.substr(1));
            } else {
               elems.push_back(substring);
            }
            return elems;
         }

         if(level == 0 || (level == 1 && i != name.end() - 1)) {
            throw Invalid_Algorithm_Name(scan_name);
         }
         --level;
      }

      if(c == ',' && level == 1) {
         if(elems.size() == 1) {
            elems.push_back(substring.substr(1));
         } else {
            elems.push_back(substring);
         }
         substring.clear();
      } else {
         substring += c;
      }
   }

   if(!substring.empty()) {
      throw Invalid_Algorithm_Name(scan_name);
   }

   return elems;
}

std::vector<std::string> split_on(std::string_view str, char delim) {
   std::vector<std::string> elems;
   if(str.empty()) {
      return elems;
   }

   std::string substr;
   for(const char c : str) {
      if(c == delim) {
         if(!substr.empty()) {
            elems.push_back(substr);
         }
         substr.clear();
      } else {
         substr += c;
      }
   }

   if(substr.empty()) {
      throw Invalid_Argument(fmt("Unable to split string '{}", str));
   }
   elems.push_back(substr);

   return elems;
}

/*
* Join a string
*/
std::string string_join(const std::vector<std::string>& strs, char delim) {
   std::ostringstream out;

   for(size_t i = 0; i != strs.size(); ++i) {
      if(i != 0) {
         out << delim;
      }
      out << strs[i];
   }

   return out.str();
}

/*
* Convert a decimal-dotted string to binary IP
*/
std::optional<uint32_t> string_to_ipv4(std::string_view str) {
   // At least 3 dots + 4 1-digit integers
   // At most 3 dots + 4 3-digit integers
   if(str.size() < 3 + 4 * 1 || str.size() > 3 + 4 * 3) {
      return {};
   }

   // the final result
   uint32_t ip = 0;
   // the number of '.' seen so far
   size_t dots = 0;
   // accumulates one quad (range 0-255)
   uint32_t accum = 0;
   // # of digits pushed to accum since last dot
   size_t cur_digits = 0;

   for(const char c : str) {
      if(c == '.') {
         // . without preceding digit is invalid
         if(cur_digits == 0) {
            return {};
         }
         dots += 1;
         // too many dots
         if(dots > 3) {
            return {};
         }

         cur_digits = 0;
         ip = (ip << 8) | accum;
         accum = 0;
      } else if(c >= '0' && c <= '9') {
         const auto d = static_cast<uint8_t>(c - '0');

         // prohibit leading zero in quad (used for octal)
         if(cur_digits > 0 && accum == 0) {
            return {};
         }
         accum = (accum * 10) + d;

         if(accum > 255) {
            return {};
         }

         cur_digits++;
         BOTAN_ASSERT_NOMSG(cur_digits <= 3);
      } else {
         return {};
      }
   }

   // no trailing digits?
   if(cur_digits == 0) {
      return {};
   }

   // insufficient # of dots
   if(dots != 3) {
      return {};
   }

   ip = (ip << 8) | accum;

   return ip;
}

std::optional<std::array<uint8_t, 16>> string_to_ipv6(std::string_view str) {
   if(str.empty()) {
      return {};
   }

   // Parsed hex groups, split by whether they appeared before or after a "::".
   // If no "::" appears, only `pre` is populated and must reach exactly 8 groups.
   std::array<uint16_t, 8> pre{};
   std::array<uint16_t, 8> post{};
   size_t pre_count = 0;
   size_t post_count = 0;
   bool seen_double_colon = false;

   auto hex_value = [](char c) -> std::optional<uint8_t> {
      if(c >= '0' && c <= '9') {
         return c - '0';
      } else if(c >= 'a' && c <= 'f') {
         return 10 + (c - 'a');
      } else if(c >= 'A' && c <= 'F') {
         return 10 + (c - 'A');
      } else {
         return {};
      }
   };

   size_t idx = 0;
   bool expect_group = true;  // set after any separator, cleared after a group

   while(idx < str.size()) {
      if(str[idx] == ':') {
         if(idx + 1 < str.size() && str[idx + 1] == ':') {
            if(seen_double_colon) {
               return {};  // at most one "::"
            }
            seen_double_colon = true;
            idx += 2;
            expect_group = (idx < str.size());
            continue;
         }
         // single ':' separator between groups — only valid after a group
         if(expect_group) {
            return {};
         }
         expect_group = true;
         idx += 1;
         continue;
      }

      // Parse a hex group of 1..4 digits
      uint32_t group = 0;
      size_t hex_chars = 0;
      while(idx < str.size() && hex_chars < 4) {
         const auto digit = hex_value(str[idx]);
         if(digit.has_value() == false) {
            break;
         }
         group = (group << 4) | static_cast<uint32_t>(digit.value());
         idx += 1;
         hex_chars += 1;
      }
      if(hex_chars == 0) {
         return {};
      }
      // If a 5th hex digit follows, the group is oversized.
      if(hex_chars == 4 && idx < str.size() && hex_value(str[idx]).has_value()) {
         return {};
      }

      if(seen_double_colon) {
         if(post_count >= 8) {
            return {};
         }
         post[post_count++] = static_cast<uint16_t>(group);
      } else {
         if(pre_count >= 8) {
            return {};
         }
         pre[pre_count++] = static_cast<uint16_t>(group);
      }
      expect_group = false;
   }

   // Trailing single ':' is invalid
   if(expect_group) {
      return {};
   }

   const size_t total_groups = pre_count + post_count;
   if(seen_double_colon) {
      // "::" has to cover at least one zero group
      if(total_groups > 7) {
         return {};
      }
   } else {
      if(total_groups != 8) {
         return {};
      }
   }

   std::array<uint8_t, 16> out{};
   for(size_t i = 0; i != pre_count; ++i) {
      out[2 * i] = get_byte<0>(pre[i]);
      out[2 * i + 1] = get_byte<1>(pre[i]);
   }
   const size_t gap = 8 - total_groups;
   for(size_t i = 0; i != post_count; ++i) {
      const size_t target = pre_count + gap + i;
      out[2 * target] = get_byte<0>(post[i]);
      out[2 * target + 1] = get_byte<1>(post[i]);
   }
   return out;
}

std::string ipv6_to_string(std::span<const uint8_t, 16> a) {
   static const char* hex = "0123456789abcdef";

   std::string out;
   out.reserve(39);

   for(size_t i = 0; i != 16; i += 2) {
      if(i != 0) {
         out.push_back(':');
      }
      const uint16_t group = make_uint16(a[i], a[i + 1]);
      bool started = false;
      // Write each nibble omitting leading 0s
      for(int s = 12; s >= 0; s -= 4) {
         const auto nibble = (group >> s) & 0xF;
         if(nibble != 0 || started || s == 0) {
            out.push_back(hex[nibble]);
            started = true;
         }
      }
   }
   return out;
}

/*
* Convert an IP address to decimal-dotted string
*/
std::string ipv4_to_string(uint32_t ip) {
   uint8_t bits[4];
   store_be(ip, bits);

   std::string str;

   for(size_t i = 0; i != 4; ++i) {
      if(i > 0) {
         str += ".";
      }
      str += std::to_string(bits[i]);
   }

   return str;
}

std::string tolower_string(std::string_view str) {
   // Locale-independent ASCII fold; the only callers (DNS name canonicalization
   // for SAN/name-constraints) work on ASCII strings per RFC 1035.
   std::string lower(str);
   for(char& c : lower) {
      if(c >= 'A' && c <= 'Z') {
         c = static_cast<char>(c + ('a' - 'A'));
      }
   }
   return lower;
}

bool host_wildcard_match(std::string_view issued, std::string_view host) {
   if(host.empty() || issued.empty()) {
      return false;
   }

   // Maximum valid DNS name
   if(host.size() > 253) {
      return false;
   }

   /*
   The wildcard if existing absorbs (host.size() - issued.size() + 1) chars,
   which must be non-negative. So issued cannot possibly exceed host.size() + 1.
   */
   if(issued.size() > host.size() + 1) {
      return false;
   }

   /*
   If there are embedded nulls in your issued name
   Well I feel bad for you son
   */
   if(issued.find('\0') != std::string_view::npos) {
      return false;
   }

   // '*' is not a valid character in DNS names so should not appear on the host side
   if(host.find('*') != std::string_view::npos) {
      return false;
   }

   // Similarly a DNS name can't end in .
   if(host.back() == '.') {
      return false;
   }

   // And a host can't have an empty name component, so reject that
   if(host.find("..") != std::string_view::npos) {
      return false;
   }

   // ASCII-only case-insensitive char equality, avoids locale overhead from tolower
   auto dns_char_eq = [](char a, char b) -> bool {
      if(a == b) {
         return true;
      }
      const auto la = static_cast<unsigned char>(a | 0x20);
      const auto lb = static_cast<unsigned char>(b | 0x20);
      return la == lb && la >= 'a' && la <= 'z';
   };

   auto dns_char_eq_range = [&](std::string_view a, std::string_view b) -> bool {
      if(a.size() != b.size()) {
         return false;
      }
      for(size_t i = 0; i != a.size(); ++i) {
         if(!dns_char_eq(a[i], b[i])) {
            return false;
         }
      }
      return true;
   };

   // Exact match: accept
   if(dns_char_eq_range(issued, host)) {
      return true;
   }

   // First detect offset of wildcard '*' if included
   const size_t first_star = issued.find('*');
   const bool has_wildcard = (first_star != std::string_view::npos);

   // At most one wildcard is allowed
   if(has_wildcard && issued.find('*', first_star + 1) != std::string_view::npos) {
      return false;
   }

   // If no * at all then not a wildcard, and so not a match
   if(!has_wildcard) {
      return false;
   }

   /*
   Now walk through the issued string, making sure every character
   matches. When we come to the (singular) '*', jump forward in the
   hostname by the corresponding amount. We know exactly how much
   space the wildcard takes because it must be exactly `len(host) -
   len(issued) + 1 chars`.

   We also verify that the '*' comes in the leftmost component, and
   doesn't skip over any '.' in the hostname.
   */
   size_t dots_seen = 0;
   size_t host_idx = 0;

   for(size_t i = 0; i != issued.size(); ++i) {
      if(issued[i] == '.') {
         dots_seen += 1;
      }

      if(issued[i] == '*') {
         // Fail: wildcard can only come in leftmost component
         if(dots_seen > 0) {
            return false;
         }

         /*
         Since there is only one * we know the tail of the issued and
         hostname must be an exact match. In this case advance host_idx
         to match.
         */
         const size_t advance = (host.size() - issued.size() + 1);

         if(host_idx + advance > host.size()) {  // shouldn't happen
            return false;
         }

         // Can't be any intervening .s that we would have skipped
         for(size_t k = host_idx; k != host_idx + advance; ++k) {
            if(host[k] == '.') {
               return false;
            }
         }

         host_idx += advance;
      } else {
         if(!dns_char_eq(issued[i], host[host_idx])) {
            return false;
         }

         host_idx += 1;
      }
   }

   // Wildcard issued name must have at least 3 components
   if(dots_seen < 2) {
      return false;
   }

   return true;
}

std::string check_and_canonicalize_dns_name(std::string_view name) {
   if(name.size() > 255) {
      throw Decoding_Error("DNS name exceeds maximum allowed length");
   }

   if(name.empty()) {
      throw Decoding_Error("DNS name cannot be empty");
   }

   if(name.starts_with(".") || name.ends_with(".")) {
      throw Decoding_Error("DNS name cannot start or end with a dot");
   }

   /*
   * Table mapping uppercase to lowercase and only including values for valid DNS names
   * namely A-Z, a-z, 0-9, hyphen, and dot, plus '*' for wildcarding. (RFC 1035)
   */
   // clang-format off
   constexpr uint8_t DNS_CHAR_MAPPING[128] = {
      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
      '\0', '\0', '\0', '\0',  '*', '\0', '\0',  '-',  '.', '\0',  '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
       '9', '\0', '\0', '\0', '\0', '\0', '\0', '\0',  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',
       'l',  'm',  'n',  'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z', '\0', '\0', '\0', '\0',
      '\0', '\0',  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',  'p',  'q',
       'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z', '\0', '\0', '\0', '\0', '\0',
   };
   // clang-format on

   std::string canon;
   canon.reserve(name.size());

   // RFC 1035: DNS labels must not exceed 63 characters
   size_t current_label_length = 0;

   for(size_t i = 0; i != name.size(); ++i) {
      const char c = name[i];

      if(c == '.') {
         if(i > 0 && name[i - 1] == '.') {
            throw Decoding_Error("DNS name contains sequential period chars");
         }

         if(current_label_length == 0) {
            throw Decoding_Error("DNS name contains empty label");
         }
         current_label_length = 0;  // Reset for next label
      } else {
         current_label_length++;

         if(current_label_length > 63) {  // RFC 1035 Maximum DNS label length
            throw Decoding_Error("DNS name label exceeds maximum length of 63 characters");
         }
      }

      const uint8_t cu = static_cast<uint8_t>(c);
      if(cu >= 128) {
         throw Decoding_Error("DNS name must not contain any extended ASCII code points");
      }
      const uint8_t mapped = DNS_CHAR_MAPPING[cu];
      if(mapped == 0) {
         throw Decoding_Error("DNS name includes invalid character");
      }

      if(mapped == '-') {
         if(i == 0 || (i > 0 && name[i - 1] == '.')) {
            throw Decoding_Error("DNS name has label with leading hyphen");
         } else if(i == name.size() - 1 || (i < name.size() - 1 && name[i + 1] == '.')) {
            throw Decoding_Error("DNS name has label with trailing hyphen");
         }
      }
      canon.push_back(static_cast<char>(mapped));
   }

   if(current_label_length == 0) {
      throw Decoding_Error("DNS name contains empty label");
   }
   return canon;
}

}  // namespace Botan
/*
* (C) 2023 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

uint64_t prefetch_array_raw(size_t bytes, const void* arrayv) noexcept {
#if defined(__cpp_lib_hardware_interference_size)
   const size_t cache_line_size = std::hardware_destructive_interference_size;
#else
   // We arbitrarily use a 64 byte cache line, which is by far the most
   // common size.
   //
   // Runtime detection adds too much overhead to this function.
   const size_t cache_line_size = 64;
#endif

   const uint8_t* array = static_cast<const uint8_t*>(arrayv);

   volatile uint64_t combiner = 1;

   for(size_t idx = 0; idx < bytes; idx += cache_line_size) {
#if BOTAN_COMPILER_HAS_BUILTIN(__builtin_prefetch)
      // we have no way of knowing if the compiler will emit anything here
      __builtin_prefetch(&array[idx]);
#endif

      combiner = combiner | array[idx];
   }

   /*
   * The combiner variable is initialized with 1, and we accumulate using OR, so
   * now combiner must be a value other than zero. This being the case we will
   * always return zero here. Hopefully the compiler will not figure this out.
   */
   return ct_is_zero(combiner);
}

}  // namespace Botan
/*
* Simple config/test file reader
* (C) 2013,2014,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

namespace {

std::string clean_ws(std::string_view s) {
   const char* ws = " \t\n";
   auto start = s.find_first_not_of(ws);
   auto end = s.find_last_not_of(ws);

   if(start == std::string::npos) {
      return "";
   }

   if(end == std::string::npos) {
      return std::string(s.substr(start, end));
   } else {
      return std::string(s.substr(start, start + end + 1));
   }
}

}  // namespace

std::map<std::string, std::string> read_cfg(std::istream& is) {
   std::map<std::string, std::string> kv;
   size_t line = 0;

   while(is.good()) {
      std::string s;

      std::getline(is, s);

      ++line;

      if(s.empty() || s[0] == '#') {
         continue;
      }

      s = clean_ws(s.substr(0, s.find('#')));

      if(s.empty()) {
         continue;
      }

      auto eq = s.find('=');

      if(eq == std::string::npos || eq == 0 || eq == s.size() - 1) {
         throw Decoding_Error("Bad read_cfg input '" + s + "' on line " + std::to_string(line));
      }

      const std::string key = clean_ws(s.substr(0, eq));
      const std::string val = clean_ws(s.substr(eq + 1, std::string::npos));

      kv[key] = val;
   }

   return kv;
}

}  // namespace Botan
/*
* (C) 2018 Ribose Inc
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

std::map<std::string, std::string> read_kv(std::string_view kv) {
   std::map<std::string, std::string> m;
   if(kv.empty()) {
      return m;
   }

   std::vector<std::string> parts;

   try {
      parts = split_on(kv, ',');
   } catch(std::exception&) {
      throw Invalid_Argument("Bad KV spec");
   }

   bool escaped = false;
   bool reading_key = true;
   std::string cur_key;
   std::string cur_val;

   for(const char c : kv) {
      if(c == '\\' && !escaped) {
         escaped = true;
      } else if(c == ',' && !escaped) {
         if(cur_key.empty()) {
            throw Invalid_Argument("Bad KV spec empty key");
         }

         if(m.contains(cur_key)) {
            throw Invalid_Argument("Bad KV spec duplicated key");
         }
         m[cur_key] = cur_val;
         cur_key = "";
         cur_val = "";
         reading_key = true;
      } else if(c == '=' && !escaped) {
         if(!reading_key) {
            throw Invalid_Argument("Bad KV spec unexpected equals sign");
         }
         reading_key = false;
      } else {
         if(reading_key) {
            cur_key += c;
         } else {
            cur_val += c;
         }

         if(escaped) {
            escaped = false;
         }
      }
   }

   if(!cur_key.empty()) {
      if(!reading_key) {
         if(m.contains(cur_key)) {
            throw Invalid_Argument("Bad KV spec duplicated key");
         }
         m[cur_key] = cur_val;
      } else {
         throw Invalid_Argument("Bad KV spec incomplete string");
      }
   }

   return m;
}

}  // namespace Botan
/*
* SCAN Name Abstraction
* (C) 2008-2009,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

namespace {

std::string make_arg(const std::vector<std::pair<size_t, std::string>>& name, size_t start) {
   std::string output = name[start].second;
   size_t level = name[start].first;

   size_t paren_depth = 0;

   for(size_t i = start + 1; i != name.size(); ++i) {
      if(name[i].first <= name[start].first) {
         break;
      }

      if(name[i].first > level) {
         output += "(" + name[i].second;
         ++paren_depth;
      } else if(name[i].first < level) {
         for(size_t j = name[i].first; j < level; j++) {
            output += ")";
            --paren_depth;
         }
         output += "," + name[i].second;
      } else {
         if(output[output.size() - 1] != '(') {
            output += ",";
         }
         output += name[i].second;
      }

      level = name[i].first;
   }

   for(size_t i = 0; i != paren_depth; ++i) {
      output += ")";
   }

   return output;
}

}  // namespace

SCAN_Name::SCAN_Name(const char* algo_spec) : SCAN_Name(std::string(algo_spec)) {}

SCAN_Name::SCAN_Name(std::string_view algo_spec) : m_orig_algo_spec(algo_spec) {
   if(algo_spec.empty()) {
      throw Invalid_Argument("Expected algorithm name, got empty string");
   }

   std::vector<std::pair<size_t, std::string>> name;
   size_t level = 0;
   std::pair<size_t, std::string> accum = std::make_pair(level, "");

   const std::string decoding_error = "Bad SCAN name '" + m_orig_algo_spec + "': ";

   for(const char c : algo_spec) {
      if(c == '/' || c == ',' || c == '(' || c == ')') {
         if(c == '(') {
            ++level;
         } else if(c == ')') {
            if(level == 0) {
               throw Decoding_Error(decoding_error + "Mismatched parens");
            }
            --level;
         }

         if(c == '/' && level > 0) {
            accum.second.push_back(c);
         } else {
            if(!accum.second.empty()) {
               name.push_back(accum);
            }
            accum = std::make_pair(level, "");
         }
      } else {
         accum.second.push_back(c);
      }
   }

   if(!accum.second.empty()) {
      name.push_back(accum);
   }

   if(level != 0) {
      throw Decoding_Error(decoding_error + "Missing close paren");
   }

   if(name.empty()) {
      throw Decoding_Error(decoding_error + "Empty name");
   }

   m_alg_name = name[0].second;

   bool in_modes = false;

   for(size_t i = 1; i != name.size(); ++i) {
      if(name[i].first == 0) {
         m_mode_info.push_back(make_arg(name, i));
         in_modes = true;
      } else if(name[i].first == 1 && !in_modes) {
         m_args.push_back(make_arg(name, i));
      }
   }
}

std::string SCAN_Name::arg(size_t i) const {
   if(i >= arg_count()) {
      throw Invalid_Argument("SCAN_Name::arg " + std::to_string(i) + " out of range for '" + to_string() + "'");
   }
   return m_args[i];
}

std::string SCAN_Name::arg(size_t i, std::string_view def_value) const {
   if(i >= arg_count()) {
      return std::string(def_value);
   }
   return m_args[i];
}

size_t SCAN_Name::arg_as_integer(size_t i, size_t def_value) const {
   if(i >= arg_count()) {
      return def_value;
   }
   return to_u32bit(m_args[i]);
}

size_t SCAN_Name::arg_as_integer(size_t i) const {
   return to_u32bit(arg(i));
}

}  // namespace Botan
/*
* Version Information
* (C) 1999-2013,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



namespace Botan {

const char* short_version_cstr() {
   return BOTAN_SHORT_VERSION_STRING;
}

const char* version_cstr() {
   return BOTAN_FULL_VERSION_STRING;
}

std::string version_string() {
   return std::string(version_cstr());
}

std::string short_version_string() {
   return std::string(short_version_cstr());
}

uint32_t version_datestamp() {
   return BOTAN_VERSION_DATESTAMP;
}

std::optional<std::string> version_vc_revision() {
#if defined(BOTAN_VC_REVISION)
   return std::string(BOTAN_VC_REVISION);
#else
   return std::nullopt;
#endif
}

std::optional<std::string> version_distribution_info() {
#if defined(BOTAN_DISTRIBUTION_INFO_STRING)
   return std::string(BOTAN_DISTRIBUTION_INFO_STRING);
#else
   return std::nullopt;
#endif
}

/*
* Return parts of the version as integers
*/
uint32_t version_major() {
   return BOTAN_VERSION_MAJOR;
}

uint32_t version_minor() {
   return BOTAN_VERSION_MINOR;
}

uint32_t version_patch() {
   return BOTAN_VERSION_PATCH;
}

bool unsafe_for_production_build() {
#if defined(BOTAN_UNSAFE_FUZZER_MODE) || defined(BOTAN_TERMINATE_ON_ASSERTS)
   return true;
#else
   return false;
#endif
}

std::string runtime_version_check(uint32_t major, uint32_t minor, uint32_t patch) {
   if(major != version_major() || minor != version_minor() || patch != version_patch()) {
      return fmt("Warning: linked version ({}) does not match version built against ({}.{}.{})\n",
                 short_version_cstr(),
                 major,
                 minor,
                 patch);
   }

   return "";
}

}  // namespace Botan
/*
* Whirlpool
* (C) 1999-2007,2020,2026 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_CPUID)
#endif

namespace Botan {

namespace {

// Derive the 256-byte S-box from the Whirlpool E and R mini-boxes
consteval std::array<uint8_t, 256> whirlpool_sbox() noexcept {
   constexpr uint8_t Ebox[16] = {1, 11, 9, 12, 13, 6, 15, 3, 14, 8, 7, 4, 10, 2, 5, 0};
   constexpr uint8_t Rbox[16] = {7, 12, 11, 13, 14, 4, 9, 15, 6, 3, 8, 10, 2, 5, 1, 0};

   // Derive the inverse of the E table
   uint8_t Eibox[16] = {};
   for(size_t i = 0; i != 16; ++i) {
      Eibox[Ebox[i]] = static_cast<uint8_t>(i);
   }

   std::array<uint8_t, 256> S = {};
   for(size_t i = 0; i != 256; ++i) {
      const uint8_t L = Ebox[i >> 4];
      const uint8_t R = Eibox[i & 0x0F];
      const uint8_t T = Rbox[L ^ R];
      S[i] = static_cast<uint8_t>((Ebox[L ^ T] << 4) | Eibox[R ^ T]);
   }
   return S;
}

// Combined S-box + MDS diffusion table
consteval std::array<uint64_t, 256> whirlpool_T_table(const std::array<uint8_t, 256>& S) noexcept {
   // MDS circulant matrix first row: [1, 1, 4, 1, 8, 5, 2, 9] over GF(2^8)
   constexpr uint64_t MDS = 0x0101040108050209;

   std::array<uint64_t, 256> T = {};
   for(size_t i = 0; i != 256; ++i) {
      T[i] = poly_mul<0x1D>(MDS, S[i]);
   }
   return T;
}

// Round constants are from the first 64 elements of the sbox
consteval std::array<uint64_t, 10> whirlpool_rc(const std::array<uint8_t, 256>& S) noexcept {
   std::array<uint64_t, 10> RC = {};
   for(size_t r = 0; r != 10; ++r) {
      RC[r] = load_be<uint64_t>(S.data(), r);
   }
   return RC;
}

constexpr auto WHIRL_S = whirlpool_sbox();
alignas(256) constexpr auto WHIRL_T = whirlpool_T_table(WHIRL_S);
constexpr auto WHIRL_RC = whirlpool_rc(WHIRL_S);

uint64_t whirl(uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4, uint64_t x5, uint64_t x6, uint64_t x7) {
   const uint64_t s0 = WHIRL_T[get_byte<0>(x0)];
   const uint64_t s1 = WHIRL_T[get_byte<1>(x1)];
   const uint64_t s2 = WHIRL_T[get_byte<2>(x2)];
   const uint64_t s3 = WHIRL_T[get_byte<3>(x3)];
   const uint64_t s4 = WHIRL_T[get_byte<4>(x4)];
   const uint64_t s5 = WHIRL_T[get_byte<5>(x5)];
   const uint64_t s6 = WHIRL_T[get_byte<6>(x6)];
   const uint64_t s7 = WHIRL_T[get_byte<7>(x7)];

   return s0 ^ rotr<8>(s1) ^ rotr<16>(s2) ^ rotr<24>(s3) ^ rotr<32>(s4) ^ rotr<40>(s5) ^ rotr<48>(s6) ^ rotr<56>(s7);
}

}  // namespace

std::string Whirlpool::provider() const {
#if defined(BOTAN_HAS_WHIRLPOOL_AVX512)
   if(auto feat = CPUID::check(CPUID::Feature::AVX512)) {
      return *feat;
   }
#endif

#if defined(BOTAN_HAS_WHIRLPOOL_AVX2)
   if(auto feat = CPUID::check(CPUID::Feature::AVX2)) {
      return *feat;
   }
#endif

   return "base";
}

/*
* Whirlpool Compression Function
*/
void Whirlpool::compress_n(digest_type& digest, std::span<const uint8_t> input, size_t blocks) {
#if defined(BOTAN_HAS_WHIRLPOOL_AVX512)
   if(CPUID::has(CPUID::Feature::AVX512)) {
      return compress_n_avx512(digest, input, blocks);
   }
#endif

#if defined(BOTAN_HAS_WHIRLPOOL_AVX2)
   if(CPUID::has(CPUID::Feature::AVX2)) {
      return compress_n_avx2(digest, input, blocks);
   }
#endif

   BufferSlicer in(input);

   for(size_t i = 0; i != blocks; ++i) {
      const auto block = in.take(block_bytes);

      uint64_t K[11 * 8] = {0};

      K[0] = digest[0];
      K[1] = digest[1];
      K[2] = digest[2];
      K[3] = digest[3];
      K[4] = digest[4];
      K[5] = digest[5];
      K[6] = digest[6];
      K[7] = digest[7];

      // Whirlpool key schedule:
      for(size_t r = 1; r != 11; ++r) {
         const uint64_t PK0 = K[8 * (r - 1) + 0];
         const uint64_t PK1 = K[8 * (r - 1) + 1];
         const uint64_t PK2 = K[8 * (r - 1) + 2];
         const uint64_t PK3 = K[8 * (r - 1) + 3];
         const uint64_t PK4 = K[8 * (r - 1) + 4];
         const uint64_t PK5 = K[8 * (r - 1) + 5];
         const uint64_t PK6 = K[8 * (r - 1) + 6];
         const uint64_t PK7 = K[8 * (r - 1) + 7];

         K[8 * r + 0] = whirl(PK0, PK7, PK6, PK5, PK4, PK3, PK2, PK1) ^ WHIRL_RC[r - 1];
         K[8 * r + 1] = whirl(PK1, PK0, PK7, PK6, PK5, PK4, PK3, PK2);
         K[8 * r + 2] = whirl(PK2, PK1, PK0, PK7, PK6, PK5, PK4, PK3);
         K[8 * r + 3] = whirl(PK3, PK2, PK1, PK0, PK7, PK6, PK5, PK4);
         K[8 * r + 4] = whirl(PK4, PK3, PK2, PK1, PK0, PK7, PK6, PK5);
         K[8 * r + 5] = whirl(PK5, PK4, PK3, PK2, PK1, PK0, PK7, PK6);
         K[8 * r + 6] = whirl(PK6, PK5, PK4, PK3, PK2, PK1, PK0, PK7);
         K[8 * r + 7] = whirl(PK7, PK6, PK5, PK4, PK3, PK2, PK1, PK0);
      }

      uint64_t M[8] = {0};
      load_be(M, block.data(), 8);

      // First round (key masking)
      uint64_t B0 = M[0] ^ K[0];
      uint64_t B1 = M[1] ^ K[1];
      uint64_t B2 = M[2] ^ K[2];
      uint64_t B3 = M[3] ^ K[3];
      uint64_t B4 = M[4] ^ K[4];
      uint64_t B5 = M[5] ^ K[5];
      uint64_t B6 = M[6] ^ K[6];
      uint64_t B7 = M[7] ^ K[7];

      for(size_t r = 1; r != 11; ++r) {
         const uint64_t T0 = whirl(B0, B7, B6, B5, B4, B3, B2, B1) ^ K[8 * r + 0];
         const uint64_t T1 = whirl(B1, B0, B7, B6, B5, B4, B3, B2) ^ K[8 * r + 1];
         const uint64_t T2 = whirl(B2, B1, B0, B7, B6, B5, B4, B3) ^ K[8 * r + 2];
         const uint64_t T3 = whirl(B3, B2, B1, B0, B7, B6, B5, B4) ^ K[8 * r + 3];
         const uint64_t T4 = whirl(B4, B3, B2, B1, B0, B7, B6, B5) ^ K[8 * r + 4];
         const uint64_t T5 = whirl(B5, B4, B3, B2, B1, B0, B7, B6) ^ K[8 * r + 5];
         const uint64_t T6 = whirl(B6, B5, B4, B3, B2, B1, B0, B7) ^ K[8 * r + 6];
         const uint64_t T7 = whirl(B7, B6, B5, B4, B3, B2, B1, B0) ^ K[8 * r + 7];

         B0 = T0;
         B1 = T1;
         B2 = T2;
         B3 = T3;
         B4 = T4;
         B5 = T5;
         B6 = T6;
         B7 = T7;
      }

      digest[0] ^= B0 ^ M[0];
      digest[1] ^= B1 ^ M[1];
      digest[2] ^= B2 ^ M[2];
      digest[3] ^= B3 ^ M[3];
      digest[4] ^= B4 ^ M[4];
      digest[5] ^= B5 ^ M[5];
      digest[6] ^= B6 ^ M[6];
      digest[7] ^= B7 ^ M[7];
   }
}

void Whirlpool::init(digest_type& digest) {
   digest.resize(8);
   zeroise(digest);
}

std::unique_ptr<HashFunction> Whirlpool::new_object() const {
   return std::make_unique<Whirlpool>();
}

std::unique_ptr<HashFunction> Whirlpool::copy_state() const {
   return std::make_unique<Whirlpool>(*this);
}

void Whirlpool::add_data(std::span<const uint8_t> input) {
   m_md.update(input);
}

void Whirlpool::final_result(std::span<uint8_t> output) {
   m_md.final(output);
}

}  // namespace Botan
/*
* XTS Mode
* (C) 2009,2013,2026 Jack Lloyd
* (C) 2016 Daniel Neus, Rohde & Schwarz Cybersecurity
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_HAS_MODE_XTS_AVX512_CLMUL)
#endif

namespace Botan {

XTS_Mode::XTS_Mode(std::unique_ptr<BlockCipher> cipher) :
      m_cipher(std::move(cipher)),
      m_cipher_block_size(m_cipher->block_size()),
      m_cipher_parallelism(m_cipher->parallel_bytes()),
      m_tweak_blocks(m_cipher_parallelism / m_cipher_block_size) {
   if(!poly_double_supported_size(m_cipher_block_size)) {
      throw Invalid_Argument(fmt("Cannot use {} with XTS", m_cipher->name()));
   }

   m_tweak_cipher = m_cipher->new_object();
}

void XTS_Mode::clear() {
   m_cipher->clear();
   m_tweak_cipher->clear();
   reset();
}

size_t XTS_Mode::update_granularity() const {
   return m_cipher_block_size;
}

size_t XTS_Mode::ideal_granularity() const {
   return m_cipher_parallelism;
}

void XTS_Mode::reset() {
   m_tweak.clear();
}

std::string XTS_Mode::name() const {
   return cipher().name() + "/XTS";
}

size_t XTS_Mode::minimum_final_size() const {
   return cipher_block_size();
}

Key_Length_Specification XTS_Mode::key_spec() const {
   return cipher().key_spec().multiple(2);
}

size_t XTS_Mode::default_nonce_length() const {
   return cipher_block_size();
}

bool XTS_Mode::valid_nonce_length(size_t n) const {
   return n <= cipher_block_size();
}

bool XTS_Mode::has_keying_material() const {
   return m_cipher->has_keying_material() && m_tweak_cipher->has_keying_material();
}

void XTS_Mode::key_schedule(std::span<const uint8_t> key) {
   const size_t key_half = key.size() / 2;

   if(key.size() % 2 == 1 || !m_cipher->valid_keylength(key_half)) {
      throw Invalid_Key_Length(name(), key.size());
   }

   m_cipher->set_key(key.first(key_half));
   m_tweak_cipher->set_key(key.last(key_half));
}

void XTS_Mode::start_msg(const uint8_t nonce[], size_t nonce_len) {
   if(!valid_nonce_length(nonce_len)) {
      throw Invalid_IV_Length(name(), nonce_len);
   }

   m_tweak.resize(m_cipher_parallelism);
   clear_mem(m_tweak.data(), m_tweak.size());
   copy_mem(m_tweak.data(), nonce, nonce_len);
   m_tweak_cipher->encrypt(m_tweak.data());

   // Just repeated doubling from first, remaining contents are junk...
   xts_compute_tweak_block(m_tweak.data(), m_tweak_cipher->block_size(), tweak_blocks());
}

//static
void XTS_Mode::update_tweak_block(uint8_t tweak[], size_t BS, size_t blocks_in_tweak) {
#if defined(BOTAN_HAS_MODE_XTS_AVX512_CLMUL)
   if(BS == 16 && blocks_in_tweak % 8 == 0 && CPUID::has(CPUID::Feature::AVX512_CLMUL)) {
      return update_tweak_block_avx512_clmul(tweak, BS, blocks_in_tweak);
   }
#endif

   /*
   * If we don't have a fast method available, just set the first tweak block to
   * the doubling of the last tweak block, and recompute all the rest via
   * successive doublings.
   */
   poly_double_n_le(tweak, &tweak[(blocks_in_tweak - 1) * BS], BS);
   xts_compute_tweak_block(tweak, BS, blocks_in_tweak);
}

void XTS_Mode::update_tweak(size_t consumed) {
   const size_t BS = m_tweak_cipher->block_size();
   const size_t blocks_in_tweak = tweak_blocks();

   BOTAN_ASSERT_NOMSG(consumed > 0 && consumed <= blocks_in_tweak);

   if(consumed == blocks_in_tweak) {
      // Update all in parallel
      update_tweak_block(m_tweak.data(), BS, blocks_in_tweak);
   } else {
      /*
      The last remaining tweaks can just be shifted over

      This could be a lot better though! We can copy all of the remaining tweaks
      and just recompute the last few
      */
      copy_mem(m_tweak.data(), &m_tweak[(consumed * BS)], BS);
      xts_compute_tweak_block(m_tweak.data(), BS, blocks_in_tweak);
   }
}

size_t XTS_Encryption::output_length(size_t input_length) const {
   return input_length;
}

size_t XTS_Encryption::process_msg(uint8_t buf[], size_t sz) {
   BOTAN_STATE_CHECK(tweak_set());
   const size_t BS = cipher_block_size();

   BOTAN_ARG_CHECK(sz % BS == 0, "Input is not full blocks");
   size_t blocks = sz / BS;

   const size_t blocks_in_tweak = tweak_blocks();

   while(blocks > 0) {
      const size_t to_proc = std::min(blocks, blocks_in_tweak);
      const size_t proc_bytes = to_proc * BS;

      xor_buf(buf, tweak(), proc_bytes);
      cipher().encrypt_n(buf, buf, to_proc);
      xor_buf(buf, tweak(), proc_bytes);

      buf += proc_bytes;
      blocks -= to_proc;

      update_tweak(to_proc);
   }

   return sz;
}

void XTS_Encryption::finish_msg(secure_vector<uint8_t>& buffer, size_t offset) {
   BOTAN_ARG_CHECK(buffer.size() >= offset, "Offset is out of range");
   const size_t sz = buffer.size() - offset;
   uint8_t* buf = buffer.data() + offset;

   BOTAN_ARG_CHECK(sz >= minimum_final_size(), "missing sufficient final input in XTS encrypt");

   const size_t BS = cipher_block_size();

   if(sz % BS == 0) {
      update(buffer, offset);
   } else {
      // steal ciphertext
      const size_t full_blocks = ((sz / BS) - 1) * BS;
      const size_t final_bytes = sz - full_blocks;
      BOTAN_ASSERT(final_bytes > BS && final_bytes < 2 * BS, "Left over size in expected range");

      secure_vector<uint8_t> last(buf + full_blocks, buf + full_blocks + final_bytes);
      buffer.resize(full_blocks + offset);
      update(buffer, offset);

      xor_buf(last, tweak(), BS);
      cipher().encrypt(last);
      xor_buf(last, tweak(), BS);

      for(size_t i = 0; i != final_bytes - BS; ++i) {
         last[i] ^= last[i + BS];
         last[i + BS] ^= last[i];
         last[i] ^= last[i + BS];
      }

      xor_buf(last, tweak() + BS, BS);
      cipher().encrypt(last);
      xor_buf(last, tweak() + BS, BS);

      buffer += last;
   }
}

size_t XTS_Decryption::output_length(size_t input_length) const {
   return input_length;
}

size_t XTS_Decryption::process_msg(uint8_t buf[], size_t sz) {
   BOTAN_STATE_CHECK(tweak_set());
   const size_t BS = cipher_block_size();

   BOTAN_ARG_CHECK(sz % BS == 0, "Input is not full blocks");
   size_t blocks = sz / BS;

   const size_t blocks_in_tweak = tweak_blocks();

   while(blocks > 0) {
      const size_t to_proc = std::min(blocks, blocks_in_tweak);
      const size_t proc_bytes = to_proc * BS;

      xor_buf(buf, tweak(), proc_bytes);
      cipher().decrypt_n(buf, buf, to_proc);
      xor_buf(buf, tweak(), proc_bytes);

      buf += proc_bytes;
      blocks -= to_proc;

      update_tweak(to_proc);
   }

   return sz;
}

void XTS_Decryption::finish_msg(secure_vector<uint8_t>& buffer, size_t offset) {
   BOTAN_ARG_CHECK(buffer.size() >= offset, "Offset is out of range");
   const size_t sz = buffer.size() - offset;
   uint8_t* buf = buffer.data() + offset;

   BOTAN_ARG_CHECK(sz >= minimum_final_size(), "missing sufficient final input in XTS decrypt");

   const size_t BS = cipher_block_size();

   if(sz % BS == 0) {
      update(buffer, offset);
   } else {
      // steal ciphertext
      const size_t full_blocks = ((sz / BS) - 1) * BS;
      const size_t final_bytes = sz - full_blocks;
      BOTAN_ASSERT(final_bytes > BS && final_bytes < 2 * BS, "Left over size in expected range");

      secure_vector<uint8_t> last(buf + full_blocks, buf + full_blocks + final_bytes);
      buffer.resize(full_blocks + offset);
      update(buffer, offset);

      xor_buf(last, tweak() + BS, BS);
      cipher().decrypt(last);
      xor_buf(last, tweak() + BS, BS);

      for(size_t i = 0; i != final_bytes - BS; ++i) {
         last[i] ^= last[i + BS];
         last[i + BS] ^= last[i];
         last[i] ^= last[i + BS];
      }

      xor_buf(last, tweak(), BS);
      cipher().decrypt(last);
      xor_buf(last, tweak(), BS);

      buffer += last;
   }
}

}  // namespace Botan
