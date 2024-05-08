#include "CompilerPasses.h"
#include "ast/AstNode.h"
#include "ast/Decl.h"
#include "semantic/AstValidator.h"
#include "semantic/ConstantTypeResolver.h"
#include "semantic/DataflowAnalysis.h"
#include "semantic/ExprResolver.h"
#include "semantic/ExprStaticChecker.h"
#include "semantic/ExprTypeResolver.h"
#include "semantic/HierarchyChecker.h"
#include "utils/PassManager.h"

using std::string_view;
using utils::Pass;
using utils::PassManager;

namespace passes::joos1 {

/* ===--------------------------------------------------------------------=== */
// HierarchyCheckerPass
/* ===--------------------------------------------------------------------=== */

void HierarchyChecker::Run() {
   auto lu = GetPass<Linker>().LinkingUnit();
   HC = std::make_unique<semantic::HierarchyChecker>(PM().Diag());
   HC->Check(lu);
   for(auto* cu : lu->compliationUnits()) {
      auto* classDecl = dyn_cast_or_null<ast::ClassDecl>(cu->body());
      if(!classDecl) continue;
      // Check for each class in the LU, the super classes have a default ctor
      for(auto* super : classDecl->superClasses()) {
         if(!super) continue;
         if(!dyn_cast_or_null<ast::ClassDecl>(super->decl())) continue;
         if(cast<ast::ClassDecl>(super->decl())->hasDefaultCtor()) continue;
         PM().Diag().ReportError(super->location())
               << "super class "
               << (super->decl()->hasCanonicalName()
                         ? super->decl()->getCanonicalName()
                         : super->decl()->name())
               << " of "
               << (classDecl->hasCanonicalName() ? classDecl->getCanonicalName()
                                                 : classDecl->name())
               << " does not have a default constructor";
         break;
      }
   }
}

/* ===--------------------------------------------------------------------=== */
// NameResolverPass
/* ===--------------------------------------------------------------------=== */

void NameResolver::Run() {
   auto lu = GetPass<Linker>().LinkingUnit();
   auto sema = &GetPass<AstContext>().Sema();
   auto& alloc = NewAlloc(Lifetime::Managed);
   NR = std::make_unique<semantic::NameResolver>(alloc, PM().Diag());
   NR->Init(lu, sema);
   if(PM().Diag().hasErrors()) return;
   resolveRecursive(lu);
}

void NameResolver::replaceObjectClass(ast::AstNode* node) {
   auto decl = dyn_cast_or_null<ast::ClassDecl*>(node);
   if(!decl) return;
   // Check if the class is Object
   if(decl != NR->GetJavaLang().Object) return;
   // Go through the superclasses and replace Object with nullptr
   for(int i = 0; i < 2; i++) {
      auto super = decl->superClasses()[i];
      if(!super) continue;
      // If the superclass is not resolved, then we should just bail out
      if(!PM().Diag().hasErrors())
         assert(super->isResolved() && "Superclass should be resolved");
      else
         continue;
      // Do not allow Object to extend Object
      if(super->decl() == NR->GetJavaLang().Object)
         decl->mut_superClasses()[i] = nullptr;
   }
}

void NameResolver::resolveExpr(ast::Expr* expr) {
   if(!expr) return;
   for(auto node : expr->mut_nodes()) {
      auto tyNode = dyn_cast<ast::exprnode::TypeNode>(node);
      if(!tyNode) continue;
      if(tyNode->isTypeResolved()) continue;
      tyNode->resolveUnderlyingType(*NR);
   }
}

void NameResolver::resolveRecursive(ast::AstNode* node) {
   assert(node && "Node must not be null here!");
   for(auto child : node->mut_children()) {
      if(!child) continue;
      if(auto cu = dyn_cast<ast::CompilationUnit*>(child)) {
         // If the CU has no body, then we can skip to the next CU :)
         if(!cu->body()) return;
         // Resolve the current compilation unit's body
         NR->BeginContext(cu);
         resolveRecursive(cu->mut_body());
         replaceObjectClass(cu->mut_body());
         NR->EndContext();
      } else if(auto ty = dyn_cast<ast::Type*>(child)) {
         if(ty->isInvalid()) continue;
         // If the type is not resolved, then we should resolve it
         if(!ty->isResolved()) ty->resolve(*NR);
      } else {
         // Resolve any Type in expressions
         if(auto decl = dyn_cast<ast::TypedDecl*>(child)) {
            resolveExpr(decl->mut_init());
         } else if(auto stmt = dyn_cast<ast::Stmt*>(child)) {
            for(auto expr : stmt->mut_exprs()) resolveExpr(expr);
         }
         // This is a generic node, just resolve its children
         resolveRecursive(child);
      }
   }
}

/* ===--------------------------------------------------------------------=== */
// ExprResolverPass
/* ===--------------------------------------------------------------------=== */

class ExprResolver final : public Pass {
   struct Data {
      semantic::ExprResolver& ER;
      semantic::ExprTypeResolver& TR;
      semantic::ExprStaticChecker& ESC;
      semantic::ExprStaticCheckerState state;
   };

public:
   ExprResolver(PassManager& PM) noexcept : Pass(PM) {}
   string_view Name() const override { return "sema-expr"; }
   string_view Desc() const override { return "Expression Resolution"; }
   int Tag() const override { return static_cast<int>(PassTag::FrontendPass); }
   void Run() override {
      auto LU = GetPass<Linker>().LinkingUnit();
      auto& NR = GetPass<NameResolver>().Resolver();
      auto& HC = GetPass<HierarchyChecker>().Checker();
      auto& Sema = GetPass<AstContext>().Sema();
      semantic::ExprResolver ER{PM().Diag(), NewHeap(Lifetime::TemporaryNoReuse)};
      semantic::ExprTypeResolver TR{
            PM().Diag(), NewHeap(Lifetime::TemporaryNoReuse), Sema};
      semantic::ExprStaticChecker ESC{PM().Diag(), NR, HC};
      semantic::AstChecker AC{NewAlloc(Lifetime::Temporary), PM().Diag(), TR};
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
      if(PM().Diag().Verbose(3)) {
         auto dbg = PM().Diag().ReportDebug(2);
         dbg << "[*] Location: ";
         expr->location().print(dbg.get()) << "\n";
         dbg << "[*] Printing expression before resolution:\n";
         expr->print(dbg.get(), 1);
      }
      ast::ExprNodeList list = d.ER.Evaluate(expr);
      if(PM().Diag().Verbose(3)) {
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
            if(PM().Diag().Verbose(3)) {
               PM().Diag().ReportDebug(3)
                     << "[*] Resolving initializer for variable: " << decl->name();
            }
            evaluateAsList(d, init);
         }
      } else if(auto* stmt = dynamic_cast<ast::Stmt*>(node)) {
         for(auto* expr : stmt->mut_exprs()) {
            if(!expr) continue;
            if(PM().Diag().Verbose(3)) {
               PM().Diag().ReportDebug(3)
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
   void ComputeDependencies() override {
      AddDependency(GetPass<AstContext>());
      AddDependency(GetPass<NameResolver>());
      AddDependency(GetPass<HierarchyChecker>());
   }
};

/* ===--------------------------------------------------------------------=== */
// DFAPass
/* ===--------------------------------------------------------------------=== */

class Dataflow final : public Pass {
public:
   Dataflow(PassManager& PM) noexcept : Pass(PM) {}
   string_view Name() const override { return "dfa"; }
   string_view Desc() const override { return "Dataflow Analysis"; }
   int Tag() const override { return static_cast<int>(PassTag::FrontendPass); }
   void Init() override {
      optEnable = PM().GetExistingOption("--enable-dfa-check")->count();
   }
   void Run() override {
      if(!optEnable) return;
      auto LU = GetPass<Linker>().LinkingUnit();
      auto& Sema = GetPass<AstContext>().Sema();
      semantic::ConstantTypeResolver CTR{NewAlloc(Lifetime::Temporary)};
      semantic::DataflowAnalysis DFA{
            PM().Diag(), NewAlloc(Lifetime::Temporary), Sema, LU};
      semantic::CFGBuilder builder{
            PM().Diag(), &CTR, NewAlloc(Lifetime::Temporary), Sema};
      DFA.init(&builder);

      try {
         DFA.Check();
      } catch(const diagnostics::DiagnosticBuilder&) {
         // Print the errors from diag in the next step
      }
   }

private:
   void ComputeDependencies() override {
      AddDependency(GetPass<AstContext>());
      AddDependency(GetPass<Linker>());
      AddDependency(GetPass<ExprResolver>());
   }
   bool optEnable;
};

} // namespace passes::joos1

/* ===--------------------------------------------------------------------=== */
// Register all the passes here
/* ===--------------------------------------------------------------------=== */

REGISTER_PASS_NS(passes::joos1, NameResolver);
REGISTER_PASS_NS(passes::joos1, HierarchyChecker);
REGISTER_PASS_NS(passes::joos1, ExprResolver);
REGISTER_PASS_NS(passes::joos1, Dataflow);
