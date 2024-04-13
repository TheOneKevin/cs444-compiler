#pragma once

#include "tir/Context.h"
namespace target::x86 {

class X86TargetInfo final : public tir::TargetInfo {
public:
   /// @brief Returns the size of the stack alignment in bytes
   int getStackAlignment() const override { return 8; }
   /// @brief Returns the size of the pointer in bits
   int getPointerSizeInBits() const override { return 64; }
};

} // namespace target::x86
