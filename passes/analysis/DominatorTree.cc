#include "DominatorTree.h"

#include "diagnostics/Diagnostics.h"
#include "tir/BasicBlock.h"
#include "tir/Constant.h"
#include "tir/Instructions.h"
#include "utils/BumpAllocator.h"
#include "utils/PassManager.h"

using std::string_view;
using DE = diagnostics::DiagnosticEngine;
using namespace tir;
using namespace analysis;

/* ===--------------------------------------------------------------------=== */
// DominatorTree implementation
/* ===--------------------------------------------------------------------=== */

DominatorTree::DominatorTree(tir::Function* func, BumpAllocator& alloc)
      : func{func}, alloc{alloc} {
   computePostorderIdx(func);
   computeDominators(func);
   computeFrontiers(func);
   for(auto bb : func->body())
      if(doms[bb] != bb) domt[doms[bb]].push_back(bb);
}

std::ostream& DominatorTree::print(std::ostream& os) const {
   // Print the dominator tree
   os << "*** Dominator Tree ***\n";
   for(auto b : func->body()) {
      if(doms.contains(b)) {
         os << "  Dom(";
         b->printName(os) << ") = ";
         doms.at(b)->printName(os) << std::endl;
      }
   }
   // Print the dominance frontier
   os << "*** Dominance Frontier ***\n";
   for(auto [b, frontier] : frontiers) {
      os << "  DF(";
      b->printName(os) << ") = {";
      bool first = true;
      for(auto f : frontier) {
         if(!first) os << ", ";
         first = false;
         f->printName(os);
      }
      os << "}\n";
   }
   return os;
}

void DominatorTree::computePostorderIdx(Function* func) {
   int i = 0;
   for(auto b : func->reversePostOrder()) poidx[b] = i++;
}

void DominatorTree::computeDominators(Function* func) {
   doms[func->getEntryBlock()] = func->getEntryBlock();
   bool changed = true;
   while(changed) {
      changed = false;
      for(auto b : func->reversePostOrder()) {
         BasicBlock* newIdom = nullptr;
         for(auto pred : b->predecessors()) {
            if(doms.contains(pred)) {
               if(!newIdom) {
                  newIdom = pred;
               } else {
                  newIdom = intersect(pred, newIdom);
               }
            }
         }
         if(!newIdom) continue;
         if(!doms.contains(b) || doms[b] != newIdom) {
            doms[b] = newIdom;
            changed = true;
         }
      }
   }
}

tir::BasicBlock* DominatorTree::intersect(tir::BasicBlock* b1,
                                          tir::BasicBlock* b2) {
   auto* finger1 = b1;
   auto* finger2 = b2;
   while(finger1 != finger2) {
      while(poidx[finger1] > poidx[finger2]) finger1 = doms[finger1];
      while(poidx[finger2] > poidx[finger1]) finger2 = doms[finger2];
   }
   return finger1;
}

void DominatorTree::computeFrontiers(Function* func) {
   for(auto b : func->body()) {
      int numPreds = 0;
      for(auto p : b->predecessors()) {
         (void)p;
         numPreds++;
      }
      if(numPreds < 2) continue;
      for(auto pred : b->predecessors()) {
         BasicBlock* runner = pred;
         while(runner != doms[b]) {
            frontiers[runner].insert(b);
            runner = doms[runner];
         }
      }
   }
}

REGISTER_PASS_NS(passes, DominatorTreeWrapper);
