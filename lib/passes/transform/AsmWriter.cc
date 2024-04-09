#include <unordered_map>

#include "../IRContextPass.h"
#include "tir/BasicBlock.h"
#include "tir/Constant.h"
#include "tir/Instructions.h"
#include "utils/PassManager.h"
#include "utils/Utils.h"

using std::string_view;
using utils::Pass;
using utils::PassManager;

class AsmWriter final : public Pass {
public:
   AsmWriter(PassManager& PM) noexcept : Pass(PM), outfile("output/output.s") {}
   void Run() override {
      tir::CompilationUnit& CU = GetPass<IRContextPass>().CU();
      for(auto* F : CU.functions()) {
         if(F->hasBody()) emitFunction(F);
      }
   }
   string_view Name() const override { return "asmwriter"; }
   string_view Desc() const override { return "Emit Assembly"; }

private:
   void emitFunction(tir::Function* F);
   void emitBasicBlock(tir::BasicBlock* BB);
   void emitInstruction(tir::Instruction* instr);
   void emitBinaryInstruction(tir::BinaryInst* instr);
   void emitPredicateInstruction(tir::CmpInst* instr);
   void emitICastInstruction(tir::ICastInst* instr);

   void computeDependencies() override {
      ComputeDependency(GetPass<IRContextPass>());
   }

   std::ofstream outfile;
   // Map is reset per function
   std::unordered_map<tir::Value*, int> valueStackMap;
};

REGISTER_PASS(AsmWriter);

void AsmWriter::emitFunction(tir::Function* F) {
   // 1. Emit label for the function
   F->printName(outfile);
   outfile << ":\n";

   // 2. Map all values to a stack slot

   // 3. Emit the function prologue
   outfile << "push rbp\n";     //  Save the old base pointer
   outfile << "mov rbp, rsp\n"; // Set the new base pointer
   int stackOffset = 0;
   for(auto instr : *F->getEntryBlock()) {
      if(auto* alloca = dynamic_cast<tir::AllocaInst*>(instr)) {
         stackOffset++;
      }
   }
   outfile << "sub " << stackOffset * 8 << ", rsp\n";

   // 4. Emit the instructions in each basic block
   for(auto block : F->body()) {
      emitBasicBlock(block);
   }

   // 5. Emit the function epilogue
   outfile << "leave"; // This resets the stack pointer to the base pointer, and
                       // pops the base pointer
   outfile << "ret";   // Return to the caller
}

void AsmWriter::emitBasicBlock(tir::BasicBlock* BB) {
   // 1. Emit the label for the basic block
   BB->printName(outfile);
   outfile << ":\n";

   // 2. Emit the instructions in the basic block
   for(auto instr = BB->begin(); instr != BB->end(); ++instr) {
      emitInstruction(*instr);
   }
}

void AsmWriter::emitInstruction(tir::Instruction* instr) {
   if(auto* branch = dyn_cast<tir::BranchInst*>(instr)) {
      // 1. Emit the branch instruction
      auto cond = instr->getChild(0);
      auto dest1 = instr->getChild(1);
      auto dest2 = instr->getChild(2);

      if (auto* condConstant = dyn_cast<tir::ConstantInt*>(cond)) {
         if (condConstant->sextValue() == 1) {
            outfile << "jmp " << dest1->getName() << "\n";
         } else {
            outfile << "jmp " << dest2->getName() << "\n";
         }
      } else {
         outfile << "mov eax, [rsp - " << valueStackMap[cond] << "]\n";
         outfile << "cmp eax, 1\n";
         outfile << "je " << dest1->getName() << "\n";
         outfile << "jmp " << dest2->getName() << "\n";
      }
   } else if(auto* ret = dyn_cast<tir::ReturnInst*>(instr)) {
      // 2. Emit the return instruction
   } else if(auto* store = dyn_cast<tir::StoreInst*>(instr)) {
      // 3. Emit the store instruction
   } else if(auto* load = dyn_cast<tir::LoadInst*>(instr)) {
      // 4. Emit the load instruction
   } else if(auto* call = dyn_cast<tir::CallInst*>(instr)) {
      // 5. Emit the store instruction
   } else if(auto* binary = dyn_cast<tir::BinaryInst*>(instr)) {
      // 6. Emit the binary instruction
      emitBinaryInstruction(binary);
   } else if(auto* cmp = dyn_cast<tir::CmpInst*>(instr)) {
      // 7. Emit the cmp instruction
      emitPredicateInstruction(cmp);
   } else if(auto* iCast = dyn_cast<tir::ICastInst*>(instr)) {
      // 8. Emit the icast instruction
      emitICastInstruction(iCast);
   } else if(auto* alloca = dyn_cast<tir::AllocaInst*>(instr)) {
      // 9. Emit the alloca instruction
   } else if(auto* elementPtr = dyn_cast<tir::GetElementPtrInst*>(instr)) {
      // 10. Emit the element pointer instruction
   } else if(auto* phi = dyn_cast<tir::PhiNode*>(instr)) {
      // 11. Emit the phi instruction
   } else {
      assert(false && "Unknown instruction");
   }
}

