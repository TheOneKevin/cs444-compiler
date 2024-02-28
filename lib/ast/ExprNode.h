#pragma once

#include "ast/AstNode.h"
#include "utils/EnumMacros.h"
#include "utils/Generator.h"

namespace ast {
class ExprNodeList;

/* ===--------------------------------------------------------------------=== */
// ExprNode and ExprNodeList
/* ===--------------------------------------------------------------------=== */

class ExprNode {
public:
   virtual std::ostream& print(std::ostream& os) const { return os << "ExprNode"; }
   virtual ~ExprNode() = default;

private:
   friend class ExprNodeList;
   ExprNode* next_;
};

/// @brief A list of ExprNodes* that can be iterated and concatenated
class ExprNodeList {
public:
   explicit ExprNodeList(ExprNode* node) : head_{node}, tail_{node}, size_{1} {}
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
         tail_->next_ = node;
         tail_ = node;
      }
      size_ += 1;
   }

   /**
    * @brief Concatenates another list to the end of this list.
    * @param other The list to concatenate (rvalue reference)
    */
   void concat(ExprNodeList&& other) { concat(other); }

   /**
    * @brief Concatenates another list to the end of this list. The other list
    * will be invalidated and empty after this operation.
    * @param other The list to concatenate
    */
   void concat(ExprNodeList& other) {
      if(other.size_ == 0) return;
      if(head_ == nullptr) {
         head_ = other.head_;
         tail_ = other.tail_;
      } else {
         tail_->next_ = other.head_;
         tail_ = other.tail_;
      }
      size_ += other.size_;
      other.head_ = nullptr;
      other.tail_ = nullptr;
      other.size_ = 0;
   }

   /// @brief Returns the number of nodes in the list
   [[nodiscard]] auto size() const { return size_; }

   /// @brief Returns a generator that yields each node in the list
   utils::Generator<ExprNode const*> nodes() const {
      for(ExprNode* node = head_; node != nullptr; node = node->next_) {
         co_yield node;
      }
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
   virtual bool isResolved() const = 0;

private:
};

class MethodName : public ExprValue {
   std::pmr::string name;

public:
   MethodName(std::string_view name) : name{name} {}
   bool isResolved() const override { return false; }
   std::ostream& print(std::ostream& os) const override {
      return os << "(Method name:" << name << ")";
   }
};

class FieldName : public ExprValue {
   std::pmr::string name;

public:
   FieldName(std::string_view name) : name{name} {}
   bool isResolved() const override { return false; }
   std::ostream& print(std::ostream& os) const override {
      return os << "(Field name:" << name << ")";
   }
};

class MemberName : public ExprValue {
   std::pmr::string name;

public:
   MemberName(std::string_view name) : name{name} {}
   bool isResolved() const override { return false; }
   std::ostream& print(std::ostream& os) const override {
      return os << "(Member name:" << name << ")";
   }
};

class ThisNode : public ExprValue {
   std::ostream& print(std::ostream& os) const override { return os << "(THIS)"; }
   bool isResolved() const override { return false; }
};

class TypeNode : public ExprValue {
   Type* type;

public:
   TypeNode(Type* type) : type{type} {}
   bool isResolved() const override { return false; }
   std::ostream& print(std::ostream& os) const override {
      return os << "(Type: " << type->toString() << ")";
   }
};

class LiteralNode : public ExprValue {
#define LITERAL_TYPE_LIST(F) \
   F(Integer)                \
   F(Character)              \
   F(String)                 \
   F(Boolean)                \
   F(Null)
public:
   /// @brief The enum for each literal type
   DECLARE_ENUM(Type, LITERAL_TYPE_LIST)
private:
   DECLARE_STRING_TABLE(Type, literal_strings, LITERAL_TYPE_LIST)
#undef LITERAL_TYPE_LIST

private:
   std::pmr::string value;
   Type type;

public:
   LiteralNode(std::string_view value, Type type) : value{value}, type{type} {}
   bool isResolved() const override { return true; }
   std::ostream& print(std::ostream& os) const override {
      return os << "(" << Type_to_string(type, "unknown literal type") << " "
                << value << ")";
   }
};

/* ===--------------------------------------------------------------------=== */
// ExprOp Subclasses
/* ===--------------------------------------------------------------------=== */

class ExprOp : public ExprNode {
protected:
   ExprOp(int num_args) : num_args{num_args} {}
   int num_args;

public:
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
      return os << "(ClassInstanceCreation args: " << std::to_string(num_args)
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
private:
   DECLARE_STRING_TABLE(OpType, type_strings, UNARY_OP_TYPE_LIST)
#undef UNARY_OP_TYPE_LIST

private:
   OpType type;

public:
   UnaryOp(OpType type) : ExprOp(1), type{type} {}
   std::ostream& print(std::ostream& os) const override {
      return os << OpType_to_string(type, "(Unknown unary op)");
   }
};

class BinaryOp : public ExprOp {
#define BINARY_OP_TYPE_LIST(F) \
   F(Assignment)               \
   F(GreaterThan)              \
   F(LessThan)                 \
   F(Equal)                    \
   F(LessThanOrEqual)          \
   F(GreaterThanOrEqual)       \
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
private:
   DECLARE_STRING_TABLE(OpType, type_strings, BINARY_OP_TYPE_LIST)
#undef BINARY_OP_TYPE_LIST

private:
   OpType type;

public:
   BinaryOp(OpType type) : ExprOp(2), type{type} {}
   std::ostream& print(std::ostream& os) const override {
      return os << OpType_to_string(type, "(Unknown binary op)");
   }
};

} // namespace ast::expr
