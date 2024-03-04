#pragma once

#include <memory_resource>
#include <variant>

#include "ast/AstNode.h"
#include "ast/DeclContext.h"
#include "ast/ExprEvaluator.h"
#include "ast/ExprNode.h"
#include "diagnostics/Diagnostics.h"
#include "semantic/TypeResolver.h"
#include "utils/BumpAllocator.h"

namespace semantic {

/* ===--------------------------------------------------------------------=== */
// internal::
/* ===--------------------------------------------------------------------=== */

namespace internal {

struct ExprNameWrapper;

/**
 * @brief The ExprResolverTy struct is a variant that represents the different
 * things an expression can resolve to. They are as follows:
 * 1. A name wrapper is a chain of names that are being resolved
 * 2. An expression node is a single unresolved expression
 * 3. An expression node list is a list of resolved expressions
 */
using ExprResolverTy =
      std::variant<ExprNameWrapper*, ast::ExprNode*, ast::ExprNodeList>;

/**
 * @brief Represents a wrapper around a name that is being resolved. This
 * is a list of either ExprNameWrapper or ast::ExprNodeList.
 */
struct ExprNameWrapper {
   using PrevTy = std::variant<ExprNameWrapper*, ast::ExprNodeList>;
   enum class Type {
      PackageName,
      TypeName,
      ExpressionName,
      MethodName,
      SingleAmbiguousName
   };
   // Build an unresolved wrapper of Type given a name node
   ExprNameWrapper(Type type, ast::exprnode::MemberName* node)
         : type{type}, node{node}, resolution{std::nullopt}, prev{std::nullopt} {}
   // Reclassifies the name as a type name based on JLS 6.5.2
   void reclassify(Type type, ast::Decl const* resolution) {
      this->resolution = resolution;
      this->type = type;
   }
   // Reclassifies the name as a package name based on JLS 6.5.2
   void reclassify(Type type, NameResolver::Pkg const* resolution) {
      this->resolution = resolution;
      this->type = type;
   }
   // Resolves the underlying name to a declaration
   void resolve(ast::Decl const* resolution) { this->resolution = resolution; }
   // Function to verify invariants of the wrapper
   void verifyInvariants(Type expectedTy) const;
   // Get the previous value of the wrapper as a wrapper
   ExprNameWrapper* prevAsWrapper() const {
      assert(prev.has_value() && "No previous value");
      assert(std::holds_alternative<ExprNameWrapper*>(prev.value()) &&
             "Previous value is not a wrapper");
      return std::get<ExprNameWrapper*>(prev.value());
   }
   // Get the previous value of the wrapper as a list of expressions
   ExprNameWrapper* prevIfWrapper() const {
      assert(prev.has_value() && "No previous value");
      if(auto* list = std::get_if<ExprNameWrapper*>(&prev.value())) return *list;
      return nullptr;
   }
   /**
    * @brief If the previous value is a wrapper, unwrap the declaration.
    * However, if the previous value is a list of expressions, return the
    * the class representation of the type of the expression.
    *
    * @param TR The expression type resolver
    * @param NR The name resolver
    * @return ast::Decl const* Either a decl or a type represented as decl
    */
   ast::Decl const* prevAsDecl(ExprTypeResolver& TR, NameResolver& NR) const;

   Type type;
   ast::exprnode::MemberName* node;
   NameResolver::ConstImportOpt resolution;
   std::optional<PrevTy> prev;
};

} // namespace internal

/* ===--------------------------------------------------------------------=== */
// ExprResolver
/* ===--------------------------------------------------------------------=== */

class ExprResolver final : ast::ExprEvaluator<internal::ExprResolverTy> {
   using ETy = internal::ExprResolverTy;
   using Heap = std::pmr::memory_resource;

public:
   ExprResolver(diagnostics::DiagnosticEngine& diag, Heap* heap)
         : diag{diag}, heap{heap}, alloc{heap} {}
   void Init(ExprTypeResolver* TR, NameResolver* NR) {
      this->TR = TR;
      this->NR = NR;
   }
   void Resolve(ast::LinkingUnit* lu);

private:
   ETy EvaluateList(ast::ExprNodeList subexpr) override final {
      // Clear the heap
      if(auto h = dyn_cast<utils::CustomBufferResource*>(heap)) h->reset();
      // Call the base class implementation
      return ast::ExprEvaluator<internal::ExprResolverTy>::EvaluateList(subexpr);
   }

   void resolveRecursive(ast::AstNode* node);

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
   using ty_array = std::pmr::vector<ast::Type const*>;

   // Given a single ambiguous name, reclassify it into a package or type name
   internal::ExprNameWrapper* reclassifySingleAmbiguousName(
         internal::ExprNameWrapper* data) const;
   // Try to reclassify "data" into a declaration against "ctx"
   bool tryReclassifyDecl(internal::ExprNameWrapper& data,
                          ast::DeclContext const* ctx) const;
   // Try to reclassify "data" against an imported object/pkg "import"
   bool tryReclassifyImport(internal::ExprNameWrapper& data,
                            NameResolver::ConstImportOpt import) const;
   // Resolve access into a context (i.e., field member access)
   void resolveFieldAccess(internal::ExprNameWrapper* access) const;
   // Resolve access into a type (i.e., static member access)
   void resolveTypeAccess(internal::ExprNameWrapper* access) const;
   // Resolve access into a package -> either package or type is resolved
   void resolvePackageAccess(internal::ExprNameWrapper* access) const;
   // Resolve an expression node into a list (i.e., removes expr wrapper)
   ast::ExprNodeList resolveExprNode(const ETy node) const;
   // Resolves a single name node into a wrapped name. This is just a light
   // function over reclassifySingleAmbiguousName that allocates the wrapper.
   internal::ExprNameWrapper* resolveSingleName(
         ast::exprnode::MemberName* node) const {
      if(auto method = dyn_cast<ast::exprnode::MethodName*>(node))
         return alloc.new_object<internal::ExprNameWrapper>(
               internal::ExprNameWrapper::Type::MethodName, method);
      return reclassifySingleAmbiguousName(
            alloc.new_object<internal::ExprNameWrapper>(
                  internal::ExprNameWrapper::Type::SingleAmbiguousName, node));
   }
   // ???
   ast::ExprNodeList recursiveReduce(internal::ExprNameWrapper* node) const;
   // Gets the parent context the method is declared under
   ast::DeclContext const* getMethodParent(internal::ExprNameWrapper* node) const;
   // Resolves a method overload given a context and a list of argument types
   ast::MethodDecl const* resolveMethodOverload(ast::DeclContext const* ctx,
                                                std::string_view name,
                                                const ty_array& argtys) const;

private:
   diagnostics::DiagnosticEngine& diag;
   ast::CompilationUnit const* cu_;
   ast::DeclContext const* lctx_;
   semantic::NameResolver* NR;
   semantic::ExprTypeResolver* TR;
   mutable Heap* heap;
   mutable BumpAllocator alloc;
};

} // namespace semantic
