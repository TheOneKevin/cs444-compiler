#include "common.h"
#include "parsetree/ParseTreeVisitor.h"
#include <gtest/gtest.h>

parsetree::Node* testing::get_parsetree(const string& str) {
    // Now that we have the string, lex it
    YY_BUFFER_STATE state;
    if (!(state = yy_scan_bytes(str.c_str(), str.size()))) {
        return nullptr;
    }

    // Parse the tokens using Bison generated parser
    int what;
    parsetree::Node* parse_tree = nullptr;
    int result = yyparse(&what, &parse_tree);

    // Clean up Bison stuff
    yy_delete_buffer(state);

    if (!parse_tree) {
        return nullptr;
    }

    // Check if the tree is poisoned
    EXPECT_FALSE(parse_tree->is_poisoned());

    if(result) {
        return nullptr;
    }

    return parse_tree;
}

bool testing::parse_grammar(const string& str) {
    if (!get_parsetree(str)) {
        return false;
    }
    return true;
}

bool testing::build_ast(const string& str) {
    auto parse_tree = get_parsetree(str);
    if (!parse_tree) {
        return false;
    }
    try {
        auto ast = parsetree::visitCompilationUnit(parse_tree);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}
