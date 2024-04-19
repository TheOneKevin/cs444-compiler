#include "mc/MCPatterns.h"
#include "target/x86/x86TargetInfo.h"

namespace target::x86 {

// Aliases
using R = x86RegClass;
using I = x86MCInst;
using F = x86MCFrag;
using N = mc::NodeKind;

// MC target instruction patterns
class x86Patterns : mc::MCPatterns<x86MCTargetDesc> {
public:
   static consteval auto AddRMEncoding(I inst, N node) {
      // clang-format off
      return std::make_tuple(
         // r32, r32
         define{inst}
            << inputs{reg(R::GPR32), reg(R::GPR32)}
            << outputs{reg(R::GPR32)}
            << pattern{node, 0, 1},
         // r32, m32
         define{inst}
            << inputs{reg(R::GPR32), frag(F::M32Frag)}
            << outputs{reg(R::GPR32)}
            << pattern{node, 0, {N::LOAD, 1}},
         // r64, r64
         define{inst}
            << inputs{reg(R::GPR64), reg(R::GPR64)}
            << outputs{reg(R::GPR64)}
            << pattern{node, 0, 1},
         // r64, m64
         define{inst}
            << inputs{reg(R::GPR64), frag(F::M64Frag)}
            << outputs{reg(R::GPR64)}
            << pattern{node, 0, {N::LOAD, 1}}
      );
      // clang-format on
   }

   static consteval auto AddMIEncoding(I inst, N node) {
      // clang-format off
      return std::make_tuple(
         // r32, imm32
         define{inst}
            << inputs{reg(R::GPR32), imm(32)}
            << outputs{reg(R::GPR32)}
            << pattern{node, 0, 1},
         // m32, imm32
         define{inst}
            << inputs{frag(F::M32Frag), imm(32)}
            << outputs{/* Nothing, as it's a store */}
            << pattern{N::STORE, 0, {node, {N::LOAD, 0}, 1}},
         // r64, imm32
         define{inst}
            << inputs{reg(R::GPR64), imm(64)}
            << outputs{reg(R::GPR64)}
            << pattern{node, 0, 1},
         // m64, imm32
         define{inst}
            << inputs{frag(F::M64Frag), imm(64)}
            << outputs{/* Nothing, as it's a store */}
            << pattern{N::STORE, 0, {node, {N::LOAD, 0}, 1}}
      );
      // clang-format on
   }

   static consteval auto AddMREncoding(I inst, N node) {
      // clang-format off
      return std::make_tuple(
         // m32, r32
         define{inst}
            << inputs{frag(F::M32Frag), reg(R::GPR32)}
            << outputs{/* Nothing, as it's a store */}
            << pattern{N::STORE, 0, {node, 1}},
         // m64, r64
         define{inst}
            << inputs{frag(F::M64Frag), reg(R::GPR64)}
            << outputs{/* Nothing, as it's a store */}
            << pattern{N::STORE, 0, {node, 1}}
      );
      // clang-format on
   }

   static consteval auto AddScalarInsts() {
      return std::tuple_cat(
         AddRMEncoding(I::ADD, N::ADD),
         AddMIEncoding(I::ADD, N::ADD),
         AddMREncoding(I::ADD, N::ADD)
      );
   }
};

} // namespace target::x86
