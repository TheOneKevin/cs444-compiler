#include "semantic/ExprResolver.h"

#include <string_view>
#include <variant>

#include "ast/AST.h"
#include "ast/AstNode.h"
#include "ast/Decl.h"
#include "ast/DeclContext.h"
#include "ast/ExprNode.h"

namespace semantic {

namespace ex = ast::exprnode;
using ETy = internal::ExprResolverTy;
using PrevTy = internal::ExprNameWrapper::PrevTy;
using ER = ExprResolver;
using internal::ExprNameWrapper;
using Pkg = semantic::NameResolver::Pkg;

/* ===--------------------------------------------------------------------=== */
//
/* ===--------------------------------------------------------------------=== */

bool ER::tryReclassifyDecl(ExprNameWrapper& data,
                           ast::DeclContext const* ctx) const {
   // Search in this context
   if(auto decl = ctx->lookupDecl(data.node->name())) {
      if(auto varDecl = dynamic_cast<ast::VarDecl const*>(decl)) {
         data.reclassify(ExprNameWrapper::Type::ExpressionName, varDecl);
         return true;
      } else if(auto fieldDecl = dynamic_cast<ast::FieldDecl const*>(decl)) {
         data.reclassify(ExprNameWrapper::Type::ExpressionName, fieldDecl);
         return true;
      }
   }
   // If not found in this context, search in parent context
   // If the parent context does not exist, return the original data
   auto ctxAsDecl = dynamic_cast<ast::Decl const*>(ctx);
   if(!ctxAsDecl) return false;
   auto parentCtx = dynamic_cast<ast::DeclContext const*>(ctxAsDecl->parent());
   if(!parentCtx) return false;
   return tryReclassifyDecl(data, parentCtx);
}

bool ER::tryReclassifyImport(ExprNameWrapper& data,
                             NameResolver::ConstImportOpt import) const {
   if(!import.has_value()) {
      throw diag.ReportError(cu_->location())
            << "cannot resolve name: " << data.node->name();
   }
   if(std::holds_alternative<const ast::Decl*>(import.value())) {
      auto decl = std::get<const ast::Decl*>(import.value());
      // If declaration is null, then there is an import-on-demand conflict
      if(!decl) {
         throw diag.ReportError(cu_->location())
               << "ambiguous import-on-demand conflict";
      }
      data.reclassify(ExprNameWrapper::Type::TypeName, decl);
      return true;
   } else if(std::holds_alternative<const Pkg*>(import.value())) {
      auto pkg = std::get<const Pkg*>(import.value());
      assert(pkg && "expected non-null package here");
      data.reclassify(ExprNameWrapper::Type::PackageName, pkg);
      return true;
   }
   return false;
}

ExprNameWrapper* ER::reclassifySingleAmbiguousName(ExprNameWrapper* data) const {
   // JLS 6.5.2 Reclassification of Contextually Ambiguous Names

   assert(data->type == ExprNameWrapper::Type::SingleAmbiguousName &&
          "Expected SingleAmbiguousName here");
   auto copy = alloc.new_object<ExprNameWrapper>(*data);

   /**
    * 1. If the Identifier appears within the scope (§6.3) of a local variable
    *    declaration (§14.4) or parameter declaration (§8.4.1, §8.8.1, §14.19)
    *    or field declaration (§8.3) with that name, then the AmbiguousName is
    *    reclassified as an ExpressionName.
    * 2. Otherwise, if a type of that name is declared in the compilation unit
    *    (§7.3) containing the Identifier, either by a single-type-import
    *    declaration (§7.5.1) or by a top level class (§8) or interface type
    *    declaration (§9), then the AmbiguousName is reclassified as a TypeName.
    * 3. Otherwise, if a type of that name is declared in another compilation unit
    *    (§7.3) of the package (§7.1) of the compilation unit containing the
    *    Identifier, then the AmbiguousName is reclassified as a TypeName.
    * 4. Otherwise, if a type of that name is declared by exactly one
    *    type-import-on-demand declaration (§7.5.2) of the compilation unit
    *    containing the Identifier, then the AmbiguousName is reclassified as a
    *    TypeName.
    * 5. Otherwise, if a type of that name is declared by more than one
    *    type-import-on-demand declaration of the compilation unit containing the
    *    Identifier, then a compile-time error results.
    * 6. Otherwise, the AmbiguousName is reclassified as a PackageName. A later
    *    step determines whether or not a package of that name actually exists.
    */

   // Criteria 1 and part of 2 (checks body of CU)
   if(tryReclassifyDecl(*copy, lctx_)) return copy;

   // Criteria 2 through 6 are handled by the NameResolver
   if(tryReclassifyImport(*copy, NR->GetImport(cu_, data->node->name())))
      return copy;

   // If all else fails, we probably hit criteria 6
   throw diag.ReportError(cu_->location())
         << "Unknown error when attempting to resolve import type";
}

void ER::resolveFieldAccess(ExprNameWrapper* access) const {
   // First, verify invariants if access is a field access
   access->verifyInvariants(ExprNameWrapper::Type::SingleAmbiguousName);
   if(auto p = access->prevIfWrapper())
      p->verifyInvariants(ExprNameWrapper::Type::ExpressionName);
   // Next, fetch the type or declaration
   auto name = access->node->name();
   auto typeOrDecl = access->prevAsDecl(*TR, *NR);
   auto refTy = dynamic_cast<ast::DeclContext const*>(typeOrDecl);
   if(access->prevIfWrapper()) {
      // If the previous node is a wrapper, then we resolve the type
      auto decl = typeOrDecl;
      auto typeddecl = dynamic_cast<ast::TypedDecl const*>(decl);
      if(!typeddecl) {
         throw diag.ReportError(cu_->location())
               << "field access \"" << name
               << "\" to non-typed declaration: " << decl->name();
      }
      auto type = typeddecl->type();
      if(!type) {
         throw diag.ReportError(cu_->location())
               << "field access \"" << name
               << "\" to void-typed declaration: " << decl->name();
      }
      refTy = NR->GetTypeAsClass(type);
      if(!refTy) {
         throw diag.ReportError(cu_->location())
               << "field access \"" << name << "\" to non-class type: "
               << (decl->hasCanonicalName() ? decl->getCanonicalName()
                                            : decl->name());
      }
   }
   // Now we check if "name" is a field of "decl"
   assert(refTy && "Expected non-null refTy here");
   auto field = refTy->lookupDecl(name);
   if(!field) {
      throw diag.ReportError(cu_->location())
            << "field access to undeclared field: " << name;
   }
   // Field must be either a FieldDecl or a MethodDecl
   assert(dynamic_cast<ast::FieldDecl const*>(field) ||
          dynamic_cast<ast::MethodDecl const*>(field));
   // Now we can reclassify the access node
   access->reclassify(ExprNameWrapper::Type::ExpressionName, field);
}

void ER::resolveTypeAccess(internal::ExprNameWrapper* access) const {
   // First, verify invariants if access is a type access
   access->verifyInvariants(ExprNameWrapper::Type::SingleAmbiguousName);
   if(auto p = access->prevIfWrapper())
      p->verifyInvariants(ExprNameWrapper::Type::TypeName);
   // Next, fetch the type or declaration
   auto name = access->node->name();
   auto typeOrDecl = access->prevAsDecl(*TR, *NR);
   // We note this must be a class type or we have a type error
   auto type = dynamic_cast<ast::ClassDecl const*>(typeOrDecl);
   if(!type) {
      throw diag.ReportError(cu_->location())
            << "static member access \"" << name
            << "\" to non-class type: " << typeOrDecl->name();
   }
   // Now we check if "name" is a field of "decl".
   auto field = type->lookupDecl(name);
   if(!field) {
      throw diag.ReportError(cu_->location())
            << "static member access to undeclared field: " << name;
   }
   // With the additional constraint that the field must be static
   ast::Modifiers mods;
   if(auto fieldDecl = dynamic_cast<ast::FieldDecl const*>(field)) {
      mods = fieldDecl->modifiers();
   } else if(auto methodDecl = dynamic_cast<ast::MethodDecl const*>(field)) {
      mods = methodDecl->modifiers();
   }
   if(!mods.isStatic()) {
      throw diag.ReportError(cu_->location())
            << "attempted to access non-static member: "
            << field->getCanonicalName();
   }
   // We've reached here which means the field access is valid
   access->reclassify(ExprNameWrapper::Type::ExpressionName, field);
   access->prev = nullptr;
}

void ER::resolvePackageAccess(internal::ExprNameWrapper* access) const {
   // First, verify invariants if access is a package access
   auto prev = access->prevAsWrapper();
   assert(prev && "Expected non-null previous node");
   access->verifyInvariants(ExprNameWrapper::Type::SingleAmbiguousName);
   prev->verifyInvariants(ExprNameWrapper::Type::PackageName);
   // Now we can get the "pkg" and "name" to resolve against
   auto pkg = std::get<Pkg const*>(prev->resolution.value());
   auto name = access->node->name();
   assert(pkg && "Expected non-null package here");
   // Now we check if "name" is a package of "pkg"
   auto subpkg = pkg->lookup(name, alloc);
   if(!subpkg.has_value()) {
      throw diag.ReportError(cu_->location())
            << "package access to undeclared member: " << name;
   }
   // Now we can reclassify the access node depending on what we found
   if(std::holds_alternative<ast::Decl const*>(subpkg.value())) {
      access->reclassify(ExprNameWrapper::Type::TypeName,
                         std::get<ast::Decl const*>(subpkg.value()));
   } else {
      access->reclassify(ExprNameWrapper::Type::PackageName,
                         std::get<Pkg const*>(subpkg.value()));
   }
   access->prev = nullptr;
}

void ExprNameWrapper::verifyInvariants(ExprNameWrapper::Type expectedTy) const {
   assert(type == expectedTy && "Expected type does not match actual type");
   // TODO(kevin): Add more invariants here
}

ast::Decl const* ExprNameWrapper::prevAsDecl(ExprTypeResolver& TR,
                                             NameResolver& NR) const {
   // Simple case, the previous node is wrapped so we know the decl
   if(auto prev = prevIfWrapper())
      return std::get<ast::Decl const*>(prev->resolution.value());
   // Complex case, the previous node is an expression list
   auto prevList = std::get<ast::ExprNodeList>(prev.value());
   auto type = TR.EvaluateList(prevList);
   // FIXME(kevin): What about interfaces here?
   return NR.GetTypeAsClass(type);
}

ast::ExprNodeList ER::recursiveReduce(ExprNameWrapper* node) const {
   node->verifyInvariants(ExprNameWrapper::Type::ExpressionName);
   auto decl = std::get<ast::Decl const*>(node->resolution.value());
   auto* expr = node->node;

   // Base case, no more nodes to reduce. Return singleton list.
   if(!node->prev.has_value() /* Either no more previous */ ||
      (node->prevIfWrapper() /* Or the previous node is irreducible */ &&
       node->prevAsWrapper()->type != ExprNameWrapper::Type::ExpressionName)) {
      expr->resolve(decl);
      return ast::ExprNodeList{expr};
   }

   // Otherwise, chain the previous node to the current node
   ast::ExprNodeList list{};
   if(auto prev = node->prevIfWrapper()) {
      list = recursiveReduce(prev);
   } else {
      list = std::get<ast::ExprNodeList>(node->prev.value());
   }
   expr->resolve(decl);
   list.push_back(expr);
   return list;
}

ast::ExprNodeList ER::resolveExprNode(const ETy node) const {
   ExprNameWrapper* Q = nullptr;
   // 1. If node is an ExprNode, then resolution is only needed for names
   // 2. If the node is a list, then no resolution is needed
   // 3. Otherwise, get the wrapped node and build resolution
   if(std::holds_alternative<ast::ExprNode*>(node)) {
      auto expr = std::get<ast::ExprNode*>(node);
      auto thisNode = dyn_cast<ex::ThisNode*>(expr);
      auto name = dynamic_cast<ex::MemberName*>(expr);
      if(thisNode) {
         thisNode->resolve(cast<ast::Decl>(cu_->body()));
      }
      if(!name) return ast::ExprNodeList{expr};
      Q = resolveSingleName(name);
   } else if(std::holds_alternative<ast::ExprNodeList>(node)) {
      return std::get<ast::ExprNodeList>(node);
   } else {
      Q = std::get<ExprNameWrapper*>(node);
   }
   // Now we can reduce the expression to a list of nodes
   return recursiveReduce(Q);
}

/* ===--------------------------------------------------------------------=== */
//
/* ===--------------------------------------------------------------------=== */

ETy ER::mapValue(ExprValue& node) const { return &node; }

ETy ER::evalMemberAccess(const ETy lhs, const ETy id) const {
   // Resolves expr of form: Q . Id
   //    Where Q is either a simple identifier or a qualified
   //    identifier. If the LHS is an ExprNode, then it is simple and we need to
   //    reclassify it. Otherwise, we can continue to resolve recursively.
   PrevTy Q = nullptr;
   if(std::holds_alternative<ast::ExprNode*>(lhs)) {
      if(auto simpleName =
               dynamic_cast<ex::MemberName*>(std::get<ast::ExprNode*>(lhs))) {
         Q = resolveSingleName(simpleName);
      } else if(auto thisNode =
                      dynamic_cast<ex::ThisNode*>(std::get<ast::ExprNode*>(lhs))) {
         Q = resolveExprNode(thisNode);
      } else {
         assert(false && "Malformed node. Expected MemberName here.");
      }

   } else if(std::holds_alternative<ExprNameWrapper*>(lhs)) {
      Q = std::get<ExprNameWrapper*>(lhs);
   } else {
      Q = std::get<ast::ExprNodeList>(lhs);
   }
   // The LHS must be an ExpressionName, TypeName or PackageName
   auto* P = std::get_if<ExprNameWrapper*>(&Q);
   if(P) {
      assert(((*P)->type == ExprNameWrapper::Type::ExpressionName ||
              (*P)->type == ExprNameWrapper::Type::TypeName ||
              (*P)->type == ExprNameWrapper::Type::PackageName) &&
             "Malformed node. Expected ExpressionName, TypeName or PackageName "
             "here.");
   }
   // Grab the Id as an expr node
   auto exprNode = std::get<ast::ExprNode*>(id);
   // Special case: If "Id" in Q . Id is a method name, then defer resolution
   if(auto methodNode = dynamic_cast<ex::MethodName*>(exprNode)) {
      auto newQ = alloc.new_object<ExprNameWrapper>(
            ExprNameWrapper::Type::MethodName, methodNode);
      newQ->prev = Q;
      return newQ;
   }
   // Now grab the id and cast it to the appropriate type
   auto fieldNode = dynamic_cast<ex::MemberName*>(exprNode);
   assert(fieldNode && "Malformed node. Expected MemberName here.");
   // Allocate a new node as the member access to represent "Id" in Lhs . Id
   auto newQ = alloc.new_object<ExprNameWrapper>(
         ExprNameWrapper::Type::SingleAmbiguousName, fieldNode);
   newQ->prev = Q;
   // And we can build the reduced expression now
   // 1. If the previous node is a wrapper, then newQ can be anything
   // 2. If the previous is a list, then newQ must be ExpressionName
   //    FIXME: Is this true? What about (Class).Field?
   if(P) {
      switch((*P)->type) {
         case ExprNameWrapper::Type::ExpressionName:
            resolveFieldAccess(newQ);
            break;
         case ExprNameWrapper::Type::TypeName:
            resolveTypeAccess(newQ);
            break;
         case ExprNameWrapper::Type::PackageName:
            resolvePackageAccess(newQ);
            break;
         default:
            assert(false && "Not reached");
      }
   } else {
      resolveFieldAccess(newQ);
   }
   return newQ;
}

ETy ER::evalBinaryOp(BinaryOp& op, const ETy lhs, const ETy rhs) const {
   ast::ExprNodeList list{};
   list.concat(resolveExprNode(lhs));
   list.concat(resolveExprNode(rhs));
   list.push_back(&op);
   return list;
}

ETy ER::evalUnaryOp(UnaryOp& op, const ETy rhs) const {
   ast::ExprNodeList list{};
   list.concat(resolveExprNode(rhs));
   list.push_back(&op);
   return list;
}

ast::DeclContext const* ER::getMethodParent(ExprNameWrapper* Q) const {
   Q->verifyInvariants(ExprNameWrapper::Type::MethodName);
   // If there's no previous, use the current context
   if(!Q->prev.has_value()) return cu_->body();
   auto declOrType = Q->prevAsDecl(*TR, *NR);
   auto ty = dynamic_cast<ast::DeclContext const*>(declOrType);
   if(Q->prevIfWrapper() && !ty) {
      auto typedDecl = dynamic_cast<ast::TypedDecl const*>(declOrType);
      if(!typedDecl) {
         throw diag.ReportError(cu_->location())
               << "method call to non-typed declaration: " << declOrType->name();
      }
      auto type = typedDecl->type();
      if(!type) {
         throw diag.ReportError(cu_->location())
               << "method call to void-typed declaration: " << declOrType->name();
      }
      ty = NR->GetTypeAsClass(type);
   }
   assert(ty && "Expected non-null type here");
   return ty;
}

ETy ER::evalMethodCall(const ETy method, const op_array& args) const {
   // Q is the incompletely resolved method name
   ExprNameWrapper* Q = nullptr;

   // 1. It could be a raw ExprNode, in which case is something like Fun()
   // 2. Or it could be a wrapped ExprNameWrapper, in which case is something
   //    like org.pkg.Class.Fun()
   // 3. Or it could be a list of nodes, in which case is something like
   //    (1 + 2 + 3)()
   if(std::holds_alternative<ast::ExprNode*>(method)) {
      auto expr = std::get<ast::ExprNode*>(method);
      auto name = dynamic_cast<ex::MethodName*>(expr);
      assert(name && "Malformed node. Expected MethodName here.");
      Q = resolveSingleName(name);
   } else if(std::holds_alternative<ExprNameWrapper*>(method)) {
      Q = std::get<ExprNameWrapper*>(method);
   } else {
      assert(false && "This is not supported");
   }

   // Begin resolution of the method call
   auto ctx = getMethodParent(Q);
   ast::Decl* methodDecl = nullptr;
   // TODO: Grab the type array and resolve the overloads here
   // ...
   Q->reclassify(ExprNameWrapper::Type::ExpressionName, methodDecl);

   // Once Q has been resolved, we can build the expression list
   ast::ExprNodeList list{};
   list.concat(recursiveReduce(Q));
   for(auto& arg : args) {
      list.concat(resolveExprNode(arg));
   }
   return list;
}

ETy ER::evalNewObject(const ETy object, const op_array& args) const {
   ast::ExprNodeList list{};
   if(std::holds_alternative<ast::ExprNode*>(object)) {
      auto expr = cast<ex::TypeNode>(std::get<ast::ExprNode*>(object));
      list.push_back(expr);
   } else {
      assert(false && "Grammar wrong, object must be an atomic ExprNode*");
   }
   for(auto& arg : args) list.concat(resolveExprNode(arg));
   list.push_back(alloc.new_object<ex::ClassInstanceCreation>(args.size()));
   return list;
}

ETy ER::evalNewArray(const ETy type, const ETy size) const {
   ast::ExprNodeList list{};
   if(std::holds_alternative<ast::ExprNode*>(type)) {
      auto expr = cast<ex::TypeNode>(std::get<ast::ExprNode*>(type));
      list.push_back(expr);
   } else {
      assert(false && "Grammar wrong, type must be an atomic ExprNode*");
   }
   list.concat(resolveExprNode(size));
   list.push_back(alloc.new_object<ex::ArrayInstanceCreation>());
   return list;
}

ETy ER::evalArrayAccess(const ETy array, const ETy index) const {
   ast::ExprNodeList list{};
   list.concat(resolveExprNode(array));
   list.concat(resolveExprNode(index));
   return list;
}

ETy ER::evalCast(const ETy type, const ETy value) const {
   ast::ExprNodeList list{};
   if(std::holds_alternative<ast::ExprNode*>(type)) {
      auto expr = cast<ex::TypeNode>(std::get<ast::ExprNode*>(type));
      list.push_back(expr);
   } else {
      assert(false && "Grammar wrong, type must be an atomic ExprNode*");
   }
   list.concat(resolveExprNode(value));
   list.push_back(alloc.new_object<ex::Cast>());
   return list;
}

void ER::Resolve(ast::LinkingUnit* lu) { resolveRecursive(lu); }

void ER::resolveRecursive(ast::AstNode* node) {
   // Set the CU
   if(auto* cu = dynamic_cast<ast::CompilationUnit*>(node)) {
      cu_ = cu;
      if(diag.Verbose(2)) {
         auto dbg = diag.ReportDebug(2);
         dbg << "[*] Resolving compilation unit in: " << cu->location().toString()
             << "\n";
      }
   }
   // Set the CTX
   if(auto* ctx = dynamic_cast<ast::DeclContext*>(node)) {
      lctx_ = ctx;
   }
   // Visit the expression nodes
   if(auto* decl = dynamic_cast<ast::VarDecl*>(node)) {
      if(auto* init = decl->mut_init()) {
         if(diag.Verbose(2)) {
            auto dbg = diag.ReportDebug(2);
            dbg << "[*] Resolving initializer for variable: " << decl->name()
                << "\n";
            init->print(dbg.get(), 1);
         }
         Evaluate(init);
         TR->Evaluate(init);
         init->clear();
      }
   } else if(auto* stmt = dynamic_cast<ast::Stmt*>(node)) {
      for(auto* expr : stmt->mut_exprs()) {
         if(!expr) continue;
         if(diag.Verbose(2)) {
            auto dbg = diag.ReportDebug(2);
            dbg << "[*] Resolving expression in statement:\n";
            expr->print(dbg.get(), 1);
         }
         Evaluate(expr);
         TR->Evaluate(expr);
         expr->clear();
      }
   }
   // We want to avoid visiting nodes twice
   if(dynamic_cast<ast::DeclStmt*>(node)) return;
   // Visit the children recursively
   for(auto* child : node->mut_children()) {
      if(child) resolveRecursive(child);
   }
}

} // namespace semantic
