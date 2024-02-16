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
         if(std::holds_alternative<Pkg*>(child)) {
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
      auto body = dynamic_cast<ast::Decl*>(cu->body());
      if(subPkg->children.find(body->name().data()) != subPkg->children.end()) {
         diag.ReportError(SourceRange{})
               << "The declaration name is not unique in the subpackage.";
      }
      // Now add the CU's declaration to the subpackage.
      subPkg->children[body->name().data()] = body;
   }
}

void NameResolver::BeginContext(ast::CompilationUnit* cu) {
   // Set the current compilation unit and clear the imports map
   cu_ = cu;
   importsMap_.clear();
   // Now we add all the visible type names to importsMap_ ...
}

ast::ReferenceType* NameResolver::ResolveType(ast::UnresolvedType* type) {
   (void) type;
   return nullptr;
}

} // namespace semantic
