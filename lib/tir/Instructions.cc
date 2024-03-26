#include "tir/Instructions.h"

#include "tir/TIR.h"

namespace tir {

static std::ostream& printNameOrConst(std::ostream& os, Value* val) {
   if(auto* ci = dyn_cast<ConstantInt>(val)) {
      os << *ci;
   } else {
      val->printName(os);
   }
   return os;
}

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

std::ostream& BranchInst::print(std::ostream& os) const {
   os << "br " << *getChild(0)->type() << " ";
   printNameOrConst(os, getChild(0)) << ", ";
   printNameOrConst(os, getChild(1)) << ", ";
   printNameOrConst(os, getChild(2));
   return os;
}

/* ===--------------------------------------------------------------------=== */
// ReturnInst implementation
/* ===--------------------------------------------------------------------=== */

ReturnInst::ReturnInst(Context& ctx, Value* ret)
      : Instruction{ctx, ret ? ret->type() : Type::getVoidTy(ctx)} {
   if(ret) addChild(ret);
}

std::ostream& ReturnInst::print(std::ostream& os) const {
   os << "ret";
   if(numChildren() > 0) {
      os << " ";
      printNameOrConst(os, getChild(0));
   }
   return os;
}

/* ===--------------------------------------------------------------------=== */
// StoreInst implementation
/* ===--------------------------------------------------------------------=== */

StoreInst::StoreInst(Context& ctx, Value* val, Value* ptr)
      : Instruction{ctx, Type::getVoidTy(ctx)} {
   addChild(val);
   addChild(ptr);
}

std::ostream& StoreInst::print(std::ostream& os) const {
   os << "store " << *getChild(0)->type() << " ";
   printNameOrConst(os, getChild(0)) << ", " << *getChild(1)->type();
   printNameOrConst(os, getChild(1));
   return os;
}

/* ===--------------------------------------------------------------------=== */
// LoadInst implementation
/* ===--------------------------------------------------------------------=== */

LoadInst::LoadInst(Context& ctx, Type* type, Value* ptr) : Instruction{ctx, type} {
   addChild(ptr);
}

std::ostream& LoadInst::print(std::ostream& os) const {
   printName(os) << " = ";
   os << "load " << *type() << ", ";
   printNameOrConst(os, getChild(0));
   return os;
}

/* ===--------------------------------------------------------------------=== */
// CallInst implementation
/* ===--------------------------------------------------------------------=== */

CallInst::CallInst(Context& ctx, Value* callee, utils::range_ref<Value*> args)
      : Instruction{ctx, cast<FunctionType>(callee->type())->getReturnType()} {
   addChild(callee);
   args.for_each([this](Value* arg) { addChild(arg); });
}

std::ostream& CallInst::print(std::ostream& os) const {
   printName(os) << " = ";
   os << "call " << getChild(0)->name() << "(";
   for(unsigned i = 1; i < numChildren(); ++i) {
      printNameOrConst(os, getChild(i));
      if(i != numChildren() - 1) os << ", ";
   }
   os << ")";
   return os;
}

/* ===--------------------------------------------------------------------=== */
// BinaryInst implementation
/* ===--------------------------------------------------------------------=== */

BinaryInst::BinaryInst(Context& ctx, BinOp binop, Value* lhs, Value* rhs)
      : Instruction{ctx, lhs->type(), binop} {
   addChild(lhs);
   addChild(rhs);
}

std::ostream& BinaryInst::print(std::ostream& os) const {
   printName(os) << " = ";
   const char* name = BinOp_to_string(binop(), "unknown");
   // Print name lower case
   for(unsigned i = 0; name[i] != '\0'; ++i)
      os << static_cast<char>(tolower(name[i]));
   // Print operands
   os << " " << *type() << ", ";
   printNameOrConst(os, getChild(0)) << ", ";
   printNameOrConst(os, getChild(1));
   return os;
}

/* ===--------------------------------------------------------------------=== */
// AllocaInst implementation
/* ===--------------------------------------------------------------------=== */

AllocaInst::AllocaInst(Context& ctx, Type* type)
      : Instruction{ctx, Type::getPointerTy(ctx)}, allocType_{type} {}

std::ostream& AllocaInst::print(std::ostream& os) const {
   os << "alloca " << *allocType_;
   return os;
}

} // namespace tir
