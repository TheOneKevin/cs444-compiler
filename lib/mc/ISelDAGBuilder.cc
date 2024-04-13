#include "mc/ISelDAGBuilder.h"

#include <unordered_map>
#include <utility>

#include "mc/InstSelectNode.h"
#include "mc/MCFunction.h"
#include "tir/BasicBlock.h"
#include "tir/Constant.h"
#include "tir/Instructions.h"
#include "tir/TIR.h"
#include "tir/Type.h"
#include "tir/Value.h"

namespace mc {

using namespace tir;
using DAG = ISelDAGBuilder;
using ISN = InstSelectNode;

/* ===--------------------------------------------------------------------=== */
// Function to actually build the DAG for a given TIR function
/* ===--------------------------------------------------------------------=== */

MCFunction* DAG::Build(BumpAllocator& alloc, tir::Function const* F) {
   assert(F->hasBody());
   // Allocate the MCFunction to store the DAGs
   void* buf = alloc.allocate_bytes(sizeof(MCFunction), alignof(MCFunction));
   auto MCF = new(buf) MCFunction{alloc, F->ctx().TI()};
   // Create the builder now
   ISelDAGBuilder builder{alloc, MCF};
   // Build dummy DAG nodes for each basic block
   for(auto* bb : F->reversePostOrder()) {
      auto* const node = ISN::CreateLeaf(alloc, NodeType::Entry);
      builder.bbMap[bb] = node;
      MCF->graphs_.push_back(node);
   }
   // Build the DAG for each basic block
   for(auto* bb : F->body()) {
      builder.curbb = bb;
      builder.instMap.clear();
      ISN* entry = nullptr;
      for(auto* inst : *bb) {
         entry = builder.buildInst(inst);
      }
      assert(entry && "Basic block has no terminator");
      builder.bbMap[bb]->addChild(entry);
   }
   return builder.MCF;
}

/* ===--------------------------------------------------------------------=== */
// DAG building helper functions (to allocate things + caching nodes)
/* ===--------------------------------------------------------------------=== */

int DAG::findOrAllocVirtReg(tir::Value* v) {
   auto it = vregMap.find(v);
   if(it != vregMap.end()) {
      return it->second;
   }
   return vregMap[v] = ++highestVregIdx;
}

ISN::StackSlot DAG::findOrAllocStackSlot(tir::AllocaInst* alloca) {
   auto it = allocaMap.find(alloca);
   if(it != allocaMap.end()) {
      return it->second;
   }
   auto& TI = MCF->TI_;
   int idx = ++highestStackSlotIdx;
   int bytes = (alloca->allocatedType()->getSizeInBits() + 1) / 8;
   int slots = (bytes + TI.getStackAlignment() - 1) / TI.getStackAlignment();
   return allocaMap[alloca] = ISN::StackSlot{static_cast<uint16_t>(idx),
                                             static_cast<uint16_t>(slots)};
}

InstSelectNode* DAG::buildVReg(tir::Instruction* v) {
   if(instMap.contains(v)) return instMap[v];
   int vreg = findOrAllocVirtReg(v);
   auto* const node = ISN::CreateLeaf(alloc, NodeType::Register, ISN::VReg{vreg});
   instMap[v] = node;
   return node;
}

InstSelectNode* DAG::buildCC(tir::Instruction::Predicate pred) {
   return ISN::CreateLeaf(alloc, NodeType::Predicate, pred);
}

InstSelectNode* DAG::findValue(tir::Value* v) {
   if(v->isBasicBlock()) {
      if(instMap.contains(v)) return instMap[v];
      auto& subgraph = bbMap[cast<BasicBlock>(v)];
      auto node = ISN::CreateLeaf(alloc, NodeType::BasicBlock);
      node->addChild(subgraph);
      instMap[v] = node;
      return node;
   } else if(v->isInstruction()) {
      auto* instr = cast<Instruction>(v);
      // If this is an alloca, use a stack slot
      if(auto* alloca = dyn_cast<AllocaInst>(v)) {
         return ISN::CreateLeaf(
               alloc, NodeType::FrameIndex, findOrAllocStackSlot(alloca));
      }
      // If the instruction is declared in another bb, use a vreg
      // Otherwise, grab the emitted instruction
      return instr->parent() != curbb
                   ? buildVReg(instr)
                   : (assert(instMap.contains(instr) &&
                             "Instruction does not dominate all uses"),
                      instMap[instr]);
   } else if(v->isFunction()) {
      return ISN::CreateLeaf(
            alloc, NodeType::GlobalAddress, cast<GlobalObject>(v));
   } else if(v->isFunctionArg()) {
      auto arg = cast<Argument>(v);
      int idx = arg->index();
      return ISN::CreateLeaf(alloc, NodeType::Argument, ISN::VReg{idx});
   } else if(v->isConstant()) {
      auto c = cast<Constant>(v);
      if(c->isNumeric()) {
         auto CI = cast<ConstantInt>(c);
         auto bits = CI->type()->getSizeInBits();
         return ISN::CreateImm(alloc, bits, CI->zextValue());
      } else if(c->isGlobalVariable()) {
         return ISN::CreateLeaf(
               alloc, NodeType::GlobalAddress, cast<GlobalObject>(c));
      } else if(c->isNullPointer()) {
         return ISN::CreateImm(alloc, MCF->TI_.getPointerSizeInBits(), 0);
      }
   }
   assert(false && "Unknown value type");
   std::unreachable();
}

/* ===--------------------------------------------------------------------=== */
// Main DAG building function to translate TIR instruction -> DAG nodes
/* ===--------------------------------------------------------------------=== */

InstSelectNode* DAG::buildInst(tir::Instruction* inst) {
   // Build the current instruction
   InstSelectNode* node = nullptr;
   // Go through the different types of instruction and emit it!
   if(auto alloca = dyn_cast<AllocaInst>(inst)) {
      return nullptr;
   } else if(auto br = dyn_cast<BranchInst>(inst)) {
      auto cond = br->getCondition();
      auto bb1 = br->getSuccessor(0);
      auto bb2 = br->getSuccessor(1);
      // If it's an unconditional branch, emit a br
      if(bb1 == bb2) {
         node = ISN::Create(alloc, NodeType::BR, {findValue(bb1)});
      }
      // If the condition comes from a cmp, emit a br_cc
      else if(auto cmp = dyn_cast<CmpInst>(cond)) {
         auto lhs = findValue(cmp->getChild(0));
         auto rhs = findValue(cmp->getChild(1));
         auto cc = buildCC(cmp->predicate());
         node = ISN::Create(alloc,
                            NodeType::BR_CC,
                            {cc, lhs, rhs, findValue(bb1), findValue(bb2)});
      }
      // Else, emit a br_cc with a comparison against 0
      else {
         auto cc = buildCC(CmpInst::Predicate::NE);
         const auto zero = ISN::CreateImm(alloc, cond->type()->getSizeInBits(), 0);
         auto lhs = findValue(cond);
         node = ISN::Create(alloc,
                            NodeType::BR_CC,
                            {cc, lhs, zero, findValue(bb1), findValue(bb2)});
      }
   } else if(auto ri = dyn_cast<ReturnInst>(inst)) {
      if(!ri->isReturnVoid()) {
         node = ISN::Create(alloc, NodeType::RETURN, {findValue(ri->getChild(0))});
      } else {
         node = ISN::Create(alloc, NodeType::RETURN, {});
      }
   } else if(dyn_cast<StoreInst>(inst)) {
      auto src = inst->getChild(0);
      auto dst = inst->getChild(1);
      node = ISN::Create(alloc, NodeType::STORE, {findValue(src), findValue(dst)});
      // Chain to the previous instruction
      auto dep = inst->prev();
      if(!(dep == src || dep == dst) && dep) {
         node->addChild(findValue(dep));
      }
   } else if(dyn_cast<LoadInst>(inst)) {
      auto src = inst->getChild(0);
      node = ISN::Create(alloc, NodeType::LOAD, {findValue(src)});
      // Chain to the previous instruction
      auto dep = inst->prev();
      if(dep != src && dep) {
         node->addChild(findValue(dep));
      }
   } else if(auto bin = dyn_cast<BinaryInst>(inst)) {
      auto op = bin->binop();
      auto lhs = findValue(bin->getChild(0));
      auto rhs = findValue(bin->getChild(1));
      auto nodeType = NodeType::None;
      switch(op) {
         case tir::Instruction::BinOp::Add:
            nodeType = NodeType::ADD;
            break;
         case tir::Instruction::BinOp::Sub:
            nodeType = NodeType::SUB;
            break;
         case tir::Instruction::BinOp::Mul:
            nodeType = NodeType::MUL;
            break;
         case tir::Instruction::BinOp::Div:
            nodeType = NodeType::SDIV;
            break;
         case tir::Instruction::BinOp::Rem:
            nodeType = NodeType::SREM;
            break;
         case tir::Instruction::BinOp::And:
            nodeType = NodeType::AND;
            break;
         case tir::Instruction::BinOp::Or:
            nodeType = NodeType::OR;
            break;
         case tir::Instruction::BinOp::Xor:
            nodeType = NodeType::XOR;
            break;
         case tir::Instruction::BinOp::None:
         case tir::Instruction::BinOp::LAST_MEMBER:
            assert(false);
            std::unreachable();
      }
      node = ISN::Create(alloc, nodeType, {lhs, rhs});
   } else if(auto ci = dyn_cast<CallInst>(inst)) {
      // FIXME(kevin): Calls should have a call begin + call end
      findValue(ci->getCallee());
      std::vector<InstSelectNode*> args;
      if(ci->nargs() > 0) {
         for(auto* arg : ci->args()) args.push_back(findValue(arg));
      }
      node = ISN::Create(alloc, NodeType::CALL, args);
   } else if(auto ici = dyn_cast<ICastInst>(inst)) {
      auto src = findValue(ici->getChild(0));
      auto nodeType = NodeType::None;
      switch(ici->castop()) {
         case tir::Instruction::CastOp::Trunc:
            nodeType = NodeType::TRUNCATE;
            break;
         case tir::Instruction::CastOp::ZExt:
            nodeType = NodeType::ZERO_EXTEND;
            break;
         case tir::Instruction::CastOp::SExt:
            nodeType = NodeType::SIGN_EXTEND;
            break;
         case tir::Instruction::CastOp::LAST_MEMBER:
            assert(false);
            std::unreachable();
      }
      node = ISN::Create(alloc, nodeType, {src});
   } else if(auto gep = dyn_cast<GetElementPtrInst>(inst)) {
      auto base = findValue(gep->getPointerOperand());
      auto type = gep->getContainedType();
      auto ptrbits = tir::Type::getPointerTy(inst->ctx())->getSizeInBits();
      for(auto* idx : gep->indices()) {
         // If the type is a struct, we need to add the offset of the field
         // Whereas arrays can be indexed dynamically
         if(type->isStructType()) {
            assert(idx->isConstant() && "Struct type requires constant index");
            auto nidx = cast<ConstantInt>(idx)->zextValue();
            auto STy = cast<StructType>(type);
            auto offs = STy->getTypeOffsetAtIndex(nidx);
            auto offs_node = ISN::CreateImm(alloc, ptrbits, offs);
            base = ISN::Create(alloc, NodeType::ADD, {base, offs_node});
            type = STy->getTypeAtIndex(nidx);
         } else if(type->isArrayType()) {
            auto ATy = cast<ArrayType>(type);
            auto idx_node = findValue(idx);
            auto elem_sz = ATy->getElementType()->getSizeInBits();
            auto elem_sz_node = ISN::CreateImm(alloc, ptrbits, elem_sz);
            auto offs_node =
                  ISN::Create(alloc, NodeType::MUL, {idx_node, elem_sz_node});
            base = ISN::Create(alloc, NodeType::ADD, {base, offs_node});
            type = ATy->getElementType();
         } else {
            assert(false && "Unsupported GEP type");
         }
      }
      node = base;
   } else if(auto cmp = dyn_cast<CmpInst>(inst)) {
      // Emit cmp as set_cc even if it's not used
      auto lhs = findValue(cmp->getChild(0));
      auto rhs = findValue(cmp->getChild(1));
      auto cc = buildCC(cmp->predicate());
      node = ISN::Create(alloc, NodeType::SET_CC, {cc, lhs, rhs});
   } else if(dyn_cast<PhiNode>(inst)) {
      node = ISN::Create(alloc, NodeType::PHI, {});
      for(auto [bb, val] : cast<PhiNode>(inst)->incomingValues()) {
         node->addChild(findValue(val));
         node->addChild(findValue(bb));
      }
   } else {
      assert(false &&
             "Instruction selection DAG does not support this instruction");
      std::unreachable();
   }
   // Store the instruction
   instMap[inst] = node;
   return node;
}

} // namespace mc
