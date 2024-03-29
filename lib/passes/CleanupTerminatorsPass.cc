#include "IRContextPass.h"
#include "tir/BasicBlock.h"
#include "utils/PassManager.h"

using std::string_view;
using utils::Pass;
using utils::PassManager;

class CleanupTerminatorsPass final : public Pass {
public:
   CleanupTerminatorsPass(PassManager& PM) noexcept : Pass(PM) {}
   void Run() override {
      tir::CompilationUnit& CU = GetPass<IRContextPass>().CU();
      for(auto func : CU.functions()) {
         for(auto bb : func->body()) {
            auto firstTerminator = bb->begin();
            for(auto it = bb->begin(); it != bb->end(); ++it) {
               if(it->isTerminator()) {
                  firstTerminator = it;
                  break;
               }
            }
            if(firstTerminator == --bb->end()) continue;
            auto instr = *(++firstTerminator);
            while(instr != nullptr) {
               auto next = instr->next();
               bb->erase(instr);
               instr = next;
            }
         }
      }
   }
   string_view Name() const override { return "cleanup-term"; }
   string_view Desc() const override { return "Terminators Cleanup"; }

private:
   void computeDependencies() override {
      ComputeDependency(GetPass<IRContextPass>());
   }
};

REGISTER_PASS(CleanupTerminatorsPass);
