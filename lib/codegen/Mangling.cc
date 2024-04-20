#include "codegen/Mangling.h"

#include <string_view>

#include "ast/AST.h"

namespace codegen {

void Mangler::MangleCanonicalName(std::string_view name) {
   for(unsigned i = 0; i < name.size(); i++) {
      unsigned partSize = 0;
      for(unsigned j = i; j < name.size() && name[j] != '.'; j++, partSize++)
         ;
      ss << partSize << name.substr(i, partSize);
      i += partSize;
   }
   ss << "E";
}

void Mangler::MangleType(ast::Type const* ty) {
   if(ty->isPrimitive()) {
      auto* bt = cast<ast::BuiltInType>(ty);
      switch(bt->getKind()) {
         case ast::BuiltInType::Kind::Boolean:
            ss << "B";
            break;
         case ast::BuiltInType::Kind::Char:
            ss << "c";
            break;
         case ast::BuiltInType::Kind::Short:
            ss << "s";
            break;
         case ast::BuiltInType::Kind::Int:
            ss << "i";
            break;
         case ast::BuiltInType::Kind::Byte:
            ss << "b";
            break;
         case ast::BuiltInType::Kind::String:
            ss << "S";
            break;
         default:
            assert(false && "None type not supported");
      }
   } else if(ty->isArray()) {
      ss << "A";
      MangleType(cast<ast::ArrayType>(ty)->getElementType());
   } else {
      auto* rt = cast<ast::ReferenceType>(ty)->decl();
      // If java.lang.String
      if(rt == NR.GetJavaLang().String) {
         ss << "S";
      } else if(rt == NR.GetJavaLang().Object) {
         ss << "O";
      } else {
         ss << "R";
         MangleCanonicalName(rt->getCanonicalName());
      }
   }
}

void Mangler::MangleFunctionName(ast::MethodDecl const* decl) {
   ss << "_JF";
   if(!decl->modifiers().isStatic()) ss << "C";
   // Split canonical name into parts
   MangleCanonicalName(decl->getCanonicalName());
   // Add the function signature
   if(decl->returnTy().type)
      MangleType(decl->returnTy().type);
   else
      ss << "v";
   for(auto* param : decl->parameters()) {
      MangleType(param->type());
   }
}

void Mangler::MangleGlobalName(ast::FieldDecl const* decl) {
   ss << "_JG";
   MangleType(decl->type());
   MangleCanonicalName(decl->getCanonicalName());
}

void Mangler::MangleDecl(ast::Decl const* decl) {
   if(auto* fd = dyn_cast<ast::FieldDecl>(decl)) {
      MangleGlobalName(fd);
   } else if(auto* md = dyn_cast<ast::MethodDecl>(decl)) {
      MangleFunctionName(md);
   } else if (auto *cd = dyn_cast<ast::ClassDecl>(decl)) {
      ss << "_JC";
      MangleCanonicalName(cd->getCanonicalName());
   } else {
      assert(false && "Unknown decl type");
   }
}

void Mangler::MangleVTable(ast::Decl const* decl) {
   ss << "_JV";
   MangleCanonicalName(decl->getCanonicalName());
}

} // namespace codegen