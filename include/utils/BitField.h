#pragma once

#include <cstdint>
#include <type_traits>

namespace utils {

/**
 * @brief Implements platform-independent bitfields.
 *
 * @tparam T     The containing type for the bitfield.
 * @tparam Index The bit offset of the field in the containing type.
 * @tparam Bits  The number of bits this field occupies.
 */
template <typename T, int Index, int Bits>
struct BitField {
private:
   static constexpr int Size = sizeof(T) * 8;
   // Underlying type of T is T2
   // TODO: This needs extra error checking.
   typedef std::conditional_t<
         Size == 0, void,
         std::conditional_t<
               Size <= 8, uint8_t,
               std::conditional_t<
                     Size <= 16, uint16_t,
                     std::conditional_t<
                           Size <= 32, uint32_t,
                           std::conditional_t<Size <= 64, uint64_t, void>>>>>
         T2;
   // Mask of Bits bits starting at Index
   static constexpr T2 Mask = (1u << Bits) - 1u;

public:
   // Constructor for this specific field
   BitField(T value) : value_(0) { operator=(value); }
   // Constructor for single-bit fields, enabled if Bits == 1
   template <int B = Bits, typename = std::enable_if_t<B == 1>>
   BitField(bool value) : BitField(value ? 1 : 0) {}
   // Assignment operator from T to T2
   BitField operator=(T value) {
      value_ = (value_ & ~(Mask << Index)) | (((T2)value & Mask) << Index);
      return *this;
   }
   // Get value as T
   operator T() const { return (T)((value_ >> Index) & Mask); }
   // Get value as bool
   explicit operator bool() const { return value_ & (Mask << Index); }

private:
   // Bitfield underlying value T2
   T2 value_;
};

} // namespace utils
