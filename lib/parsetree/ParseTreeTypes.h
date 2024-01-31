#pragma once

#include <string>
#include <iostream>
#include <array>
#include "utils/EnumMacros.h"

namespace parsetree {

/**
 * @brief A parse tree node base class.
 */
struct Node {
    #define NODE_TYPE_LIST(F) \
        /* Leaf nodes */ \
        F(Literal) \
        F(Identifier) \
        F(Operator) \
        F(BasicType) \
        F(Modifier) \
        F(ArrayType) \
        F(Type) \
        /* Rule nodes */ \
        F(CompilationUnit) \
        F(PackageDeclaration) \
        F(ImportDeclarations) \
        F(TypeDeclarations) \
        F(ClassModifiers) \
        F(InterfaceTypeList) \
        F(ClassBodyDeclarations) \
        F(FieldDeclaration) \
        F(MemberModifiers) \
        F(MethodDeclaration) \
        F(FormalParameterList) \
        F(FormalParameter) \
        F(ConstructorDeclaration) \
        F(ConstructorModifiers) \
        F(InterfaceDeclaration) \
        F(ExtendsInterfaces) \
        F(InterfaceMemberDeclarations) \
        F(AbstractMethodDeclaration) \
        F(Expression) \
        F(FieldAccess) \
        F(ArrayAccess) \
        F(CastExpression) \
        F(MethodInvocation) \
        F(ArrayCreationExpression) \
        F(ClassInstanceCreationExpression) \
        F(ArgumentList) \
        F(Block) \
        F(LocalVariableDeclaration) \
        F(VariableDeclarators) \
        F(IfThenStatement) \
        F(WhileStatement) \
        F(ForStatement) \
        F(ClassDeclaration) \
        F(Extends) \
        F(MethodHeader)
    
    DECLARE_ENUM(Type, NODE_TYPE_LIST)

    DECLARE_STRING_TABLE(Type, type_strings, NODE_TYPE_LIST)

    #undef NODE_TYPE_LIST

    Type type;
    Node** args;
    size_t num_args;
    
    Node(Type type)
        : type{type}
        , args{nullptr}
        , num_args{0}
    { }

    template<typename... Args>
    Node(Type type, Args&&... args)
        : type{type}
        , args{new Node*[sizeof...(Args)]{args...}}
        , num_args{sizeof...(Args)}
    {}

    virtual std::ostream& print(std::ostream& os) const;
    void print_type(std::ostream& os) const;

    virtual ~Node() {
        delete[] args;
    }
};

/**
 * @brief Output stream operator for a parse tree node.
 */
std::ostream& operator<< (std::ostream& os, const Node& node);

/**
 * @brief A lex node in the parse tree representing a literal value.
 */
struct Literal : public Node {
    #define LITERAL_TYPE_LIST(F) \
        F(Integer) \
        F(Character) \
        F(String) \
        F(Boolean) \
        F(Null)
    DECLARE_ENUM(Type, LITERAL_TYPE_LIST)
    DECLARE_STRING_TABLE(Type, literal_strings, LITERAL_TYPE_LIST)
    #undef LITERAL_TYPE_LIST
    
    Type type;
    std::string value;

    Literal(Type type, char const* value)
        : Node{Node::Type::Literal}, type{type}, value{value}
    { }

    std::ostream& print(std::ostream& os) const override;
};

/**
 * @brief A lex node in the parse tree representing an identifier.
 */
struct Identifier : public Node {
    std::string name;

    Identifier(char const* name)
        : Node{Node::Type::Identifier}, name{name}
    { }

    std::ostream& print(std::ostream& os) const override;
};

/**
 * @brief A lex node in the parse tree representing an operator.
 */
struct Operator : public Node {
    enum class Type {
        Assign,
        GreaterThan,
        LessThan,
        Not,
        Equal,
        LessThanOrEqual,
        GreaterThanOrEqual,
        NotEqual,
        And,
        Or,
        BitwiseAnd,
        BitwiseOr,
        BitwiseXor,
        BitwiseNot,
        Add,
        Subtract,
        Multiply,
        Divide,
        Modulo,
        Plus, 
        Minus,
        InstanceOf
    };

    Type type;

    Operator(Type type)
        : Node{Node::Type::Operator}, type{type}
    { }

    std::ostream& print(std::ostream& os) const override;
    std::string to_string() const;
};

struct Modifier : public Node {
    #define MODIFIER_TYPE_LIST(F) \
        F(Public) \
        F(Protected) \
        F(Static) \
        F(Abstract) \
        F(Final) \
        F(Native)
    DECLARE_ENUM(Type, MODIFIER_TYPE_LIST)
    DECLARE_STRING_TABLE(Type, modifier_strings, MODIFIER_TYPE_LIST)
    #undef MODIFIER_TYPE_LIST

    Type type;

    Modifier(Type type)
        : Node{Node::Type::Modifier}, type{type}
    { }

    std::ostream& print(std::ostream& os) const;
};

struct BasicType : public Node {
    #define BASIC_TYPE_LIST(F) \
        F(Byte) \
        F(Short) \
        F(Int) \
        F(Char) \
        F(Boolean)
    DECLARE_ENUM(Type, BASIC_TYPE_LIST)
    DECLARE_STRING_TABLE(Type, basic_type_strings, BASIC_TYPE_LIST)
    #undef BASIC_TYPE_LIST
    
    Type type;

    BasicType(Type type)
        : Node{Node::Type::BasicType}, type{type}
    { }

    std::ostream& print(std::ostream& os) const;
};

} // namespace parsetree
