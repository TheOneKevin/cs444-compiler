#include <queue>
#include <stack>
#include <unordered_set>

#include "../IRContextPass.h"
#include "../analysis/DominatorTree.h"
#include "diagnostics/Diagnostics.h"
#include "tir/BasicBlock.h"
#include "tir/Constant.h"
#include "tir/Instructions.h"
#include "utils/PassManager.h"

using std::string_view;
using utils::Pass;
using utils::PassManager;
using DE = diagnostics::DiagnosticEngine;
using namespace tir;

// Enclose everything but the pass in an anonymous namespace
namespace {

/* ===--------------------------------------------------------------------=== */
// Class definitions
/* ===--------------------------------------------------------------------=== */

/**
 * @brief Hoist alloca instructions into registers by placing phi nodes
 * in the dominance frontier of the alloca.
 * Citation: Simple and Efficient Construction of Static Single Assignment Form
 * Authors: Ron Cytron, et al.
 * URL: https://pages.cs.wisc.edu/~fischer/cs701.f14/ssa.pdf
 * Also referenced: https://c9x.me/compile/bib/braun13cc.pdf
 */
class HoistAlloca {
public:
   HoistAlloca(Function* func, DE& diag) : func{func}, DT{nullptr} {
      // 1. Print out the dominator tree
      if(diag.Verbose(2)) {
         auto dbg = diag.ReportDebug();
         DT->print(dbg.get());
      }
      // 2. Place PHI nodes for each alloca and record the uses
      for(auto alloca : func->allocas()) {
         if(canAllocaBeReplaced(alloca)) {
            placePHINodes(alloca);
            for(auto* user : alloca->users()) {
               if(auto* store = dyn_cast<StoreInst>(user)) {
                  storesToRewrite.insert(store);
               } else if(auto* load = dyn_cast<LoadInst>(user)) {
                  loadsToRewrite.insert(load);
               }
            }
         }
      }
      // 3. Print out the alloca and phi nodes
      if(diag.Verbose()) {
         print(diag.ReportDebug().get());
      }
      // 4. Replace the uses of the alloca with the phi nodes
      replaceUses(func->getEntryBlock());
      // 5. Do DCE on the stores and loads
      for(auto store : storesToRewrite) {
         assert(store->uses().size() == 0);
         store->eraseFromParent();
      }
      for(auto load : loadsToRewrite) {
         assert(load->uses().size() == 0);
         load->eraseFromParent();
      }
   }

   std::ostream& print(std::ostream& os) const {
      os << "*** PHI node insertion points ***\n";
      for(auto [phi, alloca] : phiAllocaMap) {
         os << "  ";
         phi->printName(os) << " -> ";
         alloca->printName(os) << std::endl;
      }
      return os;
   }

private:
   Function* func;
   DominatorTree* DT;
   // Stores and loads to rewrite
   std::unordered_set<Instruction*> storesToRewrite;
   std::unordered_set<Instruction*> loadsToRewrite;
   // Phi -> Alloca map
   std::unordered_map<PhiNode*, AllocaInst*> phiAllocaMap;
   // Variable stack
   std::unordered_map<Value*, std::stack<Value*>> varStack;

private:
   void placePHINodes(AllocaInst* alloca);
   bool canAllocaBeReplaced(AllocaInst* alloca);
   void replaceUses(BasicBlock* alloca);
};

/* ===--------------------------------------------------------------------=== */
// HoistAlloca implementation
/* ===--------------------------------------------------------------------=== */

bool HoistAlloca::canAllocaBeReplaced(AllocaInst* alloca) {
   // 1. The allocas must be a scalar type
   if(!alloca->type()->isIntegerType() && !alloca->type()->isPointerType())
      return false;
   // 2. Each use must be a load or store instruction
   for(auto user : alloca->users()) {
      if(!dyn_cast<LoadInst>(user) && !dyn_cast<StoreInst>(user)) return false;
   }
   return true;
}

// Ref from paper, Figure 4. Placement of PHI-functions
void HoistAlloca::placePHINodes(AllocaInst* V) {
   std::unordered_set<BasicBlock*> DFPlus;
   std::unordered_set<BasicBlock*> Work;
   std::queue<BasicBlock*> W;
   // NOTE: A(V) = set of stores to V
   for(auto user : V->users()) {
      if(auto X = dyn_cast<StoreInst>(user)) {
         Work.insert(X->parent());
         W.push(X->parent());
      }
   }
   while(!W.empty()) {
      BasicBlock* X = W.front();
      W.pop();
      for(auto Y : DT->DF(X)) {
         if(DFPlus.contains(Y)) continue;
         auto phi = PhiNode::Create(V->ctx(), V->allocatedType(), {}, {});
         phi->setName("phi");
         Y->insertBeforeBegin(phi);
         phiAllocaMap[phi] = V;
         DFPlus.insert(Y);
         if(!Work.contains(Y)) {
            Work.insert(Y);
            W.push(Y);
         }
      }
   }
}

// Ref from paper, Figure 5. Construction of SSA form
void HoistAlloca::replaceUses(BasicBlock* X) {
   std::vector<Value*> pushed_vars;
   for(auto phi : X->phis()) {
      auto alloca = phiAllocaMap[phi];
      pushed_vars.push_back(alloca);
      varStack[alloca].push(phi);
   }
   for(auto inst : *X) {
      // If inst is an alloca, push it onto the stack and mark it as undef
      // and DO NOT add it to varStack[] (i.e., do not pop)
      if(auto alloca = dyn_cast<AllocaInst>(inst)) {
         varStack[alloca].push(
               Undef::Create(alloca->ctx(), alloca->allocatedType()));
         continue;
      }
      if(storesToRewrite.contains(inst)) { // "LHS"
         auto alloca = cast<AllocaInst>(inst->getChild(1));
         pushed_vars.push_back(alloca);
         varStack[alloca].push(inst->getChild(0));
      } else if(loadsToRewrite.contains(inst)) { // "RHS"
         auto alloca = cast<AllocaInst>(inst->getChild(0));
         auto newVar = varStack[alloca].top();
         inst->replaceAllUsesWith(newVar);
      }
   }
   for(auto Y : X->successors()) {
      for(auto phi : Y->phis()) {
         phi->replaceOrAddOperand(X, varStack[phiAllocaMap[phi]].top());
      }
   }
   for(auto Y : DT->getChildren(X)) {
      replaceUses(Y);
   }
   for(auto V : pushed_vars) varStack[V].pop();
}

} // namespace

/* ===--------------------------------------------------------------------=== */
// MemToReg pass wrapper
/* ===--------------------------------------------------------------------=== */

class MemToReg final : public Pass {
public:
   MemToReg(PassManager& PM) noexcept : Pass(PM) {}
   void Run() override {
      tir::CompilationUnit& CU = GetPass<IRContextPass>().CU();
      for(auto func : CU.functions()) {
         if(!func->hasBody() || !func->getEntryBlock()) continue;
         if(PM().Diag().Verbose()) {
            PM().Diag().ReportDebug()
                  << "*** Running on function: " << func->name() << " ***";
         }
         HoistAlloca hoister{func, PM().Diag()};
      }
   }
   string_view Name() const override { return "mem2reg"; }
   string_view Desc() const override { return "Promote memory to register"; }

private:
   void ComputeDependencies() override {
      AddDependency(GetPass<IRContextPass>());
   }
};

REGISTER_PASS(MemToReg);
