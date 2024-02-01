#include <iostream>
#include <string>
#include <unordered_set>
#include "utils/FragmentGenerator.h"
#include "basic_fragments.h"

int main() {
    testing::BasicGrammarGenerator g{};
    for(auto outputs : g.match_string("$<stmt>$")) {
        std::cout << outputs << std::endl;
    }
    return 0;
}
