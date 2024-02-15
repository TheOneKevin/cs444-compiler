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

// CompilationUnit /////////////////////////////////////////////////////////////

CompilationUnit::CompilationUnit(BumpAllocator& alloc,
                                 ReferenceType* package,
                                 array_ref<ImportDeclaration> imports,
                                 DeclContext* body) noexcept
      : package_{package}, imports_{alloc}, body_{body} {
   if(auto decl = dynamic_cast<Decl*>(body)) {
      decl->setParent(this);
   } else {
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
   dp.printTableSingleRow("CompilationUnit");
   dp.printTableDoubleRow("package", package_ ? package_->toString() : "??");
   std::string imports;
   for(auto& import : imports_) {
      imports +=
            import.type->toString() + (import.isOnDemand ? ".*" : "") + "\n";
   }
   dp.printTableDoubleRow("imports", imports);
   dp.endTLabel();
   if(body_) dp.printConnection(id, body_->printDotNode(dp));
   return id;
}

// ClassDecl ///////////////////////////////////////////////////////////////////

ClassDecl::ClassDecl(BumpAllocator& alloc,
                     Modifiers modifiers,
                     string_view name,
                     ReferenceType* superClass,
                     array_ref<ReferenceType*> interfaces,
                     array_ref<Decl*> classBodyDecls) throw()
      : Decl{alloc, name},
        modifiers_{modifiers},
        superClass_{superClass},
        interfaces_{alloc},
        fields_{alloc},
        methods_{alloc},
        constructors_{alloc} {
   utils::move_vector<ReferenceType*>(interfaces, interfaces_);
   // Sort the classBodyDecls into fields, methods, and constructors
   for(auto bodyDecl : classBodyDecls) {
      if(auto fieldDecl = dynamic_cast<FieldDecl*>(bodyDecl)) {
         fields_.push_back(fieldDecl);
         fieldDecl->setParent(this);
      } else if(auto methodDecl = dynamic_cast<MethodDecl*>(bodyDecl)) {
         methodDecl->setParent(this);
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
      << i2 << "name: " << this->name() << "\n"
      << i2
      << "superClass: " << (superClass_ ? superClass_->toString() : "null")
      << "\n"
      << i2 << "interfaces: []\n"
      << i2 << "fields:\n";
   for(auto& field : fields_) field->print(os, indentation + 2);
   os << i2 << "methods:\n";
   for(auto& method : methods_) method->print(os, indentation + 2);
   os << i2 << "constructors:\n";
   for(auto& constructor : constructors_)
      constructor->print(os, indentation + 2);
   os << i1 << "}\n";
   return os;
}

int ClassDecl::printDotNode(DotPrinter& dp) const {
   int id = dp.id();
   dp.startTLabel(id);
   dp.printTableSingleRow("ClassDecl");
   dp.printTableDoubleRow("modifiers", modifiers_.toString());
   dp.printTableDoubleRow("name", name());
   dp.printTableDoubleRow("superClass",
                          superClass_ ? superClass_->toString() : "");
   string intf;
   for(auto& i : interfaces()) intf += i->toString() + "\\n";
   dp.printTableDoubleRow("interfaces", intf);
   dp.printTableDoubleRow("fields", "", {}, {"port", "fields"});
   dp.printTableDoubleRow("constructors", "", {"port", "constructors"}, {});
   dp.printTableDoubleRow("methods", "", {}, {"port", "methods"});
   dp.endTLabel();

   for(auto& f : fields())
      dp.printConnection(id, ":fields", f->printDotNode(dp));
   for(auto& f : constructors())
      dp.printConnection(id, ":constructors:w", f->printDotNode(dp));
   for(auto& f : methods())
      dp.printConnection(id, ":methods:e", f->printDotNode(dp));
   return id;
}

// InterfaceDecl ///////////////////////////////////////////////////////////////

InterfaceDecl::InterfaceDecl(BumpAllocator& alloc,
                             Modifiers modifiers,
                             string_view name,
                             array_ref<ReferenceType*> extends,
                             array_ref<Decl*> interfaceBodyDecls) throw()
      : Decl{alloc, name},
        modifiers_{modifiers},
        extends_{alloc},
        methods_{alloc} {
   for(auto bodyDecl : interfaceBodyDecls) {
      if(auto methodDecl = dynamic_cast<MethodDecl*>(bodyDecl)) {
         methods_.push_back(methodDecl);
         methodDecl->setParent(this);
      } else {
         assert(false && "Unexpected interface body declaration type");
      }
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
   dp.printTableSingleRow("InterfaceDecl");
   dp.printTableDoubleRow("modifiers", modifiers_.toString());
   dp.printTableDoubleRow("name", name());
   string ext;
   for(auto& e : extends()) {
      ext += e->toString() + "\\n";
   }
   dp.printTableDoubleRow("extends", ext);
   dp.printTableDoubleRow("methods", "", {}, {"port", "methods"});
   dp.endTLabel();

   for(auto& f : methods())
      dp.printConnection(id, ":methods", f->printDotNode(dp));
   return id;
}

// MethodDecl //////////////////////////////////////////////////////////////////

ostream& MethodDecl::print(ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   auto i2 = indent(indentation + 1);
   auto i3 = indent(indentation + 2);
   os << i1 << "MethodDecl {\n"
      << i2 << "modifiers: " << modifiers_.toString() << "\n"
      << i2 << "name: " << this->name() << "\n"
      << i2
      << "returnType: " << (returnType_ ? returnType_->toString() : "null")
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
   int bodySubgraphId = dp.id();

   // Print the method declaration node itself
   dp.startTLabel(id);
   {
      dp.printTableSingleRow("MethodDecl");
      dp.printTableDoubleRow("modifiers", modifiers_.toString());
      dp.printTableDoubleRow("name", name());
      dp.printTableDoubleRow("returnType",
                             returnType_ ? returnType_->toString() : "null");
      dp.printTableDoubleRow("parameters", "", {}, {"port", "parameters"});
      dp.printTableSingleRow("Body", {"port", "body"});
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
   if(auto stmt = dynamic_cast<BlockStatement*>(body_)) {
      int firstBodyId = -1;
      dp.startSubgraph(bodySubgraphId);
      dp.print("label=\"Body\"");
      dp.print("color=lightblue");
      firstBodyId = printDotNodeList(dp, stmt->stmts());
      dp.endSubgraph();
      if(firstBodyId != -1)
         dp.printConnection(id, ":body", firstBodyId, bodySubgraphId);
   } else {
      dp.printConnection(id, ":body", body_->printDotNode(dp));
   }
   return id;
}

} // namespace ast
