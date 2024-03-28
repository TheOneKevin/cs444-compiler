#include "tir/CompilationUnit.h"

#include <iostream>
#include <ostream>

#include "tir/Constant.h"

namespace tir {

CompilationUnit::CompilationUnit(Context& ctx) : ctx_{ctx}, globals_{ctx.alloc()} {
   // Declare the "ptr* __malloc(i32)" function
   {
      auto fnty = FunctionType::get(
            ctx, Type::getPointerTy(ctx), {Type::getInt32Ty(ctx)});
      CreateFunction(fnty, "__malloc");
   }
   // Declare the "void __exception()" function
   {
      auto fnty = FunctionType::get(ctx, Type::getVoidTy(ctx), {});
      auto fn = CreateFunction(fnty, "__exception");
      fn->setNoReturn(true);
   }
}

std::ostream& CompilationUnit::print(std::ostream& os) const {
   // Print all the struct types
   for(auto const* structTy : ctx_.pimpl().structTypes) {
      structTy->printDetail(os) << "\n";
   }

   // Print all the functions
   for(auto* func : functions()) {
      func->print(os) << "\n";
   }
   return os;
}

void CompilationUnit::dump() const { print(std::cerr) << "\n"; }

} // namespace tir
