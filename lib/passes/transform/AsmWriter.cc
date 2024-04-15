#include <string_view>
#include <unordered_map>

#include "passes/IRContextPass.h"
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
      outfile << "section .text" << std::endl << std::endl;
      outfile << "global _start" << std::endl;
      outfile << "_start:" << std::endl;
      outfile << "mov eax, 1" << std::endl;
      outfile << "mov ebx, 0" << std::endl;
      outfile << "int 0x80" << std::endl;
      // for(auto* F : CU.functions()) {
      //    if(F->hasBody()) emitFunction(F);
      // }
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
   void emitCallInstruction(tir::CallInst* instr);

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

   if(F->name() == "_JF4Main4testEi") {
      outfile << "global _start" << std::endl;
      outfile << "_start:" << std::endl;
   }

   // 2. Map all values to a stack slot
   valueStackMap.clear();
   int offset = 0;
   for(auto block : F->body()) {
      for(auto instr : *block) {
         if(dynamic_cast<tir::StoreInst*>(instr)) {
            continue;
         }
         if(dynamic_cast<tir::ReturnInst*>(instr)) {
            continue;
         }
         if(dynamic_cast<tir::BranchInst*>(instr)) {
            continue;
         }
         if(valueStackMap.find(instr) == valueStackMap.end()) {
            offset += 8;
            valueStackMap[instr] = offset;
         }
      }
   }
   // 3. Emit the function prologue
   outfile << "push rbp" << std::endl;     //  Save the old base pointer
   outfile << "mov rbp, rsp" << std::endl; // Set the new base pointer
   if(offset > 0)
      outfile << "sub rsp, " << offset << std::endl; // Allocate space for local variables

   // 4. Emit the instructions in each basic block
   for(auto block : F->body()) {
      emitBasicBlock(block);
   }

   // 5. Emit the function epilogue
   outfile << "add rsp " << offset << std::endl; // Deallocate space for local variables
   outfile << "pop rbp" << std::endl;                      // Restore the old base pointer
   outfile << "ret" << std::endl << std::endl; // Return to the caller
}

void AsmWriter::emitBasicBlock(tir::BasicBlock* BB) {
   outfile << std::endl;
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
      // // 1. Emit the branch instruction
      // auto cond = instr->getChild(0);
      // auto dest1 = instr->getChild(1);
      // auto dest2 = instr->getChild(2);

      // if (auto* condConstant = dyn_cast<tir::ConstantInt*>(cond)) {
      //    if (condConstant->sextValue() == 1) {
      //       outfile << "jmp " << dest1->getName() << "\n";
      //    } else {
      //       outfile << "jmp " << dest2->getName() << "\n";
      //    }
      // } else {
      //    outfile << "mov rax, [rsp - " << valueStackMap[cond] << "]\n";
      //    outfile << "cmp rax, 1\n";
      //    outfile << "je " << dest1->getName() << "\n";
      //    outfile << "jmp " << dest2->getName() << "\n";
      // }
   } else if(auto* ret = dyn_cast<tir::ReturnInst*>(instr)) {
      if (ret->isReturnVoid()) return;
      auto retValue = instr->getChild(0);
      if (auto* retConstant = dyn_cast<tir::ConstantInt*>(retValue)) {
         outfile << "mov rax, " << retConstant->sextValue() << std::endl;
      } else {
         outfile << "mov rax, [rbp - " << valueStackMap[retValue] << "]" << std::endl;
      }
   } else if(auto* store = dyn_cast<tir::StoreInst*>(instr)) {
      // 3. Emit the store instruction
      auto val = instr->getChild(0);
      auto ptr = instr->getChild(1);

      if (auto* valConstant = dyn_cast<tir::ConstantInt*>(val)) {
         outfile << "mov rax, " << valConstant->sextValue() << "\n";
      } else {
         outfile << "mov rax, [rbp - " << valueStackMap[val] << "]\n";
      }

      outfile << "mov [rbp - " << valueStackMap[ptr] << "], rax\n";
   } else if(auto* load = dyn_cast<tir::LoadInst*>(instr)) {
      // 4. Emit the load instruction
      auto val = instr->getChild(0);

      if (auto* valConstant = dyn_cast<tir::ConstantInt*>(val)) {
         outfile << "mov rax, " << valConstant->sextValue() << "\n";
      } else {
         outfile << "mov rax, [rbp - " << valueStackMap[val] << "]\n";
      }
      outfile << "mov [rbp - " << valueStackMap[instr] << "], rax\n";
   } else if(auto* call = dyn_cast<tir::CallInst*>(instr)) {
      emitCallInstruction(call);
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
   switch(instr->binop()) {
      case tir::BinaryInst::BinOp::Add: {
         auto lhsValue = instr->getChild(0);
         auto rhsValue = instr->getChild(1);

         auto lhsConstant = dyn_cast<tir::ConstantInt*>(lhsValue);
         auto rhsConstant = dyn_cast<tir::ConstantInt*>(rhsValue);

         if (!lhsConstant && !rhsConstant) {
            outfile << "mov rax, [rbp - " << valueStackMap[lhsValue] << "]\n";
            outfile << "add rax, [rbp - " << valueStackMap[rhsValue] << "]\n";
            outfile << "mov [rbp - " << valueStackMap[instr] << "], rax\n";
         } else if (!lhsConstant && rhsConstant) {
            outfile << "mov rax, [rbp - " << valueStackMap[lhsValue] << "]\n";
            outfile << "add rax, " << rhsConstant->sextValue() << "\n";
            outfile << "mov [rbp - " << valueStackMap[instr] << "], rax\n";
         } else if (lhsConstant && !rhsConstant) {
            outfile << "mov rax, " << lhsConstant->sextValue() << "\n";
            outfile << "add rax, [rbp - " << valueStackMap[rhsValue] << "]\n";
            outfile << "mov [rbp - " << valueStackMap[instr] << "], rax\n";
         } else {
            outfile << "mov rax, " << lhsConstant->sextValue() << "\n";
            outfile << "add rax, " << rhsConstant->sextValue() << "\n";
            outfile << "mov [rbp - " << valueStackMap[instr] << "], rax\n";
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
   switch(instr->predicate()) {
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
   switch(instr->castop()) {
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

void AsmWriter::emitCallInstruction(tir::CallInst* instr) {
   string_view argRegs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
   // move the arguments into the correct registers
   for(size_t i = 1; i < instr->children().size(); i++) {
      if (auto constant = dyn_cast<tir::ConstantInt*>(instr->getChild(i))) {
         outfile << "mov " << argRegs[i] << ", " 
              << constant->sextValue() << std::endl;
      } else {
         outfile << "mov qword ptr [rbp - " 
              << valueStackMap[instr->getChild(i)] << "], " << argRegs[i]
              << std::endl;
      }
      
   }

   // call the function
   outfile << "call " << instr->getChild(0)->name() << std::endl;

   // move the return value into the correct register
   outfile << "mov qword ptr [rbp - " << valueStackMap[instr] << "]"
           << ", rax" << std::endl;
}
