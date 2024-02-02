#include "common.h"
#include <gtest/gtest.h>

bool testing::parse_grammar(const string& str) {
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

    // Check if the tree is poisoned
    EXPECT_FALSE(parse_tree->is_poisoned());
    delete parse_tree;

    if(result) {
        return false;
    }

    return true;
}
