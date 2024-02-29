#pragma once

#include <variant>

#include "ast/AstNode.h"
#include "ast/Expr.h"
#include "ast/ExprNode.h"
#include "diagnostics/Diagnostics.h"

namespace semantic {

namespace internal {

struct ExprNameWrapper {
   enum class Type {
      PackageName,
      TypeName,
      ExpressionName,
      MethodName,
      PackageOrTypeName,
      SingleAmbiguousName
   };

   ExprNameWrapper(Type type, ast::exprnode::MemberName const* node)
         : type{type}, node{node}, resolution{nullptr} {}

   void reclassify(Type type, ast::Decl const* resolution) {
      this->resolution = resolution;
      this->type = type;
   }

public:
   Type type;
   ast::exprnode::MemberName const* node;
   ast::Decl const* resolution;
};

using ExprResolverT = std::variant<ExprNameWrapper, ast::ExprNode const*>;

} // namespace internal

class ExprResolver : ast::ExprEvaluator<internal::ExprResolverT> {
   using ETy = internal::ExprResolverT;

public:
   ExprResolver(diagnostics::DiagnosticEngine& diag) : diag{diag} {}

protected:
   using Type = ast::Type;
   using BinaryOp = ast::exprnode::BinaryOp;
   using UnaryOp = ast::exprnode::UnaryOp;
   using ExprValue = ast::exprnode::ExprValue;

   ETy mapValue(ExprValue const& node) const override;
   ETy evalBinaryOp(const BinaryOp op, const ETy lhs,
                    const ETy rhs) const override;
   ETy evalUnaryOp(const UnaryOp op, const ETy rhs) const override;
   ETy evalMemberAccess(const ETy lhs, const ETy field) const override;
   ETy evalMethodCall(const ETy method, const op_array& args) const override;
   ETy evalNewObject(const ETy object, const op_array& args) const override;
   ETy evalNewArray(const ETy type, const ETy size) const override;
   ETy evalArrayAccess(const ETy array, const ETy index) const override;
   ETy evalCast(const ETy type, const ETy value) const override;

private:
   ETy reclassifySingleAmbiguousName(ETy const& node) const;

private:
   diagnostics::DiagnosticEngine& diag;
   ast::DeclContext* localContext;
};

} // namespace semantic
