#include "tir/CompilationUnit.h"

#include <iostream>
#include <ostream>

#include "tir/Constant.h"

namespace tir {

std::ostream& CompilationUnit::print(std::ostream& os) const {
   for(auto* func : functions()) {
      func->print(os) << "\n";
   }
   return os;
}

void CompilationUnit::dump() const { print(std::cerr) << "\n"; }

} // namespace tir
