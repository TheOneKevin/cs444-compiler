#include <iostream>

#include "tir/IRBuilder.h"
#include "tir/TIR.h"

int main() {
   using namespace tir; // Careful!! This can conflict with ast:: !!
   BumpAllocator allocator{};
   Context ctx{allocator};
   CompilationUnit cu{ctx};
   auto fnty = FunctionType::get(ctx, Type::getVoidTy(ctx), {});
   auto fn_main = cu.CreateFunction(fnty, "main");
   IRBuilder builder{ctx};
   auto bb0 = builder.createBasicBlock(fn_main);
   builder.setInsertPoint(bb0->begin());
   builder.createReturnInstr();
   return 0;
}