void AsmWriter::emitBinaryInstruction(tir::BinaryInst* instr) {
   switch (instr->binop()) {
      case tir::BinaryInst::BinOp::Add: {
         auto lhsValue = instr->getChild(0);
         auto rhsValue = instr->getChild(1);

         auto lhsConstant = dyn_cast<tir::ConstantInt*>(lhsValue);
         auto rhsConstant = dyn_cast<tir::ConstantInt*>(rhsValue);

         if (!lhsConstant && !rhsConstant) {
            outfile << "mov eax, [rsp - " << valueStackMap[lhsValue] << "]\n";
            outfile << "add eax, [rsp - " << valueStackMap[rhsValue] << "]\n";
            outfile << "mov [rsp - " << valueStackMap[instr] << "], eax\n";
         } else if (!lhsConstant && rhsConstant) {
            outfile << "mov eax, [rsp - " << valueStackMap[lhsValue] << "]\n";
            outfile << "add eax, " << rhsConstant->sextValue() << "\n";
            outfile << "mov [rsp - " << valueStackMap[instr] << "], eax\n";
         } else if (lhsConstant && !rhsConstant) {
            outfile << "mov eax, " << lhsConstant->sextValue() << "\n";
            outfile << "add eax, [rsp - " << valueStackMap[rhsValue] << "]\n";
            outfile << "mov [rsp - " << valueStackMap[instr] << "], eax\n";
         } else {
            outfile << "mov eax, " << lhsConstant->sextValue() << "\n";
            outfile << "add eax, " << rhsConstant->sextValue() << "\n";
            outfile << "mov [rsp - " << valueStackMap[instr] << "], eax\n";
         }

         break;
      }
      case tir::BinaryInst::BinOp::Sub: {
         break;
      }
      case tir::BinaryInst::BinOp::Mul: {
         break;
      }
      case tir::BinaryInst::BinOp::Div: {
         break;
      }
      case tir::BinaryInst::BinOp::Rem: {
         break;
      }
      case tir::BinaryInst::BinOp::And: {
         break;
      }
      case tir::BinaryInst::BinOp::Or: {
         break;
      }
      case tir::BinaryInst::BinOp::Xor: {
         break;
      }
      case tir::BinaryInst::BinOp::None: {
         break;
      }
      default: {
         assert(false && "Unknown binary instruction");
      }
   }
}

void AsmWriter::emitPredicateInstruction(tir::CmpInst* instr) {
   switch (instr->predicate()) {
      case tir::CmpInst::Predicate::EQ: {
         break;
      }
      case tir::CmpInst::Predicate::NE: {
         break;
      }
      case tir::CmpInst::Predicate::LT: {
         break;
      }
      case tir::CmpInst::Predicate::GT: {
         break;
      }
      case tir::CmpInst::Predicate::LE: {
         break;
      }
      case tir::CmpInst::Predicate::GE: {
         break;
      }
      default: {
         assert(false && "Unknown predicate instruction");
      }
   }
}

void AsmWriter::emitICastInstruction(tir::ICastInst* instr) {
   switch (instr->castop()) {
      case tir::ICastInst::CastOp::Trunc: {
         break;
      }
      case tir::ICastInst::CastOp::ZExt: {
         break;
      }
      case tir::ICastInst::CastOp::SExt: {
         break;
      }
      default: {
         assert(false && "Unknown cast instruction");
      }
   }
}
