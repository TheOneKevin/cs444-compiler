#pragma once

#include <iostream>
#include <memory_resource>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <variant>

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
class MethodDecl;
class ReferenceType;
class Semantic;
class Expr;
} // namespace ast

namespace semantic {

class NameResolver final : public ast::TypeResolver {
public:
   struct Pkg;
   using ConstImport = std::variant<ast::Decl const*, Pkg const*>;
   using ConstImportOpt = std::optional<ConstImport>;

   /// @brief Represents a tree of packages. The leaf nodes are declarations.
   struct Pkg {
   private:
      using Child = std::variant<ast::Decl*, Pkg*>;
      friend class NameResolver;
      std::string_view name;
      std::pmr::unordered_map<std::pmr::string, Child> children;

   public:
      Pkg(BumpAllocator& alloc) : name{}, children{alloc} {}
      Pkg(BumpAllocator& alloc, std::string_view name)
            : name{name}, children{alloc} {}
      
      /**
       * @brief Gets a child package by name. If the child is not found, then
       * nullptr is returned.
       * @param name The name of the child package to get.
       * @param alloc The allocator to use for the string.
       * @return Pkg const* The child package if found, otherwise nullptr.
       */
      ConstImportOpt lookup(std::string_view name, BumpAllocator& alloc) const {
         auto it = children.find(std::pmr::string{name, alloc});
         if(it == children.end()) return std::nullopt;
         if(auto* pkg = std::get_if<Pkg*>(&it->second)) return *pkg;
         return std::get<ast::Decl*>(it->second);
      }

   public:
      std::ostream& print(std::ostream& os, int indentation = 0) const;
      void dump() const;
   };

private:
   // Delete copy and move constructors and assignment operators
   NameResolver(NameResolver const&) = delete;
   NameResolver(NameResolver&&) = delete;
   NameResolver& operator=(NameResolver const&) = delete;
   NameResolver& operator=(NameResolver&&) = delete;

public:
   /**
    * @brief Construct a new Name Resolver object.
    *
    * @param alloc The bump allocator to use for all allocations.
    * @param diag The diagnostic engine to use for all diagnostics.
    */
   NameResolver(BumpAllocator& alloc, diagnostics::DiagnosticEngine& diag)
         : alloc{alloc}, diag{diag}, lu_{nullptr}, importsMap_{alloc} {}

   /**
    * @brief Initializes a new Name Resolver object given the entire linking
    * unit (and all its symbols). After this, call Resolve() to resolve all
    * types in the linking unit.
    *
    * @param lu The linking unit to resolve.
    */
   void Init(ast::LinkingUnit* lu, ast::Semantic* sema) {
      lu_ = lu;
      sema_ = sema;
      buildSymbolTable();
      populateJavaLangCache();
   }

   /**
    * @brief Resolves all types in the current linking unit.
    */
   void Resolve();

   /**
    * @brief Resolves the type in-place (does not return anything)
    *
    * @param type The unresolved type to resolve.
    */
   void ResolveType(ast::UnresolvedType* type) override;

   /**
    * @brief Get an import object from the import table of the given
    * compilation unit. This object can either be a package or a declaration.
    *
    * @param cu The compilation unit to get the import from.
    * @param name The name of the import to get.
    * @param r An optional memory resource to use for allocations.
    * @return ConstImport Returns nullopt if the import is not found. Otheriwse,
    * returns the import object from the import table. Note, the package object
    * will never be null. However, a declaration object can be null if the
    * import-on-demand results in an unresolvable declaration.
    */
   ConstImportOpt GetImport(
         ast::CompilationUnit const* cu, std::string_view name,
         std::pmr::memory_resource* r = std::pmr::get_default_resource()) const;

   /**
    * @brief Get a struct with all the java.lang.* classes and interfaces.
    * @return auto An anonymous struct with all the java.lang.* types.
    */
   auto GetJavaLang() const { return java_lang_; }
   /**
    * @brief Get the Array Prototype object. This is a decl representing
    * the prototype of the array class.
    */
   auto GetArrayPrototype() const { return arrayPrototype_; }

public:
   /// @brief Dumps the symbol and import tables to the output stream.
   void dump() const;

   /// @brief Dumps the import table to the output stream.
   void dumpImports(ast::CompilationUnit const* cu) const;

   /// @brief Dumps the import table to the output stream.
   void dumpImports() const;

private:
   using ChildOpt = std::optional<Pkg::Child>;

   /// @brief Resolves the AST recursively
   void resolveRecursive(ast::AstNode* node);

   /// @brief Resolves the expression recursively
   void resolveExpr(ast::Expr* node);
   
   /// @brief Disallows java.lang.Object from extending itself
   void replaceObjectClass(ast::AstNode* node);

   /**
    * @brief Called to begin the resolution of a compilation unit.
    * This will build the import table (and any other data structures) to help
    * resolve types within the compilation unit.
    *
    * @param cu The compilation unit to begin resolution for.
    */
   void beginContext(ast::CompilationUnit* cu);

   /// @brief Builds the symbol lookup tables and any other data structures
   /// or maps to facilitate name resolution.
   void buildSymbolTable();

   /**
    * @brief Populates the java.lang.* cache with all the classes and interfaces
    * into the java_lang_ struct.
    */
   void populateJavaLangCache();

   /// @brief Resolves an unresolved type to a non-leaf node in the tree
   /// @param t The unresolved type to resolve.
   /// @return The package node that the unresolved type resolves to.
   ChildOpt resolveImport(ast::UnresolvedType const* t) const;

private:
   BumpAllocator& alloc;
   diagnostics::DiagnosticEngine& diag;
   ast::Semantic* sema_;
   ast::LinkingUnit* lu_;
   /// @brief The current compilation unit being resolved
   ast::CompilationUnit* currentCU_;
   /// @brief The import map for all the compilation units
   std::pmr::unordered_map<ast::CompilationUnit const*,
                           std::pmr::unordered_map<std::pmr::string, Pkg::Child>>
         importsMap_;
   /// @brief The root of the symbol table (more of a tree than table).
   Pkg* rootPkg_;
   /// @brief A cache of the java.lang.* pkg
   struct {
      ast::ClassDecl* Boolean;
      ast::ClassDecl* Byte;
      ast::ClassDecl* Character;
      ast::ClassDecl* Class;
      ast::InterfaceDecl* Cloneable;
      ast::ClassDecl* Integer;
      ast::ClassDecl* Number;
      ast::ClassDecl* Object;
      ast::ClassDecl* Short;
      ast::ClassDecl* String;
      ast::ClassDecl* System;
   } java_lang_;
   // Array class prototype
   ast::ClassDecl* arrayPrototype_;
   // The type for the array prototype
   ast::ReferenceType* arrayClassType_;
};

} // namespace semantic
