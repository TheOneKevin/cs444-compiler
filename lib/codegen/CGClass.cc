#include "codegen/CodeGen.h"

#include "ast/AstNode.h"
#include "ast/Type.h"
#include "tir/Type.h"

namespace codegen {

void CodeGenerator::emitClassDecl(ast::ClassDecl const* decl) {
   // 1. Emit the function declarations
   for(auto* method : decl->methods())
      emitFunctionDecl(method);
   // 2. Emit any static fields as globals
   // 3. Construct the class struct type as well
   std::vector<tir::Type*> fieldTypes{};
   for(auto* field : decl->fields()) {
      auto ty = emitType(field->type());
      if(field->modifiers().isStatic()) {
         gvMap[field] = cu.CreateGlobalVariable(ty, field->getCanonicalName());
      } else {
         fieldTypes.push_back(ty);
      }
   }
   if(!fieldTypes.empty())
      typeMap[decl] = tir::StructType::get(ctx, fieldTypes);
}

void CodeGenerator::emitClass(ast::ClassDecl const* decl) {
   for(auto* method : decl->methods()) {
      if(method->modifiers().isStatic()) {
         emitFunction(method);
      }
   }
}

} // namespace codegen
