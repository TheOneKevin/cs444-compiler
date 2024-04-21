#include <unordered_map>

#include "mc/InstSelectNode.h"
#include "mc/MCPatterns.h"
#include "target/x86/x86TargetInfo.h"

namespace target::x86 {

// Aliases
using R = x86RegClass;
using I = x86MCInst;
using F = x86MCFrag;
using N = mc::NodeKind;

std::unordered_map<int, std::vector<mc::MCPatternDef const*>> PatternMap;

// MC target instruction patterns
class x86Patterns final : public mc::MCPatternsImpl<x86MCTargetDesc> {
private:
   // Adds the x86 RM encoding for an inst -> node pair
   static consteval auto AddRMEncoding(I inst, N node, bool commutes) {
      // clang-format off
      return std::make_tuple(
         // r32, r32
         define{inst}
            << inputs{reg(R::GPR32), reg(R::GPR32)}
            << outputs{reg(R::GPR32)}
            << pattern{node, 0, 1}
            << (commutes ? pattern{node, 1, 0} : pattern{}),
         // r32, m32
         define{inst}
            << inputs{reg(R::GPR32), frag(F::M32Frag)}
            << outputs{reg(R::GPR32)}
            << pattern{node, 0, {N::LOAD, 1}}
            << (commutes ? pattern{node, {N::LOAD, 1}, 0} : pattern{}),
         // r64, r64
         define{inst}
            << inputs{reg(R::GPR64), reg(R::GPR64)}
            << outputs{reg(R::GPR64)}
            << pattern{node, 0, 1}
            << (commutes ? pattern{node, 1, 0} : pattern{}),
         // r64, m64
         define{inst}
            << inputs{reg(R::GPR64), frag(F::M64Frag)}
            << outputs{reg(R::GPR64)}
            << pattern{node, 0, {N::LOAD, 1}}
            << (commutes ? pattern{node, {N::LOAD, 1}, 0} : pattern{})
      );
      // clang-format on
   }

   // Adds the x86 MI encoding for an inst -> node pair
   static consteval auto AddMIEncoding(I inst, N node, bool commutes) {
      // clang-format off
      return std::make_tuple(
         // r32, imm32
         define{inst}
            << inputs{reg(R::GPR32), imm(32)}
            << outputs{reg(R::GPR32)}
            << pattern{node, 0, 1}
            << (commutes ? pattern{node, 1, 0} : pattern{}),
         // m32, imm32
         define{inst}
            << inputs{frag(F::M32Frag), imm(32)}
            << outputs{/* Nothing, as it's a store */}
            << pattern{N::STORE, 0, {node, {N::LOAD, 0}, 1}}
            << (commutes ? pattern{N::STORE, 0, {node, 1, {N::LOAD, 0}}} : pattern{}),
         // r64, imm32
         define{inst}
            << inputs{reg(R::GPR64), imm(64)}
            << outputs{reg(R::GPR64)}
            << pattern{node, 0, 1}
            << (commutes ? pattern{node, 1, 0} : pattern{}),
         // m64, imm32
         define{inst}
            << inputs{frag(F::M64Frag), imm(64)}
            << outputs{/* Nothing, as it's a store */}
            << pattern{N::STORE, 0, {node, {N::LOAD, 0}, 1}}
            << (commutes ? pattern{N::STORE, 0, {node, 1, {N::LOAD, 0}}} : pattern{})
      );
      // clang-format on
   }

   // Adds the x86 MR encoding for an inst -> node pair
   static consteval auto AddMREncoding(I inst, N node, bool commutes) {
      // clang-format off
      return std::make_tuple(
         // m32, r32
         define{inst}
            << inputs{frag(F::M32Frag), reg(R::GPR32)}
            << outputs{/* Nothing, as it's a store */}
            << pattern{N::STORE, 0, {node, {N::LOAD, 0}, 1}}
            << (commutes ? pattern{N::STORE, 0, {node, 1, {N::LOAD, 0}}} : pattern{}),
         // m64, r64
         define{inst}
            << inputs{frag(F::M64Frag), reg(R::GPR64)}
            << outputs{/* Nothing, as it's a store */}
            << pattern{N::STORE, 0, {node, {N::LOAD, 0}, 1}}
            << (commutes ? pattern{N::STORE, 0, {node, 1, {N::LOAD, 0}}} : pattern{})
      );
      // clang-format on
   }

   // Adds the RM, MI, MR encoding for a scalar instruction
   static consteval auto AddScalarInst(I inst, N node) {
      return std::tuple_cat(AddRMEncoding(inst, node, true),
                            AddMIEncoding(inst, node, true),
                            AddMREncoding(inst, node, true));
   }

   // Adds all basic scalar instructions
   static consteval auto AddAllScalarInsts() {
      return std::tuple_cat(AddScalarInst(I::ADD, N::ADD),
                            AddScalarInst(I::SUB, N::SUB),
                            AddScalarInst(I::OR, N::OR),
                            AddScalarInst(I::AND, N::AND),
                            AddScalarInst(I::XOR, N::XOR));
   }

   utils::Generator<mc::MCPatternDef const*> getPatternFor(
         mc::NodeKind kind) const override DISABLE_ASAN {
      for(const auto* def : PatternMap[(int)kind]) {
         co_yield def;
      }
   }

   utils::Generator<mc::MCPatternDef const*> patterns() const override
         DISABLE_ASAN {
      for(const auto& [_, list] : PatternMap) {
         for(const auto* def : list) {
            co_yield def;
         }
      }
   }

public:
   static consteval auto GetPatterns() { return AddAllScalarInsts(); }
};

template <typename tuple_t>
constexpr auto GetArrayFromTuple(tuple_t&& tuple) {
   constexpr auto get_array = [](auto&&... x) {
      return std::array{std::forward<decltype(x)>(x)...};
   };
   return std::apply(get_array, std::forward<tuple_t>(tuple));
}

// Add the patterns to the map
static constexpr auto Patterns = GetArrayFromTuple(x86Patterns::GetPatterns());

void Initializex86Patterns() {
   for(auto& Def : Patterns) {
      PatternMap[(int)Def.getDAGKind()].push_back(&Def);
   }
}

const mc::MCPatterns& x86MCTargetDesc::getMCPatterns() const {
   static constexpr x86Patterns Patterns{};
   return Patterns;
}

} // namespace target::x86
