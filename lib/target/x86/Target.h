#pragma once

#include <string_view>

#include "target/Target.h"
#include "utils/EnumMacros.h"

namespace target::x86 {

// Instruction patterns
#define x86PatternList(F) F(ADD) F(SUB) F(XOR) F(AND) F(OR) F(MOV)
DECLARE_ENUM(x86Pattern, x86PatternList)

// Instruction pattern variants
#define x86VariantList(F) F(RI) F(MI) F(MR) F(RR) F(RM)
DECLARE_ENUM(x86Variant, x86VariantList)

// Pattern fragments
#define x86FragmentList(F) F(MemFrag)
DECLARE_ENUM(x86Fragment, x86FragmentList)

// Register classes
#define x86RegClassList(F) F(GPR8) F(GPR16) F(GPR32) F(GPR64)
DECLARE_ENUM(x86RegClass, x86RegClassList)

// Registers (all of them)
// clang-format off
#define x86RegList(F) \
   F(RAX) F(EAX)  F(AX)   F(AL)     \
   F(RBX) F(EBX)  F(BX)   F(BL)     \
   F(RCX) F(ECX)  F(CX)   F(CL)     \
   F(RDX) F(EDX)  F(DX)   F(DL)     \
   F(RSI) F(ESI)  F(SI)   F(SIL)    \
   F(RDI) F(EDI)  F(DI)   F(DIL)    \
   F(R8)  F(R8d)  F(R8w)  F(R8b)    \
   F(R9)  F(R9d)  F(R9w)  F(R9b)    \
   F(R10) F(R10d) F(R10w) F(R10b)   \
   F(R11) F(R11d) F(R11w) F(R11b)   \
   F(R12) F(R12d) F(R12w) F(R12b)   \
   F(R13) F(R13d) F(R13w) F(R13b)   \
   F(R14) F(R14d) F(R14w) F(R14b)   \
   F(R15) F(R15d) F(R15w) F(R15b)
DECLARE_ENUM(x86Reg, x86RegList)
// clang-format on

bool MatchMemoryPatternFragment(mc::MatchOptions&, unsigned);

// x86 IR target info
class x86TargetInfo final : public target::TargetInfo {
public:
   /// @brief Returns the size of the stack alignment in bytes
   int getStackAlignment() const override { return 8; }
   /// @brief Returns the size of the pointer in bits
   int getPointerSizeInBits() const override { return 64; }
};

// MC target description
class x86TargetDesc final : public target::TargetDesc {
public:
   using PatternType = x86Pattern;
   using PatternVariantType = x86Variant;
   using FragmentType = x86Fragment;
   using VariantType = x86Variant;
   using RegClass = x86RegClass;
   static constexpr int MaxStates = 40;
   static constexpr int MaxOperands = 5;
   static constexpr int MaxPatternsPerDef = 2;

   /// @brief Gets the name of the pattern
   static constexpr std::string_view GetPatternName(PatternType ty) {
      return x86Pattern_to_string(ty, "??");
   }

   /// @brief Gets the name of the fragment
   static constexpr std::string_view GetFragmentName(FragmentType ty) {
      return x86Fragment_to_string(ty, "??");
   }

   /// @brief Gets the name of the register class
   static constexpr std::string_view GetRegClassName(RegClass ty) {
      return x86RegClass_to_string(ty, "??");
   }

public:
   /// @brief Returns the MCPattern class for DAG pattern matching
   const mc::PatternProviderBase& patternProvider() const override;

   /// @brief Checks if the x86 register class can be assigned to the MIR type
   bool isRegisterClass(unsigned classIdx,
                        mc::InstSelectNode::Type type) const override;

private:
   DECLARE_STRING_TABLE(x86Pattern, x86MCInstStringTable, x86PatternList)
   DECLARE_STRING_TABLE(x86Fragment, x86MCFragStringTable, x86FragmentList)
   DECLARE_STRING_TABLE(x86RegClass, x86RegClassStringTable, x86RegClassList)
};

} // namespace target::x86

#undef x86PatternList
#undef x86VariantList
#undef x86FragmentList
#undef x86RegClassList
#undef x86RegList
