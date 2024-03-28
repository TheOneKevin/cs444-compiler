#pragma once

#include <sstream>
#include <string>

#include "ast/Type.h"
#include "semantic/NameResolver.h"

namespace codegen {

class Mangler {
public:
   Mangler(semantic::NameResolver const& nr) : NR{nr} {}
   void MangleFunctionName(ast::MethodDecl const* decl);
   void MangleType(ast::Type const* ty);
   std::string getMangledName() { return ss.str(); }

private:
   void MangleCanonicalName(std::string_view name);

private:
   semantic::NameResolver const& NR;
   std::ostringstream ss;
};

} // namespace codegen
