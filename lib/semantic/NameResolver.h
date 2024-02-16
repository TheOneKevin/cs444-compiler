#pragma once

#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "ast/AstNode.h"
#include "diagnostics/Diagnostics.h"
#include "utils/BumpAllocator.h"

// Forward declarations
namespace ast {
class LinkingUnit;
class CompilationUnit;
class UnresolvedType;
class InterfaceDecl;
class ClassDecl;
} // namespace ast

namespace semantic {

class NameResolver {
private:
   /// @brief Represents a tree of packages. The leaf nodes are declarations.
   struct Pkg {
      using Child = std::variant<ast::Decl*, Pkg*>;
      std::string_view name;
      std::pmr::unordered_map<std::pmr::string, Child> children;
      Pkg(BumpAllocator& alloc) : name{}, children{alloc} {}
      Pkg(BumpAllocator& alloc, std::string_view name)
            : name{name}, children{alloc} {}
   };

public:
   /**
    * @brief Construct a new Name Resolver object given the entire linking
    * unit (and all its symbols).
    */
   NameResolver(BumpAllocator& alloc, diagnostics::DiagnosticEngine& diag,
                ast::LinkingUnit* lu)
         : alloc{alloc}, diag{diag}, lu_{lu}, importsMap_{alloc} {
      buildSymbolTable();
   }

   /**
    * @brief Called to begin the resolution of a compilation unit.
    * This will build the import table (and any other data structures) to help
    * resolve types within the compilation unit.
    *
    * @param cu The compilation unit to begin resolution for.
    */
   void BeginContext(ast::CompilationUnit* cu);

   /// @brief Resolves all types in the current linking unit.
   void Resolve();

   /// @brief Resolves the type in-place (does not return anything)
   /// @param type The unresolved type to resolve.
   void ResolveType(ast::UnresolvedType* type);

private:
   void resolveInterface(ast::InterfaceDecl* decl);
   void resolveClass(ast::ClassDecl* decl);

private:
   /// @brief Builds the symbol lookup tables and any other data structures
   /// or maps to facilitate name resolution.
   void buildSymbolTable();
   /// @brief Resolves an unresolved type to a non-leaf node in the tree
   /// @param t The unresolved type to resolve.
   /// @return The package node that the unresolved type resolves to.
   NameResolver::Pkg* resolveAstTy(ast::UnresolvedType const* t) const;

private:
   BumpAllocator& alloc;
   diagnostics::DiagnosticEngine& diag;
   ast::LinkingUnit* lu_;
   /// @brief The current compilation unit being resolved.
   ast::CompilationUnit* cu_;
   /// @brief The import map for the current compilation unit.
   std::pmr::unordered_map<std::pmr::string, Pkg::Child> importsMap_;
   /// @brief The root of the symbol table (more of a tree than table).
   Pkg* rootPkg_;
};

} // namespace semantic
