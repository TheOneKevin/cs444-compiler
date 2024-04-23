#include <algorithm>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "Target.h"
#include "mc/InstSelectNode.h"
#include "mc/MCPatterns.h"
#include "utils/Utils.h"

namespace target::x86 {

// Aliases
using R = x86RegClass;
using I = x86MCInst;
using F = x86MCFrag;
using N = mc::NodeKind;
using PatternGenerator = utils::Generator<mc::MCPatternDef const*>;

/* ===--------------------------------------------------------------------=== */
// x86 patterns definition
/* ===--------------------------------------------------------------------=== */

// MC target instruction patterns
class x86Patterns final : public mc::MCPatternsImpl<x86TargetDesc> {
private:
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
   // Adds the x86 RM encoding for an inst -> node pair
   static consteval auto AddRMEncoding(I inst, N node, int bits, bool commutes) {
      R r = GetRegClass(bits);
      // clang-format off
      return std::make_tuple(
         // r8/16/32/64, r8/16/32/64
         define{inst}
            << inputs{reg(r), reg(r)}
            << outputs{reg(r)}
            << pattern{node, 0, 1}
            << (commutes ? pattern{node, 1, 0} : pattern{}),
         // r8/16/32/64, m*
         define{inst}
            << inputs{reg(r), frag(F::MemFrag)}
            << outputs{reg(r)}
            << pattern{node, 0, {N::LOAD, 1}}
            << (commutes ? pattern{node, {N::LOAD, 1}, 0} : pattern{})
      );
      // clang-format on
   }

   // Adds the x86 MI encoding for an inst -> node pair
   static consteval auto AddMIEncoding(I inst, N node, int bits, bool commutes) {
      R r = GetRegClass(bits);
      // clang-format off
      return std::make_tuple(
         // r8/16/32/64, imm8/16/32
         define{inst}
            << inputs{reg(r), imm(bits == 64 ? 32 : bits)}
            << outputs{reg(r)}
            << pattern{node, 0, 1}
            << (commutes ? pattern{node, 1, 0} : pattern{}),
         // m*, imm8/16/32
         define{inst}
            << inputs{frag(F::MemFrag), imm(bits)}
            << outputs{/* Nothing, as it's a store */}
            << pattern{N::STORE, 0, {node, {N::LOAD, 0}, 1}}
            << (commutes ? pattern{N::STORE, 0, {node, 1, {N::LOAD, 0}}} : pattern{})
      );
      // clang-format on
   }

   // Adds the x86 MR encoding for an inst -> node pair
   static consteval auto AddMREncoding(I inst, N node, int bits, bool commutes) {
      R r = GetRegClass(bits);
      // clang-format off
      return std::make_tuple(
         // m*, r8/16/32/64
         define{inst}
            << inputs{frag(F::MemFrag), reg(r)}
            << outputs{/* Nothing, as it's a store */}
            << pattern{N::STORE, 0, {node, {N::LOAD, 0}, 1}}
            << (commutes ? pattern{N::STORE, 0, {node, 1, {N::LOAD, 0}}} : pattern{})
      );
      // clang-format on
   }

   // Adds the RM, MI, MR encoding for a scalar instruction
   static consteval auto AddScalarInst(I rm, I mi, I mr, N node) {
      return std::tuple_cat(AddRMEncoding(rm, node, 8, true),
                            AddRMEncoding(rm, node, 16, true),
                            AddRMEncoding(rm, node, 32, true),
                            AddRMEncoding(rm, node, 64, true),
                            AddMIEncoding(mi, node, 8, true),
                            AddMIEncoding(mi, node, 16, true),
                            AddMIEncoding(mi, node, 32, true),
                            AddMIEncoding(mi, node, 64, true),
                            AddMREncoding(mr, node, 8, true),
                            AddMREncoding(mr, node, 16, true),
                            AddMREncoding(mr, node, 32, true),
                            AddMREncoding(mr, node, 64, true));
   }

   // Adds all basic scalar instructions
   static consteval auto AddAllScalarInsts() {
      return std::tuple_cat(AddScalarInst(I::ADD_RM, I::ADD_MR, I::ADD_MI, N::ADD),
                            AddScalarInst(I::SUB_RM, I::SUB_MR, I::SUB_MI, N::SUB),
                            AddScalarInst(I::AND_RM, I::AND_MR, I::AND_MI, N::AND),
                            AddScalarInst(I::XOR_RM, I::XOR_MR, I::XOR_MI, N::XOR),
                            AddScalarInst(I::OR_RM, I::OR_MR, I::OR_MI, N::OR));
   }

