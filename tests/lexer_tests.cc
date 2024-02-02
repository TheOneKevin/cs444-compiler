#include <string>
#include <vector>
#include <variant>
#include <iostream>
#include <gtest/gtest.h>

#include "lexer.h"
#include "parser.h"

int extract_token(std::variant<yytokentype, char> token) {
    if (std::holds_alternative<yytokentype>(token)) {
        return static_cast<int>(std::get<yytokentype>(token));
    } else {
        return static_cast<int>(std::get<char>(token));
    }
}

/**
 * @brief Lexes a string and compares the tokens to the expected tokens.
 * @returns True if the tokens match, false otherwise.
*/
bool lex_string_internal(
    const std::string& str, 
    std::initializer_list<std::variant<yytokentype, char>> expected_tokens
) {
    YY_BUFFER_STATE state;
    if (!(state = yy_scan_bytes(str.c_str(), str.size()))) {
        return false;
    }

    int i = 0;
    auto ptr = expected_tokens.begin();
    YYSTYPE lexval;
    YYLTYPE location;
    while(int lextok = yylex(&lexval, &location)) {
        auto exptok = extract_token(*ptr);
        if (lextok != exptok) {
            std::cout << "Expected token[" << i << "] to be "
                      << exptok << " but got " << lextok << "\n";
            return false;
        }
        ptr++;
        i++;
    }
    if(extract_token(*ptr++) != YYEOF) {
        std::cout << "Expected EOF but got " << extract_token(*ptr) << "\n";
        return false;
    }
    if(ptr != expected_tokens.end()) {
        std::cout << "Expected end of initializer list\n";
        return false;
    }
    yy_delete_buffer(state);
    return true;
}

void lex_string(
    const std::string& str, 
    std::initializer_list<std::variant<yytokentype, char>> expected_tokens
) {
    if(!lex_string_internal(str, expected_tokens)) {
        std::cout << "Failed when lexing string: " << str << "\n";
        EXPECT_TRUE(false);
    } else {
        EXPECT_TRUE(true);
    }
}

TEST(LexerTests, SubcaseHelloWorld) {
    lex_string(
        "int main() { return 0; }",
        { INT, IDENTIFIER, '(', ')', '{', RETURN, LITERAL, ';', '}', YYEOF }
    );
}

TEST(LexerTests, SubcaseImports) {
    lex_string("import java.util.*;", { IMPORT, IDENTIFIER, '.', IDENTIFIER, '.', OP_MUL, ';', YYEOF });
}

TEST(LexerTests, IntegerLiteral) {
    lex_string("-10 43532 0", {OP_MINUS, LITERAL, LITERAL, LITERAL, YYEOF});
}

TEST(LexerTests, CharacterLiteral) {
   lex_string("'a'", {LITERAL, YYEOF});
   lex_string("'%'", {LITERAL, YYEOF});
   lex_string("'\\b'", {LITERAL, YYEOF});
   lex_string("'\051'", {LITERAL, YYEOF});
}

TEST(LexerTests, StringLiteral) {
    lex_string("\"\"", {LITERAL, YYEOF});
    lex_string("\"foo\"", {LITERAL, YYEOF});
    lex_string("\"\\b\\t\\n\\f\\r\\\"\\'064\"", {LITERAL, YYEOF});
    lex_string("\"\\b\", \"b\")", {LITERAL, YYEOF});
    lex_string("\"\1\2\3(\"", {LITERAL, YYEOF});
}

TEST(LexerTests, SubcaseWhitespace) {
    lex_string(" ", {YYEOF});
    lex_string(" \f  \t\t \n\n  \r \f  ", {YYEOF});
    lex_string("//this is a comment \n", {YYEOF});
    lex_string("/* this is a comment */", {YYEOF});
    lex_string("/** this \n is \n a \n comment */", {YYEOF});
    lex_string("/**comment*/ +  /*comment\n2\n*/ +", {OP_PLUS, OP_PLUS, YYEOF});
    lex_string("/******//", {OP_DIV, YYEOF});
}

