#include "HierarchyChecker.h"

namespace semantic {

bool dfs(ast::Decl* decl, std::pmr::map<ast::Decl*, bool>& visited,
         std::pmr::map<ast::Decl*, std::pmr::vector<ast::Decl*>>& hierarchyMap) {
   if(visited[decl]) return true;

   visited[decl] = true;

   for(auto child : hierarchyMap[decl]) {
      if(!visited[child]) {
         if(dfs(child, visited, hierarchyMap)) {
            return true;
         }
      }
   }

   return false;
}

bool isAcyclic(
      std::pmr::map<ast::Decl*, std::pmr::vector<ast::Decl*>>& hierarchyMap) {
   std::pmr::map<ast::Decl*, bool> visited;

   for(auto [decl, children] : hierarchyMap) {
      if(!visited[decl]) {
         if(dfs(decl, visited, hierarchyMap)) {
            return true;
         }
      }
   }

   return false;
}

bool isSameMethodSignature(ast::MethodDecl* method1, ast::MethodDecl* method2) {
   if(method1->name() != method2->name()) return false;
   if(method1->parameters().size() != method2->parameters().size()) return false;

   for(size_t i = 0; i < method1->parameters().size(); i++) {
      if(method1->parameters()[i]->type() != method2->parameters()[i]->type()) {
         return false;
      }
   }

   return true;
}

void HierarchyChecker::checkInheritance() {
   for(auto cu : lu_->compliationUnits()) {
      auto body = cu->body();
      // if the body is null, continue to the next iteration
      if(!body) continue;

      if(auto classDecl = dynamic_cast<ast::ClassDecl*>(body)) {
         // check if super class is valid
         if(auto superClass = classDecl->superClass()) {
            hierarchyMap[classDecl].emplace_back(classDecl->superClass()->decl());
            if(dynamic_cast<ast::InterfaceDecl*>(superClass->decl())) {
               diag.ReportError(classDecl->location())
                     << "A class must not extend an interface."
                     << classDecl->name();
            } else if(auto superClass = dynamic_cast<ast::ClassDecl*>(
                            classDecl->superClass()->decl())) {
               if(superClass->modifiers().isFinal()) {
                  diag.ReportError(classDecl->location())
                        << "A class must not extend a final class"
                        << classDecl->name();
               }
            }
         }

         // check if interfaces are valid
         for(auto interface : classDecl->interfaces()) {
            for(auto other : classDecl->interfaces()) {
               if(interface == other) continue;
               if(interface->decl() == other->decl()) {
                  diag.ReportError(classDecl->location())
                        << "A class must not implement the same interface twice. "
                        << classDecl->name();
               }
            }

            if(dynamic_cast<ast::ClassDecl*>(interface->decl())) {
               diag.ReportError(classDecl->location())
                     << "A class must not implement a class. "
                     << classDecl->name();
            }

            hierarchyMap[classDecl].emplace_back(interface->decl());
         }

         checkClassConstructors(classDecl);
         checkClassMethod(classDecl);
      } else if(auto interfaceDecl = dynamic_cast<ast::InterfaceDecl*>(body)) {
         for(auto extends : interfaceDecl->extends()) {
            for(auto other : interfaceDecl->extends()) {
               if(extends == other) continue;
               if(extends->decl() == other->decl()) {
                  diag.ReportError(interfaceDecl->location())
                        << "A interface must not extend the same interface twice. "
                        << interfaceDecl->name();
               }
            }
            if(dynamic_cast<ast::ClassDecl*>(extends->decl())) {
               diag.ReportError(interfaceDecl->location())
                     << "A interface must not extend a class"
                     << interfaceDecl->name();
            }

            hierarchyMap[interfaceDecl].emplace_back(extends->decl());
         }
      }
   }

   if(isAcyclic(hierarchyMap)) {
      diag.ReportError(SourceRange{}) << "The inheritance hierarchy is cyclic.";
   }
}

void HierarchyChecker::checkClassMethod(ast::ClassDecl* classDecl) {
   for(auto methods : classDecl->methods()) {
      if(auto methodDecl = dynamic_cast<ast::MethodDecl*>(methods)) {
         if(!classDecl->modifiers().isAbstract() &&
            methodDecl->modifiers().isAbstract()) {
            diag.ReportError(methodDecl->location())
                  << "A non-abstract class must not contain abstract methods. "
                  << methodDecl->name();
         }
      }

      for(auto other : classDecl->methods()) {
         if(isSameMethodSignature(methods, other)) {
            if(methods->name() == other->name()) {
               diag.ReportError(methods->location())
                  << "A class must not declare two methods with the same signature. "
                  << methods->name();
            }
         }
      }
   }

   if(auto superClassDecl = dynamic_cast<ast::ClassDecl*>(classDecl->superClass()->decl())) {
      for(auto superClassMethod : superClassDecl->methods()) {
         if(!classDecl->modifiers().isAbstract() && superClassDecl->modifiers().isAbstract()) {
            for(auto classMethod : classDecl->methods()) {
               if(isSameMethodSignature(superClassMethod, classMethod)) {
                  diag.ReportError(superClassDecl->location())
                     << "A non-abstract class must not inherit abstract methods. "
                     << superClassDecl->name();
               }
            }
         }

         for(auto classMethod : classDecl->methods()) {
            if(isSameMethodSignature(superClassMethod, classMethod)) {
               diag.ReportError(superClassDecl->location())
                  << "A class must not contain two methods with the same signature. "
                  << superClassDecl->name();
            }
         }
      }
   }
}

void HierarchyChecker::checkClassConstructors(ast::ClassDecl* classDecl) {
   for(auto constructor : classDecl->constructors()) {
      for(auto other : classDecl->constructors()) {
         if(constructor == other) continue;
         if(constructor->parameters().size() !=
            other->parameters().size()) continue;
         
         bool isSameSignature = true;
         
         for(size_t i = 0; i < constructor->parameters().size(); i++) {
            if(constructor->parameters()[i]->type() !=
               other->parameters()[i]->type()) {
               isSameSignature = false;
               break;
            }
         }

         if(isSameSignature) {
            diag.ReportError(constructor->location())
                  << "A class must not declare two constructors with the same "
                     "signature. "
                  << classDecl->name();
         }
      }
   } 
}

void HierarchyChecker::checkInterfaceMethod(ast::InterfaceDecl* interfaceDecl) {
   for(auto methods : interfaceDecl->methods()) {
      for(auto other : interfaceDecl->methods()) {
         if(methods == other) continue;
         if(methods->name() != other->name()) continue;

         bool isSameSignature = true;
         if(methods->parameters().size() == other->parameters().size()) {
            for(size_t i = 0; i < methods->parameters().size(); i++) {
               if(methods->parameters()[i]->type() !=
                  other->parameters()[i]->type()) {
                  isSameSignature = false;
                  break;
               }
            }
         }

         if(isSameSignature) {
            if(methods->name() == other->name()) {
               diag.ReportError(methods->location())
                     << "An interface must not declare two methods with the same "
                        "signature. "
                     << methods->name();
            }
         }
      }
   }
}

} // namespace semantic