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
        ForStatement
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
        : type{type}, value{value}, Node{Node::Type::Literal}
    { }

    std::ostream& print(std::ostream& os) const override;
};

/**
 * @brief A lex node in the parse tree representing an identifier.
 */
struct Identifier : public Node {
    std::string name;

    Identifier(char const* name)
        : name{name}, Node{Node::Type::Identifier}
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
        : type{type}, Node{Node::Type::Operator}
    { }

    std::ostream& print(std::ostream& os) const override;
    std::string to_string() const;
};

} // namespace parsetree
