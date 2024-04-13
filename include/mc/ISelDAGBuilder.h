#pragma once

#include "mc/MCFunction.h"
#include "tir/BasicBlock.h"
#include "tir/Constant.h"
#include "tir/Instructions.h"
#include "utils/BumpAllocator.h"

namespace mc {

class InstSelectNode;
class MCFunction;

/* ===--------------------------------------------------------------------=== */
// ISelDAGBuilder
/* ===--------------------------------------------------------------------=== */

class ISelDAGBuilder final {
private:
   ISelDAGBuilder(BumpAllocator& alloc, MCFunction* MCF)
         : alloc{alloc}, MCF{MCF} {}

public:
   static MCFunction* Build(BumpAllocator&, tir::Function const* F);

private:
   // Main DAG building instruction to translate TIR -> instruction node
   InstSelectNode* buildInst(tir::Instruction*);
   // Build (if not exist) or get (if exist) a virtual register node
   InstSelectNode* buildVReg(tir::Instruction*);
   // Find an already built instruction or constant or (etc.)
   InstSelectNode* findValue(tir::Value*);
   // Create a condition code leaf node
   InstSelectNode* buildCC(tir::Instruction::Predicate);
   // Allocate (if not exist) or get (if exist) a virtual register index
   // corresponding to the given value
   int findOrAllocVirtReg(tir::Value* v);
   // Allocate (if not exist) or get (if exist) a stack slot index chunk
   // corresponding to the given alloca instruction
   InstSelectNode::StackSlot findOrAllocStackSlot(tir::AllocaInst* alloca);

private:
   BumpAllocator& alloc;
   MCFunction* MCF;
   tir::BasicBlock* curbb;
   // Maps TIR instruction -> node, this is cleared per BB
   std::unordered_map<tir::Value*, InstSelectNode*> instMap;
   std::unordered_map<tir::Value*, int> vregMap;
   std::unordered_map<tir::AllocaInst*, InstSelectNode::StackSlot> allocaMap;
   std::unordered_map<tir::BasicBlock*, InstSelectNode*> bbMap;
   int highestVregIdx = 0;
   int highestStackSlotIdx = 0;
};

} // namespace mc
