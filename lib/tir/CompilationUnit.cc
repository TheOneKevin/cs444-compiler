#include "tir/CompilationUnit.h"

#include <iostream>
#include <ostream>

#include "tir/Constant.h"

namespace tir {

CompilationUnit::CompilationUnit(Context& ctx)
      : ctx_{ctx}, globals_{ctx.alloc()}, intrinsics_{ctx.alloc()} {
   RegisterAllIntrinsics(*this);
}

std::ostream& CompilationUnit::print(std::ostream& os) const {
   // Print all the struct types
   for(auto const* structTy : ctx_.pimpl().structTypes) {
      structTy->printDetail(os) << "\n";
   }
   // Print all globals
   for(auto const* global : global_variables()) {
      global->print(os) << "\n";
   }
   // Print all the functions
   for(auto* func : functions()) {
      func->print(os) << "\n";
   }
   return os;
}

void CompilationUnit::dump() const { print(std::cerr) << "\n"; }

} // namespace tir
