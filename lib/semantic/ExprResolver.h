#pragma once

#include <variant>

#include "ast/AstNode.h"
#include "ast/DeclContext.h"
#include "ast/ExprEvaluator.h"
#include "ast/ExprNode.h"
#include "diagnostics/Diagnostics.h"
#include "utils/BumpAllocator.h"

namespace semantic {

namespace internal {

struct ExprNameWrapper {
   enum class Type {
      PackageName,
      TypeName,
      ExpressionName,
      MethodName,
      SingleAmbiguousName
   };

   ExprNameWrapper(Type type, ast::exprnode::MemberName* node)
         : type{type}, node{node}, resolution{std::nullopt} {}

   void reclassify(Type type, ast::Decl const* resolution) {
      this->resolution = resolution;
      this->type = type;
   }

   void reclassify(Type type, NameResolver::Pkg const* resolution) {
      this->resolution = resolution;
      this->type = type;
   }

   void resolve(ast::Decl const* resolution) { this->resolution = resolution; }

   void verifyInvariants(Type expectedTy) const;

   Type type;
   ast::exprnode::MemberName* node;
   NameResolver::ConstImportOpt resolution;
   ExprNameWrapper* prev = nullptr;
};

using ExprResolverT =
      std::variant<ExprNameWrapper*, ast::ExprNode*, ast::ExprNodeList>;

} // namespace internal

class ExprResolver : ast::ExprEvaluator<internal::ExprResolverT> {
   using ETy = internal::ExprResolverT;

public:
   ExprResolver(diagnostics::DiagnosticEngine& diag, BumpAllocator& alloc)
         : diag{diag}, alloc{alloc} {}
   void Init(NameResolver* NR) { this->NR = NR; }
   void Resolve(ast::LinkingUnit* lu);

private:
   void resolveRecursive(ast::AstNode* node);

protected:
   using Type = ast::Type;
   using BinaryOp = ast::exprnode::BinaryOp;
   using UnaryOp = ast::exprnode::UnaryOp;
   using ExprValue = ast::exprnode::ExprValue;

   ETy mapValue(ExprValue& node) const override;
   ETy evalBinaryOp(BinaryOp& op, const ETy lhs, const ETy rhs) const override;
   ETy evalUnaryOp(UnaryOp& op, const ETy rhs) const override;
   ETy evalMemberAccess(const ETy lhs, const ETy field) const override;
   ETy evalMethodCall(const ETy method, const op_array& args) const override;
   ETy evalNewObject(const ETy object, const op_array& args) const override;
   ETy evalNewArray(const ETy type, const ETy size) const override;
   ETy evalArrayAccess(const ETy array, const ETy index) const override;
   ETy evalCast(const ETy type, const ETy value) const override;

private:
   // Given a single ambiguous name, reclassify it into a package or type name
   ETy reclassifySingleAmbiguousName(internal::ExprNameWrapper* data) const;
   // Try to reclassify "data" into a declaration against "ctx"
   bool tryReclassifyDecl(internal::ExprNameWrapper& data,
                          ast::DeclContext const* ctx) const;
   // Try to reclassify "data" against an imported object/pkg "import"
   bool tryReclassifyImport(internal::ExprNameWrapper& data,
                            NameResolver::ConstImportOpt import) const;
   // Resolve a field access (given ctx and field name inside "access")
   void resolveFieldAccess(internal::ExprNameWrapper* access) const;
   void resolveTypeAccess(internal::ExprNameWrapper* access) const;
   void resolvePackageAccess(internal::ExprNameWrapper* access) const;
   ast::ExprNodeList resolveExprNode(const ETy node) const;
   ast::ExprNodeList resolveMethodNode(const ETy node) const;

private:
   diagnostics::DiagnosticEngine& diag;
   ast::CompilationUnit const* cu_;
   ast::DeclContext const* lctx_;
   semantic::NameResolver* NR;
   BumpAllocator& alloc;
};

} // namespace semantic
