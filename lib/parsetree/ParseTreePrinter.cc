#include "ParseTreeTypes.h"
#include <iostream>

std::ostream& parsetree::operator<< (std::ostream& os, const Node& node) {
    node.print(os);
    return os;
}
