#pragma once

#include "tir/TIR.h"

/**
 * @brief A class to compute and store the dominator tree and
 * dominance frontiers of a given function.
 * Citation: A Simple, Fast Dominance Algorithm
 * By: Keith D. Cooper, Timothy J. Harvey, and Ken Kennedy
 */
class DominatorTree {
public:
   DominatorTree(tir::Function*);
   // Print the dominator tree and dominance frontiers
   std::ostream& print(std::ostream& os) const;
   // Get the dominance frontier of block b
   auto& DF(tir::BasicBlock* b) { return frontiers[b]; }
   // Get the immediate dominator of block b, nullptr if it does not exist
   tir::BasicBlock* getIDom(tir::BasicBlock* b) {
      return doms.contains(b) ? doms.at(b) : nullptr;
   }
   // Get the children of the dominator tree
   auto& getChildren(tir::BasicBlock* b) { return domt[b]; }

private:
   tir::Function* func;
   std::unordered_map<tir::BasicBlock*, tir::BasicBlock*> doms;
   std::unordered_map<tir::BasicBlock*, int> poidx;
   std::unordered_map<tir::BasicBlock*, std::unordered_set<tir::BasicBlock*>>
         frontiers;
   std::unordered_map<tir::BasicBlock*, std::vector<tir::BasicBlock*>> domt;

   void computePostorderIdx(tir::Function* func);
   void computeDominators(tir::Function* func);
   tir::BasicBlock* intersect(tir::BasicBlock* b1, tir::BasicBlock* b2);
   void computeFrontiers(tir::Function* func);
};
