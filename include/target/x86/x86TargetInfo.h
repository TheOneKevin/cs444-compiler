#pragma once

#include "mc/MCTargetDesc.h"
#include "tir/Context.h"
namespace target::x86 {

// General target information for code generation
class X86TargetInfo final : public tir::TargetInfo {
public:
   /// @brief Returns the size of the stack alignment in bytes
   int getStackAlignment() const override { return 8; }
   /// @brief Returns the size of the pointer in bits
   int getPointerSizeInBits() const override { return 64; }
};

// MC patterns
enum class x86MCInst { ADD, SUB, LAST_MEMBER };
enum class x86MCFrag { M32Frag, M64Frag, LAST_MEMBER };
enum class x86RegClass { GPR32, GPR64 };

// MC target description
class x86MCTargetDesc final : public mc::MCTargetDesc {
public:
   using InstType = x86MCInst;
   using FragType = x86MCFrag;
   using RegClass = x86RegClass;
   static constexpr int MaxStates = 100;
   static constexpr int MaxOperands = 3;

   /// @brief Gets the number of MC register classes
   int numMCRegClasses() const override { return 0; }
   /// @brief Gets the total number of distinct MC registers
   int numMCRegisters() const override { return 0; }
   /// @brief Gets the MC register description for the i-th MC register
   /// where i is in the range [0, numMCRegisters())
   mc::MCRegDesc getMCRegDesc(int i) const override { return mc::MCRegDesc{i, 0}; }
};

} // namespace target::x86
