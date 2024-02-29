#include "semantic/ExprResolver.h"

#include <string>
#include <string_view>

#include "ast/AST.h"
#include "ast/AstNode.h"
#include "ast/ExprNode.h"

namespace semantic {

namespace ex = ast::exprnode;
using ETy = internal::ExprResolverT;
using ER = ExprResolver;
using internal::ExprNameWrapper;

/* ===--------------------------------------------------------------------=== */
//
/* ===--------------------------------------------------------------------=== */

static bool recursiveReclassify(ExprNameWrapper& data,
                                ast::DeclContext const* ctx) {
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
   return recursiveReclassify(data, parentCtx);
}

ETy ER::reclassifySingleAmbiguousName(ETy const& node) const {
   // JLS 6.5.2 Reclassification of Contextually Ambiguous Names

   auto data = std::get<ExprNameWrapper>(node);
   assert(data.type == ExprNameWrapper::Type::SingleAmbiguousName &&
          "Expected SingleAmbiguousName here");
   auto copy = data;

   // 1. If the Identifier appears within the scope (§6.3) of a local variable
   //    declaration (§14.4) or parameter declaration (§8.4.1, §8.8.1, §14.19)
   //    or field declaration (§8.3) with that name, then the AmbiguousName is
   //    reclassified as an ExpressionName.
   if(recursiveReclassify(copy, localContext)) return copy;

   // 2. Otherwise, if a type of that name is declared in the compilation unit
   //   (§7.3) containing the Identifier, either by a single-type-import
   //   declaration (§7.5.1) or by a top level class (§8) or interface type
   //   declaration (§9), then the AmbiguousName is reclassified as a TypeName.
   
   // ...
}

/* ===--------------------------------------------------------------------=== */
//
/* ===--------------------------------------------------------------------=== */

ETy ER::mapValue(ExprValue const& node) const {
   using namespace internal;
   if(auto name = dynamic_cast<ex::MemberName const*>(&node)) {
      return ExprNameWrapper{ExprNameWrapper::Type::SingleAmbiguousName, name};
   }
   return &node;
}

ETy ER::evalMemberAccess(const ETy lhs, const ETy field) const {
   auto data = std::get<internal::ExprNameWrapper>(lhs);
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
