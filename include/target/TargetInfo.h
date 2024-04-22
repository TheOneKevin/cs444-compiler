#pragma once

#include "utils/Assert.h" // IWYU pragma: keep

namespace target {

enum class ArchType;

/**
 * @brief Abstract class that describes the target information
 */
class TargetInfo {
public:
   /// @brief Returns the size of the stack alignment in bytes
   virtual int getStackAlignment() const = 0;
   /// @brief Returns the size of the pointer in bits
   virtual int getPointerSizeInBits() const = 0;
   /**
    * @brief
    *
    * @tparam ArchType
    * @return TargetDesc&
    */
   template <ArchType>
   static TargetInfo& Get();
};

} // namespace target
