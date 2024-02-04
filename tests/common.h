#pragma once

#include <string>

using std::string;

namespace testing {

bool parse_grammar(const string& str);
bool build_ast(const string& str);

} // namespace testing
