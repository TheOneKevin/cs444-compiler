#pragma once

namespace ast {

// Forward delcarations of AST nodes

class CompilationUnit;
class PackageDeclaration;

class AstNode {
    // ...
};

class CompilationUnit : public AstNode {
public:
    CompilationUnit(
        PackageDeclaration* packageDeclaration,
        ImportDeclarations* importDeclarations,
        TypeDeclarations* typeDeclarations
    ) {}
};

class PackageDeclaration : public AstNode {
public:
    
};

class ImportDeclarations : public AstNode {
public:

};

class TypeDeclarations : public AstNode {
public:

};

} // namespace ast
