#pragma once

#include <string>
#include <iostream>
#include <array>
#include <vector>
#include "utils/EnumMacros.h"

namespace parsetree {

struct Node;
class Literal;
class Identifier;
class Operator;
class Modifier;
class BasicType;

/// @brief The context for parsing, keeps track of errors and such.
class ParseContext {
    static std::vector<struct Node*> poison_pool_;
public:
    ParseContext() {}

};

Node* make_poison();
void clear_poison_pool();

/// @brief The basic type-tagged node in the parse tree.
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
    /// @brief The enum for each node type
    DECLARE_ENUM(Type, NODE_TYPE_LIST)
private:
    DECLARE_STRING_TABLE(Type, type_strings, NODE_TYPE_LIST)
    #undef NODE_TYPE_LIST

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
    // Virtual destructor to delete the tree
    virtual ~Node() {
        if(args == nullptr) return;
        for (size_t i = 0; i < num_args; i++) {
            if (args[i] != nullptr)
                delete args[i];
        }
        delete[] args;
    }

public:
    // Virtual function to print the node
    virtual std::ostream& print(std::ostream& os) const;
    // Print the node as a dot file
    std::ostream& printDot(std::ostream& os) const;
    // Output stream operator for a parse tree node
    std::ostream& operator<< (std::ostream& os) const { return print(os); }

private:
    // Print the type of the node
    void printType(std::ostream& os) const;
    // Print the type and value of the node
    void printTypeAndValue(std::ostream& os) const;
    // Recursively print the DOT graph
    int printDotRecursive(
        std::ostream& os, const Node& node, int& id_counter) const;

private:
    Type type;
    Node** args;
    size_t num_args;
};

/// @brief A lex node in the parse tree representing a literal value.
class Literal : public Node {
    #define LITERAL_TYPE_LIST(F) \
        F(Integer) \
        F(Character) \
        F(String) \
        F(Boolean) \
        F(Null)
public:
    /// @brief The enum for each literal type
    DECLARE_ENUM(Type, LITERAL_TYPE_LIST)
private:
    DECLARE_STRING_TABLE(Type, literal_strings, LITERAL_TYPE_LIST)
    #undef LITERAL_TYPE_LIST

public:
    Literal(Type type, char const* value)
        : Node{Node::Type::Literal}, type{type},
          isNegative{false}, value{value} { }
    // Override printing for this leaf node
    std::ostream& print(std::ostream& os) const override;
    // Set the value of the literal to negative
    void setNegative() {
        isNegative = true;
    }
    // Check if the literal is valid
    bool isValid() const;

private:
    Type type;
    bool isNegative;
    std::string value;
};

/// @brief A lex node in the parse tree representing an identifier.
class Identifier : public Node {
public:
    Identifier(char const* name) : Node{Node::Type::Identifier}, name{name} { }
    // Get the name of the identifier
    std::string get_name() const {
        return name;
    }
    // Override printing for this leaf node
    std::ostream& print(std::ostream& os) const override;

private:
    std::string name;
};

/// @brief A lex node in the parse tree representing an operator.
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
public:
    Operator(Type type) : Node{Node::Type::Operator}, type{type} { }
    // Get the type of the operator
    std::ostream& print(std::ostream& os) const override {
        return os << to_string();
    }
    // Get the string representation of the operator
    std::string to_string() const;

private:
    Type type;
};

/// @brief A lex node in the parse tree representing a modifier.
class Modifier : public Node {
    #define MODIFIER_TYPE_LIST(F) \
        F(Public) \
        F(Protected) \
        F(Static) \
        F(Abstract) \
        F(Final) \
        F(Native)
public:
    /// @brief The enum for each modifier type
    DECLARE_ENUM(Type, MODIFIER_TYPE_LIST)
private:
    DECLARE_STRING_TABLE(Type, modifier_strings, MODIFIER_TYPE_LIST)
    #undef MODIFIER_TYPE_LIST

public:
    Modifier(Type type) : Node{Node::Type::Modifier}, modty{type} { }
    // Get the type of the modifier
    Type get_type() const {
        return modty;
    }
    // Print the string representation of the modifier
    std::ostream& print(std::ostream& os) const override;

private:
    Type modty;
};

/// @brief A lex node in the parse tree representing a basic type.
class BasicType : public Node {
    #define BASIC_TYPE_LIST(F) \
        F(Byte) \
        F(Short) \
        F(Int) \
        F(Char) \
        F(Boolean)
public:
    /// @brief The enum for each basic type
    DECLARE_ENUM(Type, BASIC_TYPE_LIST)
private:
    DECLARE_STRING_TABLE(Type, basic_type_strings, BASIC_TYPE_LIST)
    #undef BASIC_TYPE_LIST
public:
    BasicType(Type type) : Node{Node::Type::BasicType}, type{type} { }
    // Get the type of the basic type
    Type get_type() const {
        return type;
    }
    // Print the string representation of the basic type
    std::ostream& print(std::ostream& os) const override;

private:
    Type type;
};

} // namespace parsetree