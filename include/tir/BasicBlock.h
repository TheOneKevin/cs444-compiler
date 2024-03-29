#pragma once

#include "tir/Value.h"
#include "utils/DotPrinter.h"
#include "utils/Generator.h"

namespace tir {

class Instruction;
class Function;

class BasicBlock final : public Value {
   friend class Instruction;

private:
   // Private implementation of the iterator
   struct iterator_pimpl {
      // True if this iterator is after the last instruction in the list
      bool afterEnd;
      // True if this iterator is before the first instruction in the list
      bool beforeBegin;
      // The current instruction, this is never nullptr unless list is empty
      Instruction* inst;
      void next();
      void prev();
      bool equal(const iterator_pimpl& other) const {
         return afterEnd == other.afterEnd && beforeBegin == other.beforeBegin &&
                inst == other.inst;
      }
   };

   // The iterator template to support both const and non-const iterators
   template <typename T>
   class iterator_impl {
   private:
      friend class Instruction;
      friend class BasicBlock;
      iterator_impl(Instruction* inst, T bb, bool afterEnd, bool beforeBegin)
            : pimpl_{afterEnd, beforeBegin, inst}, bb_{bb} {}

   public:
      iterator_impl() : pimpl_{true, false, nullptr} {}
      Instruction* operator*() const {
         assert(!pimpl_.beforeBegin && !pimpl_.afterEnd);
         return pimpl_.inst;
      }
      Instruction* operator->() const {
         assert(!pimpl_.beforeBegin && !pimpl_.afterEnd);
         return pimpl_.inst;
      }
      iterator_impl& operator++() {
         pimpl_.next();
         return *this;
      }
      iterator_impl& operator--() {
         pimpl_.prev();
         return *this;
      }
      bool operator==(const iterator_impl& other) const {
         return pimpl_.equal(other.pimpl_);
      }
      bool operator!=(const iterator_impl& other) const {
         return !(*this == other);
      }
      T getBB() const { return bb_; }
      // Does this iterator point before the first instruction?
      bool isBeforeFirst() const { return pimpl_.beforeBegin; }
      // Does this iterator point past the end of the list?
      bool isAfterLast() const { return pimpl_.afterEnd; }

   private:
      // Private implementation of the iterator
      iterator_pimpl pimpl_;
      // The parent basic block, in case this iterator is nullptr
      T bb_;
   };

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
   using iterator = iterator_impl<BasicBlock*>;

   // See BasicBlock::iterator
   using const_iterator = iterator_impl<const BasicBlock*>;

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
   // Gets an iterator to the first instruction of the block (if not empty)
   auto begin() { return iterator{first_, this, first_ == nullptr, false}; }
   // Gets an iterator to AFTER the last instruction of the block
   auto end() { return iterator{last_, this, true, false}; }
   // Const iterator
   auto begin() const {
      return const_iterator{first_, this, first_ == nullptr, false};
   }
   // Const iterator
   auto end() const { return const_iterator{last_, this, true, false}; }
   // Append an instruction to the end of the basic block
   void appendAfterEnd(Instruction* instr);
   // Insert an instruction before the first instruction in the basic block
   void insertBeforeBegin(Instruction* instr);
   // Print the basic block to the given output stream
   std::ostream& print(std::ostream& os) const override;
   // Get the terminator instruction of this basic block
   auto* terminator() const { return last_; }
   // Erase an instruction from the basic block
   void erase(Instruction* instr);
   // Erases this basic block from the parent function
   void eraseFromParent();
   // Prints the DOT representation of this basic block
   int printDotNode(utils::DotPrinter&) const;
   // Grab the sucessor basic blocks of this basic block
   utils::Generator<BasicBlock*> successors() const;

private:
   Instruction* first_;
   Instruction* last_;
   Function* parent_;
};

} // namespace tir
