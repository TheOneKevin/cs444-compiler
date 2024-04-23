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
class DAGBuilder;

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
   // Private constructors, used by the DAGBuilder
   /* ===-----------------------------------------------------------------=== */
private:
   friend class DAGBuilder;
   // Internal constructor for N-ary nodes
   InstSelectNode(BumpAllocator& alloc, NodeKind type, unsigned arity,
                  DataOpt data)
         : utils::GraphNodeUser<InstSelectNode>{alloc},
           utils::GraphNode<InstSelectNode>{alloc},
           kind_{type},
           data_{data},
           arity_{arity} {}
   // Build any non-leaf node of some type, with N arguments
   static InstSelectNode* Create(BumpAllocator& alloc, Type ty, NodeKind kind,
                                 utils::range_ref<InstSelectNode*> args) {
      auto* buf =
            alloc.allocate_bytes(sizeof(InstSelectNode), alignof(InstSelectNode));
      auto* node = new(buf)
            InstSelectNode{alloc, kind, static_cast<unsigned>(args.size()), ty};
      args.for_each([&node](auto* arg) { node->addChild(arg); });
      return node;
   }
   // Build a leaf node (zero arity) with type and leaf data. Note that leaf
   // nodes can have children through chaining.
   static InstSelectNode* CreateLeaf(BumpAllocator& alloc, NodeKind kind,
                                     DataOpt data = std::nullopt) {
      auto* buf =
            alloc.allocate_bytes(sizeof(InstSelectNode), alignof(InstSelectNode));
      auto* node = new(buf) InstSelectNode{alloc, kind, 0, data};
      return node;
   }
   // Build a constant immediate leaf node
   static InstSelectNode* CreateImm(BumpAllocator& alloc, int bits,
                                    uint64_t value) {
      return CreateLeaf(
            alloc, NodeKind::Constant, InstSelectNode::ImmValue{bits, value});
   }

public:
   /// @brief Gets the kind/operation of the node
   NodeKind kind() const { return kind_; }
   /// @brief Prints the node in the DOT format
   int printDotNode(utils::DotPrinter& dp,
                    std::unordered_set<InstSelectNode const*>& visited) const;
   /// @brief Prints the node table in the DOT format
   void printNodeTable(utils::DotPrinter& dp) const;
   /// @brief Iterates over the children of the node (including chains)
   utils::Generator<InstSelectNode*> childNodes() const;
   /// @brief Remove all the chains from the node
   void clearChains() { children_.resize(arity_); }
   /// @brief Gets the ith child of the node
   InstSelectNode* getChild(unsigned idx) const {
      return static_cast<InstSelectNode*>(getRawChild(idx));
   }
   /// @brief Gets the live range of this node as a tuple of (from, to)
   /// where, noting the topoIdx decreases from-top-to-bottom of the basic
   /// block, we must have from >= to.
   auto liveRange() const { return std::make_tuple(topoIdx_, liveRangeTo_); }
   /// @brief Gets the topological index of this node
   auto topoIdx() const { return topoIdx_; }
   /// @brief Build the adjacency list of the DAG
   void buildAdjacencyList(std::unordered_map<InstSelectNode*, std::vector<InstSelectNode*>> &adj);
   /// @brief Sets the topological index of this node and updates live range.
   void setTopoIdx(int idx) {
      topoIdx_ = idx;
      liveRangeTo_ = idx;
   }
   /// @brief Updates the live-range-to of this node, takes the minimum of the
   /// current range and the given one (to maximum live range).
   void updateLiveRange(int to) {
      liveRangeTo_ = liveRangeTo_ == -1 ? to : std::min(liveRangeTo_, to);
   }
   /**
    * @brief Insert this node before the given node in the list
    *
    * @param node The node to insert before
    */
   void insertBefore(InstSelectNode* node) {
      assert(prev_ == nullptr && next_ == nullptr);
      prev_ = node->prev_;
      next_ = node;
      if(prev_) prev_->next_ = this;
      node->prev_ = this;
   }
   // @brief Link the current node with the next node
   void link(InstSelectNode* node) {
      assert(next_ == nullptr);
      next_ = node;
      if(next_) node->prev_ = this;
   }
   
   InstSelectNode* prev() const { return prev_; }
   InstSelectNode* next() const { return next_; }

private:
   const NodeKind kind_;
   const DataOpt data_;
   const unsigned arity_;
   int topoIdx_ = -1;
   int liveRangeTo_ = -1;
   InstSelectNode* prev_ = nullptr;
   InstSelectNode* next_ = nullptr;
   int mcRegIndex_ = -1;
};

} // namespace mc

#undef NodeTypeList
