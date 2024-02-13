#include "semantic/Semantic.h"

#include <string>

namespace ast {

using std::string;

/* ===--------------------------------------------------------------------=== */
// ast/Decl.h
/* ===--------------------------------------------------------------------=== */

VarDecl* Semantic::BuildVarDecl(Type* type, string_view name) {
   return alloc.new_object<VarDecl>(alloc, type, name);
}

FieldDecl* Semantic::BuildFieldDecl(Modifiers modifiers,
                                    Type* type,
                                    string_view name) {
   if(modifiers.isFinal()) {
      throw std::runtime_error("FieldDecl cannot be final");
   }
   if(modifiers.isAbstract()) {
      throw std::runtime_error("FieldDecl cannot be abstract");
   }
   if(modifiers.isNative()) {
      throw std::runtime_error("FieldDecl cannot be native");
   }
   if(modifiers.isPublic() && modifiers.isProtected()) {
      throw std::runtime_error("A method cannot be both public and protected.");
   }
   if(!modifiers.isPublic() && !modifiers.isProtected()) {
      throw std::runtime_error("Field must have a visibility modifier");
   }
   return alloc.new_object<FieldDecl>(alloc, modifiers, type, name);
}

/* ===--------------------------------------------------------------------=== */
// ast/DeclContext.h
/* ===--------------------------------------------------------------------=== */

CompilationUnit* Semantic::BuildCompilationUnit(
      QualifiedIdentifier* package,
      array_ref<ImportDeclaration> imports,
      DeclContext* body) {
   return alloc.new_object<CompilationUnit>(alloc, package, imports, body);
}

ClassDecl* Semantic::BuildClassDecl(Modifiers modifiers,
                                    string_view name,
                                    QualifiedIdentifier* superClass,
                                    array_ref<QualifiedIdentifier*> interfaces,
                                    array_ref<Decl*> classBodyDecls) {
   // Check that the modifiers are valid for a class
   if(modifiers.isAbstract() && modifiers.isFinal())
      throw std::runtime_error("Class cannot be both abstract and final");
   if(!modifiers.isPublic())
      throw std::runtime_error("Class must have a visibility modifier");
   // Create the AST node
   auto node = alloc.new_object<ClassDecl>(
         alloc, modifiers, name, superClass, interfaces, classBodyDecls);
   // Check if the class has at least one constructor
   if(node->constructors().size() == 0) {
      throw std::runtime_error("A class must have at least one constructor");
   }
   return node;
}

InterfaceDecl* Semantic::BuildInterfaceDecl(
      Modifiers modifiers,
      string_view name,
      array_ref<QualifiedIdentifier*> extends,
      array_ref<Decl*> interfaceBodyDecls) {
   // Check that the modifiers are valid for an interface
   if(modifiers.isFinal())
      throw std::runtime_error("Interface cannot be final");
   if(!modifiers.isPublic())
      throw std::runtime_error("Interface must have a visibility modifier");
   // Verify the methods are abstract
   for(auto decl : interfaceBodyDecls) {
      if(auto methodDecl = dynamic_cast<MethodDecl*>(decl)) {
         if(!methodDecl->modifiers().isAbstract()) {
            throw std::runtime_error("Interface methods must be abstract");
         }
      }
   }
   // Create the AST node
   return alloc.new_object<InterfaceDecl>(
         alloc, modifiers, name, extends, interfaceBodyDecls);
}

MethodDecl* Semantic::BuildMethodDecl(Modifiers modifiers,
                                      string_view name,
                                      Type* returnType,
                                      array_ref<VarDecl*> parameters,
                                      bool isConstructor,
                                      Stmt* body) {
   // Check modifiers
   if((body == nullptr) != (modifiers.isAbstract() || modifiers.isNative())) {
      throw std::runtime_error(
            "A method has a body if and only if it is neither abstract nor"
            "native.");
   }
   if(modifiers.isAbstract() &&
      (modifiers.isFinal() || modifiers.isStatic() || modifiers.isNative())) {
      throw std::runtime_error(
            "An abstract method cannot be static, final or native.");
   }
   if(modifiers.isStatic() && modifiers.isFinal()) {
      throw std::runtime_error("A static method cannot be final.");
   }
   if(modifiers.isNative()) {
      if(!modifiers.isStatic()) {
         throw std::runtime_error("A native method must be static.");
      }
      if(auto ty = dynamic_cast<BuiltInType*>(returnType)) {
         if(ty->getKind() != BuiltInType::Kind::Int) {
            throw std::runtime_error("A native method must return type int.");
         }
      }
      if(parameters.size() != 1) {
         throw std::runtime_error(
               "A native method must have exactly one parameter.");
      }
      if(auto ty = dynamic_cast<BuiltInType*>(parameters[0]->type())) {
         if(ty->getKind() != BuiltInType::Kind::Int) {
            throw std::runtime_error(
                  "A native method must have paramter type int.");
         }
      }
   }
   if(modifiers.isPublic() && modifiers.isProtected()) {
      throw std::runtime_error("A method cannot be both public and protected.");
   }
   if(!modifiers.isPublic() && !modifiers.isProtected()) {
      throw std::runtime_error("Method must have a visibility modifier");
   }
   // Create the AST node
   return alloc.new_object<MethodDecl>(
         alloc, modifiers, name, returnType, parameters, isConstructor, body);
}

/* ===-----------------------------------------------------------------=== */
// ast/DeclContext.h
/* ===-----------------------------------------------------------------=== */

Expr* Semantic::BuildExpr(std::list<ExprNode> rpn_ops) {
   return alloc.new_object<Expr>(alloc, rpn_ops);
}
} // namespace ast
