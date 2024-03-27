#pragma once

#include <string_view>

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
   static ConstantInt* CreateBool(Context& ctx, bool value);
   static ConstantInt* CreateInt32(Context& ctx, uint32_t value);
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
      auto* buf =
            ctx.alloc().allocate_bytes(sizeof(ConstantInt), alignof(ConstantInt));
      return new(buf) ConstantInt{ctx, type, value};
   }

public:
   std::ostream& print(std::ostream& os) const override;
   uint64_t value() const { return value_; }
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
   ConstantNullPointer(Context& ctx) : Constant{ctx, Type::getPointerTy(ctx)} {}

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
};

/**
 * @brief
 */
class GlobalVariable final : public GlobalObject {
public:
   std::ostream& print(std::ostream& os) const override;
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
   BasicBlock* getEntryBlock() const {
      if(body_.empty()) return nullptr;
      return body_.front();
   }
   auto body() const { return std::views::all(body_); }

private:
   void addBlock(BasicBlock* block) { body_.push_back(block); }

private:
   std::pmr::vector<BasicBlock*> body_;
   tir::CompilationUnit* parent_;
};

} // namespace tir
