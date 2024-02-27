#pragma once

#include <list>

#include "ast/AstNode.h"
#include "utils/EnumMacros.h"

namespace ast {

class ExprNode {
public:
   virtual std::ostream& print(std::ostream& os) const { return os << "ExprNode"; }
   virtual ~ExprNode() = default;
};

class Expr {
   std::list<ExprNode*> rpn_ops;

public:
   Expr(std::list<ExprNode*> rpn_ops) : rpn_ops{rpn_ops} {}
   std::ostream& print(std::ostream& os, int indentation) const;
   int printDotNode(DotPrinter& dp) const;
};

class ExprValue : public ExprNode {
public:
   virtual bool isResolved() const = 0;
   /// @brief If the value is a method, returns true
   bool isMethod() {
      // TODO(Kevin)
      return false;
   }
   /// @brief If the value is a method, gets the parent context of the method
   DeclContext* getMethodContext() {
      assert(isMethod() && "Value is not a method");
      return nullptr;
   }
   /// @brief If the value is a method, gets the method's name
   std::string_view getMethodName() {
      assert(isMethod() && "Value is not a method");
      return "";
   }
   /// @brief If the value is not a method, gets the Decl* of the value
   Decl* getDecl() {
      assert(!isMethod() && "Value is a method");
      return nullptr;
   }

private:

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

} // namespace ast
