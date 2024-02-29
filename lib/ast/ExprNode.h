#pragma once

#include <string>
#include <string_view>

#include "ast/AstNode.h"
#include "ast/Type.h"
#include "utils/EnumMacros.h"
#include "utils/Generator.h"

namespace ast {
class ExprNodeList;

template <typename T>
   requires std::movable<T>
class ExprEvaluator;

/* ===--------------------------------------------------------------------=== */
// ExprNode and ExprNodeList
/* ===--------------------------------------------------------------------=== */

class ExprNode {
public:
   virtual std::ostream& print(std::ostream& os) const { return os << "ExprNode"; }
   virtual ~ExprNode() = default;

public:
   void setNext(ExprNode* new_next_) {
      assert(!locked_ && "Attempt to mutate locked node");
      next_ = new_next_;
   }
   const ExprNode* next() const { return next_; }
   ExprNode* mut_next() const { return next_; }
   void dump() const;

private:
   template <typename T>
      requires std::movable<T>
   friend class ExprEvaluator;

   void const_lock() const { locked_ = true; }
   void const_unlock() const { locked_ = false; }

private:
   // The next node is mutable because it can be modified on-the-fly during
   // evaluation. That being said, it's the responsibility of the evaluator
   // to ensure the correct order of unlocking nodes.
   ExprNode* next_;

   // The lock for the previous node.
   mutable bool locked_ = false;
};

/// @brief A list of ExprNodes* that can be iterated and concatenated
class ExprNodeList {
public:
   explicit ExprNodeList(ExprNode* node) : head_{node}, tail_{node}, size_{1} {
      node->setNext(nullptr);
   }
   ExprNodeList() : head_{nullptr}, tail_{nullptr}, size_{0} {}

   /**
    * @brief Pushes a node to the back of the list
    * @param node The node to push
    */
   void push_back(ExprNode* node) {
      if(node == nullptr) return;
      if(head_ == nullptr) {
         head_ = node;
         tail_ = node;
      } else {
         tail_->setNext(node);
         tail_ = node;
      }
      node->setNext(nullptr);
      size_ += 1;
      check_invariants();
   }

   /**
    * @brief Concatenates another list to the end of this list.
    * @param other The list to concatenate (rvalue reference)
    */
   void concat(ExprNodeList&& other) {
      if(other.size_ == 0) return;
      if(head_ == nullptr) {
         head_ = other.head_;
         tail_ = other.tail_;
      } else {
         tail_->setNext(other.head_);
         tail_ = other.tail_;
      }
      size_ += other.size_;
      check_invariants();
   }

   /**
    * @brief Concatenates another list to the end of this list. The other list
    * will be invalidated and empty after this operation.
    * @param other The list to concatenate
    */
   void concat(ExprNodeList& other) {
      concat(std::move(other));
      other.head_ = nullptr;
      other.tail_ = nullptr;
      other.size_ = 0;
   }

   /// @brief Returns the number of nodes in the list
   [[nodiscard]] auto size() const { return size_; }

   /// @brief Returns a generator that yields each node in the list
   utils::Generator<ExprNode const*> nodes() const {
      size_t i = 0;
      for(auto const* node = head_; i < size_; node = node->next(), i++) {
         co_yield node;
      }
   }

   /// @brief Non-const version of nodes()
   utils::Generator<ExprNode*> mut_nodes() const {
      size_t i = 0;
      for(auto node = head_; i < size_; node = node->mut_next(), i++) {
         co_yield node;
      }
   }

private:
   inline void check_invariants() const {
      assert((!tail_ || tail_->next() == nullptr) &&
             "Tail node should not have a next node");
      assert(((head_ != nullptr) == (tail_ != nullptr)) &&
             "Head is null if and only if tail is null");
      assert(((head_ == nullptr) == (size_ == 0)) &&
             "Size should be 0 if and only if head is null");
   }

private:
   ExprNode* head_;
   ExprNode* tail_;
   size_t size_;
};

} // namespace ast

