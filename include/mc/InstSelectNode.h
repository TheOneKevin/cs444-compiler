#pragma once

#include <optional>
#include <unordered_set>
#include <variant>

#include "tir/BasicBlock.h"
#include "tir/Constant.h"
#include "tir/Instructions.h"
#include "utils/BumpAllocator.h"
#include "utils/DotPrinter.h"
#include "utils/EnumMacros.h"
#include "utils/Generator.h"
#include "utils/User.h"
#include "utils/Utils.h"

namespace mc {

class InstSelectNode;
class ISelDAGBuilder;

/* ===--------------------------------------------------------------------=== */
// NodeType enum
/* ===--------------------------------------------------------------------=== */

#define NodeTypeList(F) \
   F(None)              \
   F(Entry)             \
   /* Leaf nodes */     \
   F(Argument)          \
   F(Register)          \
   F(Constant)          \
   F(GlobalAddress)     \
   F(FrameIndex)        \
   F(BasicBlock)        \
   F(Predicate)         \
   /* Special ops*/     \
   F(LoadFromReg)       \
   F(LoadToReg)         \
   /* Operations */     \
   F(LOAD)              \
   F(STORE)             \
   F(AND)               \
   F(OR)                \
   F(XOR)               \
   F(ADD)               \
   F(SUB)               \
   F(MUL)               \
   F(SDIV)              \
   F(SREM)              \
   F(SIGN_EXTEND)       \
   F(ZERO_EXTEND)       \
   F(TRUNCATE)          \
   F(SET_CC)            \
   F(SELECT_CC)         \
   /* Control flow */   \
   F(CALL)              \
   F(BR_CC)             \
   F(BR)                \
   F(PHI)               \
   F(RETURN)            \
   F(UNREACHABLE)

DECLARE_ENUM(NodeType, NodeTypeList)

/* ===--------------------------------------------------------------------=== */
// InstSelectNode class
/* ===--------------------------------------------------------------------=== */

class InstSelectNode final : public utils::GraphNodeUser<InstSelectNode>,
                             public utils::GraphNode<InstSelectNode> {
   /* ===-----------------------------------------------------------------=== */
   // Data structure and type definitions
   /* ===-----------------------------------------------------------------=== */
public:
   struct StackSlot {
      uint16_t idx;
      uint16_t count;
   };

   struct ImmValue {
      int bits;
      uint64_t value;
   };

   struct VReg {
      int idx;
      explicit VReg(int idx) : idx{idx} {}
   };

   using DataUnion =
         std::variant</* Stack slot index for allocas */ StackSlot,
                      /* Virtual register index */ VReg,
                      /* Constant immediate value */ ImmValue,
                      /* Predicate value */ tir::Instruction::Predicate,
                      /* Global object pointer */ tir::GlobalObject*>;
   using DataOpt = std::optional<DataUnion>;

   DECLARE_STRING_TABLE(NodeType, NodeTypeStrings, NodeTypeList)

   /* ===-----------------------------------------------------------------=== */
   // Private constructors, used by the ISelDAGBuilder
   /* ===-----------------------------------------------------------------=== */
private:
   friend class ISelDAGBuilder;
   // Internal constructor for N-ary nodes
   InstSelectNode(BumpAllocator& alloc, NodeType type, unsigned arity)
         : utils::GraphNodeUser<InstSelectNode>{alloc},
           utils::GraphNode<InstSelectNode>{alloc},
           type_{type},
           arity{arity} {}
   // Build any non-leaf node of some type, with N arguments
   static InstSelectNode* Create(BumpAllocator& alloc, NodeType type,
                                 utils::range_ref<InstSelectNode*> args) {
      auto* buf =
            alloc.allocate_bytes(sizeof(InstSelectNode), alignof(InstSelectNode));
      auto* node =
            new(buf) InstSelectNode{alloc, type, static_cast<unsigned>(args.size())};
      args.for_each([&node](auto* arg) { node->addChild(arg); });
      return node;
   }
   // Build a leaf node (zero arity) with type and leaf data. Note that leaf
   // nodes can have children through chaining.
   static InstSelectNode* CreateLeaf(BumpAllocator& alloc, NodeType type,
                                     DataOpt data = std::nullopt) {
      auto* buf =
            alloc.allocate_bytes(sizeof(InstSelectNode), alignof(InstSelectNode));
      auto* node = new(buf) InstSelectNode{alloc, type, 0};
      node->data_ = data;
      return node;
   }
   // Build a constant immediate leaf node
   static InstSelectNode* CreateImm(BumpAllocator& alloc, int bits,
                                    uint64_t value) {
      return CreateLeaf(
            alloc, NodeType::Constant, InstSelectNode::ImmValue{bits, value});
   }

public:
   NodeType type() const { return type_; }
   int printDotNode(utils::DotPrinter& dp,
                    std::unordered_set<InstSelectNode const*>& visited) const;
   void printNodeTable(utils::DotPrinter& dp) const;
   utils::Generator<InstSelectNode*> childNodes() const;
   void clearChains() { children_.resize(arity); }

private:
   const NodeType type_;
   DataOpt data_;
   const unsigned arity;
};

} // namespace mc

#undef NodeTypeList
