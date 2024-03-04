#include <algorithm>
#include <cassert>
#include <ostream>
#include <ranges>
#include <string>

#include "ast/AST.h"
#include "utils/BumpAllocator.h"
#include "utils/Utils.h"

namespace ast {

using std::ostream;
using std::string;

// LinkingUnit /////////////////////////////////////////////////////////////

LinkingUnit::LinkingUnit(BumpAllocator& alloc,
                         array_ref<CompilationUnit*> compilationUnits) noexcept
      : compilationUnits_{alloc} {
   utils::move_vector<CompilationUnit*>(compilationUnits, compilationUnits_);
}

ostream& LinkingUnit::print(ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   auto i2 = indent(indentation + 1);
   auto i3 = indent(indentation + 2);
   os << i1 << "LinkingUnit {\n" << i2 << "asts: ";

   for(auto& cu : compilationUnits_) {
      cu->print(os, indentation + 1) << "\n";
   }

   os << i1 << "}\n";
   return os;
}

int LinkingUnit::printDotNode(DotPrinter& dp) const {
   int id = dp.id();
   dp.startTLabel(id);
   dp.printTableSingleRow("LinkingUnit", {"bgcolor", "lightblue"});
   dp.endTLabel();

   for(auto& cu : compilationUnits_) {
      dp.printConnection(id, cu->printDotNode(dp));
   }
   return id;
}

// CompilationUnit /////////////////////////////////////////////////////////////

CompilationUnit::CompilationUnit(BumpAllocator& alloc, ReferenceType* package,
                                 array_ref<ImportDeclaration> imports,
                                 SourceRange location, DeclContext* body) noexcept
      : package_{package}, imports_{alloc}, body_{body}, location_{location} {
   if(auto decl = dyn_cast_or_null<Decl*>(body)) {
      decl->setParent(this);
   } else if(decl != nullptr) {
      assert(false && "Body must be a Decl.");
   }
   utils::move_vector<ImportDeclaration>(imports, imports_);
}

ostream& CompilationUnit::print(ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   auto i2 = indent(indentation + 1);
   auto i3 = indent(indentation + 2);
   os << i1 << "CompilationUnit {\n"
      << i2 << "package: " << (package_ ? package_->toString() : "null") << "\n"
      << i2 << "imports: [";
   for(auto& import : imports_) {
      os << "\"" << import.type->toString() << (import.isOnDemand ? ".*" : "")
         << "\",";
   }
   os << "]\n";
   if(body_) {
      body_->print(os, indentation + 1);
   }
   os << i1 << "}\n";
   return os;
}

int CompilationUnit::printDotNode(DotPrinter& dp) const {
   int id = dp.id();
   dp.startTLabel(id);
   dp.printTableSingleRow("CompilationUnit", {"bgcolor", "lightblue"});
   dp.printTableDoubleRow("package", package_ ? package_->toString() : "??");
   std::string imports;
   for(auto& import : imports_) {
      imports += import.type->toString();
      imports += import.isOnDemand ? ".*" : "";
      imports += "\n";
   }
   dp.printTableDoubleRow("imports", imports);
   dp.endTLabel();
   if(body_) dp.printConnection(id, body_->printDotNode(dp));
   return id;
}

// ClassDecl ///////////////////////////////////////////////////////////////////

ClassDecl::ClassDecl(BumpAllocator& alloc, Modifiers modifiers,
                     SourceRange location, string_view name, ReferenceType* super1,
                     ReferenceType* super2, array_ref<ReferenceType*> interfaces,
                     array_ref<Decl*> classBodyDecls) throw()
      : Decl{alloc, name},
        modifiers_{modifiers},
        superClasses_{super1, super2},
        interfaces_{alloc},
        fields_{alloc},
        methods_{alloc},
        constructors_{alloc},
        location_{location} {
   utils::move_vector<ReferenceType*>(interfaces, interfaces_);
   // Sort the classBodyDecls into fields, methods, and constructors
   for(auto bodyDecl : classBodyDecls) {
      if(auto fieldDecl = dyn_cast<FieldDecl*>(bodyDecl)) {
         fields_.push_back(fieldDecl);
      } else if(auto methodDecl = dyn_cast<MethodDecl*>(bodyDecl)) {
         if(methodDecl->isConstructor())
            constructors_.push_back(methodDecl);
         else
            methods_.push_back(methodDecl);
      } else {
         assert(false && "Unexpected class body declaration type");
      }
   }
}

ostream& ClassDecl::print(ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   auto i2 = indent(indentation + 1);
   auto i3 = indent(indentation + 2);
   os << i1 << "ClassDecl {\n"
      << i2 << "modifiers: " << modifiers_.toString() << "\n"
      << i2 << "name: " << this->name()
      << "\n"
      // FIXME(kevin): Fix printing of superclasses
      // << i2 << "superClass: " << (superClass_ ? superClass_->toString() :
      // "null")
      // << "\n"
      << i2 << "interfaces: []\n"
      << i2 << "fields:\n";
   for(auto& field : fields_) field->print(os, indentation + 2);
   os << i2 << "methods:\n";
   for(auto& method : methods_) method->print(os, indentation + 2);
   os << i2 << "constructors:\n";
   for(auto& constructor : constructors_) constructor->print(os, indentation + 2);
   os << i1 << "}\n";
   return os;
}

int ClassDecl::printDotNode(DotPrinter& dp) const {
   int id = dp.id();
   dp.startTLabel(id);
   dp.printTableSingleRow("ClassDecl", {"bgcolor", "lightblue"});
   dp.printTableDoubleRow("modifiers", modifiers_.toString());
   dp.printTableDoubleRow("name", name());
   // FIXME(kevin): Fix printing of superclasses
   // dp.printTableDoubleRow("superClass",
   //                        superClass_ ? superClass_->toString() : "");
   string intf;
   for(auto& i : interfaces()) intf += string{i->toString()} + "\n";
   dp.printTableDoubleRow("interfaces", intf);
   dp.printTableTripleRow("fields",
                          "ctors",
                          "methods",
                          {"port", "fields"},
                          {"port", "constructors"},
                          {"port", "methods"});
   dp.endTLabel();

   for(auto& f : fields())
      dp.printConnection(id, ":fields:w", f->printDotNode(dp));
   for(auto& f : constructors())
      dp.printConnection(id, ":constructors:c", f->printDotNode(dp));
   for(auto& f : methods())
      dp.printConnection(id, ":methods:e", f->printDotNode(dp));
   return id;
}

void ClassDecl::setParent(DeclContext* parent) {
   auto cu = cast<CompilationUnit*>(parent);
   // Set the parent of the class
   Decl::setParent(parent);
   // Build the canonical name
   if(cu->package() != nullptr) {
      canonicalName_ = cu->getPackageName();
      canonicalName_ += ".";
   }
   canonicalName_ += name();

   // Propagate the setParent call to the fields, methods, and constructors
   for(auto& field : fields_) field->setParent(this);
   for(auto& method : methods_) method->setParent(this);
   for(auto& constructor : constructors_) constructor->setParent(this);
}

// InterfaceDecl ///////////////////////////////////////////////////////////////

InterfaceDecl::InterfaceDecl(BumpAllocator& alloc, Modifiers modifiers,
                             SourceRange location, string_view name,
                             array_ref<ReferenceType*> extends,
                             ReferenceType* objectSuperclass,
                             array_ref<Decl*> interfaceBodyDecls) throw()
      : Decl{alloc, name},
        modifiers_{modifiers},
        extends_{alloc},
        methods_{alloc},
        location_{location},
        objectSuperclass_{objectSuperclass} {
   utils::move_vector<ReferenceType*>(extends, extends_);
   for(auto bodyDecl : interfaceBodyDecls) {
      methods_.push_back(cast<MethodDecl*>(bodyDecl));
   }
}

ostream& InterfaceDecl::print(ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   auto i2 = indent(indentation + 1);
   auto i3 = indent(indentation + 2);
   os << i1 << "InterfaceDecl {\n"
      << i2 << "modifiers: " << modifiers_.toString() << "\n"
      << i2 << "name: " << this->name() << "\n"
      << i2 << "extends: []\n"
      << i2 << "methods:\n";
   for(auto& method : methods_) {
      method->print(os, indentation + 2);
   }
   os << i1 << "}\n";
   return os;
}

int InterfaceDecl::printDotNode(DotPrinter& dp) const {
   int id = dp.id();
   dp.startTLabel(id);
   dp.printTableSingleRow("InterfaceDecl", {"bgcolor", "lightblue"});
   dp.printTableDoubleRow("modifiers", modifiers_.toString());
   dp.printTableDoubleRow("name", name());
   string ext;
   for(auto& e : extends()) ext += string{e->toString()} + "\n";
   dp.printTableDoubleRow("extends", ext);
   dp.printTableDoubleRow("methods", "", {}, {"port", "methods"});
   dp.endTLabel();

   for(auto& f : methods())
      dp.printConnection(id, ":methods", f->printDotNode(dp));
   return id;
}

void InterfaceDecl::setParent(DeclContext* parent) {
   auto cu = cast<CompilationUnit*>(parent);
   // Set the parent of the interface
   Decl::setParent(parent);
   // Build the canonical name
   canonicalName_ = cu->getPackageName();
   canonicalName_ += ".";
   canonicalName_ += name();
   // Propagate the setParent call to the methods
   for(auto& method : methods_) method->setParent(this);
}

// MethodDecl //////////////////////////////////////////////////////////////////

ostream& MethodDecl::print(ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   auto i2 = indent(indentation + 1);
   auto i3 = indent(indentation + 2);
   os << i1 << "MethodDecl {\n"
      << i2 << "modifiers: " << modifiers_.toString() << "\n"
      << i2 << "name: " << this->name() << "\n"
      << i2 << "returnType: " << (returnType_ ? returnType_->toString() : "null")
      << "\n"
      << i2 << "parameters:\n";
   for(auto& parameter : parameters_) {
      parameter->print(os, indentation + 2);
   }
   if(body_) {
      body_->print(os, indentation + 1);
   }
   os << i1 << "}\n";
   return os;
}

int MethodDecl::printDotNode(DotPrinter& dp) const {
   int id = dp.id();
   int paramSubgraphId = dp.id();

   // Print the method declaration node itself
   dp.startTLabel(id);
   {
      dp.printTableSingleRow("MethodDecl", {"bgcolor", "lightblue"});
      dp.printTableDoubleRow("modifiers", modifiers_.toString());
      dp.printTableDoubleRow("name", name());
      dp.printTableDoubleRow("returnType",
                             returnType_ ? returnType_->toString() : "null");
      dp.printTableDoubleRow(
            "Params", "Body", {"port", "parameters"}, {"port", "body"});
   }
   dp.endTLabel();

   // Print the parameter subgraph and connect it to the method node
   int firstParamId = -1;
   dp.startSubgraph(paramSubgraphId);
   if(!parameters().empty()) {
      dp.print("label=\"Parameters\"");
      dp.print("color=lightcoral");
      firstParamId = printDotNodeList(dp, parameters());
   }
   dp.endSubgraph();
   if(firstParamId != -1)
      dp.printConnection(id, ":parameters", firstParamId, paramSubgraphId);

   // If there's no body, just bail out here
   if(!body_) return id;

   // Print the body subgraph and connect it to the method
   auto ids = printStmtSubgraph(dp, body_);
   if(ids.second != -1)
      dp.printConnection(id, ":body:c", ids.first, ids.second);
   else
      dp.printConnection(id, ":body:c", ids.first);

   // Print backedges here
   for(auto& p : locals_) {
      if(int d = dp.getId(p)) {
         if(d != -1) dp.printBackedge(d, id);
      }
   }
   return id;
}

void MethodDecl::setParent(DeclContext* parent) {
   // Check that parent is either a ClassDecl or an InterfaceDecl
   auto decl = cast<Decl*>(parent);
   assert(dyn_cast<ClassDecl*>(parent) || dyn_cast<InterfaceDecl*>(parent));
   // Set the parent of the method
   Decl::setParent(parent);
   // Build the canonical name
   canonicalName_ = decl->getCanonicalName();
   canonicalName_ += ".";
   canonicalName_ += name();
   // Propagate the setParent call to the parameters and the body
   for(auto& parameter : locals_) parameter->setParent(this);
   // if(body_) body_->setParent(this);
}

} // namespace ast
