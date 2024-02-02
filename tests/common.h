#pragma once

#include "parser.h"
#include "lexer.h"
#include <string>

using std::string;

namespace testing {

bool parse_grammar(const string& str);

} // namespace testing
