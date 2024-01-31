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
bool lex_string(
    const std::string& str, 
    std::initializer_list<std::variant<yytokentype, char>> expected_tokens
) {
    std::cout << "Lexing string: " << str << "\n";
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

TEST(LexerTests, SubcaseHelloWorld) {
    EXPECT_TRUE(lex_string(
        "int main() { return 0; }",
        { INT, IDENTIFIER, '(', ')', '{', RETURN, LITERAL, ';', '}', YYEOF }
    ));
}

TEST(LexerTests, IntegerLiteral) {
    EXPECT_TRUE(lex_string("-10 43532 0", {LITERAL, LITERAL, LITERAL, YYEOF}));
}

TEST(LexerTests, CharacterLiteral) {
   EXPECT_TRUE(lex_string("'a'", {LITERAL, YYEOF}));
   EXPECT_TRUE(lex_string("'%'", {LITERAL, YYEOF}));
   EXPECT_TRUE(lex_string("'\\b'", {LITERAL, YYEOF}));
}

TEST(LexerTests, StringLiteral) {
   EXPECT_TRUE(lex_string("\"\"", {LITERAL, YYEOF}));
   EXPECT_TRUE(lex_string("\"foo\"", {LITERAL, YYEOF}));
   EXPECT_TRUE(lex_string("\"\\b\\t\\n\\f\\r\\\"\\'064\"", {LITERAL, YYEOF}));
}

TEST(LexerTests, SubcaseWhitespace) {
    EXPECT_TRUE(lex_string(" ", {YYEOF}));
    EXPECT_TRUE(lex_string(" \f  \t\t \n\n  \r \f  ", {YYEOF}));
    EXPECT_TRUE(lex_string("//this is a comment \n", {COMMENT, YYEOF}));
    EXPECT_TRUE(lex_string("/* this is a comment */", {COMMENT, YYEOF}));
    EXPECT_TRUE(lex_string("/** this \n is \n a \n comment */", {COMMENT, YYEOF}));
}

TEST(LexerTests, SubcaseKeywords) {
    EXPECT_TRUE(lex_string("abstract", {ABSTRACT, YYEOF}));
    EXPECT_TRUE(lex_string("boolean", {BOOLEAN, YYEOF}));
    EXPECT_TRUE(lex_string("byte", {BYTE, YYEOF}));
    EXPECT_TRUE(lex_string("char", {CHAR, YYEOF}));
    EXPECT_TRUE(lex_string("class", {CLASS, YYEOF}));
    EXPECT_TRUE(lex_string("else", {ELSE, YYEOF}));
    EXPECT_TRUE(lex_string("extends", {EXTENDS, YYEOF}));
    EXPECT_TRUE(lex_string("final", {FINAL, YYEOF}));
    EXPECT_TRUE(lex_string("for", {FOR, YYEOF}));
    EXPECT_TRUE(lex_string("if", {IF, YYEOF}));
    EXPECT_TRUE(lex_string("implements", {IMPLEMENTS, YYEOF}));
    EXPECT_TRUE(lex_string("import", {IMPORT, YYEOF}));
    EXPECT_TRUE(lex_string("instanceof", {INSTANCEOF, YYEOF}));
    EXPECT_TRUE(lex_string("int", {INT, YYEOF}));
    EXPECT_TRUE(lex_string("interface", {INTERFACE, YYEOF}));
    EXPECT_TRUE(lex_string("native", {NATIVE, YYEOF}));
    EXPECT_TRUE(lex_string("new", {NEW, YYEOF}));
    EXPECT_TRUE(lex_string("package", {PACKAGE, YYEOF}));
    EXPECT_TRUE(lex_string("protected", {PROTECTED, YYEOF}));
    EXPECT_TRUE(lex_string("public", {PUBLIC, YYEOF}));
    EXPECT_TRUE(lex_string("return", {RETURN, YYEOF}));
    EXPECT_TRUE(lex_string("short", {SHORT, YYEOF}));
    EXPECT_TRUE(lex_string("static", {STATIC, YYEOF}));
    EXPECT_TRUE(lex_string("this", {THIS, YYEOF}));
    EXPECT_TRUE(lex_string("void", {VOID, YYEOF}));
    EXPECT_TRUE(lex_string("while", {WHILE, YYEOF}));
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
    lex_string("&", {OP_AND, YYEOF});
    lex_string("|", {OP_OR, YYEOF});
    lex_string("^", {OP_XOR, YYEOF});
    lex_string("%", {OP_MOD, YYEOF});
}
