#include <ostream>
#include <string>

#include "ast/AST.h"
#include "utils/BumpAllocator.h"

namespace ast {

using std::ostream;
using std::string;

// CompilationUnit /////////////////////////////////////////////////////////////

ostream& CompilationUnit::print(ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   auto i2 = indent(indentation + 1);
   auto i3 = indent(indentation + 2);
   os << i1 << "CompilationUnit {\n"
      << i2 << "package: " << (package_ ? package_->toString() : "null") << "\n"
      << i2 << "imports: [";
   for(auto& import : imports_) {
      os << "\"" << import.qualifiedIdentifier->toString()
         << (import.isOnDemand ? ".*" : "") << "\",";
   }
   os << "]\n";
   if(body_) {
      body_->print(os, indentation + 1);
   }
   os << i1 << "}\n";
   return os;
}

// ClassDecl ///////////////////////////////////////////////////////////////////

ClassDecl::ClassDecl(BumpAllocator& alloc,
                     Modifiers modifiers,
                     string_view name,
                     QualifiedIdentifier* superClass,
                     array_ref<QualifiedIdentifier*> interfaces,
                     array_ref<Decl*> classBodyDecls)
      : Decl{alloc, name},
        modifiers_{modifiers},
        superClass_{superClass},
        interfaces_{interfaces.size(), alloc},
        fields_{alloc},
        methods_{alloc},
        constructors_{alloc} {
   // Sort the classBodyDecls into fields, methods, and constructors
   for(auto bodyDecl : classBodyDecls) {
      if(auto fieldDecl = dynamic_cast<FieldDecl*>(bodyDecl)) {
         fields_.push_back(fieldDecl);
      } else if(auto methodDecl = dynamic_cast<MethodDecl*>(bodyDecl)) {
         if(methodDecl->isConstructor())
            constructors_.push_back(methodDecl);
         else
            methods_.push_back(methodDecl);
      } else {
         throw std::runtime_error("Invalid class body declaration");
      }
   }
}

ostream& ClassDecl::print(ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   auto i2 = indent(indentation + 1);
   auto i3 = indent(indentation + 2);
   os << i1 << "ClassDecl {\n"
      << i2 << "modifiers: " << modifiers_.toString() << "\n"
      << i2 << "name: " << this->getName() << "\n"
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

// InterfaceDecl ///////////////////////////////////////////////////////////////

InterfaceDecl::InterfaceDecl(BumpAllocator& alloc,
                             Modifiers modifiers,
                             string_view name,
                             array_ref<QualifiedIdentifier*> extends,
                             array_ref<Decl*> interfaceBodyDecls)
      : Decl{alloc, name},
        modifiers_{modifiers},
        extends_{alloc},
        methods_{alloc} {
   for(auto bodyDecl : interfaceBodyDecls) {
      if(auto methodDecl = dynamic_cast<MethodDecl*>(bodyDecl)) {
         methods_.push_back(methodDecl);
      } else {
         throw std::runtime_error("Invalid interface body declaration");
      }
   }
}

ostream& InterfaceDecl::print(ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   auto i2 = indent(indentation + 1);
   auto i3 = indent(indentation + 2);
   os << i1 << "InterfaceDecl {\n"
      << i2 << "modifiers: " << modifiers_.toString() << "\n"
      << i2 << "name: " << this->getName() << "\n"
      << i2 << "extends: []\n"
      << i2 << "methods:\n";
   for(auto& method : methods_) {
      method->print(os, indentation + 2);
   }
   os << i1 << "}\n";
   return os;
}

// MethodDecl //////////////////////////////////////////////////////////////////

ostream& MethodDecl::print(ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   auto i2 = indent(indentation + 1);
   auto i3 = indent(indentation + 2);
   os << i1 << "MethodDecl {\n"
      << i2 << "modifiers: " << modifiers_.toString() << "\n"
      << i2 << "name: " << this->getName() << "\n"
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

} // namespace ast
