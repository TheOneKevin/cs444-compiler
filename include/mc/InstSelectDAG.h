#pragma once

#include "tir/BasicBlock.h"
#include "utils/BumpAllocator.h"
#include "utils/EnumMacros.h"
#include "utils/User.h"

namespace mc {

class MCFunction;

#define NodeTypeList(F) \
   F(None)              \
   /* Leaf nodes */     \
   F(Register)          \
   F(Constant)          \
   F(GlobalAddress)     \
   F(FrameIndex)        \
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
   F(BR_CC)
DECLARE_ENUM(NodeType, NodeTypeList)
DECLARE_STRING_TABLE(NodeType, NodeTypeStrings, NodeTypeList)
#undef NodeTypeList

class InstSelectNode : public utils::GraphNodeUser<InstSelectNode>,
                       public utils::GraphNode<InstSelectNode> {
   friend class InstSelectDAG;

public:
   InstSelectNode(BumpAllocator& alloc)
         : utils::GraphNodeUser<InstSelectNode>{alloc},
           utils::GraphNode<InstSelectNode>{alloc} {}

private:
   NodeType type_;
};

class InstSelectDAG {
public:
   static InstSelectDAG* BuildSelectionDAG(MCFunction* parent,
                                           tir::BasicBlock& bb);

private:
   InstSelectNode* root_;
   MCFunction* parent_;
};

} // namespace mc
