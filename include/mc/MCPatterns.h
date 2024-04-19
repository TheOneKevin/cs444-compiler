#pragma once

#include <cstdint>

#include "mc/InstSelectNode.h"

/* ===--------------------------------------------------------------------=== */
// Secret implementation details
/* ===--------------------------------------------------------------------=== */

namespace mc::details {

// Defines an MC instruction pattern's operand
struct [[gnu::packed]] MCOperand {
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

// Defines the rule to match an MC pattern
template <typename TD>
   requires IsTargetDef<TD>
class MCPattern {
   int N;
   MCOperand tape[TD::MaxStates];

public:
   // Empty pattern (matches nothing)
   constexpr MCPattern() : N{0} {}
   // 1st base case is an ISel node type
   constexpr MCPattern(mc::NodeKind op) : N{1} {
      tape[0] = MCOperand{
            MCOperand::Type::CheckOperandType, 0, static_cast<uint16_t>(op)};
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
};

// Defines MC pattern input
template <typename TD, bool isInput>
   requires IsTargetDef<TD>
struct MCPatternOpListDef {
   std::array<MCOperand, TD::MaxOperands> ops;
   constexpr MCPatternOpListDef(std::initializer_list<MCOperand> list) {
      int i = 0;
      for(const auto op : list) {
         if(i >= TD::MaxOperands)
            throw "MCPatternDef operand array is out of space! Maybe increase "
                  "MaxOperands?";
         this->ops[i++] = op;
      }
   }
};

// Defines the MC pattern, but not the rule for matching the pattern
template <typename TD>
   requires IsTargetDef<TD>
struct MCPatternDef {
   TD::InstType type;
   std::array<MCOperand, TD::MaxOperands> inputs;
   std::array<MCOperand, TD::MaxOperands> outputs;
   MCPattern<TD> pattern;

private:
   constexpr MCPatternDef(MCPatternDef<TD> const& def, MCPattern<TD> const& pat)
         : type{def.type},
           inputs{def.inputs},
           outputs{def.outputs},
           pattern{pat} {}
   template <bool isInput>
   constexpr MCPatternDef(MCPatternDef<TD> const& def,
                          MCPatternOpListDef<TD, isInput> const& list)
         : type{def.type},
           inputs{isInput ? list.ops : def.inputs},
           outputs{isInput ? def.outputs : list.ops},
           pattern{def.pattern} {}

public:
   constexpr MCPatternDef(TD::InstType type) : type{type} {}

   consteval MCPatternDef<TD> operator<<(MCPattern<TD> const& pat) const {
      return MCPatternDef{*this, pat};
   }

   template <bool isInput>
   consteval MCPatternDef<TD> operator<<(
         MCPatternOpListDef<TD, isInput> const& list) const {
      return MCPatternDef{*this, list};
   }
};

} // namespace mc::details

/* ===--------------------------------------------------------------------=== */
// MCPatterns definition
/* ===--------------------------------------------------------------------=== */

namespace mc {

template <typename TD>
class MCPatterns {
public:
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

   using define = mc::details::MCPatternDef<TD>;
   using pattern = mc::details::MCPattern<TD>;
   using inputs = mc::details::MCPatternOpListDef<TD, true>;
   using outputs = mc::details::MCPatternOpListDef<TD, false>;
};

} // namespace mc
