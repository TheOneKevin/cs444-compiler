#pragma once

#include <memory_resource>

#include "ast/AstNode.h"
#include "ast/Expr.h"
#include "ast/ExprEvaluator.h"
#include "ast/ExprNode.h"
#include "diagnostics/Diagnostics.h"
#include "diagnostics/Location.h"
#include "semantic/HierarchyChecker.h"
#include "semantic/NameResolver.h"
#include "semantic/Semantic.h"
#include "utils/BumpAllocator.h"

namespace semantic {

/**
 * @brief Will resolve all types in the Expression. If a type cannot be resolved,
 * then we assume an invalid type error has occurred (i.e., type checking).
 */
class ExprTypeResolver final : private ast::ExprEvaluator<ast::Type const*> {
   using Heap = std::pmr::memory_resource;

public:
   ExprTypeResolver(diagnostics::DiagnosticEngine& diag, Heap* heap,
                    ast::Semantic& sema)
         : diag{diag}, heap{heap}, alloc{heap}, sema{sema} {}
   void Init(HierarchyChecker* HC, NameResolver* NR) {
      this->HC = HC;
      this->NR = NR;
   }

   ast::Type const* EvalList(ast::ExprNodeList& list, const SourceRange& loc) {
      loc_ = loc;
      return ast::ExprEvaluator<ast::Type const*>::EvaluateList(list);
   }

   ast::Type const* Evaluate(ast::Expr* node) {
      loc_ = node->location();
      return ast::ExprEvaluator<ast::Type const*>::Evaluate(node);
   }

protected:
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
   using ETy = ast::Type const*;

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
   bool validatePop(ETy const& type) const override {
      if(type) return true;
      if(opStackSize() == 0) return true;
      throw diag.ReportError(loc_) << "cannot use void type in expression";
   }

public:
   // @brief Check if it is possible to convert
   // lhs to rhs (call this latter type T) by assignment conversion (§5.2);
   bool isAssignableTo(const Type* lhs, const Type* rhs) const;

   // @brief check if it is valid to cast exprType to castType
   bool isValidCast(const Type* exprType, const Type* castType) const;

   /// @brief Check if the type is a string type, including conversion to
   /// java.lang.String implicitly.
   bool isTypeString(const Type* type) const;

   /// @brief Check if the type is a reference type or an array type.
   bool isReferenceOrArrType(const Type* type) const;

private:
   diagnostics::DiagnosticEngine& diag;
   HierarchyChecker* HC;
   NameResolver* NR;
   SourceRange loc_;
   Heap* heap;
   mutable BumpAllocator alloc;
   ast::Semantic& sema;
};

} // namespace semantic
