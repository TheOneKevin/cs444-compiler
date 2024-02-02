#pragma once

#include "parser.h"
#include "lexer.h"
#include <string>

using std::string;

namespace testing {

parsetree::Node* get_parsetree(const string& str);
bool parse_grammar(const string& str);
bool build_ast(const string& str);

} // namespace testing
