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
        F(QualifiedIdentifier) \
        F(Identifier) \
        F(Operator) \
        F(BasicType) \
        F(Modifier) \
        F(ArrayType) \
        F(Type) \
        F(Poison) \
        /* Compilation Unit */ \
        F(CompilationUnit) \
        F(PackageDeclaration) \
        F(ImportDeclarationList) \
        F(SingleTypeImportDeclaration) \
        F(TypeImportOnDemandDeclaration) \
        /* Modifiers */ \
        F(ModifierList) \
        /* Classes */ \
        F(ClassDeclaration) \
        F(FieldDeclaration) \
        F(ClassBodyDeclarationList) \
        F(ConstructorDeclaration) \
        F(SuperOpt) \
        /* Interfaces */ \
        F(InterfaceDeclaration) \
        F(InterfaceMemberDeclarationList) \
        F(InterfaceTypeList) \
        /* Methods */ \
        F(AbstractMethodDeclaration) \
        F(MethodHeader) \
        F(MethodDeclaration) \
        F(FormalParameterList) \
        F(FormalParameter) \
        /* Statements */ \
        F(Statement) \
        F(Block) \
        F(IfThenStatement) \
        F(WhileStatement) \
        F(ForStatement) \
        F(ReturnStatement) \
        F(StatementExpression) \
        /* Variable declarations and such */ \
        F(VariableDeclarator) \
        F(LocalVariableDeclaration) \
        F(VariableDeclaratorList) \
        /* Expressions  */ \
        F(Expression) \
        F(ArgumentList) \
        F(FieldAccess) \
        F(ArrayAccess) \
        F(CastExpression) \
        F(MethodInvocation) \
        F(ArrayCreationExpression) \
        F(ClassInstanceCreationExpression) \
        F(Dims)
public:
    DECLARE_ENUM(Type, NODE_TYPE_LIST)
private:
    DECLARE_STRING_TABLE(Type, type_strings, NODE_TYPE_LIST)
    #undef NODE_TYPE_LIST

private:
    Type type;
    Node** args;
    size_t num_args;

public:
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

    size_t num_children() const {
        return num_args;
    }

    Node* child(size_t i) const {
        return args[i];
    }

    Type get_node_type() const {
        return type;
    }

    virtual std::ostream& print(std::ostream& os) const;

    void print_type(std::ostream& os) const;

    void print_type_and_value(std::ostream& os) const;

    // Operator to turn Type into a string
    std::string type_string() const {
        return type_strings[static_cast<int>(type)];
    }

    // Operator to turn Type into a string
    static std::string type_string(Type type) {
        return type_strings[static_cast<int>(type)];
    }

    // Check if the tree has been poisoned
    bool is_poisoned() const {
        if(type == Type::Poison)
            return true;
        else {
            for (size_t i = 0; i < num_args; i++) {
                if(args[i] == nullptr)
                    continue;
                if (args[i]->is_poisoned())
                    return true;
            }
            return false;
        }
    }

public:
    virtual ~Node() {
        if(args == nullptr) return;
        for (size_t i = 0; i < num_args; i++) {
            if (args[i] != nullptr)
                delete args[i];
        }
        delete args;
    }
};

/**
 * @brief Output stream operator for a parse tree node.
 */
std::ostream& operator<< (std::ostream& os, const Node& node);

/**
 * @brief 
 */
void print_dot(std::ostream& os, const Node& root);

/**
 * @brief A lex node in the parse tree representing a literal value.
 */
class Literal : public Node {
    #define LITERAL_TYPE_LIST(F) \
        F(Integer) \
        F(Character) \
        F(String) \
        F(Boolean) \
        F(Null)
public:
    DECLARE_ENUM(Type, LITERAL_TYPE_LIST)
private:
    DECLARE_STRING_TABLE(Type, literal_strings, LITERAL_TYPE_LIST)
    #undef LITERAL_TYPE_LIST

private:
    Type type;
    std::string value;

public:
    Literal(Type type, char const* value)
        : Node{Node::Type::Literal}, type{type}, value{value}
    { }

    std::ostream& print(std::ostream& os) const override;
};

/**
 * @brief A lex node in the parse tree representing an identifier.
 */
class Identifier : public Node {
private:
    std::string name;
public:
    Identifier(char const* name)
        : Node{Node::Type::Identifier}, name{name}
    { }

    std::string get_name() const {
        return name;
    }

    std::ostream& print(std::ostream& os) const override;
};

/**
 * @brief A lex node in the parse tree representing an operator.
 */
class Operator : public Node {
public:
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
private:
    Type type;
public:
    Operator(Type type)
        : Node{Node::Type::Operator}, type{type}
    { }

    std::ostream& print(std::ostream& os) const override;
    std::string to_string() const;
};

class Modifier : public Node {
    #define MODIFIER_TYPE_LIST(F) \
        F(Public) \
        F(Protected) \
        F(Static) \
        F(Abstract) \
        F(Final) \
        F(Native)
public:
    DECLARE_ENUM(Type, MODIFIER_TYPE_LIST)
private:
    DECLARE_STRING_TABLE(Type, modifier_strings, MODIFIER_TYPE_LIST)
    #undef MODIFIER_TYPE_LIST
private:
    Type type;
public:
    Modifier(Type type)
        : Node{Node::Type::Modifier}, type{type}
    { }

    Type get_type() const {
        return type;
    }

    std::ostream& print(std::ostream& os) const override;
};

class BasicType : public Node {
    #define BASIC_TYPE_LIST(F) \
        F(Byte) \
        F(Short) \
        F(Int) \
        F(Char) \
        F(Boolean)
public:
    DECLARE_ENUM(Type, BASIC_TYPE_LIST)
private:
    DECLARE_STRING_TABLE(Type, basic_type_strings, BASIC_TYPE_LIST)
    #undef BASIC_TYPE_LIST
private:
    Type type;
public:
    BasicType(Type type)
        : Node{Node::Type::BasicType}, type{type}
    { }

    Type get_type() const {
        return type;
    }

    std::ostream& print(std::ostream& os) const override;
};

class Poison : public Node {
public:
    Poison()
        : Node{Node::Type::Poison}
    { }
};

} // namespace parsetree