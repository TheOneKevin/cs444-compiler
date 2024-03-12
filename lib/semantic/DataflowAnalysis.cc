#include "semantic/DataflowAnalysis.h"
#include <iostream>

namespace semantic {
using DFA = DataflowAnalysis;
void DFA::LiveVariableAnalysis() const {
   std::cout << "live variable analysis starting\n";
}
} // namespace semantic