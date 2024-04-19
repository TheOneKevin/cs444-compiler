#include "codegen/CodeGen.h"

#include <utility>

#include "ast/AstNode.h"
#include "ast/Type.h"
#include "tir/Type.h"

namespace codegen {

using CG = CodeGenerator;

CG::CodeGenerator(tir::Context& ctx, tir::CompilationUnit& cu,
                  semantic::NameResolver& nr, semantic::HierarchyChecker& hc)
      : ctx{ctx}, cu{cu}, nr{nr}, hc{hc} {
   arrayType_ = tir::StructType::get(ctx,
                                     {// Length
                                      tir::Type::getInt32Ty(ctx),
                                      // Pointer
                                      tir::Type::getPointerTy(ctx)});
}

tir::Type* CG::emitType(ast::Type const* type) {
   using Kind = ast::BuiltInType::Kind;
   if(isAstTypeReference(type)) {
      return tir::Type::getPointerTy(ctx);
   } else if(type->isPrimitive()) {
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
         default:
            std::unreachable();
      }
   }
   std::unreachable();
}

void CG::run(ast::LinkingUnit const* lu) {
   // 1. Populate the RTTI mappings
   populateRtti(lu);
   // 2. Generate the class structs
   for(auto* cu : lu->compliationUnits()) {
      for(auto* decl : cu->decls()) {
         if(auto* classDecl = dyn_cast<ast::ClassDecl>(decl)) {
            emitClassDecl(classDecl);
         }
      }
   }
   // 3. Generate the class member functions
   for(auto* cu : lu->compliationUnits()) {
      for(auto* decl : cu->decls()) {
         if(auto* classDecl = dyn_cast<ast::ClassDecl>(decl)) {
            emitClass(classDecl);
         }
      }
   }
}

tir::Value* CG::emitGetArraySz(tir::Value* ptr) {
   using namespace tir;
   auto zero = Constant::CreateInt32(ctx, 0);
   auto gepSz = builder.createGEPInstr(ptr, arrayType_, {zero});
   gepSz->setName("arr.gep.sz");
   auto sz = builder.createLoadInstr(Type::getInt32Ty(ctx), gepSz);
   sz->setName("arr.sz");
   return sz;
}

tir::Value* CG::emitGetArrayPtr(tir::Value* ptr) {
   using namespace tir;
   auto one = Constant::CreateInt32(ctx, 1);
   auto gepPtr = builder.createGEPInstr(ptr, arrayType_, {one});
   gepPtr->setName("arr.gep.ptr");
   auto arrPtr = builder.createLoadInstr(Type::getPointerTy(ctx), gepPtr);
   arrPtr->setName("arr.ptr");
   return arrPtr;
}

void CG::emitSetArraySz(tir::Value* ptr, tir::Value* sz) {
   using namespace tir;
   auto zero = Constant::CreateInt32(ctx, 0);
   auto gepSz = builder.createGEPInstr(ptr, arrayType_, {zero});
   gepSz->setName("arr.gep.sz");
   builder.createStoreInstr(sz, gepSz);
}

void CG::emitSetArrayPtr(tir::Value* ptr, tir::Value* arr) {
   using namespace tir;
   auto one = Constant::CreateInt32(ctx, 1);
   auto gepPtr = builder.createGEPInstr(ptr, arrayType_, {one});
   gepPtr->setName("arr.gep.ptr");
   builder.createStoreInstr(arr, gepPtr);
}

void CG::populateRtti(ast::LinkingUnit const* lu) {
   int highestRtti = 0;
   // For each class and interface, add an entry to the RTTI map
   for(auto* cu : lu->compliationUnits()) {
      for(auto* decl : cu->decls()) {
         if(auto* classDecl = dyn_cast<ast::ClassDecl>(decl)) {
            rttiMap[classDecl] = highestRtti++;
         } else if(auto* iface = dyn_cast<ast::InterfaceDecl>(decl)) {
            rttiMap[iface] = highestRtti++;
         }
      }
   }
   // Create the RTTI table
   rttiTable.resize(highestRtti, std::vector<bool>(highestRtti, false));
   // Now populate the RTTI table (Ti, Tj)
   for (auto& [key, val] : rttiMap) {
      for (auto& [key2, val2] : rttiMap) {
         rttiTable[val][val2] = hc.isSubType(key, key2);
      }
   }
}

} // namespace codegen