#pragma once

#include "ast/ExprEvaluator.h"
#include "diagnostics/Diagnostics.h"
#include "semantic/NameResolver.h"

namespace semantic {

struct ExprStaticCheckerData {
   const ast::Decl* const decl;
   const ast::Type* const type;
   const bool isValue;
   // NOTE: After an operation, the temp value is not an instance variable
   // anymore because it is a new (local) value
   const bool isInstanceVar;
};

class ExprStaticChecker : public ast::ExprEvaluator<ExprStaticCheckerData> {
   using ETy = ExprStaticCheckerData;

public:
   ExprStaticChecker(diagnostics::DiagnosticEngine& diag,
                     semantic::NameResolver& NR)
         : diag{diag}, NR{NR}, isStaticContext{false}, loc_{} {}

public:
   void Evaluate(ast::Expr* expr, bool isStaticContext);

private: // Overriden methods
   using Type = ast::Type;
   using BinaryOp = ast::exprnode::BinaryOp;
   using UnaryOp = ast::exprnode::UnaryOp;
   using DotOp = ast::exprnode::MemberAccess;
   using MethodOp = ast::exprnode::MethodInvocation;
   using NewOp = ast::exprnode::ClassInstanceCreation;
   using NewArrayOp = ast::exprnode::ArrayInstanceCreation;
   using ArrayAccessOp = ast::exprnode::ArrayAccess;
   using CastOp = ast::exprnode::Cast;
   using ExprValue = ast::exprnode::ExprValue;

   ETy mapValue(ExprValue& node) const override;
   ETy evalBinaryOp(BinaryOp& op, const ETy lhs, const ETy rhs) const override;
   ETy evalUnaryOp(UnaryOp& op, const ETy rhs) const override;
   ETy evalMemberAccess(DotOp& op, const ETy lhs, const ETy field) const override;
   ETy evalMethodCall(MethodOp& op, const ETy method,
                      const op_array& args) const override;
   ETy evalNewObject(NewOp& op, const ETy object,
                     const op_array& args) const override;
   ETy evalNewArray(NewArrayOp& op, const ETy type, const ETy size) const override;
   ETy evalArrayAccess(ArrayAccessOp& op, const ETy array,
                       const ETy index) const override;
   ETy evalCast(CastOp& op, const ETy type, const ETy value) const override;

private:
   diagnostics::DiagnosticBuilder IllegalInstanceAccess() const;

private:
   diagnostics::DiagnosticEngine& diag;
   semantic::NameResolver& NR;
   bool isStaticContext;
   SourceRange loc_;
};

} // namespace semantic
