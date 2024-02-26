#pragma once

#include <iostream>
#include <string>
#include <unordered_set>

#include "FragmentGenerator.h"

namespace testing {

class BasicGrammarGenerator : public utils::FragmentGenerator {
   utils::Generator<std::string> get_next_fragment(std::string type) override;
};

} // namespace testing
