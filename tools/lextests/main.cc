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
    while(int lextok = yylex()) {
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
    /*EXPECT_TRUE(lex_string("-10 43532 0", {IntegerLiteral, IntegerLiteral, IntegerLiteral, YYEOF}));*/
}

TEST(LexerTests, CharacterLiteral) {
   /*EXPECT_TRUE(lex_string("'a'", {CharacterLiteral, YYEOF}));
   EXPECT_TRUE(lex_string("'%'", {CharacterLiteral, YYEOF}));
   EXPECT_TRUE(lex_string("'\\b'", {CharacterLiteral, YYEOF}));*/
}

TEST(LexerTests, StringLiteral) {
   /*EXPECT_TRUE(lex_string("\"\"", {StringLiteral, YYEOF}));
   EXPECT_TRUE(lex_string("\"foo\"", {StringLiteral, YYEOF}));
   EXPECT_TRUE(lex_string("\"\\b\\t\\n\\f\\r\\\"\\'064\"", {StringLiteral, YYEOF}));*/
}

TEST(LexerTests, SubcaseWhitespace) {
    // EXPECT_TRUE(lex_string(" ", {YYEOF}));
    // EXPECT_TRUE(lex_string(" \f  \t\t \n\n  \r \f  ", {YYEOF}));
    // EXPECT_TRUE(lex_string("//this is a comment \n", {Comment, YYEOF}));
    // EXPECT_TRUE(lex_string("/* this is a comment */", {Comment, YYEOF}));
    // EXPECT_TRUE(lex_string("/** this \n is \n a \n comment */", {Comment, YYEOF}));
}

TEST(LexerTests, SubcaseKeywords) {
    /*EXPECT_TRUE(lex_string("abstract", {KeywordAbstract, YYEOF}));
    EXPECT_TRUE(lex_string("boolean", {KeywordBoolean, YYEOF}));
    EXPECT_TRUE(lex_string("byte", {KeywordByte, YYEOF}));
    EXPECT_TRUE(lex_string("char", {KeywordChar, YYEOF}));
    EXPECT_TRUE(lex_string("class", {KeywordClass, YYEOF}));
    EXPECT_TRUE(lex_string("else", {KeywordElse, YYEOF}));
    EXPECT_TRUE(lex_string("extends", {KeywordExtends, YYEOF}));
    EXPECT_TRUE(lex_string("final", {KeywordFinal, YYEOF}));
    EXPECT_TRUE(lex_string("for", {KeywordFor, YYEOF}));
    EXPECT_TRUE(lex_string("if", {KeywordIf, YYEOF}));
    EXPECT_TRUE(lex_string("implements", {KeywordImplements, YYEOF}));
    EXPECT_TRUE(lex_string("import", {KeywordImport, YYEOF}));
    EXPECT_TRUE(lex_string("instanceof", {KeywordInstanceof, YYEOF}));
    EXPECT_TRUE(lex_string("int", {KeywordInt, YYEOF}));
    EXPECT_TRUE(lex_string("interface", {KeywordInterface, YYEOF}));
    EXPECT_TRUE(lex_string("native", {KeywordNative, YYEOF}));
    EXPECT_TRUE(lex_string("new", {KeywordNew, YYEOF}));
    EXPECT_TRUE(lex_string("package", {KeywordPackage, YYEOF}));
    EXPECT_TRUE(lex_string("protected", {KeywordProtected, YYEOF}));
    EXPECT_TRUE(lex_string("public", {KeywordPublic, YYEOF}));
    EXPECT_TRUE(lex_string("return", {KeywordReturn, YYEOF}));
    EXPECT_TRUE(lex_string("short", {KeywordShort, YYEOF}));
    EXPECT_TRUE(lex_string("static", {KeywordStatic, YYEOF}));
    EXPECT_TRUE(lex_string("this", {KeywordThis, YYEOF}));
    EXPECT_TRUE(lex_string("void", {KeywordVoid, YYEOF}));
    EXPECT_TRUE(lex_string("while", {KeywordWhile, YYEOF}));*/
}

TEST(LexerTests, SubcaseSeparators) {
/*    lex_string("(", {SeparatorLeftParenthesis, YYEOF});
    lex_string(")", {SeparatorRightParenthesis, YYEOF});
    lex_string("{", {SeparatorLeftBrace, YYEOF});
    lex_string("}", {SeparatorRightBrace, YYEOF});
    lex_string("[", {SeparatorLeftBracket, YYEOF});
    lex_string("]", {SeparatorRightBracket, YYEOF});
    lex_string(";", {SeparatorSemicolon, YYEOF});
    lex_string(",", {SeparatorComma, YYEOF});
    lex_string(".", {SeparatorDot, YYEOF});*/
}

TEST(LexerTests, SubcaseOperators) {
/*    lex_string("=", {Operator, YYEOF});
    lex_string(">", {Operator, YYEOF});
    lex_string("<", {Operator, YYEOF});
    lex_string("!", {Operator, YYEOF});
    lex_string("==", {Operator, YYEOF});
    lex_string("<=", {Operator, YYEOF});
    lex_string(">=", {Operator, YYEOF});
    lex_string("!=", {Operator, YYEOF});
    lex_string("&&", {Operator, YYEOF});
    lex_string("||", {Operator, YYEOF});
    lex_string("++", {Operator, YYEOF});
    lex_string("--", {Operator, YYEOF});
    lex_string("+", {Operator, YYEOF});
    lex_string("-", {Operator, YYEOF});
    lex_string("*", {Operator, YYEOF});
    lex_string("/", {Operator, YYEOF});
    lex_string("&", {Operator, YYEOF});
    lex_string("|", {Operator, YYEOF});
    lex_string("^", {Operator, YYEOF});
    lex_string("%", {Operator, YYEOF});*/
}
