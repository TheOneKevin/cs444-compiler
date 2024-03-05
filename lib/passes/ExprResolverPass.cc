#include "CompilerPasses.h"
#include "semantic/AstValidator.h"
#include "semantic/ExprResolver.h"
#include "utils/BumpAllocator.h"
#include "utils/PassManager.h"

using namespace joos1;
using namespace semantic;
using std::string_view;
using utils::Pass;
using utils::PassManager;

class ExprResolverPass final : public Pass {
public:
   ExprResolverPass(PassManager& PM) noexcept : Pass(PM) {}
   string_view Name() const override { return "sema-expr"; }
   string_view Desc() const override { return "Expression Resolution"; }
   void Run() override {
      auto LU = GetPass<LinkerPass>().LinkingUnit();
      auto& NR = GetPass<NameResolverPass>().Resolver();
      auto& HC = GetPass<HierarchyCheckerPass>().Checker();
      auto& Sema = GetPass<AstContextPass>().Sema();
      ExprResolver ER{PM().Diag(), NewHeap()};
      ExprTypeResolver TR{PM().Diag(), NewHeap(), Sema};
      BumpAllocator alloc{NewHeap()};
      AstChecker AC{alloc, PM().Diag()};
      ER.Init(&TR, &NR, &Sema, &HC);
      TR.Init(&HC, &NR);
      try {
         resolveRecursive(ER, LU);
         AC.ValidateLU(*LU);
      } catch(const diagnostics::DiagnosticBuilder&) {
         // Print the errors from diag in the next step
      }
   }

private:
   void evaluateAsList(ExprResolver& ER, ast::Expr* expr) {
      if(PM().Diag().Verbose(2)) {
         auto dbg = PM().Diag().ReportDebug(2);
         dbg << "[*] Location: ";
         expr->location().print(dbg.get()) << "\n";
         dbg << "[*] Printing expression before resolution:\n";
         expr->print(dbg.get(), 1);
      }
      auto result = ER.Evaluate(expr);
      ast::ExprNodeList list = ER.ResolveExprNode(result);
      if(PM().Diag().Verbose(2)) {
         auto dbg = PM().Diag().ReportDebug(2);
         dbg << "[*] Printing expression after resolution:\n  ";
         list.print(dbg.get());
      }
      expr->replace(list);
   }

   void resolveRecursive(ExprResolver& ER, ast::AstNode* node) {
      // Set the CU and context
      if(auto* cu = dyn_cast<ast::CompilationUnit>(node)) ER.BeginCU(cu);
      if(auto* ctx = dyn_cast<ast::DeclContext>(node)) ER.BeginContext(ctx);
      // Visit the expression nodes
      if(auto* decl = dynamic_cast<ast::VarDecl*>(node)) {
         if(auto* init = decl->mut_init()) {
            if(PM().Diag().Verbose(2)) {
               PM().Diag().ReportDebug(2)
                     << "[*] Resolving initializer for variable: " << decl->name();
            }
            evaluateAsList(ER, init);
         }
      } else if(auto* stmt = dynamic_cast<ast::Stmt*>(node)) {
         for(auto* expr : stmt->mut_exprs()) {
            if(!expr) continue;
            if(PM().Diag().Verbose(2)) {
               PM().Diag().ReportDebug(2)
                     << "[*] Resolving expression in statement:";
            }
            evaluateAsList(ER, expr);
         }
      }
      // We want to avoid visiting nodes twice
      if(dynamic_cast<ast::DeclStmt*>(node)) return;
      // Visit the children recursively
      for(auto* child : node->mut_children()) {
         if(child) resolveRecursive(ER, child);
      }
   }

private:
   void computeDependencies() override {
      ComputeDependency(GetPass<AstContextPass>());
      ComputeDependency(GetPass<NameResolverPass>());
      ComputeDependency(GetPass<HierarchyCheckerPass>());
   }
};

REGISTER_PASS(ExprResolverPass);
