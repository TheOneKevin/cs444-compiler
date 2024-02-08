#pragma once

#include "ast/AstNode.h"
#include "utils/EnumMacros.h"
#include <list>

namespace ast {
class Expr : public AstNode {
    std::list<ExprOp> rpn_ops;
public: 
    
};

class ExprOp {
protected:
    ExprOp(int num_args) : num_args{num_args} {}
private:
    int num_args;
};

class LiteralOp : ExprOp {
    std::string value;
};

class FunctionOp : ExprOp {
    std::string name;
public:
    FunctionOp(std::string name) : ExprOp(0), name(name) {

    }
};

class CallOp : ExprOp {
    std::string name;  
};
class IdentifierOp : ExprOp {
    std::string name;
public:
    IdentifierOp(std::string name) : ExprOp(0), name(name) {}
};

class UnaryOp : ExprOp {

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

class BinaryOp : ExprOp {

#define BINARY_OP_TYPE_LIST(F)          \
    /* Leaf nodes */                    \
    F(Assign)                           \
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
    F(Plus)                             \
    F(Minus)                            \
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
