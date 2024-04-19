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

DECLARE_ENUM(NodeKind, NodeTypeList)

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

   struct Type {
      uint32_t bits;
      Type() : bits{0 /* Meaning there's no type */} {}
      explicit Type(uint32_t bits) : bits{bits} {}
   };

   using DataUnion =
         std::variant</* Stack slot index for allocas */ StackSlot,
                      /* Virtual register index */ VReg,
                      /* Constant immediate value */ ImmValue,
                      /* Predicate value */ tir::Instruction::Predicate,
                      /* Global object pointer */ tir::GlobalObject*,
                      /* Integer type of node */ Type>;
   using DataOpt = std::optional<DataUnion>;

   DECLARE_STRING_TABLE(NodeKind, NodeTypeStrings, NodeTypeList)

   /* ===-----------------------------------------------------------------=== */
   // Private constructors, used by the ISelDAGBuilder
   /* ===-----------------------------------------------------------------=== */
private:
   friend class ISelDAGBuilder;
   // Internal constructor for N-ary nodes
   InstSelectNode(BumpAllocator& alloc, NodeKind type, unsigned arity)
         : utils::GraphNodeUser<InstSelectNode>{alloc},
           utils::GraphNode<InstSelectNode>{alloc},
           kind_{type},
           arity_{arity} {}
   // Build any non-leaf node of some type, with N arguments
   static InstSelectNode* Create(BumpAllocator& alloc, Type ty, NodeKind kind,
                                 utils::range_ref<InstSelectNode*> args) {
      auto* buf =
            alloc.allocate_bytes(sizeof(InstSelectNode), alignof(InstSelectNode));
      auto* node = new(buf)
            InstSelectNode{alloc, kind, static_cast<unsigned>(args.size())};
      args.for_each([&node](auto* arg) { node->addChild(arg); });
      node->data_ = ty;
      return node;
   }
   // Build a leaf node (zero arity) with type and leaf data. Note that leaf
   // nodes can have children through chaining.
   static InstSelectNode* CreateLeaf(BumpAllocator& alloc, NodeKind kind,
                                     DataOpt data = std::nullopt) {
      auto* buf =
            alloc.allocate_bytes(sizeof(InstSelectNode), alignof(InstSelectNode));
      auto* node = new(buf) InstSelectNode{alloc, kind, 0};
      node->data_ = data;
      return node;
   }
   // Build a constant immediate leaf node
   static InstSelectNode* CreateImm(BumpAllocator& alloc, int bits,
                                    uint64_t value) {
      return CreateLeaf(
            alloc, NodeKind::Constant, InstSelectNode::ImmValue{bits, value});
   }

public:
   NodeKind kind() const { return kind_; }
   int printDotNode(utils::DotPrinter& dp,
                    std::unordered_set<InstSelectNode const*>& visited) const;
   void printNodeTable(utils::DotPrinter& dp) const;
   utils::Generator<InstSelectNode*> childNodes() const;
   void clearChains() { children_.resize(arity_); }
   InstSelectNode* getChild(unsigned idx) const {
      return static_cast<InstSelectNode*>(getRawChild(idx));
   }

private:
   const NodeKind kind_;
   DataOpt data_;
   const unsigned arity_;
};

} // namespace mc

#undef NodeTypeList
