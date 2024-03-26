#pragma once

#include "tir/Value.h"

namespace tir {

class Instruction;
class Function;

class BasicBlock : public Value {
public:
   /**
    * @brief The iterator class for instructions in a basic block allows us to
    * express points before the first instruction and after the last instruction.
    * Otherwise, a pointer to the current instruction would be sufficient.
    *
    * Two iterators are equal iff they express the same point in the BB,
    * which means they must also be necessarily in the same BB.
    *
    * The increment and decrement moves to the previous and next iterators.
    * The minimum iterator is therefore the one before the first instruction,
    * and the maximum iterator is the one after the last instruction.
    */
   class iterator {
   private:
      friend class Instruction;
      friend class BasicBlock;
      iterator(Instruction* inst, BasicBlock* bb, bool isEnd, bool isBegin)
            : isEnd_{isEnd}, isBegin_{isBegin}, inst_{inst}, bb_{bb} {}

   public:
      iterator() : isEnd_{true}, isBegin_{false}, inst_{nullptr} {}

   public:
      Instruction* operator*() const { return inst_; }
      Instruction* operator->() const { return inst_; }
      iterator& operator++();
      iterator& operator--();
      bool operator==(const iterator& other) const {
         return inst_ == other.inst_ && isEnd_ == other.isEnd_ &&
                isBegin_ == other.isBegin_;
      }
      bool operator!=(const iterator& other) const { return !(*this == other); }
      BasicBlock* getBB() const { return bb_; }
      bool isBeforeFirst() const { return isBegin_; }
      bool isAfterLast() const { return isEnd_; }

   private:
      // True if this iterator is after the last instruction in the list
      bool isEnd_;
      // True if this iterator is before the first instruction in the list
      bool isBegin_;
      // The current instruction, this is never nullptr unless list is empty
      Instruction* inst_;
      // The parent basic block, in case this iterator is nullptr
      BasicBlock* bb_;
   };

private:
   BasicBlock(Context& ctx, Function* parent);

public:
   static BasicBlock* Create(Context& ctx, Function* parent) {
      auto* buf =
            ctx.alloc().allocate_bytes(sizeof(BasicBlock), alignof(BasicBlock));
      return new(buf) BasicBlock{ctx, parent};
   }
   // Gets the parent function of this basic block
   auto* parent() const { return parent_; }
   // Gets the begin iterator for the instructions in this basic block
   iterator begin() { return iterator{first_, this, first_ == nullptr, false}; }
   // Gets the end iterator for the instructions in this basic block
   iterator end() { return iterator{last_, this, true, false}; }
   // Append an instruction to the end of the basic block
   void appendAfterEnd(Instruction* instr);
   // Insert an instruction before the first instruction in the basic block
   void insertBeforeBegin(Instruction* instr);
private:
   Instruction* first_;
   Instruction* last_;
   Function* parent_;
};

} // namespace tir