TEST(LexerTests, SubcaseKeywords) {
    lex_string("abstract", {ABSTRACT, YYEOF});
    lex_string("boolean", {BOOLEAN, YYEOF});
    lex_string("byte", {BYTE, YYEOF});
    lex_string("char", {CHAR, YYEOF});
    lex_string("class", {CLASS, YYEOF});
    lex_string("else", {ELSE, YYEOF});
    lex_string("extends", {EXTENDS, YYEOF});
    lex_string("final", {FINAL, YYEOF});
    lex_string("for", {FOR, YYEOF});
    lex_string("if", {IF, YYEOF});
    lex_string("implements", {IMPLEMENTS, YYEOF});
    lex_string("import", {IMPORT, YYEOF});
    lex_string("instanceof", {INSTANCEOF, YYEOF});
    lex_string("int", {INT, YYEOF});
    lex_string("interface", {INTERFACE, YYEOF});
    lex_string("native", {NATIVE, YYEOF});
    lex_string("new", {NEW, YYEOF});
    lex_string("package", {PACKAGE, YYEOF});
    lex_string("protected", {PROTECTED, YYEOF});
    lex_string("public", {PUBLIC, YYEOF});
    lex_string("return", {RETURN, YYEOF});
    lex_string("short", {SHORT, YYEOF});
    lex_string("static", {STATIC, YYEOF});
    lex_string("this", {THIS, YYEOF});
    lex_string("void", {VOID, YYEOF});
    lex_string("while", {WHILE, YYEOF});
}

TEST(LexerTests, SubcaseSeparators) {
    lex_string("(", {'(', YYEOF});
    lex_string(")", {')', YYEOF});
    lex_string("{", {'{', YYEOF});
    lex_string("}", {'}', YYEOF});
    lex_string("[", {'[', YYEOF});
    lex_string("]", {']', YYEOF});
    lex_string(";", {';', YYEOF});
    lex_string(",", {',', YYEOF});
    lex_string(".", {'.', YYEOF});
}

TEST(LexerTests, SubcaseOperators) {
    lex_string("=", {OP_ASSIGN, YYEOF});
    lex_string(">", {OP_GT, YYEOF});
    lex_string("<", {OP_LT, YYEOF});
    lex_string("!", {OP_NOT, YYEOF});
    lex_string("==", {OP_EQ, YYEOF});
    lex_string("<=", {OP_LTE, YYEOF});
    lex_string(">=", {OP_GTE, YYEOF});
    lex_string("!=", {OP_NEQ, YYEOF});
    lex_string("&&", {OP_AND, YYEOF});
    lex_string("||", {OP_OR, YYEOF});
    lex_string("+", {OP_PLUS, YYEOF});
    lex_string("-", {OP_MINUS, YYEOF});
    lex_string("*", {OP_MUL, YYEOF});
    lex_string("/", {OP_DIV, YYEOF});
    lex_string("&", {OP_BIT_AND, YYEOF});
    lex_string("|", {OP_BIT_OR, YYEOF});
    lex_string("^", {OP_BIT_XOR, YYEOF});
    lex_string("%", {OP_MOD, YYEOF});
}

TEST(LexerTests, SubcaseComplexOperators) {
    lex_string("===", {OP_EQ, OP_ASSIGN, YYEOF});
    lex_string("====", {OP_EQ, OP_EQ, YYEOF});
    lex_string("==>", {OP_EQ, OP_GT, YYEOF});
    lex_string(">==", {OP_GTE, OP_ASSIGN, YYEOF});
    lex_string("=>=", {OP_ASSIGN, OP_GTE, YYEOF});
    lex_string("==>", {OP_EQ, OP_GT, YYEOF});
    lex_string("!==", {OP_NEQ, OP_ASSIGN, YYEOF});
}