   // Adds the load/store instructions
   static consteval auto AddLoadStoreInsts() {
      // clang-format off
      return std::make_tuple(
         // Load r32
         define{I::MOV_MR}
            << inputs{frag(F::MemFrag)}
            << outputs{reg(R::GPR32)}
            << pattern{N::LOAD, 0},
         // Load r64
         define{I::MOV_MR}
            << inputs{frag(F::MemFrag)}
            << outputs{reg(R::GPR64)}
            << pattern{N::LOAD, 0},
         // Store r32
         define{I::MOV_RM}
            << inputs{frag(F::MemFrag), reg(R::GPR32)}
            << outputs{}
            << pattern{N::STORE, 1, 0},
         // Store r64
         define{I::MOV_RM}
            << inputs{frag(F::MemFrag), reg(R::GPR64)}
            << outputs{}
            << pattern{N::STORE, 1, 0}
      );
      // clang-format on
   }

public:
   static consteval auto GetPatterns() {
      return std::tuple_cat(AddAllScalarInsts(), AddLoadStoreInsts());
      // return AddLoadStoreInsts();
   }

   static consteval auto GetFragments() {
      return std::make_tuple(fragment{F::MemFrag} << inputs{
                                   reg(R::GPR64) /* Base */,
                                   reg(R::GPR64) /* Index */,
                                   imm(8) /* Scale */,
                                   imm(32) /* Disp */
                             });
   }

public:
   PatternGenerator getPatternFor(mc::NodeKind kind) const override;
   PatternGenerator patterns() const override;
   mc::MCPatternFragment const& getFragment(unsigned kind) const override;
   bool matchFragment(mc::MCPatternFragment const& frag, mc::MatchOptions& mo,
                      mc::InstSelectNode*& out) const override;
};

/* ===--------------------------------------------------------------------=== */
// Class member functions
/* ===--------------------------------------------------------------------=== */

// Map of mc::NodeKind -> list of mc::MCPatternDef
static std::unordered_map<int, std::vector<mc::MCPatternDef const*>> PatternMap_;

// Map of mc::MCPatternFragment kind -> mc::MCPatternFragment
static std::unordered_map<int, mc::MCPatternFragment const*> FragmentMap_;

// Private std::array of patterns (must be global for ASAN to be happy)
static constexpr auto PatternsArray_ =
      utils::array_from_tuple(x86Patterns::GetPatterns());

// Private std::array of fragments (must be global for ASAN to be happy)
static constexpr auto FragmentsArray_ =
      utils::array_from_tuple(x86Patterns::GetFragments());

// Initialize the private global patterns class
static constexpr x86Patterns Patterns_{};

// Iterate through the patterns for the given node kind
PatternGenerator x86Patterns::getPatternFor(mc::NodeKind kind) const {
   for(const auto* def : PatternMap_[(int)kind]) co_yield def;
}

// Iterate through all the patterns
PatternGenerator x86Patterns::patterns() const {
   for(const auto& [_, list] : PatternMap_)
      for(const auto* def : list) co_yield def;
}

// Get the fragment for the given kind
mc::MCPatternFragment const& x86Patterns::getFragment(unsigned kind) const {
   return *FragmentMap_.at(kind);
}

// Match the fragment
bool x86Patterns::matchFragment(mc::MCPatternFragment const& frag,
                                mc::MatchOptions& mo,
                                mc::InstSelectNode*& out) const {
   switch(static_cast<x86MCFrag>(frag.kind())) {
      case x86MCFrag::MemFrag:
         return MatchMemoryPatternFragment(mo, out);
      case x86MCFrag::LAST_MEMBER:
         assert(false);
         std::unreachable();
   }
}

void x86TargetDesc::initialize() {
   // Add the patterns to the map
   for(auto& Def : PatternsArray_) {
      PatternMap_[(int)Def.getDAGKind()].push_back(&Def);
   }
   // Add the fragments to the map
   for(auto& Frag : FragmentsArray_) {
      FragmentMap_[Frag.kind()] = &Frag;
   }
   // Sort the patterns by its length (decending order)
   for(auto& [_, list] : PatternMap_) {
      std::sort(list.begin(), list.end(), [](auto* a, auto* b) {
         return a->maxTapeLength() > b->maxTapeLength();
      });
   }
}

// Get the patterns for the x86 target description
const mc::MCPatterns& x86TargetDesc::getMCPatterns() const { return Patterns_; }

} // namespace target::x86
