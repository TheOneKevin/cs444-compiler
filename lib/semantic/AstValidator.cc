#include "semantic/AstValidator.h"

namespace semantic {

void AstChecker::ValidateLU(const ast::LinkingUnit& LU) {
   cu_ = nullptr;
   for(auto* decl : LU.decls()) {
      if(auto* cu = dyn_cast<ast::CompilationUnit>(decl)) {
         ValidateCU(*cu);
      }
   }
}

void AstChecker::ValidateCU(const ast::CompilationUnit& CU) {
   cu_ = &CU;
   for(auto* decl : CU.decls()) {
      if(auto* method = dyn_cast<ast::MethodDecl>(decl)) {
         ValidateMethod(*method);
      }
   }
}

void AstChecker::ValidateMethod(const ast::MethodDecl& method) {
   (void) method;
}

}