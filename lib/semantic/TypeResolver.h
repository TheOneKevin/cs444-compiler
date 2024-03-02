#pragma once

#include <memory_resource>
#include "ast/AstNode.h"
#include "ast/Expr.h"
#include "ast/ExprEvaluator.h"
#include "diagnostics/Diagnostics.h"
#include "diagnostics/Location.h"
#include "semantic/HierarchyChecker.h"
#include "utils/BumpAllocator.h"

namespace semantic {

/**
 * @brief Will resolve all types in the Expression. If a type cannot be resolved,
 * then we assume an invalid type error has occurred (i.e., type checking).
 */
class ExprTypeResolver final : public ast::ExprEvaluator<ast::Type const*> {
   using Heap = std::pmr::memory_resource;
public:
   ExprTypeResolver(diagnostics::DiagnosticEngine& diag, Heap *heap)
         : diag{diag}, heap{heap}, alloc{heap} {}
   void Init(HierarchyChecker* HC, NameResolver* NR) {
      this->HC = HC;
      this->NR = NR;
   }
   void resolve(ast::Expr* expr);

protected:
   using Type = ast::Type;
   using BinaryOp = ast::exprnode::BinaryOp;
   using UnaryOp = ast::exprnode::UnaryOp;
   using ExprValue = ast::exprnode::ExprValue;

   Type const* mapValue(ExprValue& node) const override;
   Type const* evalBinaryOp(BinaryOp& op, const Type* lhs,
                            const Type* rhs) const override;
   Type const* evalUnaryOp(UnaryOp& op, const Type* rhs) const override;
   Type const* evalMemberAccess(const Type* lhs, const Type* field) const override;
   Type const* evalMethodCall(const Type* method,
                              const op_array& args) const override;
   Type const* evalNewObject(const Type* object,
                             const op_array& args) const override;
   Type const* evalNewArray(const Type* type, const Type* size) const override;
   Type const* evalArrayAccess(const Type* array,
                               const Type* index) const override;
   Type const* evalCast(const Type* type, const Type* value) const override;

private:
   diagnostics::DiagnosticEngine& diag;
   HierarchyChecker* HC;
   NameResolver* NR;
   SourceRange loc_;
   Heap *heap;
   mutable BumpAllocator alloc;

   // @brief Check if it is possible to convert
   // lhs to rhs (call this latter type T) by assignment conversion (§5.2);
   bool isAssignableTo(const Type* lhs, const Type* rhs) const;

   // @brieg check if it is valid to cast exprType to castType
   bool isValidCast(const Type* exprType, const Type* castType) const;

   // (castType) {exprType: a + b} 
};

} // namespace semantic
