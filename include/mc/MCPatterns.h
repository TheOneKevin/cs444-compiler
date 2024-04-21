#pragma once

#include <cstdint>
#include <ostream>

#include "mc/InstSelectNode.h"
#include "utils/Generator.h"

/* ===--------------------------------------------------------------------=== */
// Secret implementation details
/* ===--------------------------------------------------------------------=== */

namespace mc::details {

// Defines an MC instruction pattern's operand
struct [[gnu::packed]] MCOperand final {
   enum class Type {
      None,
      Immediate,
      Register,
      Push,
      Pop,
      CheckNodeType,
      CheckOperandType
   };
   Type type : 8;
   uint8_t data0 : 8;
   uint16_t data1 : 16;
   constexpr MCOperand() : type{Type::None}, data0{}, data1{} {}
   constexpr MCOperand(Type type, uint8_t data0, uint16_t data1)
         : type{type}, data0{data0}, data1{data1} {}
   constexpr bool operator==(MCOperand const& other) const {
      return other.type == type && other.data0 == data0 && other.data1 == data1;
   }
};

// Concept to check if T is an MCOperand
template <typename T>
concept IsMCOperand = std::is_same_v<T, MCOperand>;

// Concept to check if T is a TargetDefinition
template <typename T>
concept IsTargetDef = true;

// Abstract base class to erase template parameters
class MCPatternBase {
public:
   virtual utils::Generator<MCOperand const*> bytecode() const = 0;
   virtual ~MCPatternBase() = default;
   // Does this pattern match the given node?
   bool matches(mc::InstSelectNode* node) const;
   // Print the pattern
   std::ostream& print(std::ostream&, int indent = 0) const;
   // Dump the pattern
   void dump() const;
};

// Forward declare the MCPatternDef class
template <typename TD>
   requires IsTargetDef<TD>
class MCPatternDef;

// Defines the rule to match an MC pattern
template <typename TD>
   requires IsTargetDef<TD>
class MCPattern final : public MCPatternBase {
public:
   int N;
   MCOperand tape[TD::MaxStates];

public:
   // Empty pattern (matches nothing)
   constexpr MCPattern() : N{0} {}
   // 1st base case is an ISel node type
   constexpr MCPattern(mc::NodeKind op) : N{1} {
      tape[0] = MCOperand{
            MCOperand::Type::CheckNodeType, 0, static_cast<uint16_t>(op)};
   }
   // 2nd base case is an MCPatternDef operand index
   constexpr MCPattern(uint16_t idx) : N{1} {
      tape[0] = MCOperand{MCOperand::Type::CheckOperandType, 0, idx};
   }
   // Recursively construct the DAG pattern to match
   constexpr MCPattern(std::initializer_list<MCPattern> children) {
      // Populate the tape
      int i = 0;
      for(auto child : children) {
         // Push if child.N > 1
         if(child.N > 1) tape[i++] = MCOperand{MCOperand::Type::Push, 0, 0};
         // Add the tape contents of the children
         for(int j = 0; j < child.N; j++) {
            if(i >= TD::MaxStates)
               throw "MCPattern tape is out of space! Maybe increase MaxStates?";
            tape[i++] = child.tape[j];
         }
         // Pop if child.N > 1
         if(child.N > 1) tape[i++] = MCOperand{MCOperand::Type::Pop, 0, 0};
      }
      // Update the number of elements in our tape
      N = i;
   }
   // Grab the bytecode from the tape
   utils::Generator<MCOperand const*> bytecode() const override DISABLE_ASAN {
      for(int i = 0; i < N; i++) {
         co_yield &tape[i];
      }
   }
};

// Defines MC pattern input
template <typename TD, bool isInput>
   requires IsTargetDef<TD>
struct MCPatternOpListDef final {
   unsigned size = 0;
   std::array<MCOperand, TD::MaxOperands> ops;
   constexpr MCPatternOpListDef(std::initializer_list<MCOperand> list) {
      for(const auto op : list) {
         if(size >= TD::MaxOperands)
            throw "MCPatternDef operand array is out of space! Maybe increase "
                  "MaxOperands?";
         this->ops[size++] = op;
      }
   }
};

// Abstract base class to erase template parameters
class MCPatternDefBase {
public:
   // Dump the pattern
   void dump() const;
   // Print the pattern
   std::ostream& print(std::ostream&, int indent = 0) const;
   virtual utils::Generator<MCPatternBase const*> patterns() const = 0;
   virtual NodeKind getDAGKind() const = 0; 
   virtual std::string_view getPatternName() const = 0;
   virtual ~MCPatternDefBase() = default;
};

// Defines the MC pattern, but not the rule for matching the pattern
template <typename TD>
   requires IsTargetDef<TD>
class MCPatternDef final : public MCPatternDefBase {
private:
   TD::InstType type_;
   std::array<MCOperand, TD::MaxOperands> inputs;
   std::array<MCOperand, TD::MaxOperands> outputs;
   std::array<MCPattern<TD>, TD::MaxPatternsPerDef> patterns_;
   unsigned ninputs = 0;
   unsigned noutputs = 0;
   unsigned npatterns = 0;

public:
   // Construct a pattern definition with a given instruction type
   constexpr MCPatternDef(TD::InstType type) : type_{type} {}
   // Copy constructor (default)
   constexpr MCPatternDef(MCPatternDef<TD> const& def) = default;
   // Add a pattern to the definition
   consteval MCPatternDef<TD> operator<<(MCPattern<TD> const& pat) const {
      auto P = MCPatternDef{*this};
      if(pat.N) {
         if(P.npatterns >= TD::MaxPatternsPerDef)
            throw "Too many patterns! Increase MaxPatternsPerDef?";
         P.patterns_[P.npatterns++] = pat;
      }
      return P;
   }
   // Add an input or output operand list to the pattern
   template <bool isInput>
   consteval MCPatternDef<TD> operator<<(
         MCPatternOpListDef<TD, isInput> const& list) const {
      auto P = MCPatternDef{*this};
      if(isInput) {
         P.inputs = list.ops;
         P.ninputs = list.size;
      } else {
         P.outputs = list.ops;
         P.noutputs = list.size;
      }
      return P;
   }
   // Grab the patterns
   utils::Generator<MCPatternBase const*> patterns() const override DISABLE_ASAN {
      for(unsigned i = 0; i < npatterns; i++) {
         co_yield &patterns_[i];
      }
   }
   // Get the pattern name
   std::string_view getPatternName() const override {
      return TD::GetPatternName(type_);
   }
   // Get the DAG kind for this pattern
   NodeKind getDAGKind() const override {
      return static_cast<NodeKind>(patterns_[0].tape[0].data1);
   }
   // TODO(kevin): Validate the DAG pattern matchings
};

} // namespace mc::details

