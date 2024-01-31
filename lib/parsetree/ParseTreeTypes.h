#pragma once

#include <string>
#include <iostream>

namespace parsetree {

/**
 * @brief A parse tree node base class.
 */
struct Node {
    enum class Type {
        // Leafs
        Literal,
        Identifier,
        Operator,
        BasicType,
        Modifier,
        ArrayType,
        Type,

        // Rules
        CompilationUnit,
        PackageDeclaration,
        ImportDeclarations,
        TypeDeclarations,
        ClassModifiers,
        InterfaceTypeList,
        ClassBodyDeclarations,
        FieldDeclaration,
        MemberModifiers,
        MethodDeclaration,
        FormalParameterList,
        FormalParameter,
        ConstructorDeclaration,
        ConstructorModifiers,
        InterfaceDeclaration,
        ExtendsInterfaces,
        InterfaceMemberDeclarations,
        AbstractMethodDeclaration,
        Expression,
        FieldAccess,
        ArrayAccess,
        CastExpression,
        MethodInvocation,
        ArrayCreationExpression,
        ClassInstanceCreationExpression,
        ArgumentList,
        Block,
        LocalVariableDeclaration,
        VariableDeclarators,
        IfThenStatement,
        WhileStatement,
        ForStatement,
        ClassDeclaration,
        Extends,
        MethodHeader
    };

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

    ~Node() {
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
    enum class Type {
        Integer,
        Character,
        String,
        Boolean,
        Null,
    };
    
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
    enum class Type {
        Public,
        Protected,
        Static,
        Abstract,
        Final,
        Native
    };

    Type type;

    Modifier(Type type)
        : Node{Node::Type::Modifier}, type{type}
    { }

    std::ostream& print(std::ostream& os) const;
};

struct BasicType : public Node {
    enum class Type {
        Byte,
        Short,
        Int,
        Char,
        Boolean
    };
    Type type;

    BasicType(Type type)
        : Node{Node::Type::BasicType}, type{type}
    { }

    std::ostream& print(std::ostream& os) const;
};

} // namespace parsetree
