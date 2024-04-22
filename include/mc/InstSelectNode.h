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

namespace details {
class MCPatternDefBase;
}

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
   F(MachineInstr)      \
   F(LoadToReg)         \
   F(PHI)               \
   F(UNREACHABLE)       \
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
   /* Control flow */   \
   F(CALL)              \
   F(BR_CC)             \
   F(BR)                \
   F(RETURN)

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
                      /* For inst select */ details::MCPatternDefBase const*>;
   using DataOpt = std::optional<DataUnion>;

   DECLARE_STRING_TABLE(NodeKind, NodeTypeStrings, NodeTypeList)

   /* ===-----------------------------------------------------------------=== */
   // Private constructors, used by the DAGBuilder
   /* ===-----------------------------------------------------------------=== */
private:
   friend class DAGBuilder;
   // Internal constructor for N-ary nodes
   InstSelectNode(BumpAllocator& alloc, NodeKind kind, unsigned arity,
                  DataOpt data, Type type)
         : utils::GraphNodeUser<InstSelectNode>{alloc},
           utils::GraphNode<InstSelectNode>{alloc},
           kind_{kind},
           data_{data},
           arity_{arity},
           type_{type} {}
   // Build any non-leaf node of some type, with N arguments
   static InstSelectNode* Create(BumpAllocator& alloc, Type ty, NodeKind kind,
                                 utils::range_ref<InstSelectNode*> args) {
      auto* buf =
            alloc.allocate_bytes(sizeof(InstSelectNode), alignof(InstSelectNode));
      auto* node = new(buf) InstSelectNode{
            alloc, kind, static_cast<unsigned>(args.size()), std::nullopt, ty};
      args.for_each([&node](auto* arg) { node->addChild(arg); });
      return node;
   }
   // Build a leaf node (zero arity) with type and leaf data. Note that leaf
   // nodes can have children through chaining.
   static InstSelectNode* CreateLeaf(BumpAllocator& alloc, NodeKind kind,
                                     Type type = Type{0},
                                     DataOpt data = std::nullopt) {
      auto* buf =
            alloc.allocate_bytes(sizeof(InstSelectNode), alignof(InstSelectNode));
      auto* node = new(buf) InstSelectNode{alloc, kind, 0, data, type};
      return node;
   }
   // Build a constant immediate leaf node
   static InstSelectNode* CreateImm(BumpAllocator& alloc, int bits,
                                    uint64_t value) {
      return CreateLeaf(alloc,
                        NodeKind::Constant,
                        Type{static_cast<uint32_t>(bits)},
                        InstSelectNode::ImmValue{bits, value});
   }

public:
   /// @brief Gets the data of the node as a specific type
   template <typename T>
   T get() const {
      return std::get<T>(data_.value());
   }
   /// @brief Checks if this node has data associated with it
   bool hasData() const { return data_.has_value(); }
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
   void clearChains() {
      for(unsigned i = arity_; i < numChildren(); i++)
         getRawChild(i)->removeUse({this, i});
      children_.resize(arity_);
   }
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
   /**
    * @brief Compare if two nodes are referring to the same thing. This means
    * pointer equality for non-leaf nodes.
    *
    * @param other The other node to compare with
    */
   bool operator==(InstSelectNode const& other) const;
   /**
    * @brief Replace this node with the selected pattern
    */
   InstSelectNode* selectPattern(BumpAllocator& alloc,
                                 details::MCPatternDefBase const* pattern,
                                 std::vector<InstSelectNode*>& operands,
                                 std::vector<InstSelectNode*>& nodesToDelete);
   // Gets the arity of this node
   auto arity() const { return arity_; }
   // Iterate through the chains of this node
   utils::Generator<InstSelectNode*> chains() const {
      for(size_t i = arity_; i < numChildren(); i++) co_yield getChild(i);
   }
   // Get the type of this node
   auto type() const { return type_; }

private:
   const NodeKind kind_;
   const DataOpt data_;
   const unsigned arity_;
   const Type type_;
   int topoIdx_ = -1;
   int liveRangeTo_ = -1;
   InstSelectNode* prev_ = nullptr;
   InstSelectNode* next_ = nullptr;
   int mcRegIndex_ = -1;
};

} // namespace mc

#undef NodeTypeList
