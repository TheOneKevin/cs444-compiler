#include <unordered_set>

#include "../IRContextPass.h"
#include "tir/BasicBlock.h"
#include "tir/Instructions.h"
#include "utils/PassManager.h"

using std::string_view;
using utils::Pass;
using utils::PassManager;

namespace {

// Remove all instructions after the first terminator in a basic block
bool eliminateAfterFirstTerminator(tir::BasicBlock& bb) {
   auto firstTerminator = bb.begin();
   for(auto it = bb.begin(); it != bb.end(); ++it) {
      if(it->isTerminator()) {
         firstTerminator = it;
         break;
      }
   }
   if(firstTerminator == --bb.end()) return false;
   auto instr = *(++firstTerminator);
   while(instr != nullptr) {
      auto next = instr->next();
      bb.erase(instr);
      instr = next;
   }
   return true;
}

// Merge two basic blocks if the second one has a single predecessor
// and the first one has a single successor
bool mergeSinglePredSingleSucc(tir::BasicBlock& bb) {
   // 1. Grab the successor of the current basic block
   auto term = dyn_cast<tir::BranchInst>(bb.terminator());
   if(term == nullptr) return false;
   auto succ = term->getSuccessor(0);
   if(term->getSuccessor(1) != succ) return false;
   // 2. Check if the successor's predecessors is this BB
   for(auto pred : succ->users()) {
      if(pred != term) return false;
   }
   // 3. Move all the instructions from the successor to the current basic block
   term->eraseFromParent();
   tir::Instruction* instr = *succ->begin();
   while(instr) {
      auto next = instr->next();
      instr->eraseFromParent(true); // Keep all refs
      bb.appendAfterEnd(instr);
      instr = next;
   }
   // 4. Remove the successor from the parent function
   succ->eraseFromParent();
   return true;
}

/**
 * @brief Merge basic block with single successor in one branch.
 *
 *    +---------------+
 *    | bb0:          |
 *    |   br bb1, bb2 |  =====> Can be transformed into br bb1, bb3
 *    +-+----------+--+
 *      |          |
 *      v          v
 * +----+--+   +---+------+
 * | bb1:  |   | bb2:     |
 * |   ... |   |   br bb3 |
 * +-------+   +----------+
 *
 */
bool replaceSucessorInOneBranch(tir::BasicBlock& bb) {
   bool changed = false;
   auto term = dyn_cast<tir::BranchInst>(bb.terminator());
   if(term == nullptr) return changed;
   // Try to replace either successor
   for(int i = 0; i < 2; i++) {
      auto succ = term->getSuccessor(i);
      // 1. There must be only one successor
      if(++succ->begin() != succ->end()) continue;
      // 2. The sucessor must be an unconditional branch
      auto sterm = dyn_cast<tir::BranchInst>(succ->terminator());
      if(sterm == nullptr) continue;
      if(sterm->getSuccessor(0) != sterm->getSuccessor(1)) continue;
      // 3. Then this sucessor can be replaced
      term->replaceSuccessor(i, sterm->getSuccessor(0));
      changed = true;
   }
   return changed;
}

} // namespace

/* ===--------------------------------------------------------------------=== */
// SimplifyCFG pass infrastructure
/* ===--------------------------------------------------------------------=== */

class SimplifyCFG final : public Pass {
public:
   SimplifyCFG(PassManager& PM) noexcept : Pass(PM) {}
   void Run() override {
      tir::CompilationUnit& CU = GetPass<IRContextPass>().CU();
      std::vector<tir::BasicBlock*> toRemove;
      for(auto func : CU.functions()) {
         // 1. Iteratively simplify the CFG
         bool changed = false;
         do {
            changed = false;
            visited.clear();
            if(func->getEntryBlock()) {
               changed = visitBB(*func->getEntryBlock());
            }
         } while(changed);
         // 2. Record all the basic blocks that were not visited
         toRemove.clear();
         for(auto bb : func->body()) {
            if(visited.count(bb)) continue;
            toRemove.push_back(bb);
         }
         // 3. Remove the basic blocks that are unreachable
         for(auto bb : toRemove) {
            bb->eraseFromParent();
         }
      }
   }
   string_view Name() const override { return "simplifycfg"; }
   string_view Desc() const override { return "Simplify CFG"; }

private:
   bool visitBB(tir::BasicBlock& bb) {
      bool changed = false;
      if(visited.count(&bb)) return false;
      visited.insert(&bb);
      // 1. Run all the simplifications
      changed |= eliminateAfterFirstTerminator(bb);
      changed |= mergeSinglePredSingleSucc(bb);
      changed |= replaceSucessorInOneBranch(bb);
      // 2. Grab the next basic block
      auto term = dyn_cast<tir::BranchInst>(bb.terminator());
      if(term == nullptr) return false;
      changed |= visitBB(*term->getSuccessor(0));
      changed |= visitBB(*term->getSuccessor(1));
      return changed;
   }

private:
   void computeDependencies() override {
      ComputeDependency(GetPass<IRContextPass>());
   }
   std::unordered_set<tir::BasicBlock*> visited;
};

REGISTER_PASS(SimplifyCFG);
