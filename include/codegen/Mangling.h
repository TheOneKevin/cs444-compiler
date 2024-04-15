#pragma once

#include <sstream>
#include <string>

#include "ast/AstNode.h"
#include "ast/Decl.h"
#include "ast/Type.h"
#include "semantic/NameResolver.h"

namespace codegen {

class Mangler {
public:
   Mangler(semantic::NameResolver const& nr) : NR{nr} {}
   void MangleDecl(ast::Decl const* decl);
   std::string getMangledName() { return ss.str(); }

private:
   void MangleFunctionName(ast::MethodDecl const* decl);
   void MangleGlobalName(ast::FieldDecl const* decl);
   void MangleType(ast::Type const* ty);
   void MangleCanonicalName(std::string_view name);

private:
   semantic::NameResolver const& NR;
   std::ostringstream ss;
};

} // namespace codegen
