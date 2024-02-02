#pragma once

#include <string>
#include <vector>
#include <iostream>

namespace ast {

// Forward delcarations of AST nodes

class CompilationUnit;
class Import;
class TypeDeclaration;

// class & interface
class ClassDeclaration;
class InterfaceDeclaration;
class ClassOrInterfaceModifier;
class ClassBodyDeclaration;

class QualifiedIdentifier;
class Identifier;

class AstNode {
public:
    virtual std::ostream& print(std::ostream& os) const;
};

class CompilationUnit : public AstNode {
    QualifiedIdentifier* packageDeclaration;
    std::vector<Import*> imports;
public:
    CompilationUnit(
        QualifiedIdentifier* packageDeclaration,
        std::vector<Import *> imports,
        TypeDeclaration* typeDeclaration
    ) : packageDeclaration{packageDeclaration}, imports{imports}, typeDeclaration{typeDeclaration} {}

    std::ostream& print(std::ostream& os) const;
};

class Import : public AstNode {
    bool isSingleType;
    QualifiedIdentifier* qualifiedIdentifier;
public:
    Import(
        bool isSingleType,
        QualifiedIdentifier* qualifiedIdentifier
    ): isSingleType{isSingleType}, qualifiedIdentifier{qualifiedIdentifier} {}

    std::ostream& print(std::ostream& os) const;
};

class TypeDeclaration : public AstNode{};

class ClassDeclaration : public TypeDeclaration{
    std::vector<ClassOrInterfaceModifier*> modifiers;
    Identifier* identifier;
    QualifiedIdentifier* superClass; // Extends super class , null if no extends
    std::vector<QualifiedIdentifier*> interfaces; // Implements interfaces, empty if no implements
    std::vector<MethodDeclaration*> methods;
    std::vector<FieldDeclaration*> fields;
    std::vector<MethodDeclaration*> abstractMethods;
    std::vector<MethodDeclaration*> constructors;
public:
    ClassDeclaration(
        std::vector<ClassOrInterfaceModifier*> modifiers,
        Identifier* identifier,
        QualifiedIdentifier* superClass,
        std::vector<QualifiedIdentifier*> interfaces,
        std::vector<ClassBodyDeclaration*> classBodyDeclarations
    ): modifiers{modifiers}, identifier{identifier}, superClass{superClass}, interfaces{interfaces} {
        // Check classBodyDeclarations and sort into ....
        
    };
};

class ClassBodyDeclaration : public AstNode { };

class MethodDeclaration : public ClassBodyDeclaration {

};

class FieldDeclaration : public ClassBodyDeclaration {

};

class InterfaceDeclaration : public TypeDeclaration{

};

class ClassOrInterfaceModifier : public AstNode {
public:
    enum class ModifierType {
        PUBLIC,
        ABSTRACT,
        FINAL
    };
private:
    ModifierType type;
public:
    ClassOrInterfaceModifier(
        ModifierType type
    ): type{type} {};
};

class QualifiedIdentifier : public AstNode {
    std::vector<Identifier*> identifiers;
public:
    QualifiedIdentifier(
        std::vector<Identifier*> identifiers
    ): identifiers {identifiers} {};

    std::ostream& print(std::ostream& os) const;
};


/* ========================================================================== */
/*                               Literals                                     */
/* ========================================================================== */

class Identifier : public AstNode {
    std::string value;
public:
    Identifier(
        std::string value
    ) : value{value} {}

    std::ostream& print(std::ostream& os) const;
};

std::ostream& operator<< (std::ostream& os, const AstNode& astNode);

} // namespace ast
