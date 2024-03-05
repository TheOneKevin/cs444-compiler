#pragma once

#include <memory_resource>
#include <variant>

#include "ast/AstNode.h"
#include "ast/DeclContext.h"
#include "ast/Expr.h"
#include "ast/ExprEvaluator.h"
#include "ast/ExprNode.h"
#include "diagnostics/Diagnostics.h"
#include "semantic/ExprTypeResolver.h"
#include "semantic/HierarchyChecker.h"
#include "semantic/Semantic.h"
#include "utils/BumpAllocator.h"
#include "utils/EnumMacros.h"

namespace semantic {

/* ===--------------------------------------------------------------------=== */
// internal::
/* ===--------------------------------------------------------------------=== */

namespace internal {

class ExprNameWrapper;

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
class ExprNameWrapper {
public:
   using PrevTy = std::variant<ExprNameWrapper*, ast::ExprNodeList>;

public:
#define NAME_TYPE_LIST(F) \
   F(PackageName)         \
   F(TypeName)            \
   F(ExpressionName)      \
   F(MethodName)          \
   F(SingleAmbiguousName)

   DECLARE_ENUM(Type, NAME_TYPE_LIST)

private:
   DECLARE_STRING_TABLE(Type, type_strings, NAME_TYPE_LIST)
#undef NAME_TYPE_LIST

public:
   /**
    * @brief Build an unresolved wrapper of Type given a name node,
    * represents a name particle represented by "node" of type "type".
    *
    * @param type The Java name type of the current particle
    * @param node The expression node representing the particle
    * @param op The operator joining the particle to the previous particle.
    *           If this is a single name, then op is nullptr.
    */
   ExprNameWrapper(Type type, ast::exprnode::MemberName* node,
                   ast::exprnode::MemberAccess* op)
         : node{node},
           op{op},
           type_{type},
           prev_{std::nullopt},
           resolution_{std::nullopt} {}
   // Reclassifies the name as a type name based on JLS 6.5.2
   void reclassify(Type type, ast::Decl const* resolution,
                   ast::Type const* typeResolution) {
      this->resolution_ = resolution;
      this->type_ = type;
      this->typeResolution_ = typeResolution;
   }
   // Reclassifies the name as a package name based on JLS 6.5.2
   void reclassify(Type type, NameResolver::Pkg const* resolution) {
      this->resolution_ = resolution;
      this->type_ = type;
      this->typeResolution_ = nullptr;
   }
   // Function to verify invariants of the wrapper
   void verifyInvariants(Type expectedTy) const;
   // Get the previous value of the wrapper as a wrapper
   ExprNameWrapper* prevAsWrapper() const {
      assert(prev_.has_value() && "No previous value");
      assert(std::holds_alternative<ExprNameWrapper*>(prev_.value()) &&
             "Previous value is not a wrapper");
      return std::get<ExprNameWrapper*>(prev_.value());
   }
   // Get the previous value of the wrapper as a list of expressions
   ExprNameWrapper* prevIfWrapper() const {
      assert(prev_.has_value() && "No previous value");
      if(auto* list = std::get_if<ExprNameWrapper*>(&prev_.value())) return *list;
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
   // Gets the "type of name" the current particle has been resolved to
   Type type() const { return type_; }
   // Gets the resolution of the name particle (must exist). The resolution
   // will be either a package or a declaration.
   auto resolution() const { return resolution_.value(); }
   // Gets the type of the resolution of the name particle.
   auto typeResolution() const { return typeResolution_; }
   // Sets the previous type of the wrapper
   void setPrev(std::optional<PrevTy> prev) {
      // Make sure if prev is a wrapper, it is not nullptr
      if(prev.has_value())
         if(auto wrapper = std::get_if<ExprNameWrapper*>(&prev.value()))
            assert(*wrapper && "Previous value is nullptr");
      prev_ = prev;
   }
   std::optional<PrevTy> prev() const { return prev_; }
   void dump() const;
   void dump(int indent) const;

public:
   ast::exprnode::MemberName* node;
   ast::exprnode::MemberAccess* op;

   // Private means the semantics of the wrapper must NOT be changed!
private:
   Type type_;
   std::optional<PrevTy> prev_;
   NameResolver::ConstImportOpt resolution_;
   ast::Type const* typeResolution_;
};

} // namespace internal

/* ===--------------------------------------------------------------------=== */
// ExprResolver
/* ===--------------------------------------------------------------------=== */

class ExprResolver final : public ast::ExprEvaluator<internal::ExprResolverTy> {
   using ETy = internal::ExprResolverTy;
   using Heap = std::pmr::memory_resource;

public:
   ExprResolver(diagnostics::DiagnosticEngine& diag, Heap* heap)
         : diag{diag}, heap{heap}, alloc{heap} {}
   void Init(ExprTypeResolver* TR, NameResolver* NR, ast::Semantic* Sema,
             semantic::HierarchyChecker* HC) {
      // FIXME(kevin): This API is ugly but its low priority to remove
      this->TR = TR;
      this->NR = NR;
      this->Sema = Sema;
      this->HC = HC;
   }
   void BeginCU(ast::CompilationUnit const* cu) { cu_ = cu; }
   void BeginContext(ast::DeclContext const* ctx) { lctx_ = ctx; }

   ETy EvaluateList(ast::ExprNodeList subexpr) override final {
      // Clear the heap
      if(auto h = dyn_cast<utils::CustomBufferResource*>(heap)) h->reset();
      // Call the base class implementation
      return ast::ExprEvaluator<internal::ExprResolverTy>::EvaluateList(subexpr);
   }

   // Resolve an expression node into a list (i.e., removes expr wrapper)
   ast::ExprNodeList ResolveExprNode(const ETy node) const;

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
   bool validate(ETy const& value) const override;

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
   // Resolves a single name node into a wrapped name. This is just a light
   // function over reclassifySingleAmbiguousName that allocates the wrapper.
   internal::ExprNameWrapper* resolveSingleName(
         ast::exprnode::MemberName* node) const {
      if(auto method = dyn_cast<ast::exprnode::MethodName*>(node)) {
         return alloc.new_object<internal::ExprNameWrapper>(
               internal::ExprNameWrapper::Type::MethodName, method, nullptr);
      }
      return reclassifySingleAmbiguousName(
            alloc.new_object<internal::ExprNameWrapper>(
                  internal::ExprNameWrapper::Type::SingleAmbiguousName,
                  node,
                  nullptr));
   }
   // ???
   ast::ExprNodeList recursiveReduce(internal::ExprNameWrapper* node) const;
   // Gets the parent context the method is declared under
   ast::DeclContext const* getMethodParent(internal::ExprNameWrapper* node) const;
   // Resolves a method overload given a context and a list of argument types
   ast::MethodDecl const* resolveMethodOverload(ast::DeclContext const* ctx,
                                                std::string_view name,
                                                const ty_array& argtys,
                                                bool isCtor) const;
   // Checks if a method is more specific than another: returns a > b
   bool isMethodMoreSpecific(ast::MethodDecl const* a,
                             ast::MethodDecl const* b) const;
   // Checks if the parameter types are applicable
   bool areParameterTypesApplicable(ast::MethodDecl const* method,
                                    const ty_array& argtys) const;

private:
   diagnostics::DiagnosticEngine& diag;
   ast::CompilationUnit const* cu_;
   ast::DeclContext const* lctx_;
   semantic::NameResolver* NR;
   semantic::ExprTypeResolver* TR;
   semantic::HierarchyChecker* HC;
   ast::Semantic* Sema;
   mutable Heap* heap;
   mutable BumpAllocator alloc;
};

} // namespace semantic
