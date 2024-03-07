#include "CompilerPasses.h"
#include "ast/AstNode.h"
#include "ast/Decl.h"
#include "semantic/AstValidator.h"
#include "semantic/ExprResolver.h"
#include "semantic/ExprStaticChecker.h"
#include "semantic/ExprTypeResolver.h"
#include "utils/BumpAllocator.h"
#include "utils/PassManager.h"

using namespace joos1;
using namespace semantic;
using std::string_view;
using utils::Pass;
using utils::PassManager;

class ExprResolverPass final : public Pass {
   struct Data {
      ExprResolver& ER;
      ExprTypeResolver& TR;
      ExprStaticChecker& ESC;
      semantic::ExprStaticCheckerState state;
   };

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
      ExprStaticChecker ESC{PM().Diag(), NR, HC};
      BumpAllocator alloc{NewHeap()};
      AstChecker AC{alloc, PM().Diag(), TR};
      ER.Init(&TR, &NR, &Sema, &HC);
      TR.Init(&HC, &NR);

      Data data{ER, TR, ESC, semantic::ExprStaticCheckerState{}};

      try {
         resolveRecursive(data, LU);
         AC.ValidateLU(*LU);
      } catch(const diagnostics::DiagnosticBuilder&) {
         // Print the errors from diag in the next step
      }
   }

private:
   void evaluateAsList(Data d, ast::Expr* expr) {
      if(PM().Diag().Verbose(2)) {
         auto dbg = PM().Diag().ReportDebug(2);
         dbg << "[*] Location: ";
         expr->location().print(dbg.get()) << "\n";
         dbg << "[*] Printing expression before resolution:\n";
         expr->print(dbg.get(), 1);
      }
      ast::ExprNodeList list = d.ER.Evaluate(expr);
      if(PM().Diag().Verbose(2)) {
         auto dbg = PM().Diag().ReportDebug(2);
         dbg << "[*] Printing expression after resolution:\n  ";
         list.print(dbg.get());
      }
      expr->replace(list);
      d.TR.Evaluate(expr);
      d.ESC.Evaluate(expr, d.state);
   }

   void resolveRecursive(Data d, ast::AstNode* node) {
      // Set the CU and context
      if(auto* cu = dyn_cast<ast::CompilationUnit>(node)) d.ER.BeginCU(cu);
      if(auto* ctx = dyn_cast<ast::DeclContext>(node)) d.ER.BeginContext(ctx);
      if(auto* classDecl = dyn_cast<ast::ClassDecl>(node))
         d.state.currentClass = classDecl;

      // If we're inside a method or field decl, see if its static
      d.state.isInstFieldInitializer = false;
      d.state.fieldScope = nullptr;
      if(auto* field = dyn_cast<ast::FieldDecl>(node)) {
         d.state.isStaticContext = field->modifiers().isStatic();
         if(field->hasInit()) {
            d.state.isInstFieldInitializer = !field->modifiers().isStatic();
            d.state.fieldScope = field->init()->scope();
         }
      } else if(auto* method = dyn_cast<ast::MethodDecl>(node)) {
         d.state.isStaticContext = method->modifiers().isStatic();
      }

      // Visit the expression nodes
      if(auto* decl = dynamic_cast<ast::TypedDecl*>(node)) {
         if(auto* init = decl->mut_init()) {
            if(PM().Diag().Verbose(2)) {
               PM().Diag().ReportDebug(2)
                     << "[*] Resolving initializer for variable: " << decl->name();
            }
            evaluateAsList(d, init);
         }
      } else if(auto* stmt = dynamic_cast<ast::Stmt*>(node)) {
         for(auto* expr : stmt->mut_exprs()) {
            if(!expr) continue;
            if(PM().Diag().Verbose(2)) {
               PM().Diag().ReportDebug(2)
                     << "[*] Resolving expression in statement:";
            }
            evaluateAsList(d, expr);
         }
      }
      // We want to avoid visiting nodes twice
      if(dynamic_cast<ast::DeclStmt*>(node)) return;
      // Visit the children recursively
      for(auto* child : node->mut_children()) {
         if(child) resolveRecursive(d, child);
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
