#include <iostream>

#include "tir/BasicBlock.h"
#include "tir/Constant.h"
#include "tir/IRBuilder.h"
#include "tir/TIR.h"
#include "utils/BumpAllocator.h"

int main() {
   // Careful!! Using namespace can conflict with ast:: !!
   using namespace tir;
   using BinOp = tir::Instruction::BinOp;

   utils::CustomBufferResource resource{};
   BumpAllocator allocator{&resource};
   Context ctx{allocator};
   CompilationUnit cu{ctx};

   // Declare the "ptr* __malloc(i32)" function
   Function* fn_malloc;
   {
      auto fnty = FunctionType::get(
            ctx, Type::getPointerTy(ctx), {Type::getInt32Ty(ctx)});
      fn_malloc = cu.CreateFunction(fnty, "__malloc");
   }

   // Declare the "void __exception()" function
   Function* fn_exception;
   {
      auto fnty = FunctionType::get(ctx, Type::getVoidTy(ctx), {});
      fn_exception = cu.CreateFunction(fnty, "__exception");
   }

   // Build the "int main()" function
   Function* fn_main;
   {
      auto fnty = FunctionType::get(ctx, Type::getVoidTy(ctx), {});
      fn_main = cu.CreateFunction(fnty, "main");
      IRBuilder builder{ctx};
      auto bb0 = builder.createBasicBlock(fn_main);
      builder.setInsertPoint(bb0->begin());
      // %ptr = call ptr* @__malloc(i32 4)
      auto ptr = builder.createCallInstr(
            fn_malloc, {ConstantInt::Create(ctx, Type::getInt32Ty(ctx), 4)});
      // %val = load i32* %ptr
      auto val = builder.createLoadInstr(Type::getInt32Ty(ctx), ptr);
      // %add = add i32 %val, 1
      auto add = builder.createBinaryInstr(
            BinOp::ADD, val, ConstantInt::Create(ctx, Type::getInt32Ty(ctx), 1));
      // ret %add
      builder.createReturnInstr(add);
   }

   cu.dump();
   resource.reset();
   return 0;
}
