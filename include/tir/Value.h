#pragma once

#include <utils/Assert.h>

#include <optional>
#include <ostream>
#include <ranges>
#include <string_view>
#include <unordered_set>

#include "tir/Context.h"
#include "utils/Generator.h"

namespace tir {

class User;

struct Use {
   User* const user;
   const unsigned idx;
};

namespace detail {
struct UseEqual {
   bool operator()(const Use& lhs, const Use& rhs) const {
      return lhs.user == rhs.user && lhs.idx == rhs.idx;
   }
};
} // namespace detail

} // namespace tir

template <>
struct std::hash<tir::Use> {
   std::size_t operator()(const tir::Use& use) const {
      return std::hash<tir::User*>{}(use.user) ^ std::hash<unsigned>{}(use.idx);
   }
};

namespace tir {

/**
 * @brief
 */
class Value {
public:
   Value(Context& ctx, Type* type)
         : ctx_{ctx},
           type_{type},
           uses_{ctx.alloc()},
           name_{std::nullopt},
           valueID_{ctx.getNextValueID()} {}
   tir::Context& ctx() { return ctx_; }
   utils::Generator<User*> users() {
      for(auto user : uses_) co_yield user.user;
   }
   utils::Generator<User const*> users() const {
      for(auto use : uses_) co_yield use.user;
   }
   auto uses() const { return std::views::all(uses_); }
   Type* type() const { return type_; }
   void addUse(Use use) { uses_.insert(use); }
   void removeUse(Use use) { uses_.erase(use); }
   std::string_view name() const { return name_.value(); }
   auto nameOpt() const { return name_.value(); }
   void replaceAllUsesWith(Value* newValue);
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
   std::pmr::unordered_set<Use, std::hash<Use>, detail::UseEqual> uses_;
   std::optional<std::pmr::string> name_;
   unsigned valueID_;
};

/**
 * @brief
 */
class User : public Value {
   friend class Value;

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
      operand->addUse({this, (unsigned)numChildren() - 1});
   }
   void addChild(Value* operand, unsigned idx) {
      children_.insert(children_.begin() + idx, operand);
      operand->addUse({this, idx});
   }
   void replaceChild(unsigned idx, Value* operand) {
      assert(idx < numChildren() && "Index out of bounds");
      children_[idx]->removeUse({this, idx});
      children_[idx] = operand;
      operand->addUse({this, idx});
   }
   void destroy() {
      assert(!destroyed_);
      unsigned idx = 0;
      for(auto child : children_) {
         child->removeUse({this, idx++});
      }
   }
   bool isDestroyed() const { return destroyed_; }

private:
   std::pmr::vector<Value*> children_;
   bool destroyed_ = false;
};

std::ostream& operator<<(std::ostream& os, const Value& val);

} // namespace tir
