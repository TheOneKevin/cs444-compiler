#include "semantic/ExprResolver.h"

#include <string>
#include <string_view>
#include <variant>

#include "ast/AST.h"
#include "ast/AstNode.h"
#include "ast/DeclContext.h"
#include "ast/ExprNode.h"
#include "diagnostics/Location.h"

namespace semantic {

namespace ex = ast::exprnode;
using ETy = internal::ExprResolverT;
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
   if(!import.has_value()) return false;
   if(std::holds_alternative<const ast::Decl*>(import.value())) {
      auto decl = std::get<const ast::Decl*>(import.value());
      // If declaration is null, then there is an import-on-demand conflict
      if(!decl) {
         throw diag.ReportError(SourceRange{})
               << "Ambiguous import-on-demand conflict";
      }
      data.reclassify(ExprNameWrapper::Type::TypeName, decl);
      return true;
   } else if(std::holds_alternative<const Pkg*>(import.value())) {
      auto pkg = std::get<const Pkg*>(import.value());
      assert(pkg && "Expected non-null package here");
      data.reclassify(ExprNameWrapper::Type::PackageName, pkg);
      return true;
   }
}

ETy ER::reclassifySingleAmbiguousName(ExprNameWrapper* data) const {
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
   throw diag.ReportError(SourceRange{})
         << "Unknown error when attempting to resolve import type";
}

void ER::resolveFieldAccess(ExprNameWrapper* access) const {
   // First, verify invariants if access is a field access
   auto prev = access->prev;
   assert(prev && "Expected non-null previous node");
   access->verifyInvariants(ExprNameWrapper::Type::SingleAmbiguousName);
   prev->verifyInvariants(ExprNameWrapper::Type::ExpressionName);
   // Now we can get the "decl" and "name" to resolve against
   auto decl = std::get<ast::Decl const*>(prev->resolution.value());
   auto ctx = dynamic_cast<ast::ClassDecl const*>(decl);
   auto name = access->node->name();
   // If "decl" isnt context, then we tried to resolve against something bad :(
   if(!ctx) {
      throw diag.ReportError(SourceRange{})
            << "field access to non-class type: " << decl->getCanonicalName();
   }
   // Now we check if "name" is a field of "decl"
   auto field = ctx->lookupDecl(name);
   if(!field) {
      throw diag.ReportError(SourceRange{})
            << "field access to undeclared field: " << name;
   }
   // Field must be either a FieldDecl or a MethodDecl
   assert(dynamic_cast<ast::FieldDecl const*>(field) ||
          dynamic_cast<ast::MethodDecl const*>(field));
   // Now we can reclassify the access node
   access->reclassify(ExprNameWrapper::Type::ExpressionName, field);
}

void ER::resolveTypeAccess(internal::ExprNameWrapper* access) const {
   // We'll try to resolve the type access as a field access
   resolveFieldAccess(access);
   // With the additional constraint that the field must be static
   auto field = std::get<ast::Decl const*>(access->resolution.value());
   ast::Modifiers mods;
   if(auto fieldDecl = dynamic_cast<ast::FieldDecl const*>(field)) {
      mods = fieldDecl->modifiers();
   } else if(auto methodDecl = dynamic_cast<ast::MethodDecl const*>(field)) {
      mods = methodDecl->modifiers();
   }
   if(!mods.isStatic()) {
      throw diag.ReportError(SourceRange{})
            << "attempted to access non-static member: "
            << field->getCanonicalName();
   }
   // We've reached here which means the field access is valid
   access->prev = nullptr;
}

void ER::resolvePackageAccess(internal::ExprNameWrapper* access) const {
   // First, verify invariants if access is a package access
   auto prev = access->prev;
   assert(prev && "Expected non-null previous node");
   access->verifyInvariants(ExprNameWrapper::Type::SingleAmbiguousName);
   prev->verifyInvariants(ExprNameWrapper::Type::PackageName);
   // Now we can get the "pkg" and "name" to resolve against
   auto pkg = std::get<Pkg const*>(prev->resolution.value());
   auto name = access->node->name();
   assert(pkg && "Expected non-null package here");
   // Now we check if "name" is a package of "pkg"
   auto subpkg = pkg->lookupPkg(name, alloc);
   if(!subpkg) {
      throw diag.ReportError(SourceRange{})
            << "package access to undeclared package: " << name;
   }
   // Now we can reclassify the access node
   access->reclassify(ExprNameWrapper::Type::PackageName, subpkg);
   access->prev = nullptr;
}

