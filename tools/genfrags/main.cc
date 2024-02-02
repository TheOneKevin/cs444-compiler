#include <iostream>
#include <string>
#include <unordered_set>
#include "utils/FragmentGenerator.h"
#include "basic_fragments.h"
#include "class_fragments.h"
#include "method_fragments.h"

int main(int argc, char** argv) {
    // Grab the first command line argument
    std::string type = "basic";
    if(argc > 1) {
        type = argv[1];
    }
    
    // Compare the type and run the appropriate generator
    if(type == "basic") {
        testing::BasicGrammarGenerator g{};
        for(auto outputs : g.match_string("$<stmt>$")) {
            std::cout << outputs << std::endl;
        }
    } else if(type == "intf") {    
        testing::ClassGrammarGenerator g{};
        for(auto outputs : g.match_string("$<intf>$")) {
            std::cout << outputs << std::endl;
        }
    } else if(type == "class") {
        testing::ClassGrammarGenerator g{};
        for(auto outputs : g.match_string("$<class>$")) {
            std::cout << outputs << std::endl;
        }
    } else if(type == "class_method") {
        testing::MethodGrammarGenerator g{};
        for(auto outputs : g.match_string("$<class_method>$")) {
            std::cout << outputs << std::endl;
        }
    } else if(type == "interface_method") {
        testing::MethodGrammarGenerator g{};
        for(auto outputs : g.match_string("$<interface_method>$")) {
            std::cout << outputs << std::endl;
        }
    } else {
        std::cerr << "Invalid type: " << type << std::endl;
        std::cerr << "Usage: " << argv[0] << " [basic|intf|class]" << std::endl;
        return 1;
    }
    
    return 0;
}
