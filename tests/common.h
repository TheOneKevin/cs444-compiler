#pragma once

#include <string>

#include "ast/AST.h"

using std::string;

namespace testing {

bool parse_grammar(const string& str);
bool build_ast(const string& str);
ast::CompilationUnit* get_ast(const string& str);

} // namespace testing
