#include "tir/Instructions.h"

#include "tir/BasicBlock.h"
#include "tir/Constant.h"
#include "tir/TIR.h"

namespace tir {

static std::ostream& printNameOrConst(std::ostream& os, Value* val) {
   if(auto* ci = dyn_cast<ConstantInt>(val)) {
      os << *ci;
   } else {
      if(!val->type()->isLabelType()) os << *val->type() << " ";
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
   static_assert(sizeof(BranchInst) == sizeof(Instruction));
}

std::ostream& BranchInst::print(std::ostream& os) const {
   os << "br ";
   printNameOrConst(os, getChild(0)) << ", ";
   printNameOrConst(os, getChild(1)) << ", ";
   printNameOrConst(os, getChild(2));
   return os;
}

BasicBlock* BranchInst::getSuccessor(unsigned idx) const {
   assert(idx < 2 && "BranchInst::getSuccessor index out of bounds");
   return cast<BasicBlock>(getChild(idx + 1));
}

/* ===--------------------------------------------------------------------=== */
// ReturnInst implementation
/* ===--------------------------------------------------------------------=== */

ReturnInst::ReturnInst(Context& ctx, Value* ret)
      : Instruction{ctx, ret ? ret->type() : Type::getVoidTy(ctx)} {
   if(ret) addChild(ret);
   static_assert(sizeof(ReturnInst) == sizeof(Instruction));
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
   static_assert(sizeof(StoreInst) == sizeof(Instruction));
}

std::ostream& StoreInst::print(std::ostream& os) const {
   os << "store ";
   printNameOrConst(os, getChild(0)) << ", ";
   printNameOrConst(os, getChild(1));
   return os;
}

/* ===--------------------------------------------------------------------=== */
// LoadInst implementation
/* ===--------------------------------------------------------------------=== */

LoadInst::LoadInst(Context& ctx, Type* type, Value* ptr) : Instruction{ctx, type} {
   addChild(ptr);
   static_assert(sizeof(LoadInst) == sizeof(Instruction));
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
   static_assert(sizeof(CallInst) == sizeof(Instruction));
}

std::ostream& CallInst::print(std::ostream& os) const {
   if(!type()->isVoidType()) printName(os) << " = ";
   os << "call " << getChild(0)->name() << "(";
   for(unsigned i = 1; i < numChildren(); ++i) {
      printNameOrConst(os, getChild(i));
      if(i != numChildren() - 1) os << ", ";
   }
   os << ")";
   if(isTerminator()) os << " noreturn";
   return os;
}

Function* CallInst::getCallee() const { return cast<Function>(getChild(0)); }

bool CallInst::isTerminator() const { return getCallee()->isNoReturn(); }

/* ===--------------------------------------------------------------------=== */
// BinaryInst implementation
/* ===--------------------------------------------------------------------=== */

BinaryInst::BinaryInst(Context& ctx, BinOp binop, Value* lhs, Value* rhs)
      : Instruction{ctx, lhs->type(), binop} {
   addChild(lhs);
   addChild(rhs);
   static_assert(sizeof(BinaryInst) == sizeof(Instruction));
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
// CmpInst implementation
/* ===--------------------------------------------------------------------=== */

CmpInst::CmpInst(Context& ctx, Predicate pred, Value* lhs, Value* rhs)
      : Instruction{ctx, Type::getInt1Ty(ctx), pred} {
   addChild(lhs);
   addChild(rhs);
   static_assert(sizeof(CmpInst) == sizeof(Instruction));
}

std::ostream& CmpInst::print(std::ostream& os) const {
   printName(os) << " = cmp ";
   auto pred = get<Predicate>();
   const char* name = Predicate_to_string(pred, "unknown");
   // Print name lower case
   for(unsigned i = 0; name[i] != '\0'; ++i)
      os << static_cast<char>(tolower(name[i]));
   // Print operands
   os << " " << *type() << " ";
   printNameOrConst(os, getChild(0)) << ", ";
   printNameOrConst(os, getChild(1));
   return os;
}

/* ===--------------------------------------------------------------------=== */
// AllocaInst implementation
/* ===--------------------------------------------------------------------=== */

AllocaInst::AllocaInst(Context& ctx, Type* type)
      : Instruction{ctx, Type::getPointerTy(ctx), type} {
   static_assert(sizeof(AllocaInst) == sizeof(Instruction));
}

std::ostream& AllocaInst::print(std::ostream& os) const {
   auto allocType_ = get<Type*>();
   return printName(os) << " = "
                        << "alloca " << *allocType_;
}

/* ===--------------------------------------------------------------------=== */
// ICastInst implementation
/* ===--------------------------------------------------------------------=== */

ICastInst::ICastInst(Context& ctx, CastOp op, Value* val, Type* destTy)
      : Instruction{ctx, destTy, op} {
   addChild(val);
   static_assert(sizeof(ICastInst) == sizeof(Instruction));
}

std::ostream& ICastInst::print(std::ostream& os) const {
   printName(os) << " = ";
   os << "icast ";
   const char* name = CastOp_to_string(castop(), "unknown");
   // Print name lower case
   for(unsigned i = 0; name[i] != '\0'; ++i)
      os << static_cast<char>(tolower(name[i]));
   os << " ";
   printNameOrConst(os, getChild(0)) << " to " << *type() << "";
   return os;
}

/* ===--------------------------------------------------------------------=== */
// GetElementPtrInst implementation
/* ===--------------------------------------------------------------------=== */

GetElementPtrInst::GetElementPtrInst(Context& ctx, Value* ptr,
                                     StructType* structTy,
                                     utils::range_ref<Value*> indices)
      : Instruction{ctx, Type::getPointerTy(ctx), structTy} {
   addChild(ptr);
   indices.for_each([this](auto* idx) { addChild(idx); });
   static_assert(sizeof(GetElementPtrInst) == sizeof(Instruction));
}

std::ostream& GetElementPtrInst::print(std::ostream& os) const {
   printName(os) << " = ";
   os << "getelementptr " << *getStructType() << ", ";
   printNameOrConst(os, getChild(0));
   for(unsigned i = 1; i < numChildren(); ++i) {
      os << ", ";
      printNameOrConst(os, getChild(i));
   }
   return os;
}

/* ===--------------------------------------------------------------------=== */
// PhiNode implementation
/* ===--------------------------------------------------------------------=== */

PhiNode::PhiNode(Context& ctx, Type* type, utils::range_ref<Value*> values,
                 utils::range_ref<BasicBlock*> preds)
      : Instruction{ctx, type} {
   assert(values.size() == preds.size() && "PhiNode values and preds mismatch");
   unsigned i = 0;
   values.for_each([this, &i](Value* val) {
      addChild(val, i);
      i += 2;
   });
   i = 1;
   preds.for_each([this, &i](BasicBlock* bb) {
      addChild(bb, i);
      i += 2;
   });
}

std::ostream& PhiNode::print(std::ostream& os) const {
   printName(os) << " = ";
   os << "phi " << *type() << ", ";
   for(unsigned i = 0; i < numChildren(); i += 2) {
      printNameOrConst(os, getChild(i)) << " [";
      printNameOrConst(os, getChild(i + 1)) << "]";
      if(i != numChildren() - 2) os << ", ";
   }
   return os;
}

void PhiNode::replaceOrAddOperand(BasicBlock* pred, Value* val) {
   // Try to find the predecessor and replace the value
   for(unsigned i = 1; i < numChildren(); i += 2) {
      if(getChild(i) == pred) {
         replaceChild(i - 1, val);
         return;
      }
   }
   // Otherwise, add the new value and predecessor
   addChild(val);
   addChild(pred);
}

} // namespace tir
