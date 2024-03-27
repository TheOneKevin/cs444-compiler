#include "codegen/CodeGen.h"
#include "ast/AstNode.h"
#include "ast/Type.h"

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
            assert(false && "String type not supported yet");
         default:
            assert(false && "None type not supported");
      }
   } else if(type->isArray()) {
      auto ty = cast<ast::ArrayType>(type);

   } else {
      auto ty = cast<ast::ReferenceType>(type);

   }
}

}