/* ===--------------------------------------------------------------------=== */
// MCPatterns definition
/* ===--------------------------------------------------------------------=== */

namespace mc {

using MCPatternDef = mc::details::MCPatternDefBase;
using MCPattern = mc::details::MCPatternBase;

class MCPatterns {
public:
   /**
    * @brief Uses maximal munch to match the pattern starting from the root
    * node. Returns constructed new node with the matched pattern and linked
    * parameters. Will destroy the matched node.
    *
    * @param root The node to begin matching at
    * @return mc::InstSelectNode* The new node with the matched pattern
    */
   mc::InstSelectNode* matchAndReplace(mc::InstSelectNode* root) const;

   /**
    * @brief Gets the pattern list for a given instruction type
    */
   virtual utils::Generator<MCPatternDef const*> getPatternFor(
         mc::NodeKind) const = 0;
   /**
    * @brief Iterates over all the patterns in the target
    */
   virtual utils::Generator<MCPatternDef const*> patterns() const = 0;
};

template <typename TD>
class MCPatternsImpl : public MCPatterns {
protected:
   using define = mc::details::MCPatternDef<TD>;
   using pattern = mc::details::MCPattern<TD>;
   using inputs = mc::details::MCPatternOpListDef<TD, true>;
   using outputs = mc::details::MCPatternOpListDef<TD, false>;

protected:
   // Shorthand to create a new imm MC operand
   static consteval auto imm(uint8_t size) {
      using mc::details::MCOperand;
      return MCOperand{MCOperand::Type::Immediate, size, 0};
   }

   // Shorthand to create a new register MC operand
   static consteval auto reg(TD::RegClass T) {
      using mc::details::MCOperand;
      return MCOperand{MCOperand::Type::Register, static_cast<uint8_t>(T), 0};
   }

   // Shorthand to create a new pattern fragment MC operand
   static consteval auto frag(TD::FragType T) {
      using mc::details::MCOperand;
      return MCOperand{MCOperand::Type::Register, static_cast<uint8_t>(T), 0};
   }
};

} // namespace mc
