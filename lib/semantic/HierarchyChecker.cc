#include "HierarchyChecker.h"

#include <set>

namespace semantic {


static bool isSameMethodSignature(ast::MethodDecl* method1, ast::MethodDecl* method2) {
   if(method1->name() != method2->name()) return false;
   if(method1->parameters().size() != method2->parameters().size()) return false;

   for(size_t i = 0; i < method1->parameters().size(); i++) {
      if(method1->parameters()[i]->type() != method2->parameters()[i]->type()) {
         return false;
      }
   }

   return true;
}

void HierarchyChecker::flattenClassMapHelper(
      ast::ClassDecl* node,
      std::pmr::map<ast::ClassDecl*, std::pmr::vector<ast::ClassDecl*>> &unflattenedMap,
      std::pmr::set<ast::ClassDecl*> &visited,
      std::pmr::set<ast::ClassDecl*> &currentPath
){

   // Mark the node as visited
   visited.insert(node);

   for(auto& superClass : unflattenedMap[node]) {
      currentPath.insert(superClass);
      // If the super class has been visited, we simply add the path to the current path
      if(visited.find(superClass) != visited.end()) {
         currentPath.insert(superClassMap_[superClass].begin(), superClassMap_[superClass].end());
         continue;
      }
      
      // visit the super class
      std::pmr::set<ast::ClassDecl*> superPath;
      flattenClassMapHelper(superClass, unflattenedMap, visited, superPath);
      // Add the child path to the current path
      currentPath.insert(superPath.begin(), superPath.end());
   }

   if(currentPath.find(node) != currentPath.end()) {
      // Cycle detected
      diag.ReportError(node->location())
            << "Cycle detected in class hierarchy." << node->name();
      return;
   }

   // Add the node to the flattened map
   superClassMap_[node] = currentPath;
}

void HierarchyChecker::flattenClassMap(
      std::pmr::map<ast::ClassDecl*, std::pmr::vector<ast::ClassDecl*>>&
            unflattenedMap) {
   std::pmr::set<ast::ClassDecl*> visited;
   for(auto& entry : unflattenedMap) {
      if(visited.count(entry.first)) continue;
      std::pmr::set<ast::ClassDecl*> currentPath;
      flattenClassMapHelper(entry.first, unflattenedMap, visited, currentPath);
   }
}

void HierarchyChecker::flattenInterfaceMapHelper(
      ast::Decl* node,
      std::pmr::map<ast::Decl*, std::pmr::vector<ast::InterfaceDecl*>> &unflattenedMap,
      std::pmr::set<ast::Decl*> &visited,
      std::pmr::set<ast::InterfaceDecl*> &currentPath
) {
   // Mark the node as visited
   visited.insert(node);

   for(auto& superInterface : unflattenedMap[node]) {
      currentPath.insert(superInterface);
      // If the super interface has been visited, we simply add the path to the current path
      if(visited.find(superInterface) != visited.end()) {
         currentPath.insert(superInterfaceMap_[superInterface].begin(), superInterfaceMap_[superInterface].end());
         continue;
      }
      
      // visit the super interface
      std::pmr::set<ast::InterfaceDecl*> superPath;
      flattenInterfaceMapHelper(superInterface, unflattenedMap, visited, superPath);
      // Add the child path to the current path
      currentPath.insert(superPath.begin(), superPath.end());
   }

   if (auto classDecl = dynamic_cast<ast::ClassDecl*>(node)) {
      // The superinterfaces of the super class is also the superinterfaces of the derived class
      for (auto superClass : superClassMap_[classDecl]) {
         // If the super class has been visited, we simply add the path to the current path
         if (visited.count(superClass)) {
            currentPath.insert(superInterfaceMap_[superClass].begin(), superInterfaceMap_[superClass].end());
            continue;
         }
         std::pmr::set<ast::InterfaceDecl*> superPath;
         flattenInterfaceMapHelper(superClass, unflattenedMap, visited, superPath);
         // Add the super path to the current path
         currentPath.insert(superPath.begin(), superPath.end());
      }
   } else if (auto interfaceDecl = dynamic_cast<ast::InterfaceDecl*>(node)) {
      // Cycle detected
      if (currentPath.find(interfaceDecl) != currentPath.end()) {
         diag.ReportError(interfaceDecl->location())
               << "Cycle detected in interface hierarchy." << node->name();
         return;
      }
   }

   // Add the node to the flattened map
   superInterfaceMap_[node] = currentPath;
}

void HierarchyChecker::flattenInterfaceMap(std::pmr::map<ast::Decl*, std::pmr::vector<ast::InterfaceDecl*>> &unflattenedMap) {
   std::pmr::set<ast::Decl*> visited;
   for(auto& entry : unflattenedMap) {
      if(visited.count(entry.first)) continue;
      std::pmr::set<ast::InterfaceDecl*> currentPath;
      flattenInterfaceMapHelper(entry.first, unflattenedMap, visited, currentPath);
   }
}

void HierarchyChecker::checkInheritance() {
   std::pmr::map<ast::ClassDecl*, std::pmr::vector<ast::ClassDecl*>>
         unflattenedClassMap;
   std::pmr::map<ast::Decl*, std::pmr::vector<ast::InterfaceDecl*>>
         unflattenedInterfaceMap;
   for(auto cu : lu_->compliationUnits()) {
      auto body = cu->body();
      // if the body is null, continue to the next iteration
      if(!body) continue;

      if(auto classDecl = dynamic_cast<ast::ClassDecl*>(body)) {
         // if there is a superclass
         if(auto superClass = classDecl->superClass()) {
            auto superClassDecl =
                  dynamic_cast<ast::ClassDecl*>(superClass->decl());
            // class cannot extend an interface
            if(!superClassDecl) {
               diag.ReportError(classDecl->location())
                     << "A class must not extend a interface. "
                     << classDecl->name();
               continue;
            }
            // class cannot extend a final class
            if(superClassDecl->modifiers().isFinal()) {
               diag.ReportError(classDecl->location())
                     << "A class must not extend a final class"
                     << classDecl->name();
            } 
            unflattenedClassMap[classDecl].emplace_back(superClassDecl);
         }

         // check if interfaces are valid
         for(auto interface : classDecl->interfaces()) {
            // check if there are duplicate interfaces
            for(auto other : classDecl->interfaces()) {
               if(interface == other) continue;
               if(interface->decl() == other->decl()) {
                  diag.ReportError(classDecl->location())
                        << "A class must not implement the same interface twice. "
                        << classDecl->name();
               }
            }

            // check that the interface is not a class
            auto interfaceDecl =
                  dynamic_cast<ast::InterfaceDecl*>(interface->decl());
            if(!interfaceDecl) {
               diag.ReportError(classDecl->location())
                     << "A class must not implement a class" << classDecl->name();
            }
            unflattenedInterfaceMap[classDecl].emplace_back(interfaceDecl);
         }
      } else if(auto interfaceDecl = dynamic_cast<ast::InterfaceDecl*>(body)) {
         // no duplicate interfaces
         for(auto extends : interfaceDecl->extends()) {
            for(auto other : interfaceDecl->extends()) {
               if(extends == other) continue;
               if(extends->decl() == other->decl()) {
                  diag.ReportError(interfaceDecl->location())
                        << "A interface must not extend the same interface twice. "
                        << interfaceDecl->name();
               }
            }
            // check that the interface is not a class
            auto inferfaceDecl = dynamic_cast<ast::InterfaceDecl*>(extends->decl());
            if(!interfaceDecl) {
               diag.ReportError(interfaceDecl->location())
                     << "A interface must not extend a class"
                     << interfaceDecl->name();
            }
            unflattenedInterfaceMap[interfaceDecl].emplace_back(inferfaceDecl);
         }
      }
   }
   // Flatten the hierarchy maps so we can perform cycle detection and check for
   // correct method overriding
   flattenClassMap(unflattenedClassMap);
   flattenInterfaceMap(unflattenedInterfaceMap);
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
                     << "A class must not declare two methods with the same "
                        "signature. "
                     << methods->name();
            }
         }
      }
   }

   if(auto superClassDecl =
            dynamic_cast<ast::ClassDecl*>(classDecl->superClass()->decl())) {
      for(auto superClassMethod : superClassDecl->methods()) {
         if(!classDecl->modifiers().isAbstract() &&
            superClassDecl->modifiers().isAbstract()) {
            for(auto classMethod : classDecl->methods()) {
               if(isSameMethodSignature(superClassMethod, classMethod)) {
                  diag.ReportError(superClassDecl->location())
                        << "A non-abstract class must not inherit abstract "
                           "methods. "
                        << superClassDecl->name();
               }
            }
         }

         for(auto classMethod : classDecl->methods()) {
            if(isSameMethodSignature(superClassMethod, classMethod)) {
               diag.ReportError(superClassDecl->location())
                     << "A class must not contain two methods with the same "
                        "signature. "
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
         if(constructor->parameters().size() != other->parameters().size())
            continue;

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