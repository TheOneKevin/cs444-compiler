#include "semantic/NameResolver.h"

#include <cassert>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "ast/AST.h"
#include "ast/AstNode.h"
#include "ast/DeclContext.h"
#include "ast/Type.h"
#include "diagnostics/Location.h"

using std::string_view;
using std::pmr::string;
using std::pmr::unordered_map;
using std::pmr::vector;

namespace semantic {

void NameResolver::buildSymbolTable() {
   rootPkg_ = alloc.new_object<Pkg>(alloc);
   // Grab the package type from the compilation unit.
   for(auto cu : lu_->compliationUnits()) {
      // Grab the CU's package and mark it as immutable
      auto pkg = dynamic_cast<ast::UnresolvedType*>(cu->package());
      assert(pkg && "Package should be an unresolved type");
      pkg->lock();
      // Traverse the package name to find the leaf package.
      Pkg* subPkg = rootPkg_;
      // FIXME: Default package == package with empty parts() != nullptr
      for(auto const& id : pkg->parts()) {
         // If the subpackage name is not in the symbol table, add it
         // and continue to the next one.
         if(subPkg->children.find(id) == subPkg->children.end()) {
            auto newpkg = alloc.new_object<Pkg>(alloc, string_view{id});
            subPkg->children[id] = newpkg;
            subPkg = newpkg;
            continue;
         }
         // If the subpackage does not hold a package, then it must be a
         // a decl with the same name as the package. This is an error.
         // cf. JLS 6.4.1.
         Pkg::Child const& child = rootPkg_->children[id];
         if(std::holds_alternative<ast::Decl*>(child)) {
            auto decl = std::get<ast::Decl*>(child);
            assert(decl && "Package node holds empty decl");
            diag.ReportError(SourceRange{})
                  << "The subpackage name cannot be the same as a declaration.";
            continue;
         }
         // Otherwise, we can traverse into the next subpackage.
         subPkg = std::get<Pkg*>(child);
      }
      // Check that the declaration is unique, cf. JLS 6.4.1.
      if(subPkg->children.find(cu->bodyAsDecl()->name().data()) !=
         subPkg->children.end()) {
         diag.ReportError(SourceRange{})
               << "The declaration name is not unique in the subpackage.";
      }
      // Now add the CU's declaration to the subpackage.
      subPkg->children[cu->bodyAsDecl()->name().data()] = cu->bodyAsDecl();
   }
}

void NameResolver::BeginContext(ast::CompilationUnit* cu) {
   using ast::UnresolvedType, ast::Decl;

   // Set the current compilation unit and clear the imports map
   cu_ = cu;
   importsMap_.clear();
   auto curPkg = dynamic_cast<UnresolvedType*>(cu->package());
   assert(curPkg && "Package should be an unresolved type");

   // We can populate the imports map by order of shadowing cf. JLS 6.3.1.
   //   1. Package declarations, does not shadow anything.
   //   2. Import-on-demand declarations, never causes any declaration to be
   //      shadowed, but maybe shadow other packages.
   //   3. All declarations in the same package (different CUs).
   //   4. Single-type-import declarations, shadows any other decl in all CUs
   //      under the same package. It also shadows any import-on-demand decls.
   //   4. All declarations in the current CU.
   // We should also note the scope of types under the same package declarations
   // cf. JLS 6.3 is visible across all CUs in the same package.

   // 1. Package declarations
   for(auto& kv : rootPkg_->children)
      importsMap_[kv.first] = Pkg::Child{std::get<Pkg*>(kv.second)};
   // 2. Import-on-demand declarations
   for(auto const& imp : cu->imports()) {
      if(!imp.isOnDemand) continue;
      // First, resolve the subpackage subtree from the symbol table.
      auto subPkg = resolveAstTy(static_cast<UnresolvedType const*>(imp.type));
      if(!subPkg) continue;
      // Second, add all the Decl from the subpackage to the imports map.
      for(auto& kv : subPkg->children)
         if(std::holds_alternative<Decl*>(kv.second))
            importsMap_[kv.first] = Pkg::Child{std::get<Decl*>(kv.second)};
   }
   // 3. All declarations in the same package (different CUs). We can shadow
   //    any declarations already existing.
   {
      auto curTree = resolveAstTy(curPkg);
      assert(curTree && "Current package should exist!");
      for(auto& kv : curTree->children)
         if(std::holds_alternative<Decl*>(kv.second))
            importsMap_[kv.first] = Pkg::Child{std::get<Decl*>(kv.second)};
   }
   // 4. Single-type-import declarations. This may also shadow anything existing.
   for(auto const& imp : cu->imports()) {
      if(imp.isOnDemand) continue;
      // First, resolve the subpackage subtree from the symbol table.
      auto subPkg = resolveAstTy(static_cast<UnresolvedType const*>(imp.type));
      if(!subPkg) continue;
      // Second, add the Decl from the subpackage to the imports map.
      auto decl = subPkg->children[imp.simpleName()];
      if(std::holds_alternative<Decl*>(decl)) importsMap_[imp.simpleName()] = decl;
   }
   // 5. All declarations in the current CU. This may also shadow anything.
   importsMap_[cu_->bodyAsDecl()->name().data()] = cu_->bodyAsDecl();
}

NameResolver::Pkg* NameResolver::resolveAstTy(ast::UnresolvedType const* t) const {
   Pkg* subPkg = rootPkg_;
   for(auto const& id : t->parts()) {
      // If the subpackage does not exist, then the import is invalid.
      if(subPkg->children.find(id) == subPkg->children.end()) {
         diag.ReportError(SourceRange{})
               << "failed to resolve import as subpackage does not exist: \"" << id
               << "\"";
         return nullptr;
      }
      Pkg::Child const& child = subPkg->children[id];
      // If the subpackage is a declaration, then the import is invalid.
      if(std::holds_alternative<ast::Decl*>(child)) {
         diag.ReportError(SourceRange{})
               << "failed to resolve import as subpackage is a declaration: \""
               << id << "\"";
         return nullptr;
      }
      // Otherwise, we can traverse into the next subpackage.
      subPkg = std::get<Pkg*>(child);
   }
   return subPkg;
}

ast::ReferenceType* NameResolver::ResolveType(ast::UnresolvedType* type) {
   (void)type;
   return nullptr;
}

} // namespace semantic
