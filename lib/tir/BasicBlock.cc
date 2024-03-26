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
   setName("bb");
}

void BasicBlock::iterator_pimpl::next() {
   isBegin = false;
   auto* next = inst->next();
   if(next) {
      inst = next;
   } else {
      isEnd = true;
   }
}

void BasicBlock::iterator_pimpl::prev() {
   isEnd = false;
   auto* prev = inst->prev();
   if(prev) {
      inst = prev;
   } else {
      isBegin = true;
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

} // namespace tir
