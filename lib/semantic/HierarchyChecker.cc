#include "HierarchyChecker.h"

#include <map>
#include <vector>
namespace semantic {

void HierarchyChecker::checkInheritance() {
   std::pmr::map<ast::Decl*, std::pmr::vector<ast::Decl*>> hierarchyMap;
   for (auto cu : lu_->compliationUnits()) {
      auto body = cu->body();
      // if the body is null, continue to the next iteration
      if (!body) continue;
      
      if (auto classDecl = dynamic_cast<ast::ClassDecl*>(body)) {
         hierarchyMap[classDecl].emplace_back(classDecl->superClass()->decl());

         for (auto interface : classDecl->interfaces()) {
            for (auto other : classDecl->interfaces()) {
               if (interface == other) continue;
               if (interface->decl() == other->decl()) {
                  diag.ReportError(classDecl->location())
                     << "A class must not implement the same interface twice. " << classDecl->name();
               }
            }
            if (dynamic_cast<ast::ClassDecl*>(interface->decl())) {
               diag.ReportError(classDecl->location())
                  << "A class must not implement a class. " << classDecl->name();
            }
            hierarchyMap[classDecl].emplace_back(interface->decl());
         }
         
         if (dynamic_cast<ast::InterfaceDecl*>(classDecl->superClass()->decl())) {
            diag.ReportError(classDecl->location())
                  << "A class must not extend an interface.";
         } else if (auto superClass = dynamic_cast<ast::ClassDecl*>(classDecl->superClass()->decl())) {
            if (superClass->modifiers().isFinal()) {
               diag.ReportError(classDecl->location())
                  << "A class must not extend a final class" << classDecl->name();
            }
         }
      } else if (auto interfaceDecl = dynamic_cast<ast::InterfaceDecl*>(body)) {
         for (auto extends : interfaceDecl->extends()) {
            for (auto other : interfaceDecl->extends()) {
               if (extends == other) continue;
               if (extends->decl() == other->decl()) {
                  diag.ReportError(interfaceDecl->location())
                     << "A interface must not extend the same interface twice. " << interfaceDecl->name();
               }
            }
            if (dynamic_cast<ast::ClassDecl*>(extends->decl())) {
               diag.ReportError(interfaceDecl->location())
                  << "A interface must not extend a class" << interfaceDecl->name();
            }

            hierarchyMap[interfaceDecl].emplace_back(extends->decl());
         }
         
      }
   }
   
   if (isAcyclic(hierarchyMap)) {
      diag.ReportError(SourceRange{}) << "The inheritance hierarchy is cyclic.";
   }
}

// Write a function to check if the hierarchy is acyclic
bool isAcyclic(std::pmr::map<ast::Decl*, std::pmr::vector<ast::Decl*>>& hierarchyMap) {
   std::pmr::map<ast::Decl*, bool> visited;

   for (auto [decl, children] : hierarchyMap) {
      if (!visited[decl]) {
         if (dfs(decl, visited, hierarchyMap)) {
            return true;
         }
      }
   }

   return false;
}

bool dfs(
   ast::Decl* decl, 
   std::pmr::map<ast::Decl*, bool>& visited, 
   std::pmr::map<ast::Decl*, std::pmr::vector<ast::Decl*>>& hierarchyMap
) {
   if (visited[decl]) return true;
   
   visited[decl] = true;

   for (auto child : hierarchyMap[decl]) {
      if (!visited[child]) {
         if (dfs(child, visited, hierarchyMap)) {
            return true;
         }
      }
   }
   
   return false;
}

}