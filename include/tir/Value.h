#pragma once

#include <optional>
#include <ranges>
#include <string_view>
#include <vector>

#include "tir/Context.h"
#include "tir/Type.h"

namespace tir {

class User;

/**
 * @brief
 */
class Value {
public:
   Value(Context& ctx, Type* type)
         : ctx_{ctx}, type_{type}, users_{ctx.alloc()}, name_{std::nullopt} {}
   tir::Context& ctx() { return ctx_; }
   auto users() { return std::views::all(users_); }
   Type* type() const { return type_; }
   void addUser(User* user) { users_.push_back(user); }
   std::string_view name() const { return name_.value(); }
   auto nameOpt() const { return name_.value(); }
   void setName(std::string_view name) {
      name_ = std::pmr::string{name, ctx_.alloc()};
   }

private:
   tir::Context& ctx_;
   tir::Type* const type_;
   std::pmr::vector<User*> users_;
   std::optional<std::pmr::string> name_;
};

/**
 * @brief
 */
class User : public Value {
public:
   User(Context& ctx, Type* type) : Value{ctx, type}, children_{ctx.alloc()} {}
   auto children() const { return std::views::all(children_); }
   auto numChildren() const { return children_.size(); }

protected:
   void addChild(Value* operand) {
      children_.push_back(operand);
      operand->addUser(this);
   }

private:
   std::pmr::vector<Value*> children_;
};

} // namespace tir
