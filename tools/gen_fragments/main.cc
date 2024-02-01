#include "tools/gen_fragments/FragmentGenerator.h"
#include <unordered_set>

int main() {
    SearchSpace<string> g{get_statement};
    std::unordered_set<string> results;
    g.enumerate([&results](string s) {
        results.insert(s);
    });
    for(auto &s : results) {
        std::cout << s << std::endl;
    }
}
