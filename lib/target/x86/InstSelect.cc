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
            << pattern{N::STORE, {node, {N::LOAD, 0}, 1}, 0}
            << (commutes ? pattern{N::STORE, {node, 1, {N::LOAD, 0}}, 0} : pattern{})
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
            << pattern{N::STORE, {node, {N::LOAD, 0}, 1}, 0}
            << (commutes ? pattern{N::STORE, {node, 1, {N::LOAD, 0}}, 0} : pattern{})
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
   static consteval auto AddLoadStoreInsts(int bits) {
      auto r = GetRegClass(bits);
      // clang-format off
      return std::make_tuple(
         define{I::MOV_MR}
            << inputs{frag(F::MemFrag)}
            << outputs{reg(r)}
            << pattern{N::LOAD, 0},
         define{I::MOV_RM}
            << inputs{frag(F::MemFrag), reg(r)}
            << outputs{}
            << pattern{N::STORE, 1, 0}
      );
      // clang-format on
   }

public:
   static consteval auto GetPatterns() {
      return std::tuple_cat(AddAllScalarInsts(),
                            AddLoadStoreInsts(8),
                            AddLoadStoreInsts(16),
                            AddLoadStoreInsts(32),
                            AddLoadStoreInsts(64));
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

static bool patternCompare(const mc::MCPatternDef* a, const mc::MCPatternDef* b) {
   using Op = mc::details::MCOperand::Type;
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

using mc::details::MCOperand;
static std::ostream& printOperand(std::ostream& os, MCOperand op) {
   using Op = mc::details::MCOperand::Type;
   switch(op.type) {
      case Op::Immediate:
         os << "Imm(" << op.data << ")";
         break;
      case Op::Register:
         os << "Reg(" << x86TargetDesc::GetRegClassName((x86RegClass)op.data)
            << ")";
         break;
      case Op::Fragment:
         os << "Frag(" << x86TargetDesc::GetFragmentName((x86MCFrag)op.data)
            << ")";
         break;
      default:
         os << "??";
         break;
   }
   return os;
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
   // Sort the patterns in increasing order of priority
   for(auto& [_, list] : PatternMap_) {
      std::sort(list.begin(), list.end(), patternCompare);
   }
}

void x86TargetDesc::dumpPatterns() const { printPatterns(std::cerr); }

std::ostream& x86TargetDesc::printPatterns(std::ostream& os) const {
   for(auto& [frag, list] : PatternMap_) {
      os << "Patterns for "
         << mc::InstSelectNode::NodeKind_to_string((mc::NodeKind)frag, "??")
         << ":\n";
      for(auto* def : list) {
         os << "  " << def->name() << ": ";
         for(unsigned i = 0; i < def->numInputs(); i++)
            printOperand(os, def->getInput(i)) << " ";
         os << "-> ";
         for(unsigned i = 0; i < def->numOutputs(); i++)
            printOperand(os, def->getOutput(i)) << " ";
         os << "\n";
      }
   }
   return os;
}

// Get the patterns for the x86 target description
const mc::MCPatterns& x86TargetDesc::getMCPatterns() const { return Patterns_; }

} // namespace target::x86
