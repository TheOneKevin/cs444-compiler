#pragma once

#include <string>

namespace parsetree {

/**
 * @brief A lex node in the parse tree representing a literal value.
 */
struct Literal {
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
        : type{type}, value{value}
    { }
};

/**
 * @brief A lex node in the parse tree representing an identifier.
 */
struct Identifier {
    std::string name;

    Identifier(char const* name)
        : name{name}
    { }
};

/**
 * @brief A lex node in the parse tree representing an operator.
 */
struct Operator {
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
        Increment,
        Decrement,
        Add,
        Subtract,
        Multiply,
        Divide,
        Modulo,
        Plus, 
        Minus
    };

    Type type;

    Operator(Type type)
        : type{type}
    { }
};

} // namespace parsetree
