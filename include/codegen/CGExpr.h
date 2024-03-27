#pragma once

#include "ast/ExprEvaluator.h"
#include "tir/Context.h"
#include "tir/TIR.h"

namespace codegen {

class CGExprEvaluator final : public ast::ExprEvaluator<tir::Value*> {
public:
   explicit CGExprEvaluator(tir::Context& ctx) : ctx{ctx} {}

private:
   tir::Value* mapValue(ast::exprnode::ExprValue& node) const override;
   tir::Value* evalBinaryOp(ast::exprnode::BinaryOp& op, tir::Value* lhs,
                            tir::Value* rhs) const override;
   tir::Value* evalUnaryOp(ast::exprnode::UnaryOp& op,
                           tir::Value* rhs) const override;
   tir::Value* evalMemberAccess(ast::exprnode::MemberAccess& op, tir::Value* lhs,
                                tir::Value* field) const override;
   tir::Value* evalMethodCall(ast::exprnode::MethodInvocation& op,
                              tir::Value* method,
                              const op_array& args) const override;
   tir::Value* evalNewObject(ast::exprnode::ClassInstanceCreation& op,
                             tir::Value* object,
                             const op_array& args) const override;
   tir::Value* evalNewArray(ast::exprnode::ArrayInstanceCreation& op,
                            tir::Value* type, tir::Value* size) const override;
   tir::Value* evalArrayAccess(ast::exprnode::ArrayAccess& op, tir::Value* array,
                               tir::Value* index) const override;
   tir::Value* evalCast(ast::exprnode::Cast& op, tir::Value* type,
                        tir::Value* value) const override;

private:
   tir::Context& ctx;
};

} // namespace codegen
