#include <utils/Utils.h>

#include <tuple>
#include <utility>

#include "Target.h"
#include "mc/InstSelectNode.h"
#include "mc/Patterns.h"

namespace target::x86 {

// Aliases
using R = x86RegClass;
using I = x86Pattern;
using V = x86Variant;
using F = x86Fragment;
using N = mc::NodeKind;
using PatternGenerator = utils::Generator<mc::PatternDef const*>;

// Get the register for the given register class
static consteval auto GetRegClass(int bits) {
   switch(bits) {
      case 8:
         return R::GPR8;
      case 16:
         return R::GPR16;
      case 32:
         return R::GPR32;
      case 64:
         return R::GPR64;
      default:
         throw "Invalid register size";
   }
}

/* ===--------------------------------------------------------------------=== */
// x86 patterns definition
/* ===--------------------------------------------------------------------=== */

// MC target instruction patterns
class x86PatternBuilder final : public mc::PatternBuilderContext<x86TargetDesc> {
private:
   // Adds all the encoding for a standard scalar instruction
   static consteval auto ScalarInst(I inst, N node, int bits, bool commutes) {
      const auto r = GetRegClass(bits);
      const auto immbits = bits == 64 ? 32 : bits;
      // clang-format off
      return std::make_tuple(
         // r8/16/32/64, r8/16/32/64
         define{inst, V::RR}
            << inputs{reg(r), reg(r)}
            << outputs{reg(r)}
            << pattern{node, 0, 1}
            << (commutes ? pattern{node, 1, 0} : pattern{}),
         // r8/16/32/64, [m*]
         define{inst, V::RM}
            << inputs{reg(r), frag(F::MemFrag)}
            << outputs{reg(r)}
            << pattern{node, 0, {N::LOAD, 1}}
            << (commutes ? pattern{node, {N::LOAD, 1}, 0} : pattern{}),
         // r8/16/32/64, imm8/16/32
         define{inst, V::RI}
            << inputs{reg(r), imm(immbits)}
            << outputs{reg(r)}
            << pattern{node, 0, 1}
            << (commutes ? pattern{node, 1, 0} : pattern{}),
         // [m*], imm8/16/32
         define{inst, V::MI}
            << inputs{frag(F::MemFrag), imm(immbits)}
            << outputs{/* Nothing, as it's a store */}
            << pattern{N::STORE, {node, {N::LOAD, 0}, 1}, 0}
            << (commutes ? pattern{N::STORE, {node, 1, {N::LOAD, 0}}, 0} : pattern{}),
         // [m*], r8/16/32/64
         define{inst, V::MR}
            << inputs{frag(F::MemFrag), reg(r)}
            << outputs{/* Nothing, as it's a store */}
            << pattern{N::STORE, {node, {N::LOAD, 0}, 1}, 0}
            << (commutes ? pattern{N::STORE, {node, 1, {N::LOAD, 0}}, 0} : pattern{})
      );
      // clang-format on
   }

   // Adds the load/store instructions
   static consteval auto LoadStoreInst(int bits) {
      const auto r = GetRegClass(bits);
      const auto immbits = bits == 64 ? 32 : bits;
      // clang-format off
      return std::make_tuple(
         // MOV r8/16/32/64, [m*]
         define{I::MOV, V::RM}
            << inputs{frag(F::MemFrag)}
            << outputs{reg(r)}
            << pattern{N::LOAD, 0},
         // MOV [m*], r8/16/32/64
         define{I::MOV, V::MR}
            << inputs{frag(F::MemFrag), reg(r)}
            << outputs{}
            << pattern{N::STORE, 1, 0},
         // MOV [m*], imm8/16/32
         define{I::MOV, V::MI}
            << inputs{frag(F::MemFrag), imm(immbits)}
            << outputs{}
            << pattern{N::STORE, 1, 0}
      );
      // clang-format on
   }

public:
   static bool ComparePattern(const mc::PatternDef* a, const mc::PatternDef* b) {
      using Op = mc::details::Operand::Type;
      // Place patterns with more inputs first
      if(a->numInputs() != b->numInputs()) return a->numInputs() > b->numInputs();
      // Otherwise, prioritize the patterns with less register inputs
      int regA = 0, regB = 0;
      for(unsigned i = 0; i < a->numInputs(); i++)
         if(a->getInput(i).type == Op::Register) regA++;
      for(unsigned i = 0; i < b->numInputs(); i++)
         if(b->getInput(i).type == Op::Register) regB++;
      return regA < regB;
   }

   static consteval auto GetAllPatterns() {
      // clang-format off
      return utils::map_tuple([](auto bits) {
         return std::tuple_cat(
            ScalarInst(I::ADD, N::ADD, bits, true),
            ScalarInst(I::SUB, N::SUB, bits, false),
            ScalarInst(I::AND, N::AND, bits, true),
            ScalarInst(I::XOR, N::XOR, bits, true),
            ScalarInst(I::OR, N::OR, bits, true),
            LoadStoreInst(bits)
         );
      }, std::make_tuple(8, 16, 32, 64));
      // clang-format on
   }

   static consteval auto GetAllFragments() {
      // clang-format off
      return std::make_tuple(
         fragment{F::MemFrag} << inputs{
              reg(R::GPR64) /* Base */,
              reg(R::GPR64) /* Index */,
              imm(8) /* Scale */,
              imm(32) /* Disp */
         }
      );
      // clang-format on
   }

   static bool MatchFragment(mc::PatternFragment const& Frag, mc::MatchOptions& MO,
                             unsigned OpIdx);
};

/* ===--------------------------------------------------------------------=== */
// Class member functions
/* ===--------------------------------------------------------------------=== */

// Initialize the private global patterns class
static mc::PatternProvider<x86TargetDesc, x86PatternBuilder> Provider_{};

// Dispatch the pattern matching to the correct fragment
bool x86PatternBuilder::MatchFragment(mc::PatternFragment const& frag,
                                       mc::MatchOptions& mo, unsigned idx) {
   switch(static_cast<x86Fragment>(frag.kind())) {
      case x86Fragment::MemFrag:
         return MatchMemoryPatternFragment(mo, idx);
      default:
         assert(false);
         std::unreachable();
   }
}

// Get the patterns for the x86 target description
const mc::PatternProviderBase& x86TargetDesc::patternProvider() const {
   return Provider_;
}

} // namespace target::x86
