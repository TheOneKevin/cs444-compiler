#pragma once

#include <forward_list>
#include <string_view>

#include "tir/Instructions.h"
#include "tir/Type.h"
#include "tir/Value.h"
#include "utils/Utils.h"

namespace tir {

class BasicBlock;
class CompilationUnit;
class Function;
class ConstantInt;

/**
 * @brief
 */
class Constant : public User {
protected:
   Constant(Context& ctx, Type* type) : User{ctx, type} {}
   virtual bool isNullPointer() const { return false; }
   virtual bool isNumeric() const { return false; }
   virtual bool isBoolean() const { return false; }

public:
   static ConstantInt* CreateInt(Context& ctx, uint8_t bits, uint32_t value);
   static ConstantInt* CreateBool(Context& ctx, bool value) {
      return CreateInt(ctx, 1, value ? 1 : 0);
   }
   static ConstantInt* CreateInt32(Context& ctx, uint32_t value) {
      return CreateInt(ctx, 32, value);
   }
   static ConstantNullPointer* CreateNullPointer(Context& ctx);
};

/**
 * @brief
 */
class ConstantInt final : public Constant {
private:
   ConstantInt(Context& ctx, Type* type, uint64_t value)
         : Constant{ctx, type}, value_{value} {}

public:
   static ConstantInt* Create(Context& ctx, Type* type, uint64_t value) {
      assert(type->isIntegerType() && "Type must be an integer type");
      auto* buf =
            ctx.alloc().allocate_bytes(sizeof(ConstantInt), alignof(ConstantInt));
      return new(buf) ConstantInt{ctx, type, value};
   }
   static ConstantInt* AllOnes(Context& ctx, Type* type) {
      return Create(ctx, type, ~0ULL);
   }
   static ConstantInt* Zero(Context& ctx, Type* type) {
      return Create(ctx, type, 0);
   }

public:
   std::ostream& print(std::ostream& os) const override;
   uint64_t zextValue() const {
      return value_ & cast<IntegerType>(type())->getMask();
   }
   uint64_t sextValue() const {
      auto mask = cast<IntegerType>(type())->getMask();
      return (value_ & mask) |
             ((value_ & (1ULL << (type()->getSizeInBits() - 1))) ? ~mask : 0);
   }
   bool isNumeric() const override { return true; }
   bool isBoolean() const override { return type()->isBooleanType(); }

private:
   uint64_t value_;
};

/**
 * @brief
 */
class ConstantNullPointer final : public Constant {
private:
   friend class Context;
   ConstantNullPointer(Context& ctx, Type* ty) : Constant{ctx, ty} {}

public:
   static ConstantNullPointer* Create(Context& ctx) {
      return ctx.pimpl().nullPointer;
   }

public:
   std::ostream& print(std::ostream& os) const override;
   bool isNullPointer() const override { return true; }
};

/**
 * @brief
 */
class GlobalObject : public Constant {
protected:
   GlobalObject(Context& ctx, Type* type) : Constant{ctx, type} {}

public:
   virtual bool isExternalLinkage() const = 0;
};

/**
 * @brief
 */
class GlobalVariable final : public GlobalObject {
   friend class CompilationUnit;

private:
   GlobalVariable(Context& ctx, Type* type) : GlobalObject{ctx, type} {}

public:
   std::ostream& print(std::ostream& os) const override;
   bool isExternalLinkage() const override { return false; }
};

/**
 * @brief
 */
class Argument final : public Value {
   friend class Function;

private:
   Argument(Function* parent, Type* type, unsigned index);

public:
   std::ostream& print(std::ostream& os) const override;
   auto* parent() const { return parent_; }
   auto index() const { return index_; }

private:
   Function* parent_;
   unsigned index_;
};

/**
 * @brief
 */
class Function final : public GlobalObject {
   friend class CompilationUnit;
   friend class BasicBlock;

private:
   Function(Context& ctx, CompilationUnit* parent, FunctionType* type,
            const std::string_view name)
         : GlobalObject{ctx, type}, body_{ctx.alloc()}, parent_{parent} {
      // Set name of the function
      setName(name);
      // Build the argument list
      for(unsigned i = 0; i < type->numParams(); ++i) {
         auto* buf =
               ctx.alloc().allocate_bytes(sizeof(Argument), alignof(Argument));
         auto* arg = new(buf) Argument{this, type->getParamType(i), i};
         addChild(arg);
      }
   }

public:
   std::ostream& print(std::ostream& os) const override;
   auto* parent() const { return parent_; }
   auto args() const {
      return std::views::transform(
            children(), [](Value* val) { return static_cast<Argument*>(val); });
   }
   auto numParams() const { return cast<FunctionType>(type())->numParams(); }
   auto getParamType(int index) const {
      return cast<FunctionType>(type())->getParamType(index);
   }
   auto getReturnType() const {
      return cast<FunctionType>(type())->getReturnType();
   }
   bool hasBody() const { return !body_.empty(); }
   /// @brief Gets the entry BB of the function or nullptr if it doesn't exist.
   BasicBlock* getEntryBlock() const { return entryBB_; }
   auto body() const { return std::views::all(body_); }
   void removeBlock(BasicBlock* block) {
      assert(block->parent() == this && "Block does not belong to this function");
      assert(block != entryBB_ && "Cannot remove the entry block");
      body_.remove(block);
   }

   /**
    * @brief Create an alloca instruction for the given type. This will be
    * at the start of the entry BB of the function. If no entry BB exists,
    * this will fail.
    *
    * @param type The type of the alloca instruction.
    * @return AllocaInst* The alloca instruction created.
    */
   AllocaInst* createAlloca(Type* type) {
      assert(getEntryBlock());
      auto* inst = AllocaInst::Create(ctx(), type);
      getEntryBlock()->insertBeforeBegin(inst);
      return inst;
   }

   auto isNoReturn() const { return noReturn_; }
   void setNoReturn() { noReturn_ = true; }
   void setExternalLinkage() { external_ = true; }
   bool isExternalLinkage() const override {
      if(!external_) return !entryBB_;
      return true;
   }
   void printDot(std::ostream& os) const;

private:
   void addBlock(BasicBlock* block) {
      if(!entryBB_) entryBB_ = block;
      body_.push_front(block);
   }

private:
   std::pmr::forward_list<BasicBlock*> body_;
   BasicBlock* entryBB_ = nullptr;
   tir::CompilationUnit* parent_;
   bool noReturn_ = false;
   bool external_ = false;
};

} // namespace tir
