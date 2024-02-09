#pragma once

#include "ast/AstNode.h"
#include "utils/EnumMacros.h"
#include <list>

namespace ast {

class ExprNode;
class Expr : public AstNode {
    std::list<ExprNode> rpn_ops;
public: 
    
};

class ExprNode {

};

class TypeName: public ExprNode {
    std::string name;
};

class Field: public ExprNode {
    std::string name;
    FieldDecl* decl; // populated by semantic analysis
};

class Method: public ExprNode {
    std::string name;
    MethodDecl* decl; // populated by semantic analysis
};

class ExprOp : public ExprNode {
protected:
    ExprOp(int num_args) : num_args{num_args} {}
private:
    int num_args;
};

class FieldAccess: public ExprOp {
public:
    FieldAccess() : ExprOp(1) {}
};

class MethodAccess : public ExprOp {
public:
    MethodAccess() : ExprOp(1) {}
};

class MethodInvocation : public ExprOp {
    Method* method;
public:
    MethodInvocation(Method *method, int num_args) : ExprOp(num_args), method{method} {}
};

class UnaryOp : public ExprOp {

#define UNARY_OP_TYPE_LIST(F)           \
    /* Leaf nodes */                    \
    F(Not)                              \
    F(BitwiseNot)                       \
    F(Plus)                             \
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
};

class BinaryOp : public ExprOp {

#define BINARY_OP_TYPE_LIST(F)          \
    F(Assignment)                       \
    F(GreaterThan)                      \
    F(LessThan)                         \
    F(Equal)                            \
    F(LessThanOrEqual)                  \
    F(GreaterThanOrEqual)               \
    F(NotEqual)                         \
    F(And)                              \
    F(Or)                               \
    F(BitwiseAnd)                       \
    F(BitwiseOr)                        \
    F(BitwiseXor)                       \
    F(Add)                              \
    F(Subtract)                         \
    F(Multiply)                         \
    F(Divide)                           \
    F(Modulo)                           \
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
};

} // namespace ast
