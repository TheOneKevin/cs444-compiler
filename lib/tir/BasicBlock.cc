#include "tir/BasicBlock.h"

#include "tir/Constant.h"
#include "tir/Context.h"
#include "tir/Instructions.h"

namespace tir {

BasicBlock::BasicBlock(Context& ctx, Function* parent)
      : Value{ctx, Type::getLabelTy(ctx)},
        first_{nullptr},
        last_{nullptr},
        parent_{parent} {
   parent->addBlock(this);
}

BasicBlock::iterator& BasicBlock::iterator::operator++() {
   isBegin_ = false;
   auto* next = inst_->next();
   if(next) {
      inst_ = next;
   } else {
      isEnd_ = true;
   }
   return *this;
}

BasicBlock::iterator& BasicBlock::iterator::operator--() {
   isEnd_ = false;
   auto* prev = inst_->prev();
   if(prev) {
      inst_ = prev;
   } else {
      isBegin_ = true;
   }
   return *this;
}

void BasicBlock::appendAfterEnd(Instruction* instr) {
   if(last_) {
      instr->insertAfter(last_);
      last_ = instr;
   } else {
      first_ = last_ = instr;
   }
}

void BasicBlock::insertBeforeBegin(Instruction* instr) {
   if(first_) {
      instr->insertBefore(first_);
      first_ = instr;
   } else {
      first_ = last_ = instr;
   }
}

} // namespace tir
