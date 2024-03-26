#pragma once

#include "tir/BasicBlock.h"
#include "tir/Context.h"
#include "tir/Type.h"
#include "tir/Value.h"
#include "utils/EnumMacros.h"
#include "utils/Utils.h"

namespace tir {

/* ===--------------------------------------------------------------------=== */
// Instruction base class
/* ===--------------------------------------------------------------------=== */

/**
 * @brief
 */
class Instruction : public User {
   // Public enum definitions //////////////////////////////////////////////////
public:
#define BINOP_KINDS(F) \
   F(NONE)             \
   F(ADD)              \
   F(SUB)              \
   F(MUL)              \
   F(DIV)              \
   F(REM)
   DECLARE_ENUM(BinOp, BINOP_KINDS)
private:
   DECLARE_STRING_TABLE(BinOp, binop_strtab_, BINOP_KINDS)
#undef BINOP_KINDS

   // Constructors and public member functions //////////////////////////////////
public:
   Instruction(Context& ctx, tir::Type* resultTy)
         : Instruction{ctx, resultTy, BinOp::NONE} {}
   Instruction(Context& ctx, tir::Type* resultTy, BinOp op)
         : User{ctx, resultTy},
           next_{nullptr},
           prev_{nullptr},
           binop_{op},
           parent_{nullptr} {}
   Instruction(const Instruction&) = delete;
   Instruction(Instruction&&) = delete;
   Instruction& operator=(const Instruction&) = delete;
   Instruction& operator=(Instruction&&) = delete;
   // Returns true if this instruction is a terminator instruction
   virtual bool isTerminator() const { return false; }
   // Inserts this before the given instruction, setting the parent of this
   void insertBefore(Instruction* inst) {
      prev_ = inst->prev_;
      next_ = inst;
      if(prev_) {
         prev_->next_ = this;
      }
      inst->prev_ = this;
      parent_ = inst->parent_;
   }
   // Inserts this after the given instruction, setting the parent of this
   void insertAfter(Instruction* inst) {
      next_ = inst->next_;
      prev_ = inst;
      if(next_) {
         next_->prev_ = this;
      }
      inst->next_ = this;
      parent_ = inst->parent_;
   }
   // Gets the parent BB of this instruction, or nullptr if it has no parent
   auto* parent() const { return parent_; }
   // Gets the next instruction in the BB, or nullptr if this is the last
   Instruction* next() const { return next_; }
   // Gets the previous instruction in the BB, or nullptr if this is the first
   Instruction* prev() const { return prev_; }
   // Gets the iterator to the next instruction in the BB
   auto nextIter() {
      return BasicBlock::iterator{next_ ? next_ : this, this->parent_, !next_, false};
   }
   // Gets the iterator to the previous instruction in the BB
   auto prevIter() {
      return BasicBlock::iterator{prev_ ? prev_ : this, this->parent_, false, !prev_};
   }
   // Gets the current iterator to this instruction
   auto iter() {
      return BasicBlock::iterator{this, this->parent_, false, false};
   }

   // Private data members /////////////////////////////////////////////////////
private:
   Instruction* next_;
   Instruction* prev_;
   const BinOp binop_;
   BasicBlock* parent_;
};

/* ===--------------------------------------------------------------------=== */
// Terminal instructions
/* ===--------------------------------------------------------------------=== */

/**
 * @brief
 */
class BranchInst : public Instruction {
private:
   BranchInst(Context& ctx, Value* cond, BasicBlock* trueBB, BasicBlock* falseBB);

public:
   static BranchInst* Create(Context& ctx, Value* cond, BasicBlock* trueBB,
                             BasicBlock* falseBB) {
      auto buf =
            ctx.alloc().allocate_bytes(sizeof(BranchInst), alignof(BranchInst));
      return new(buf) BranchInst{ctx, cond, trueBB, falseBB};
   }
};

/**
 * @brief
 */
class ReturnInst : public Instruction {
private:
   ReturnInst(Context& ctx, Value* ret);

public:
   static ReturnInst* Create(Context& ctx, Value* ret) {
      auto buf =
            ctx.alloc().allocate_bytes(sizeof(ReturnInst), alignof(ReturnInst));
      return new(buf) ReturnInst{ctx, ret};
   }

   bool isReturnVoid() const { return numChildren() == 0; }
};

/* ===--------------------------------------------------------------------=== */
// Memory instructions
/* ===--------------------------------------------------------------------=== */

/**
 * @brief
 */
class StoreInst : public Instruction {
private:
   StoreInst(Context& ctx, Value* val, Value* ptr);

public:
   static StoreInst* Create(Context& ctx, Value* val, Value* ptr) {
      auto buf = ctx.alloc().allocate_bytes(sizeof(StoreInst), alignof(StoreInst));
      return new(buf) StoreInst{ctx, val, ptr};
   }
};

/**
 * @brief
 */
class LoadInst : public Instruction {
private:
   LoadInst(Context& ctx, Type* type, Value* ptr);

public:
   static LoadInst* Create(Context& ctx, Type* type, Value* ptr) {
      auto buf = ctx.alloc().allocate_bytes(sizeof(LoadInst), alignof(LoadInst));
      return new(buf) LoadInst{ctx, type, ptr};
   }
};

/* ===--------------------------------------------------------------------=== */
// Function call instruction
/* ===--------------------------------------------------------------------=== */

/**
 * @brief
 */
class CallInst : public Instruction {
private:
   CallInst(Context& ctx, Value* callee, utils::range_ref<Value*> args);

public:
   static CallInst* Create(Context& ctx, Value* callee,
                           utils::range_ref<Value*> args) {
      auto buf = ctx.alloc().allocate_bytes(sizeof(CallInst), alignof(CallInst));
      return new(buf) CallInst{ctx, callee, args};
   }
};

/* ===--------------------------------------------------------------------=== */
// Binary operators
/* ===--------------------------------------------------------------------=== */

/**
 * @brief
 */
class BinaryInst : public Instruction {
private:
   BinaryInst(Context& ctx, BinOp binop, Value* lhs, Value* rhs);

public:
   static BinaryInst* Create(Context& ctx, BinOp binop, Value* lhs, Value* rhs) {
      auto buf =
            ctx.alloc().allocate_bytes(sizeof(BinaryInst), alignof(BinaryInst));
      return new(buf) BinaryInst{ctx, binop, lhs, rhs};
   }
};

} // namespace tir
