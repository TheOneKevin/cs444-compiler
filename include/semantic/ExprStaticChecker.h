#pragma once

#include "ast/AstNode.h"
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

struct ExprStaticCheckerState {
   bool isStaticContext;
   bool isInstFieldInitializer;
   ast::ScopeID const* fieldScope;
   ExprStaticCheckerState()
         : isStaticContext{false},
           isInstFieldInitializer{false},
           fieldScope{nullptr} {}
};

class ExprStaticChecker : public ast::ExprEvaluator<ExprStaticCheckerData> {
   using ETy = ExprStaticCheckerData;

public:
   ExprStaticChecker(diagnostics::DiagnosticEngine& diag,
                     semantic::NameResolver& NR)
         : diag{diag}, NR{NR}, state{}, loc_{} {}

public:
   void Evaluate(ast::Expr* expr, ExprStaticCheckerState state);

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
   ETy evalBinaryOp(BinaryOp& op, ETy lhs, ETy rhs) const override;
   ETy evalUnaryOp(UnaryOp& op, ETy rhs) const override;
   ETy evalMemberAccess(DotOp& op, ETy lhs, ETy field) const override;
   ETy evalMethodCall(MethodOp& op, ETy method,
                      const op_array& args) const override;
   ETy evalNewObject(NewOp& op, ETy object, const op_array& args) const override;
   ETy evalNewArray(NewArrayOp& op, ETy type, ETy size) const override;
   ETy evalArrayAccess(ArrayAccessOp& op, ETy array, ETy index) const override;
   ETy evalCast(CastOp& op, ETy type, ETy value) const override;

private:
   void checkInstanceVar(ETy var) const;

private:
   diagnostics::DiagnosticEngine& diag;
   semantic::NameResolver& NR;
   ExprStaticCheckerState state;
   SourceRange loc_;
};

} // namespace semantic
