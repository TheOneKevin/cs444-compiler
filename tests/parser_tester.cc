#include <iostream>
#include <string>
#include <unordered_set>
#include <gtest/gtest.h>

#include "tools/gen_fragments/FragmentGenerator.h"

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
    SearchSpace<string> g{get_statement};
    std::unordered_set<string> results;
    g.enumerate([&results](string s) {
        // Construct template
        results.insert("class T{void f(){"+s+"}}");
    });
    for(auto& s : results) {
        std::cout << "Testing: " << s << "\n";
        EXPECT_TRUE(parse_grammar(s));
    }
}
