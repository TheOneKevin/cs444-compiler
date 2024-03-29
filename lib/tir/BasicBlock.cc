#include "tir/BasicBlock.h"

#include <sstream>

#include "tir/Constant.h"
#include "tir/Context.h"
#include "tir/Instructions.h"
#include "utils/DotPrinter.h"

namespace tir {

BasicBlock::BasicBlock(Context& ctx, Function* parent)
      : Value{ctx, Type::getLabelTy(ctx)},
        first_{nullptr},
        last_{nullptr},
        parent_{parent} {
   parent->addBlock(this);
   setName("bb");
}

void BasicBlock::iterator_pimpl::next() {
   // 1. The next instruction after beforeBegin is instr itself
   if(beforeBegin) {
      beforeBegin = false;
      return;
   }
   // 2. Fetch the next instruction
   auto* next = inst->next();
   // 3. If it does not exist, we've entered afterEnd
   if(next) {
      inst = next;
   } else {
      afterEnd = true;
   }
}

void BasicBlock::iterator_pimpl::prev() {
   // 1. The previous instruction before afterEnd is instr itself
   if(afterEnd) {
      afterEnd = false;
      return;
   }
   // 2. Fetch the previous instruction
   auto* prev = inst->prev();
   // 3. If it does not exist, we've entered beforeBegin
   if(prev) {
      inst = prev;
   } else {
      beforeBegin = true;
   }
}

void BasicBlock::appendAfterEnd(Instruction* instr) {
   if(last_) {
      instr->insertAfter(last_);
      last_ = instr;
   } else {
      first_ = last_ = instr;
   }
   instr->parent_ = this;
}

void BasicBlock::insertBeforeBegin(Instruction* instr) {
   if(first_) {
      instr->insertBefore(first_);
      first_ = instr;
   } else {
      first_ = last_ = instr;
   }
   instr->parent_ = this;
}

std::ostream& BasicBlock::print(std::ostream& os) const {
   printName(os) << ":";
   for(auto inst : *this) {
      os << "\n    ";
      inst->print(os);
   }
   return os;
}

void BasicBlock::erase(Instruction* instr) {
   assert(instr->parent_ == this && "Instruction does not belong to this BB");
   instr->eraseFromParent();
}

void BasicBlock::eraseFromParent() {
   if(parent_ == nullptr) return;
   parent_->removeBlock(this);
}

int BasicBlock::printDotNode(utils::DotPrinter& dp) const {
   auto id = dp.id();
   dp.startTLabel(id, {}, "1");
   std::ostringstream os;
   print(os) << "\n";
   dp.printTableSingleRow(os.str(), {}, true);
   if(auto term = dyn_cast_or_null<BranchInst>(terminator())) {
      if(term->getSuccessor(0) != term->getSuccessor(1))
         dp.printTableDoubleRow("T", "F", {"port", "T"}, {"port", "F"});
   }
   dp.endTLabel();
   return id;
}

utils::Generator<BasicBlock*> BasicBlock::successors() const {
   assert(terminator() && "Basic block has no terminator");
   auto term = dyn_cast<BranchInst>(terminator());
   if(term) {
      co_yield term->getSuccessor(0);
      if(term->getSuccessor(0) != term->getSuccessor(1)) {
         co_yield term->getSuccessor(1);
      }
   }
}

} // namespace tir
