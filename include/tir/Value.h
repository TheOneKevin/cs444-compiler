#pragma once

#include <utils/Assert.h>

#include <optional>
#include <ostream>
#include <ranges>
#include <string_view>
#include <unordered_set>

#include "tir/Context.h"

namespace tir {

class User;

/**
 * @brief
 */
class Value {
public:
   Value(Context& ctx, Type* type)
         : ctx_{ctx},
           type_{type},
           users_{ctx.alloc()},
           name_{std::nullopt},
           valueID_{ctx.getNextValueID()} {}
   tir::Context& ctx() { return ctx_; }
   auto users() { return std::views::all(users_); }
   Type* type() const { return type_; }
   void addUser(User* user) { users_.insert(user); }
   void removeUser(User* user) { users_.erase(user); }
   std::string_view name() const { return name_.value(); }
   auto nameOpt() const { return name_.value(); }
   void setName(std::string_view name) {
      name_ = std::pmr::string{name, ctx_.alloc()};
   }
   std::ostream& printName(std::ostream& os) const {
      os << "%";
      if(name_) os << name_.value() << ".";
      os << valueID_;
      return os;
   }
   virtual std::ostream& print(std::ostream&) const = 0;
   void dump() const;
   virtual ~Value() = default;

private:
   tir::Context& ctx_;
   tir::Type* const type_;
   std::pmr::unordered_set<User*> users_;
   std::optional<std::pmr::string> name_;
   unsigned valueID_;
};

/**
 * @brief
 */
class User : public Value {
public:
   User(Context& ctx, Type* type) : Value{ctx, type}, children_{ctx.alloc()} {}
   auto children() const { return std::views::all(children_); }
   auto numChildren() const { return children_.size(); }
   Value* getChild(unsigned idx) const {
      assert(idx < numChildren() && "Index out of bounds");
      return children_[idx];
   }

protected:
   void addChild(Value* operand) {
      children_.push_back(operand);
      operand->addUser(this);
   }
   void replaceChild(unsigned idx, Value* operand) {
      assert(idx < numChildren() && "Index out of bounds");
      children_[idx]->removeUser(this);
      children_[idx] = operand;
      operand->addUser(this);
   }
   void destroy() {
      assert(!destroyed_);
      for(auto child : children_) {
         child->removeUser(this);
      }
   }
   bool isDestroyed() const { return destroyed_; }

private:
   std::pmr::vector<Value*> children_;
   bool destroyed_ = false;
};

std::ostream& operator<<(std::ostream& os, const Value& val);

} // namespace tir
