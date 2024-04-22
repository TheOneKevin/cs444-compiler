#pragma once

#include "mc/MCTargetDesc.h"
#include "tir/Context.h"
#include "utils/EnumMacros.h"

namespace target::x86 {

// x86 IR target info
class X86TargetInfo final : public tir::TargetInfo {
public:
   /// @brief Returns the size of the stack alignment in bytes
   int getStackAlignment() const override { return 8; }
   /// @brief Returns the size of the pointer in bits
   int getPointerSizeInBits() const override { return 64; }
};

// x86 MC patterns
#define x86MCInstList(F) F(ADD) F(SUB) F(AND) F(XOR) F(OR) F(MOV)
DECLARE_ENUM(x86MCInst, x86MCInstList)

// x86 MC pattern fragments
#define x86MCFragList(F) F(M32Frag) F(M64Frag)
DECLARE_ENUM(x86MCFrag, x86MCFragList)

// x86 Register Classes
#define x86RegClassList(F) F(GPR32) F(GPR64)
DECLARE_ENUM(x86RegClass, x86RegClassList)

// x86 Registers (all of them)
// clang-format off
#define x86RegList(F) \
   F(EAX) F(RAX)      \
   F(EBX) F(RBX)      \
   F(ECX) F(RCX)      \
   F(EDX) F(RDX)      \
   F(RSI) F(ESI)      \
   F(RDI) F(EDI)      \
   F(R8)  F(R8d)      \
   F(R9)  F(R9d)      \
   F(R10) F(R10d)     \
   F(R11) F(R11d)     \
   F(R12) F(R12d)     \
   F(R13) F(R13d)     \
   F(R14) F(R14d)     \
   F(R15) F(R15d)     \
// clang-format on
DECLARE_ENUM(x86Reg, x86RegList)

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

   /// @brief Initializes the target description
   void initialize() override;
   /// @brief Gets the number of MC register classes
   int numMCRegClasses() const override { return 0; }
   /// @brief Gets the total number of distinct MC registers
   int numMCRegisters() const override { return 0; }
   /// @brief Returns the MCPattern class for DAG pattern matching
   const mc::MCPatterns& getMCPatterns() const override;
   /// @brief Checks if the x86 register class can be assigned to the MIR type
   bool isRegisterClass(unsigned classIdx, mc::InstSelectNode::Type type) const override;

private:
   DECLARE_STRING_TABLE(x86MCInst, x86MCInstStringTable, x86MCInstList)
};

} // namespace target::x86

#undef x86MCInstList
#undef x86MCFragList
#undef x86RegClassList
#undef x86RegList
