#include <memory>
#include <string_view>

#include "CompilerPasses.h"
#include "ast/Decl.h"
#include "diagnostics/Diagnostics.h"
#include "semantic/NameResolver.h"
#include "utils/BumpAllocator.h"
#include "utils/PassManager.h"

using std::string_view;

namespace joos1 {

void NameResolverPass::Init() {
   alloc = std::make_unique<BumpAllocator>(NewHeap());
}

void NameResolverPass::Run() {
   auto lu = GetPass<LinkerPass>().LinkingUnit();
   auto sema = &GetPass<AstContextPass>().Sema();
   NR = std::make_unique<semantic::NameResolver>(*alloc, PM().Diag());
   NR->Init(lu, sema);
   if(PM().Diag().hasErrors()) return;
   resolveRecursive(lu);
}

void NameResolverPass::replaceObjectClass(ast::AstNode* node) {
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

void NameResolverPass::resolveExpr(ast::Expr* expr) {
   if(!expr) return;
   for(auto node : expr->mut_nodes()) {
      auto tyNode = dyn_cast<ast::exprnode::TypeNode>(node);
      if(!tyNode) continue;
      if(tyNode->isTypeResolved()) continue;
      tyNode->resolveUnderlyingType(*NR);
   }
}

void NameResolverPass::resolveRecursive(ast::AstNode* node) {
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

NameResolverPass::~NameResolverPass() {
   // FIXME(kevin): This is a really bad bug that occurs in many places
   NR.reset();
   alloc.reset();
}

} // namespace joos1
