#pragma once

#include <cstdint>
#include <ostream>

#include "mc/InstSelectNode.h"
#include "utils/Generator.h"

namespace mc {
class MCTargetDesc;
struct MatchOptions;

} // namespace mc

/* ===--------------------------------------------------------------------=== */
// Implementation details
/* ===--------------------------------------------------------------------=== */

namespace mc::details {

struct MCOperand;
// Concept to check if T is an MCOperand
template <typename T>
concept IsMCOperand = std::is_same_v<T, MCOperand>;
// Concept to check if T is a TargetDefinition
template <typename T>
concept IsTargetDef = true;
class MCPatternDefBase;
class MCPatternBase;
template <typename TD>
   requires IsTargetDef<TD>
class MCPatternDef;

// Defines an MC instruction pattern's operand
struct [[gnu::packed]] MCOperand final {
   enum class Type {
      None,
      Immediate,
      Register,
      Fragment,
      Push,
      Pop,
      CheckNodeType,
      CheckOperandType
   };
   Type type : 8;
   uint32_t data : 24;
   constexpr MCOperand() : type{Type::None}, data{} {}
   constexpr MCOperand(Type type, uint32_t data)
         : type{type}, data{data & 0x00FF'FFFF} {}
   constexpr bool operator==(MCOperand const& other) const {
      return other.type == type && other.data == data;
   }
};

// Abstract base class to erase template parameters
class MCPatternBase {
public:
   virtual utils::Generator<MCOperand const*> bytecode() const = 0;
   virtual ~MCPatternBase() = default;
   // Does this pattern match the given node?
   bool matches(MatchOptions) const;
   // Print the pattern
   std::ostream& print(std::ostream&, int indent = 0) const;
   // Dump the pattern
   void dump() const;
};

// Abstract base class to erase template parameters
class MCPatternDefBase {
public:
   // Dump the pattern
   void dump() const;
   // Print the pattern
   std::ostream& print(std::ostream&, int indent = 0) const;
   virtual MCOperand getInput(unsigned idx) const = 0;
   virtual MCOperand getOutput(unsigned idx) const = 0;
   virtual unsigned numInputs() const = 0;
   virtual unsigned numOutputs() const = 0;
   virtual unsigned maxTapeLength() const = 0;
   virtual utils::Generator<MCPatternBase const*> patterns() const = 0;
   virtual NodeKind getDAGKind() const = 0;
   virtual std::string_view getPatternName() const = 0;
   virtual ~MCPatternDefBase() = default;
};

// Defines the rule to match an MC pattern
template <typename TD>
   requires IsTargetDef<TD>
class MCPattern final : public MCPatternBase {
public:
   unsigned N;
   MCOperand tape[TD::MaxStates];

public:
   // Empty pattern (matches nothing)
   constexpr MCPattern() : N{0} {}
   // 1st base case is an ISel node type
   constexpr MCPattern(mc::NodeKind op) : N{1} {
      tape[0] =
            MCOperand{MCOperand::Type::CheckNodeType, static_cast<uint16_t>(op)};
   }
   // 2nd base case is an MCPatternDef operand index
   constexpr MCPattern(uint16_t idx) : N{1} {
      tape[0] = MCOperand{MCOperand::Type::CheckOperandType, idx};
   }
   // Recursively construct the DAG pattern to match
   constexpr MCPattern(std::initializer_list<MCPattern> children) {
      // Populate the tape
      unsigned i = 0;
      for(auto child : children) {
         // Push if child.N > 1
         if(child.N > 1) tape[i++] = MCOperand{MCOperand::Type::Push, 0};
         // Add the tape contents of the children
         for(unsigned j = 0; j < child.N; j++) {
            if(i >= TD::MaxStates)
               throw "MCPattern tape is out of space! Maybe increase MaxStates?";
            tape[i++] = child.tape[j];
         }
         // Pop if child.N > 1
         if(child.N > 1) tape[i++] = MCOperand{MCOperand::Type::Pop, 0};
      }
      // Update the number of elements in our tape
      N = i;
   }
   // Grab the bytecode from the tape
   utils::Generator<MCOperand const*> bytecode() const override {
      for(unsigned i = 0; i < N; i++) {
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

// Defines the MC pattern, but not the rule for matching the pattern
// TODO(kevin): Validate the DAG pattern matchings
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
   unsigned maxTapeLength_ = 0;

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
         P.maxTapeLength_ = maxTapeLength_ > pat.N ? maxTapeLength_ : pat.N;
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
   utils::Generator<MCPatternBase const*> patterns() const override {
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
      return static_cast<NodeKind>(patterns_[0].tape[0].data);
   }
   // Get the nth input node
   MCOperand getInput(unsigned idx) const override {
      assert(idx < ninputs);
      return inputs[idx];
   }
   // Get the nth output node
   MCOperand getOutput(unsigned idx) const override {
      assert(idx < noutputs);
      return outputs[idx];
   }
   unsigned numInputs() const override { return ninputs; }
   unsigned numOutputs() const override { return noutputs; }
   unsigned maxTapeLength() const override { return maxTapeLength_; }
};

} // namespace mc::details

/**
 * @brief Struct of options passed to match a pattern
 */
struct mc::MatchOptions {
   mc::MCTargetDesc const& TD;
   mc::details::MCPatternDefBase const* def;
   std::vector<InstSelectNode*>& operands;
   std::vector<InstSelectNode*>& nodesToDelete;
   mc::InstSelectNode* node;
};

/* ===--------------------------------------------------------------------=== */
// MCPatterns definition
/* ===--------------------------------------------------------------------=== */

namespace mc {

using MCPatternDef = mc::details::MCPatternDefBase;
using MCPattern = mc::details::MCPatternBase;

class MCPatterns {
public:
   /**
    * @brief Gets the pattern list for a given instruction type
    */
   virtual utils::Generator<MCPatternDef const*> getPatternFor(NodeKind) const = 0;
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
      return MCOperand{MCOperand::Type::Immediate, size};
   }

   // Shorthand to create a new register MC operand
   static consteval auto reg(TD::RegClass T) {
      using mc::details::MCOperand;
      return MCOperand{MCOperand::Type::Register, static_cast<uint8_t>(T)};
   }

   // Shorthand to create a new pattern fragment MC operand
   static consteval auto frag(TD::FragType T) {
      using mc::details::MCOperand;
      return MCOperand{MCOperand::Type::Fragment, static_cast<uint8_t>(T)};
   }
};

} // namespace mc
