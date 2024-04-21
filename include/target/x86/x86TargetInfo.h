#pragma once

#include "mc/MCTargetDesc.h"
#include "tir/Context.h"
#include "utils/EnumMacros.h"

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
#define x86MCInstList(F) F(ADD) F(SUB) F(AND) F(XOR) F(OR)
DECLARE_ENUM(x86MCInst, x86MCInstList)

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
   static constexpr int MaxPatternsPerDef = 2;

   /// @brief Gets the name of the pattern
   static constexpr std::string_view GetPatternName(InstType ty) {
      return x86MCInst_to_string(ty, "??");
   }

   /// @brief Gets the number of MC register classes
   int numMCRegClasses() const override { return 0; }
   /// @brief Gets the total number of distinct MC registers
   int numMCRegisters() const override { return 0; }
   /// @brief Gets the MC register description for the i-th MC register
   /// where i is in the range [0, numMCRegisters())
   mc::MCRegDesc getMCRegDesc(int i) const override { return mc::MCRegDesc{i, 0}; }
   /// @brief Returns the MCPattern class for DAG pattern matching
   const mc::MCPatterns& getMCPatterns() const override;

private:
   DECLARE_STRING_TABLE(x86MCInst, x86MCInstStringTable, x86MCInstList)
};

void Initializex86Patterns();

} // namespace target::x86
