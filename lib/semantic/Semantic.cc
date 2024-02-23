#include "semantic/Semantic.h"

#include <set>
#include <string>

#include "ast/AstNode.h"
#include "diagnostics/Location.h"

namespace ast {

using std::string;

/* ===--------------------------------------------------------------------=== */
// ast/Type.h
/* ===--------------------------------------------------------------------=== */

UnresolvedType* Semantic::BuildUnresolvedType(SourceRange loc) {
   return alloc.new_object<UnresolvedType>(alloc, loc);
}

ArrayType* Semantic::BuildArrayType(Type* elementType, SourceRange loc) {
   return alloc.new_object<ArrayType>(alloc, elementType, loc);
}

BuiltInType* Semantic::BuildBuiltInType(parsetree::BasicType::Type type,
                                        SourceRange loc) {
   return alloc.new_object<BuiltInType>(type, loc);
}

/* ===--------------------------------------------------------------------=== */
// ast/Decl.h
/* ===--------------------------------------------------------------------=== */

VarDecl* Semantic::BuildVarDecl(Type* type, SourceRange loc, string_view name,
                                Expr* init) {
   auto decl = alloc.new_object<VarDecl>(alloc, loc, type, name, init);
   if(!AddLexicalLocal(decl)) {
      diag.ReportError(loc) << "local variable \"" << name
                            << "\" already declared in this scope.";
   }
   return decl;
}

FieldDecl* Semantic::BuildFieldDecl(Modifiers modifiers, SourceRange loc,
                                    Type* type, string_view name, Expr* init) {
   if(modifiers.isFinal()) {
      diag.ReportError(modifiers.getLocation(Modifiers::Type::Final))
            << "field declaration cannot be final.";
   }
   if(modifiers.isAbstract()) {
      diag.ReportError(modifiers.getLocation(Modifiers::Type::Abstract))
            << "field declaration cannot be abstract.";
   }
   if(modifiers.isNative()) {
      diag.ReportError(modifiers.getLocation(Modifiers::Type::Native))
            << "field declaration cannot be native.";
   }
   if(modifiers.isPublic() && modifiers.isProtected()) {
      diag.ReportError(modifiers.getLocation(Modifiers::Type::Public))
            << "field cannot be both public and protected.";
   }
   if(!modifiers.isPublic() && !modifiers.isProtected()) {
      diag.ReportError(loc) << "field must have a visibility modifier.";
   }
   return alloc.new_object<FieldDecl>(alloc, loc, modifiers, type, name, init);
}

/* ===--------------------------------------------------------------------=== */
// ast/DeclContext.h
/* ===--------------------------------------------------------------------=== */

LinkingUnit* Semantic::BuildLinkingUnit(
      array_ref<CompilationUnit*> compilationUnits) {
   std::pmr::set<std::string_view> names;

   for(auto cu : compilationUnits) {
      if(!cu->body()) continue;
      std::string_view name = cu->bodyAsDecl()->getCanonicalName();

      if(names.count(name) > 0) {
         diag.ReportError(cu->location())
               << "No two classes or interfaces can have the same canonical name.";
      }

      names.insert(name);
   }

   // Build the java.lang package
   auto javaLangPackage = BuildUnresolvedType(SourceRange{});
   javaLangPackage->addIdentifier("java");
   javaLangPackage->addIdentifier("lang");
   std::pmr::vector<ImportDeclaration> imports;
   compilationUnits.push_back(
         BuildCompilationUnit(javaLangPackage, imports, SourceRange(), nullptr));

   return alloc.new_object<LinkingUnit>(alloc, compilationUnits);
}

CompilationUnit* Semantic::BuildCompilationUnit(
      ReferenceType* package, array_ref<ImportDeclaration> imports,
      SourceRange loc, DeclContext* body) {
   std::pmr::set<std::pmr::string> names;
   std::pmr::set<std::pmr::string> fullImportNames;
   for(auto import : imports) {
      if(import.isOnDemand) continue;
      std::pmr::string name(import.simpleName());
      std::pmr::string fullName(import.type->toString());
      // Check that no two single-type-import declarations clash with each other
      // We allow duplicate if they refer to the same type
      if(names.count(name) > 0 && fullImportNames.count(fullName) == 0) {
         diag.ReportError(loc)
               << "No two single-type-import declarations clash with each other.";
      }
      names.insert(name);
      fullImportNames.insert(fullName);
   }

   // cf. JLS 7.5.2, we must import java.lang.*
   auto javaLang = BuildUnresolvedType(SourceRange{});
   javaLang->addIdentifier("java");
   javaLang->addIdentifier("lang");
   ImportDeclaration javaLangImport{javaLang, true};
   imports.push_back(javaLangImport);

   // Create the AST node
   return alloc.new_object<CompilationUnit>(alloc, package, imports, loc, body);
}

ClassDecl* Semantic::BuildClassDecl(Modifiers modifiers, SourceRange loc,
                                    string_view name, ReferenceType* superClass,
                                    array_ref<ReferenceType*> interfaces,
                                    array_ref<Decl*> classBodyDecls) {
   // Check that the modifiers are valid for a class
   if(modifiers.isAbstract() && modifiers.isFinal())
      diag.ReportError(loc) << "class cannot be both abstract and final";
   if(!modifiers.isPublic())
      diag.ReportError(loc) << "class must have a visibility modifier";
   // Create the AST node
   auto node = alloc.new_object<ClassDecl>(
         alloc, modifiers, loc, name, superClass, interfaces, classBodyDecls);
   // Check if the class has at least one constructor
   if(node->constructors().size() == 0)
      diag.ReportError(loc) << "class must have at least one constructor";
   // Check if multiple fields have the same name
   for(auto field : node->fields()) {
      for(auto other : node->fields()) {
         if(field != other && field->name() == other->name()) {
            diag.ReportError(field->location())
                  << "field \"" << field->name() << "\" is already declared.";
         }
      }
   }
   // Check that interfaces cannot be repeated in implements clause
   for(auto interface : node->interfaces()) {
      for(auto other : node->interfaces()) {
         if(interface != other && interface->toString() == other->toString()) {
            diag.ReportError(loc) << "interface \"" << interface->toString()
                                  << "\" is already implemented.";
         }
      }
   }
   return node;
}

InterfaceDecl* Semantic::BuildInterfaceDecl(Modifiers modifiers, SourceRange loc,
                                            string_view name,
                                            array_ref<ReferenceType*> extends,
                                            array_ref<Decl*> interfaceBodyDecls) {
   // Check that the modifiers are valid for an interface
   if(modifiers.isFinal())
      diag.ReportError(modifiers.getLocation(Modifiers::Type::Final))
            << "interface cannot be final";
   if(!modifiers.isPublic())
      diag.ReportError(loc) << "interface must have a visibility modifier";
   // Verify the methods are abstract
   for(auto decl : interfaceBodyDecls) {
      if(auto methodDecl = dynamic_cast<MethodDecl*>(decl)) {
         if(!methodDecl->modifiers().isAbstract()) {
            diag.ReportError(methodDecl->location())
                  << "interface method must be abstract";
         }
      }
   }
   // Check that interfaces cannot be repeated in extends clause
   for(auto interface : extends) {
      for(auto other : extends) {
         if(interface != other && interface->toString() == other->toString()) {
            diag.ReportError(loc) << "interface \"" << interface->toString()
                                  << "\" is already implemented.";
         }
      }
   }
   // Create the AST node
   return alloc.new_object<InterfaceDecl>(
         alloc, modifiers, loc, name, extends, interfaceBodyDecls);
}

MethodDecl* Semantic::BuildMethodDecl(Modifiers modifiers, SourceRange loc,
                                      string_view name, Type* returnType,
                                      array_ref<VarDecl*> parameters,
                                      bool isConstructor, Stmt* body) {
   // Check modifiers
   if((body == nullptr) != (modifiers.isAbstract() || modifiers.isNative())) {
      diag.ReportError(loc) << "method has a body if and only if it is "
                               "neither abstract nor native.";
   }
   if(modifiers.isAbstract() &&
      (modifiers.isFinal() || modifiers.isStatic() || modifiers.isNative())) {
      diag.ReportError(loc) << "an abstract method cannot be static, final, "
                               "or native.";
   }
   if(modifiers.isStatic() && modifiers.isFinal()) {
      diag.ReportError(modifiers.getLocation(Modifiers::Type::Final))
            << "static method cannot be final.";
   }
   if(modifiers.isNative()) {
      if(!modifiers.isStatic()) {
         diag.ReportError(loc) << "native method must be static.";
      }
      if(auto ty = dynamic_cast<BuiltInType*>(returnType)) {
         if(ty->getKind() != BuiltInType::Kind::Int) {
            diag.ReportError(loc) << "native method must have return type int.";
         }
      }
      if(parameters.size() != 1) {
         diag.ReportError(loc) << "native method must have exactly one "
                                  "parameter of type int.";
      } else if(auto ty = dynamic_cast<BuiltInType*>(parameters[0]->type())) {
         if(ty->getKind() != BuiltInType::Kind::Int) {
            diag.ReportError(loc) << "native method must have exactly one "
                                     "parameter of type int.";
         }
      }
   }
   if(modifiers.isPublic() && modifiers.isProtected()) {
      diag.ReportError(modifiers.getLocation(Modifiers::Type::Public))
            << "method cannot be both public and protected.";
   }
   if(!modifiers.isPublic() && !modifiers.isProtected()) {
      diag.ReportError(loc) << "method must have a visibility modifier.";
   }
   // Create the AST node
   return alloc.new_object<MethodDecl>(
         alloc, modifiers, loc, name, returnType, parameters, isConstructor, body);
}

/* ===-----------------------------------------------------------------=== */
// ast/Stmt.h
/* ===-----------------------------------------------------------------=== */

BlockStatement* Semantic::BuildBlockStatement(array_ref<Stmt*> stmts) {
   return alloc.new_object<BlockStatement>(alloc, stmts);
}

DeclStmt* Semantic::BuildDeclStmt(VarDecl* decl) {
   return alloc.new_object<DeclStmt>(decl);
}

ExprStmt* Semantic::BuildExprStmt(Expr* expr) {
   return alloc.new_object<ExprStmt>(expr);
}

IfStmt* Semantic::BuildIfStmt(Expr* condition, Stmt* thenStmt, Stmt* elseStmt) {
   return alloc.new_object<IfStmt>(condition, thenStmt, elseStmt);
}

WhileStmt* Semantic::BuildWhileStmt(Expr* condition, Stmt* body) {
   return alloc.new_object<WhileStmt>(condition, body);
}

ForStmt* Semantic::BuildForStmt(Stmt* init, Expr* condition, Stmt* update,
                                Stmt* body) {
   return alloc.new_object<ForStmt>(init, condition, update, body);
}

ReturnStmt* Semantic::BuildReturnStmt(Expr* expr) {
   return alloc.new_object<ReturnStmt>(expr);
}

} // namespace ast
