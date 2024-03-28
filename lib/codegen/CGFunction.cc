#include <string_view>

#include "codegen/CodeGen.h"
#include "tir/IRBuilder.h"
#include "tir/Instructions.h"
#include "codegen/Mangling.h"
#include "tir/Type.h"

namespace codegen {

void CodeGenerator::emitFunctionDecl(ast::MethodDecl const* decl) {
   if(!decl->modifiers().isStatic()) {
      assert(false && "Only static methods are supported");
   }
   // 1. Create the function signature type
   auto astRetTy = decl->returnTy().type;
   tir::Type* tirRetTy = tir::Type::getVoidTy(ctx);
   if(astRetTy)
      tirRetTy = emitType(astRetTy);
   std::vector<tir::Type*> paramTys;
   std::vector<std::string_view> paramNames;
   for(auto* param : decl->parameters()) {
      paramTys.push_back(emitType(param->type()));
      paramNames.push_back(param->name());
   }
   auto* funcTy = tir::FunctionType::get(ctx, tirRetTy, paramTys);
   // 2. Create the function and set the argument names (for debugging)
   Mangler m{nr};
   m.MangleFunctionName(decl);
   auto* func = cu.CreateFunction(funcTy, m.getMangledName());
   gvMap[decl] = func;
   assert(func && "Function already exists");
   for(auto arg : func->args()) {
      arg->setName(paramNames[arg->index()]);
   }
}

void CodeGenerator::emitFunction(ast::MethodDecl const* decl) {
   // 1. Grab the function from the gvMap and clear the valueMap
   auto func = cast<tir::Function>(gvMap[decl]);
   curFn = func;
   valueMap.clear();
   // 2. Emit the function body and add the allocas for the locals
   auto entry = builder.createBasicBlock(func);
   builder.setInsertPoint(entry->begin());
   for(auto* local : decl->decls()) {
      auto* typedLocal = cast<ast::TypedDecl>(local);
      auto* localTy = emitType(typedLocal->type());
      auto* val = func->createAlloca(localTy);
      val->setName(typedLocal->name());
      valueMap[local] = cast<tir::AllocaInst>(val);
   }
   emitStmt(decl->body());
   // 3. End the function by clearing the curFn
   curFn = nullptr;
}

} // namespace codegen
