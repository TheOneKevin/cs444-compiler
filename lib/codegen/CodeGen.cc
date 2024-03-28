#include "codegen/CodeGen.h"
#include "ast/AstNode.h"
#include "ast/Type.h"
#include "tir/Type.h"

namespace codegen {

tir::Type* CodeGenerator::emitType(ast::Type const* type) {
   using Kind = ast::BuiltInType::Kind;
   if(type->isPrimitive()) {
      auto ty = cast<ast::BuiltInType>(type);
      switch(ty->getKind()) {
         case Kind::Boolean:
            return tir::Type::getInt1Ty(ctx);
         case Kind::Char:
            return tir::Type::getInt16Ty(ctx);
         case Kind::Short:
            return tir::Type::getInt16Ty(ctx);
         case Kind::Int:
            return tir::Type::getInt32Ty(ctx);
         case Kind::Byte:
            return tir::Type::getInt8Ty(ctx);
         case Kind::String:
            // TODO: Implement this
            return tir::Type::getPointerTy(ctx);
         default:
            // NONE type is just a null pointer
            return tir::Type::getPointerTy(ctx);
      }
   } else if(type->isArray()) {
      return tir::StructType::get(
         ctx, {
            // Length
            tir::Type::getInt32Ty(ctx),
            // Pointer
            tir::Type::getPointerTy(ctx)
         }
      );
   } else {
      // TODO: Implement this
      return tir::Type::getPointerTy(ctx);
   }
}

void CodeGenerator::run(ast::LinkingUnit const* lu) {
   for(auto* cu : lu->compliationUnits()) {
      for(auto* decl : cu->decls()) {
         if(auto* classDecl = dyn_cast<ast::ClassDecl>(decl)) {
            emitClassDecl(classDecl);
         }
      }
   }
   for(auto* cu : lu->compliationUnits()) {
      for(auto* decl : cu->decls()) {
         if(auto* classDecl = dyn_cast<ast::ClassDecl>(decl)) {
            emitClass(classDecl);
         }
      }
   }
}

void CodeGenerator::emitClassDecl(ast::ClassDecl const* decl) {
   for(auto* method : decl->methods()) {
      if(method->modifiers().isStatic()) {
         emitFunctionDecl(method);
      }
   }
   for(auto* field : decl->fields()) {
      if(field->modifiers().isStatic()) {
         auto value = cu.CreateGlobalVariable(
            emitType(field->type()),
            field->name()
         );
         gvMap[field] = value;
      }
   }
}

void CodeGenerator::emitClass(ast::ClassDecl const* decl) {
   for(auto* method : decl->methods()) {
      if(method->modifiers().isStatic()) {
         emitFunction(method);
      }
   }
}

}