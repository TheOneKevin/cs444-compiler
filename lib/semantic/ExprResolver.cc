#include "semantic/ExprResolver.h"

#include <string>
#include <string_view>
#include <variant>

#include "ast/AST.h"
#include "ast/AstNode.h"
#include "ast/ExprNode.h"
#include "diagnostics/Location.h"

namespace semantic {

namespace ex = ast::exprnode;
using ETy = internal::ExprResolverT;
using ER = ExprResolver;
using internal::ExprNameWrapper;

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
   using Pkg = semantic::NameResolver::Pkg;
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

void ER::validateFieldAccess(ExprNameWrapper* access) const {
   // First, verify invariants if access is a field access
   auto prev = access->prev;
   assert(prev && "Expected non-null previous node");
   access->verifyInvariants(ExprNameWrapper::Type::SingleAmbiguousName);
   prev->verifyInvariants(ExprNameWrapper::Type::ExpressionName);
   // Now we can get the "decl" and "name" to resolve against
   auto decl = std::get<ast::Decl const*>(prev->resolution.value());
   auto ctx = dynamic_cast<ast::DeclContext const*>(decl);
   auto name = access->node->name();
   // If "decl" isnt context, then we tried to resolve against something bad :(
   if(!ctx) {
      throw diag.ReportError(SourceRange{})
            << "Field access to non-context type: " << decl->getCanonicalName();
   }
   // Now we check if "name" is a field of "decl"
   auto field = ctx->lookupDecl(name);
   if(!field) {
      throw diag.ReportError(SourceRange{})
            << "Field access to undeclared field: " << name;
   }
   access->resolve(field);
}

/* ===--------------------------------------------------------------------=== */
//
/* ===--------------------------------------------------------------------=== */

void ExprNameWrapper::verifyInvariants(ExprNameWrapper::Type expectedTy) const {
   assert(type == expectedTy && "Expected type does not match actual type");
   // TODO(kevin): Add more invariants here
}

/* ===--------------------------------------------------------------------=== */
//
/* ===--------------------------------------------------------------------=== */

ETy ER::mapValue(ExprValue const& node) const {
   using namespace internal;
   if(auto name = dynamic_cast<ex::MemberName const*>(&node)) {
      auto node = alloc.new_object<ExprNameWrapper>(
            ExprNameWrapper::Type::SingleAmbiguousName, name);
      node = std::get<ExprNameWrapper*>(reclassifySingleAmbiguousName(node));
   }
   return &node;
}

ETy ER::evalMemberAccess(const ETy lhs, const ETy field) const {
   auto lhsdata{std::get<ExprNameWrapper*>(lhs)};
   // The LHS must be an ExpressionName, TypeName or PackageName
   assert(
         (lhsdata->type == ExprNameWrapper::Type::ExpressionName ||
          lhsdata->type == ExprNameWrapper::Type::TypeName ||
          lhsdata->type == ExprNameWrapper::Type::PackageName) &&
         "Malformed node. Expected ExpressionName, TypeName or PackageName here.");
   // Now grab the field and cast it to the appropriate type
   auto const* fieldNode = dynamic_cast<ex::MemberName const*>(
         std::get<ast::ExprNode const*>(field));
   assert(fieldNode && "Malformed node. Expected MemberName here.");
   // And we can build the reduced expression now
   switch(lhsdata->type) {
      case ExprNameWrapper::Type::ExpressionName: {
         auto newNode = alloc.new_object<ExprNameWrapper>(
               ExprNameWrapper::Type::ExpressionName, fieldNode);
         newNode->prev = lhsdata;
         validateFieldAccess(newNode);
         return newNode;
      }
      case ExprNameWrapper::Type::TypeName: {
         // FIXME(kevin): TypeName . Id means Id must be a static member
         break;
      }
      case ExprNameWrapper::Type::PackageName: {
         // FIXME(kevin): PackageName . Id means Id must be a type or package
         break;
      }
      default:
         break;
   }
   assert(false && "Not reached");
}

ETy ER::evalBinaryOp(const BinaryOp op, const ETy lhs, const ETy rhs) const {
   // Implementation goes here
}

ETy ER::evalUnaryOp(const UnaryOp op, const ETy rhs) const {
   // Implementation goes here
}

ETy ER::evalMethodCall(const ETy method, const op_array& args) const {
   // Implementation goes here
}

ETy ER::evalNewObject(const ETy object, const op_array& args) const {
   // Implementation goes here
}

ETy ER::evalNewArray(const ETy type, const ETy size) const {
   // Implementation goes here
}

ETy ER::evalArrayAccess(const ETy array, const ETy index) const {
   // Implementation goes here
}

ETy ER::evalCast(const ETy type, const ETy value) const {
   // Implementation goes here
}

} // namespace semantic
