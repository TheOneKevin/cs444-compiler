#include "HierarchyChecker.h"

#include <set>

namespace semantic {

static bool isSameMethodSignature(ast::MethodDecl* method1,
                                  ast::MethodDecl* method2) {
   if(method1->name() != method2->name()) return false;
   if(method1->parameters().size() != method2->parameters().size()) return false;

   for(size_t i = 0; i < method1->parameters().size(); i++) {
      if(*method1->parameters()[i]->type() != *method2->parameters()[i]->type()) {
         return false;
      }
   }

   return true;
}

void HierarchyChecker::checkMethodInheritanceHelper(
      ast::Decl* node, std::pmr::set<ast::Decl*>& visited) {
   // Mark the node as visited
   visited.insert(node);
   std::pmr::vector<ast::MethodDecl*> inheritedMethods;

   for(auto super : inheritanceMap_[node]) {
      if(auto superClass = dynamic_cast<ast::ClassDecl*>(super)) {
         if(!visited.count(superClass)) {
            checkMethodInheritanceHelper(superClass, visited);
         } else if(!superClass->isInheritedSet()) {
            diag.ReportError(superClass->location())
                  << "Cycle is detected in the inheritance graph. "
                  << superClass->name();
            continue;
         }
         for(auto method : superClass->inheritedMethods()) {
            inheritedMethods.emplace_back(method);
         }
      } else if(auto superInterface = dynamic_cast<ast::InterfaceDecl*>(super)) {
         if(!visited.count(superInterface)) {
            checkMethodInheritanceHelper(superInterface, visited);
         } else if(!superInterface->isInheritedSet()) {
            diag.ReportError(superInterface->location())
                  << "Cycle is detected in the inheritance graph. "
                  << superInterface->name();
            continue;
         }
         for(auto method : superInterface->inheritedMethods()) {
            inheritedMethods.emplace_back(method);
         }
      } else {
         assert(false && "Unreachable");
      }
   }
   if(auto classDecl = dynamic_cast<ast::ClassDecl*>(node)) {
      checkClassMethod(classDecl, inheritedMethods);
      checkClassConstructors(classDecl);
   } else if(auto interfaceDecl = dynamic_cast<ast::InterfaceDecl*>(node)) {
      checkInterfaceMethod(interfaceDecl, inheritedMethods);
   }
}

void HierarchyChecker::checkMethodInheritance() {
   std::pmr::set<ast::Decl*> visited;
   for(auto cu : lu_->compliationUnits()) {
      // FIXME(kevin, larry): Do we really need mutable here?
      auto body = cu->mut_body();
      // if the body is null, continue to the next iteration
      if(!body) continue;
      if(auto classDecl = dynamic_cast<ast::ClassDecl*>(body)) {
         if(visited.count(classDecl)) continue;
         checkMethodInheritanceHelper(classDecl, visited);
      } else if(auto interfaceDecl = dynamic_cast<ast::InterfaceDecl*>(body)) {
         if(visited.count(interfaceDecl)) continue;
         checkMethodInheritanceHelper(interfaceDecl, visited);
      }
   }
}

void HierarchyChecker::checkInheritance() {
   for(auto cu : lu_->compliationUnits()) {
      // FIXME(kevin, larry): Do we really need mutable here?
      auto body = cu->mut_body();
      // if the body is null, continue to the next iteration
      if(!body) continue;

      if(auto classDecl = dynamic_cast<ast::ClassDecl*>(body)) {
         // if there is a superclass
         if(auto superClass = classDecl->superClasses()[0]) {
            auto superClassDecl =
                  dynamic_cast<ast::ClassDecl*>(superClass->decl());
            // class cannot extend an interface
            if(!superClassDecl) {
               diag.ReportError(classDecl->location())
                     << "A class must not extend an interface. "
                     << classDecl->name();
               continue;
            }
            // class cannot extend a final class
            if(superClassDecl->modifiers().isFinal()) {
               diag.ReportError(classDecl->location())
                     << "A class must not extend a final class"
                     << classDecl->name();
            }
            inheritanceMap_[classDecl].insert(superClassDecl);
         } else if(auto objectClass = classDecl->superClasses()[1]) {
            // if the class does not extend any class, it extends the object class
            inheritanceMap_[classDecl].insert(
                  dynamic_cast<ast::ClassDecl*>(objectClass->decl()));
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
            inheritanceMap_[classDecl].insert(interfaceDecl);
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
            auto superInterface =
                  dynamic_cast<ast::InterfaceDecl*>(extends->decl());
            if(!superInterface) {
               diag.ReportError(superInterface->location())
                     << "A interface must not extend a class"
                     << superInterface->name();
            }
            inheritanceMap_[interfaceDecl].insert(superInterface);
            // print debug information
            if(diag.Verbose()) {
               diag.ReportDebug() << "Interface: " << interfaceDecl->name()
                                  << " extends " << superInterface->name() << "\n";
            }
         }
      }
   }
   checkMethodInheritance();
}

void HierarchyChecker::checkClassMethod(
      ast::ClassDecl* classDecl,
      std::pmr::vector<ast::MethodDecl*>& inheritedMethods) {
   std::pmr::vector<ast::MethodDecl*> allMethods;
   std::pmr::vector<ast::MethodDecl*> inheritedNotOverriden;
   // check for duplicate methods
   for(auto method : classDecl->methods()) {
      allMethods.emplace_back(method);
      for(auto other : classDecl->methods()) {
         if(method == other) continue;
         if(isSameMethodSignature(method, other)) {
            diag.ReportError(method->location())
                  << "A class must not declare two methods with the same "
                     "signature. "
                  << method->name();
         }
      }
   }

   // check for abstract declaration
   for(auto method : classDecl->methods()) {
      if(method->modifiers().isAbstract() &&
         !classDecl->modifiers().isAbstract()) {
         diag.ReportError(classDecl->location())
               << "A class that contains (declares or inherits) any "
                  "abstract methods must be abstract. "
               << classDecl->name();
         break;
      }
   }

   // check for method replacement
   for(auto* other : inheritedMethods) {
      bool isOverriden = false;
      for(auto* method : classDecl->methods()) {
         if(!isSameMethodSignature(method, other)) continue;
         isOverriden = true;
         if(method->returnTy() != other->returnTy()) {
            diag.ReportError(classDecl->location())
                  << "A method must not replace a method with a "
                     "different return type. "
                  << other->name();
         }
         if(!method->modifiers().isStatic() && other->modifiers().isStatic()) {
            diag.ReportError(classDecl->location())
                  << "A nonstatic method must not replace a static "
                     "method. "
                  << other->name();
         }
         if(method->modifiers().isStatic() && !other->modifiers().isStatic()) {
            diag.ReportError(classDecl->location())
                  << "A static method must not replace a nonstatic "
                     "method. "
                  << other->name();
         }
         if(method->modifiers().isProtected() && other->modifiers().isPublic()) {
            diag.ReportError(classDecl->location())
                  << "A protected method must not replace a public "
                     "method. "
                  << other->name();
         }
         if(other->modifiers().isFinal()) {
            diag.ReportError(classDecl->location())
                  << "A method must not replace a final method. " << other->name();
         }
      }
      if(!isOverriden) inheritedNotOverriden.emplace_back(other);
   }

   // check for abstract method implementation
   for(auto method : inheritedNotOverriden) {
      bool isImplemented = !method->modifiers().isAbstract();
      for(auto other : inheritedNotOverriden) {
         if(isSameMethodSignature(method, other)) {
            if(method->returnTy() != other->returnTy()) {
               diag.ReportError(other->location())
                     << "A method must not replace a method with a "
                        "different return type. "
                     << other->name();
            } else if(!other->modifiers().isAbstract()) {
               if(other->modifiers().isProtected() &&
                  method->modifiers().isPublic()) {
                  diag.ReportError(other->location())
                        << "A protected method must not replace a public "
                           "method. "
                        << other->name();
               }
               isImplemented = true;
            }
         }
      }
      if(!isImplemented && !classDecl->modifiers().isAbstract()) {
         diag.ReportError(classDecl->location())
               << "An abstract method must be implemented in an "
                  "non-abstract class "
               << method->name();
      } else if(isImplemented == !method->modifiers().isAbstract()) {
         allMethods.emplace_back(method);
      }
   }
   // record the inherited methods
   classDecl->setInheritedMethods(allMethods);

   // print debug information
   if(diag.Verbose()) {
      diag.ReportDebug() << "Class: " << classDecl->name();
      diag.ReportDebug() << "Inherited methods: ";
      for(auto method : allMethods) diag.ReportDebug() << "\t" << method->name();
   }
}

void HierarchyChecker::checkClassConstructors(ast::ClassDecl* classDecl) {
   for(auto constructor : classDecl->constructors()) {
      for(auto other : classDecl->constructors()) {
         if(constructor == other) continue;
         if(isSameMethodSignature(constructor, other)) {
            diag.ReportError(constructor->location())
                  << "A class must not declare two constructors with the same "
                     "signature. "
                  << classDecl->name();
         }
      }
   }
}

void HierarchyChecker::checkInterfaceMethod(
      ast::InterfaceDecl* interfaceDecl,
      std::pmr::vector<ast::MethodDecl*>& inheritedMethods) {
   std::pmr::vector<ast::MethodDecl*> allMethods;

   for(auto method : interfaceDecl->methods()) {
      allMethods.emplace_back(method);
      for(auto other : interfaceDecl->methods()) {
         if(method == other) continue;
         if(isSameMethodSignature(method, other)) {
            diag.ReportError(method->location())
                  << "An interface must not declare two methods with the same "
                     "signature. "
                  << method->name();
         }
      }
   }

   // for some reason we have to check against the object class
   auto objectClass =
         dynamic_cast<ast::ClassDecl*>(interfaceDecl->objectSuperclass()->decl());
   for(auto method : interfaceDecl->methods()) {
      for(auto other : objectClass->methods()) {
         if(!isSameMethodSignature(method, other)) continue;
         if(method->returnTy() != other->returnTy()) {
            diag.ReportError(interfaceDecl->location())
                  << "A method must not replace a method with a "
                     "different return type. "
                  << other->name();
         }
         if(method->modifiers().isProtected() && other->modifiers().isPublic()) {
            diag.ReportError(interfaceDecl->location())
                  << "A protected method must not replace a public "
                     "method. "
                  << other->name();
         }
         if(other->modifiers().isFinal()) {
            diag.ReportError(interfaceDecl->location())
                  << "A method must not replace a final method. " << other->name();
         }
      }
   }

   for(auto method : inheritedMethods) {
      bool isOverriden = false;
      for(auto other : interfaceDecl->methods()) {
         if(isSameMethodSignature(method, other)) {
            if(method->returnTy() != other->returnTy()) {
               diag.ReportError(method->location())
                     << "An interface must not contain two methods with the same "
                        "signature. "
                     << method->name();
            } else {
               isOverriden = true;
            }
         }
      }
      if(!isOverriden) allMethods.emplace_back(method);
   }

   for(auto method : allMethods) {
      for(auto other : allMethods) {
         if(method == other) continue;
         if(isSameMethodSignature(method, other) &&
            method->returnTy() != other->returnTy()) {
            diag.ReportError(method->location())
                  << "An interface must not contain two methods with the same "
                     "signature. "
                  << method->name();
         }
      }
   }

   // record the inherited methods
   interfaceDecl->setInheritedMethods(allMethods);

   // print debug information
   if(diag.Verbose()) {
      diag.ReportDebug() << "Interface: " << interfaceDecl->name();
      diag.ReportDebug() << "Inherited methods:";
      for(auto method : allMethods) diag.ReportDebug() << "\t" << method->name();
   }
}

} // namespace semantic