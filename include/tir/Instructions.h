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
   friend class BasicBlock;
   // Public enum definitions //////////////////////////////////////////////////
public:
#define BINOP_KINDS(F) \
   F(None)             \
   F(Add)              \
   F(Sub)              \
   F(Mul)              \
   F(Div)              \
   F(Rem)              \
   F(And)              \
   F(Or)               \
   F(Xor)
   DECLARE_ENUM(BinOp, BINOP_KINDS)
protected:
   DECLARE_STRING_TABLE(BinOp, binop_strtab_, BINOP_KINDS)
#undef BINOP_KINDS

   // Constructors and public member functions //////////////////////////////////
public:
   Instruction(Context& ctx, tir::Type* resultTy)
         : Instruction{ctx, resultTy, BinOp::None} {}
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
      // If this is the first instruction in the BB, update the BB's first pointer
      if(!prev_ && parent_) {
         parent_->first_ = this;
      }
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
      // If this is the last instruction in the BB, update the BB's last pointer
      if(!next_ && parent_) {
         parent_->last_ = this;
      }
   }
   // Gets the parent BB of this instruction, or nullptr if it has no parent
   auto* parent() const { return parent_; }
   // Gets the next instruction in the BB, or nullptr if this is the last
   Instruction* next() const { return next_; }
   // Gets the previous instruction in the BB, or nullptr if this is the first
   Instruction* prev() const { return prev_; }
   // Gets the iterator to the next instruction in the BB
   auto nextIter() {
      return BasicBlock::iterator{
            next_ ? next_ : this, this->parent_, !next_, false};
   }
   // Gets the iterator to the previous instruction in the BB
   auto prevIter() {
      return BasicBlock::iterator{
            prev_ ? prev_ : this, this->parent_, false, !prev_};
   }
   // Gets the current iterator to this instruction
   auto iter() { return BasicBlock::iterator{this, this->parent_, false, false}; }
   // Gets the binary operator of this instruction
   auto binop() const { return binop_; }

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
class BranchInst final : public Instruction {
private:
   BranchInst(Context& ctx, Value* cond, BasicBlock* trueBB, BasicBlock* falseBB);

public:
   static BranchInst* Create(Context& ctx, Value* cond, BasicBlock* trueBB,
                             BasicBlock* falseBB) {
      auto buf =
            ctx.alloc().allocate_bytes(sizeof(BranchInst), alignof(BranchInst));
      return new(buf) BranchInst{ctx, cond, trueBB, falseBB};
   }

public:
   bool isTerminator() const override { return true; }
   std::ostream& print(std::ostream& os) const override;
};

/**
 * @brief
 */
class ReturnInst final : public Instruction {
private:
   ReturnInst(Context& ctx, Value* ret);

public:
   static ReturnInst* Create(Context& ctx, Value* ret) {
      auto buf =
            ctx.alloc().allocate_bytes(sizeof(ReturnInst), alignof(ReturnInst));
      return new(buf) ReturnInst{ctx, ret};
   }

public:
   bool isReturnVoid() const { return numChildren() == 0; }
   bool isTerminator() const override { return true; }
   std::ostream& print(std::ostream& os) const override;
};

/* ===--------------------------------------------------------------------=== */
// Memory instructions
/* ===--------------------------------------------------------------------=== */

/**
 * @brief
 */
class StoreInst final : public Instruction {
private:
   StoreInst(Context& ctx, Value* val, Value* ptr);

public:
   static StoreInst* Create(Context& ctx, Value* val, Value* ptr) {
      auto buf = ctx.alloc().allocate_bytes(sizeof(StoreInst), alignof(StoreInst));
      return new(buf) StoreInst{ctx, val, ptr};
   }

public:
   std::ostream& print(std::ostream& os) const override;
};

/**
 * @brief
 */
class LoadInst final : public Instruction {
private:
   LoadInst(Context& ctx, Type* type, Value* ptr);

public:
   static LoadInst* Create(Context& ctx, Type* type, Value* ptr) {
      auto buf = ctx.alloc().allocate_bytes(sizeof(LoadInst), alignof(LoadInst));
      return new(buf) LoadInst{ctx, type, ptr};
   }

public:
   std::ostream& print(std::ostream& os) const override;
};

/* ===--------------------------------------------------------------------=== */
// Function call instruction
/* ===--------------------------------------------------------------------=== */

/**
 * @brief
 */
class CallInst final : public Instruction {
private:
   CallInst(Context& ctx, Value* callee, utils::range_ref<Value*> args);

public:
   static CallInst* Create(Context& ctx, Value* callee,
                           utils::range_ref<Value*> args) {
      auto buf = ctx.alloc().allocate_bytes(sizeof(CallInst), alignof(CallInst));
      return new(buf) CallInst{ctx, callee, args};
   }

public:
   std::ostream& print(std::ostream& os) const override;
};

/* ===--------------------------------------------------------------------=== */
// Binary operators
/* ===--------------------------------------------------------------------=== */

/**
 * @brief
 */
class BinaryInst final : public Instruction {
private:
   BinaryInst(Context& ctx, BinOp binop, Value* lhs, Value* rhs);

public:
   static BinaryInst* Create(Context& ctx, BinOp binop, Value* lhs, Value* rhs) {
      auto buf =
            ctx.alloc().allocate_bytes(sizeof(BinaryInst), alignof(BinaryInst));
      return new(buf) BinaryInst{ctx, binop, lhs, rhs};
   }

public:
   std::ostream& print(std::ostream& os) const override;
};

class CmpInst final : public Instruction {
public:
#define PREDICATE_KINDS(F) \
   F(EQ)                   \
   F(NE)                   \
   F(LT)                   \
   F(GT)                   \
   F(LE)                   \
   F(GE)
   DECLARE_ENUM(Predicate, PREDICATE_KINDS)
private:
   DECLARE_STRING_TABLE(Predicate, predicate_strtab_, PREDICATE_KINDS)
#undef PREDICATE_KINDS

private:
   CmpInst(Context& ctx, Predicate pred, Value* lhs, Value* rhs);

public:
   static CmpInst* Create(Context& ctx, Predicate pred, Value* lhs, Value* rhs) {
      auto buf = ctx.alloc().allocate_bytes(sizeof(CmpInst), alignof(CmpInst));
      return new(buf) CmpInst{ctx, pred, lhs, rhs};
   }

public:
   std::ostream& print(std::ostream& os) const override;
   Predicate predicate() const { return pred_; }

private:
   Predicate pred_;
};

/* ===--------------------------------------------------------------------=== */
// Alloca instruction
/* ===--------------------------------------------------------------------=== */

/**
 * @brief
 */
class AllocaInst final : public Instruction {
private:
   AllocaInst(Context& ctx, Type* type);

public:
   static AllocaInst* Create(Context& ctx, Type* type) {
      auto buf =
            ctx.alloc().allocate_bytes(sizeof(AllocaInst), alignof(AllocaInst));
      return new(buf) AllocaInst{ctx, type};
   }

public:
   std::ostream& print(std::ostream& os) const override;

private:
   Type* allocType_;
};

} // namespace tir