namespace ast::exprnode {

/* ===--------------------------------------------------------------------=== */
// ExprValue Subclasses
/* ===--------------------------------------------------------------------=== */

class ExprValue : public ExprNode {
public:
   ExprValue() : decl_{nullptr} {}
   ast::Decl const* decl() { return decl_; }
   virtual bool isResolved() const { return decl_ != nullptr; }
   void resolve(ast::Decl const* decl) { decl_ = decl; }

private:
   ast::Decl const* decl_;
};

class MethodName : public ExprValue {
   std::pmr::string name;

public:
   MethodName(std::string_view name) : name{name} {}
   std::ostream& print(std::ostream& os) const override {
      return os << "(Method name:" << name << ")";
   }
};

class MemberName : public ExprValue {
   std::pmr::string name_;

public:
   MemberName(std::string_view name) : name_{name} {}
   std::ostream& print(std::ostream& os) const override {
      return os << "(Member name:" << name_ << ")";
   }
   std::string_view name() const { return name_; }
};

class ThisNode final : public ExprValue {
   // FIXME(kevin): ThisNode requires decl which points to the class decl
   std::ostream& print(std::ostream& os) const override { return os << "(THIS)"; }
};

class TypeNode final : public ExprValue {
   Type const* type_;

public:
   TypeNode(Type const* type) : type_{type} {}
   bool isResolved() const override { return true; }
   ast::Type const* type() { return type_; }
   std::ostream& print(std::ostream& os) const override {
      return os << "(Type: " << type_->toString() << ")";
   }
};

class LiteralNode final : public ExprValue {
public:
   LiteralNode(std::string_view value, ast::BuiltInType const* type)
         : value_{value}, type_{type} {}
   bool isResolved() const override { return true; }
   std::ostream& print(std::ostream& os) const override {
      // TODO(kevin): re-implement this
      return os << "(Literal)";
   }

private:
   std::pmr::string value_;
   ast::BuiltInType const* type_;
};

/* ===--------------------------------------------------------------------=== */
// ExprOp Subclasses
/* ===--------------------------------------------------------------------=== */

class ExprOp : public ExprNode {
protected:
   ExprOp(int num_args) : num_args_{num_args} {}
   int num_args_;

public:
   auto nargs() const { return num_args_; }
   std::ostream& print(std::ostream& os) const override { return os << "ExprOp"; }
};

class MemberAccess : public ExprOp {
public:
   MemberAccess() : ExprOp(1) {}
   std::ostream& print(std::ostream& os) const override {
      return os << "MemberAccess";
   }
};

class MethodInvocation : public ExprOp {
public:
   MethodInvocation(int num_args) : ExprOp(num_args) {}
   std::ostream& print(std::ostream& os) const override {
      return os << "MethodInvocation";
   }
};

class ClassInstanceCreation : public ExprOp {
public:
   ClassInstanceCreation(int num_args) : ExprOp(num_args) {}
   std::ostream& print(std::ostream& os) const override {
      return os << "(ClassInstanceCreation args: " << std::to_string(nargs())
                << ")";
   }
};

class ArrayInstanceCreation : public ExprOp {
public:
   ArrayInstanceCreation() : ExprOp(2) {}
   std::ostream& print(std::ostream& os) const override {
      return os << "ArrayInstanceCreation";
   }
};

class ArrayAccess : public ExprOp {
public:
   ArrayAccess() : ExprOp(2) {}
   std::ostream& print(std::ostream& os) const override {
      return os << "ArrayAccess";
   }
};

class Cast : public ExprOp {
public:
   Cast() : ExprOp(2) {}
   std::ostream& print(std::ostream& os) const override { return os << "Cast"; }
};

class UnaryOp : public ExprOp {
#define UNARY_OP_TYPE_LIST(F) \
   /* Leaf nodes */           \
   F(Not)                     \
   F(BitwiseNot)              \
   F(Plus)                    \
   F(Minus)
public:
   DECLARE_ENUM(OpType, UNARY_OP_TYPE_LIST)
   DECLARE_STRING_TABLE(OpType, type_strings, UNARY_OP_TYPE_LIST)
#undef UNARY_OP_TYPE_LIST

private:
   OpType type;

public:
   UnaryOp(OpType type) : ExprOp(1), type{type} {}
   std::ostream& print(std::ostream& os) const override {
      return os << OpType_to_string(type, "(Unknown unary op)");
   }
   OpType getType() const { return type; }
};

class BinaryOp : public ExprOp {
#define BINARY_OP_TYPE_LIST(F) \
   F(Assignment)               \
   F(GreaterThan)              \
   F(GreaterThanOrEqual)       \
   F(LessThan)                 \
   F(LessThanOrEqual)          \
   F(Equal)                    \
   F(NotEqual)                 \
   F(And)                      \
   F(Or)                       \
   F(BitwiseAnd)               \
   F(BitwiseOr)                \
   F(BitwiseXor)               \
   F(Add)                      \
   F(Subtract)                 \
   F(Multiply)                 \
   F(Divide)                   \
   F(Modulo)                   \
   F(InstanceOf)
public:
   DECLARE_ENUM(OpType, BINARY_OP_TYPE_LIST)
   DECLARE_STRING_TABLE(OpType, type_strings, BINARY_OP_TYPE_LIST)
#undef BINARY_OP_TYPE_LIST

private:
   OpType type;

public:
   BinaryOp(OpType type) : ExprOp(2), type{type} {}
   std::ostream& print(std::ostream& os) const override {
      return os << OpType_to_string(type, "(Unknown binary op)");
   }
   OpType getType() const { return type; }
};

} // namespace ast::exprnode
