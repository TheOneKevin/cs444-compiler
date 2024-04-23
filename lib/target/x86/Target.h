#pragma once

#include <string_view>

#include "target/Target.h"
#include "utils/EnumMacros.h"

namespace target::x86 {

// x86 IR target info
class x86TargetInfo final : public target::TargetInfo {
public:
   /// @brief Returns the size of the stack alignment in bytes
   int getStackAlignment() const override { return 8; }
   /// @brief Returns the size of the pointer in bits
   int getPointerSizeInBits() const override { return 64; }
};

// clang-format off
// x86 MC patterns
#define x86MCInstList(F)          \
   F(ADD_RM) F(ADD_MR) F(ADD_MI)  \
   F(SUB_RM) F(SUB_MR) F(SUB_MI)  \
   F(AND_RM) F(AND_MR) F(AND_MI)  \
   F(XOR_RM) F(XOR_MR) F(XOR_MI)  \
   F( OR_RM) F( OR_MR) F( OR_MI)  \
   F(MOV_RM) F(MOV_MR) F(MOV_MI)
DECLARE_ENUM(x86MCInst, x86MCInstList)
// clang-format on

// x86 MC pattern fragments
#define x86MCFragList(F) F(MemFrag)
DECLARE_ENUM(x86MCFrag, x86MCFragList)

// x86 Register Classes
#define x86RegClassList(F) F(GPR8) F(GPR16) F(GPR32) F(GPR64)
DECLARE_ENUM(x86RegClass, x86RegClassList)

// x86 Registers (all of them)
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

// MC target description
class x86TargetDesc final : public target::TargetDesc {
public:
   using InstType = x86MCInst;
   using FragType = x86MCFrag;
   using RegClass = x86RegClass;
   static constexpr int MaxStates = 40;
   static constexpr int MaxOperands = 5;
   static constexpr int MaxPatternsPerDef = 2;

   /// @brief Gets the name of the pattern
   static constexpr std::string_view GetPatternName(InstType ty) {
      return x86MCInst_to_string(ty, "??");
   }

   /// @brief Gets the name of the fragment
   static constexpr std::string_view GetFragmentName(FragType ty) {
      return x86MCFrag_to_string(ty, "??");
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
   bool isRegisterClass(unsigned classIdx,
                        mc::InstSelectNode::Type type) const override;

private:
   DECLARE_STRING_TABLE(x86MCInst, x86MCInstStringTable, x86MCInstList)
   DECLARE_STRING_TABLE(x86MCFrag, x86MCFragStringTable, x86MCFragList)
};

bool MatchMemoryPatternFragment(mc::MatchOptions&, mc::InstSelectNode*&);

} // namespace target::x86

#undef x86MCInstList
#undef x86MCFragList
#undef x86RegClassList
#undef x86RegList
