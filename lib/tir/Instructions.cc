#include "tir/Instructions.h"

#include "tir/Type.h"

namespace tir {

/* ===--------------------------------------------------------------------=== */
// BranchInst implementation
/* ===--------------------------------------------------------------------=== */

BranchInst::BranchInst(Context& ctx, Value* cond, BasicBlock* trueBB,
                       BasicBlock* falseBB)
      : Instruction{ctx, Type::getVoidTy(ctx)} {
   addChild(cond);
   addChild(trueBB);
   addChild(falseBB);
}

/* ===--------------------------------------------------------------------=== */
// ReturnInst implementation
/* ===--------------------------------------------------------------------=== */

ReturnInst::ReturnInst(Context& ctx, Value* ret) : Instruction{ctx, ret->type()} {
   if(ret) addChild(ret);
}

/* ===--------------------------------------------------------------------=== */
// StoreInst implementation
/* ===--------------------------------------------------------------------=== */

StoreInst::StoreInst(Context& ctx, Value* val, Value* ptr)
      : Instruction{ctx, Type::getVoidTy(ctx)} {
   addChild(val);
   addChild(ptr);
}

/* ===--------------------------------------------------------------------=== */
// LoadInst implementation
/* ===--------------------------------------------------------------------=== */

LoadInst::LoadInst(Context& ctx, Type* type, Value* ptr) : Instruction{ctx, type} {
   addChild(ptr);
}

/* ===--------------------------------------------------------------------=== */
// CallInst implementation
/* ===--------------------------------------------------------------------=== */

CallInst::CallInst(Context& ctx, Value* callee, utils::range_ref<Value*> args)
      : Instruction{ctx, cast<FunctionType>(callee->type())->getReturnType()} {
   addChild(callee);
   args.for_each([this](Value* arg) { addChild(arg); });
}

/* ===--------------------------------------------------------------------=== */
// BinaryInst implementation
/* ===--------------------------------------------------------------------=== */

BinaryInst::BinaryInst(Context& ctx, BinOp binop, Value* lhs, Value* rhs)
      : Instruction{ctx, lhs->type(), binop} {
   addChild(lhs);
   addChild(rhs);
}

} // namespace tir