void ExprNameWrapper::verifyInvariants(ExprNameWrapper::Type expectedTy) const {
   assert(type == expectedTy && "Expected type does not match actual type");
   // TODO(kevin): Add more invariants here
}

ast::ExprNodeList recursiveReduce(ExprNameWrapper* node) {
   node->verifyInvariants(ExprNameWrapper::Type::ExpressionName);
   auto decl = std::get<ast::Decl const*>(node->resolution.value());
   auto* expr = node->node;

   // Base case, no more nodes to reduce. Return singleton list.
   if(!node->prev || node->prev->type != ExprNameWrapper::Type::ExpressionName) {
      expr->resolve(decl);
      return ast::ExprNodeList{expr};
   }

   // Otherwise, chain the previous node to the current node
   auto list = recursiveReduce(node->prev);
   expr->resolve(decl);
   list.push_back(expr);
   return list;
}

ast::ExprNodeList resolveExprNode(const ETy node) {
   // If node is an ExprNode, then no resolution is needed
   if(std::holds_alternative<ast::ExprNode*>(node))
      return ast::ExprNodeList{std::get<ast::ExprNode*>(node)};
   // If the node is a list, then also no resolution is needed
   if(std::holds_alternative<ast::ExprNodeList>(node))
      return std::get<ast::ExprNodeList>(node);
   // Otherwise, get the wrapped node and build resolution
   return recursiveReduce(std::get<ExprNameWrapper*>(node));
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
   ExprNameWrapper* Q = nullptr;
   if(std::holds_alternative<ast::ExprNode*>(lhs)) {
      auto simpleName =
            dynamic_cast<ex::MemberName*>(std::get<ast::ExprNode*>(lhs));
      assert(simpleName && "Malformed node. Expected MemberName here.");
      Q = alloc.new_object<ExprNameWrapper>(
            ExprNameWrapper::Type::SingleAmbiguousName, simpleName);
      Q = std::get<ExprNameWrapper*>(reclassifySingleAmbiguousName(Q));
   } else if(std::holds_alternative<ExprNameWrapper*>(lhs)) {
      Q = std::get<ExprNameWrapper*>(lhs);
   } else {
      assert(false && "Not implemented. Cannot access field of complex expressions.");
   }
   // The LHS must be an ExpressionName, TypeName or PackageName
   assert(
         (Q->type == ExprNameWrapper::Type::ExpressionName ||
          Q->type == ExprNameWrapper::Type::TypeName ||
          Q->type == ExprNameWrapper::Type::PackageName) &&
         "Malformed node. Expected ExpressionName, TypeName or PackageName here.");
   // Now grab the id and cast it to the appropriate type
   auto const* fieldNode =
         dynamic_cast<ex::MemberName*>(std::get<ast::ExprNode*>(id));
   assert(fieldNode && "Malformed node. Expected MemberName here.");
   // Allocate a new node as the member access to represent "Id" in Lhs . Id
   auto newQ = alloc.new_object<ExprNameWrapper>(
         ExprNameWrapper::Type::SingleAmbiguousName, fieldNode);
   newQ->prev = Q;
   // And we can build the reduced expression now
   switch(Q->type) {
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

ETy ER::evalMethodCall(const ETy method, const op_array& args) const {
   ast::ExprNodeList list{};
   list.concat(resolveExprNode(method));
   for(auto& arg : args) list.concat(resolveExprNode(arg));
   return list;
}

ETy ER::evalNewObject(const ETy object, const op_array& args) const {
   ast::ExprNodeList list{};
   list.concat(resolveExprNode(object));
   for(auto& arg : args) list.concat(resolveExprNode(arg));
   return list;
}

ETy ER::evalNewArray(const ETy type, const ETy size) const {
   ast::ExprNodeList list{};
   list.concat(resolveExprNode(type));
   list.concat(resolveExprNode(size));
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
   list.concat(resolveExprNode(type));
   list.concat(resolveExprNode(value));
   return list;
}

} // namespace semantic
