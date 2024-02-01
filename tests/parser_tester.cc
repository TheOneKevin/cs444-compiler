#include <iostream>
#include <string>
#include <unordered_set>
#include <gtest/gtest.h>

#include "tools/gen_fragments/basic_fragments.h"

#include "parser.h"
#include "lexer.h"

using std::string;

bool parse_grammar(const string& str) {
    // Now that we have the string, lex it
    YY_BUFFER_STATE state;
    if (!(state = yy_scan_bytes(str.c_str(), str.size()))) {
        return false;
    }

    // Parse the tokens using Bison generated parser
    int what;
    parsetree::Node* parse_tree = nullptr;
    int result = yyparse(&what, &parse_tree);

    // Clean up Bison stuff
    yy_delete_buffer(state);

    if (!parse_tree) {
        return false;
    }

    if(result) {
        return false;
    }

    delete parse_tree;
    return true;
}

TEST(ParserTests, SimpleGrammar) {
    testing::BasicGrammarGenerator g{};
    for(auto x : g.match_string("class T{void f(){$<stmt>$}}")) {
        if(!parse_grammar(x)) {
            std::cout << "Failed to parse: " << x << std::endl;
            EXPECT_TRUE(false);
        }
        EXPECT_TRUE(true);
    }
}
