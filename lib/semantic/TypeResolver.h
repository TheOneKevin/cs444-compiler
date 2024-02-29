#pragma once

#include "ast/AST.h"
#include "ast/AstNode.h"
#include "ast/Expr.h"
#include "diagnostics/Diagnostics.h"
#include "diagnostics/Location.h"

namespace semantic {

/**
 * @brief Will resolve all types in the Expression. If a type cannot be resolved,
 * then we assume an invalid type error has occurred (i.e., type checking).
 */
class ExprTypeResolver final : public ast::ExprEvaluator<ast::Type const*> {
public:
   ExprTypeResolver(diagnostics::DiagnosticEngine& diag) : diag{diag} {}
   void resolve(ast::Expr *expr);

protected:
   using Type = ast::Type;
   using BinaryOp = ast::exprnode::BinaryOp;
   using UnaryOp = ast::exprnode::UnaryOp;
   using ExprValue = ast::exprnode::ExprValue;

   Type* mapValue(ExprValue& node) const override;
   Type* evalBinaryOp(BinaryOp& op, const Type* lhs,
                      const Type* rhs) const override;
   Type* evalUnaryOp(UnaryOp& op, const Type* rhs) const override;
   Type* evalMemberAccess(const Type* lhs, const Type* field) const override;
   Type* evalMethodCall(const Type* method, const op_array& args) const override;
   Type* evalNewObject(const Type* object, const op_array& args) const override;
   Type* evalNewArray(const Type* type, const Type* size) const override;
   Type* evalArrayAccess(const Type* array, const Type* index) const override;
   Type* evalCast(const Type* type, const Type* value) const override;

private:
   diagnostics::DiagnosticEngine& diag;
   SourceRange loc_;
};

} // namespace semantic
