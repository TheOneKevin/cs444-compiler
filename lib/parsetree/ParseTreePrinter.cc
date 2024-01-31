#include "ParseTreeTypes.h"
#include <iostream>

using namespace parsetree;

void Node::print_type(std::ostream& os) const {
    switch(type) {
        // Leafs
        case Type::Literal: os << "Literal"; break;
        case Type::Identifier: os << "Identifier"; break;
        case Type::Operator: os << "Operator"; break;
        case Type::Type: os << "Type"; break;
        case Type::Modifier: os << "Modifier"; break;

        // Rules
        case Type::CompilationUnit: os << "CompilationUnit"; break;
        case Type::PackageDeclaration: os << "PackageDeclaration"; break;
        case Type::ImportDeclarations: os << "ImportDeclarations"; break;
        case Type::TypeDeclarations: os << "TypeDeclarations"; break;
        case Type::ClassModifiers: os << "ClassModifiers"; break;
        case Type::InterfaceTypeList: os << "InterfaceTypeList"; break;
        case Type::ClassBodyDeclarations: os << "ClassBodyDeclarations"; break;
        case Type::FieldDeclaration: os << "FieldDeclaration"; break;
        case Type::MemberModifiers: os << "MemberModifiers"; break;
        case Type::MethodDeclaration: os << "MethodDeclaration"; break;
        case Type::FormalParameterList: os << "FormalParameterList"; break;
        case Type::FormalParameter: os << "FormalParameter"; break;
        case Type::ConstructorDeclaration: os << "ConstructorDeclaration"; break;
        case Type::ConstructorModifiers: os << "ConstructorModifiers"; break;
        case Type::InterfaceDeclaration: os << "InterfaceDeclaration"; break;
        case Type::ExtendsInterfaces: os << "ExtendsInterfaces"; break;
        case Type::InterfaceMemberDeclarations: os << "InterfaceMemberDeclarations"; break;
        case Type::AbstractMethodDeclaration: os << "AbstractMethodDeclaration"; break;
        case Type::Expression: os << "Expression"; break;
        case Type::FieldAccess: os << "FieldAccess"; break;
        case Type::ArrayAccess: os << "ArrayAccess"; break;
        case Type::CastExpression: os << "CastExpression"; break;
        case Type::MethodInvocation: os << "MethodInvocation"; break;
        case Type::ArrayCreationExpression: os << "ArrayCreationExpression"; break;
        case Type::ClassInstanceCreationExpression: os << "ClassInstanceCreationExpression"; break;
        case Type::ArgumentList: os << "ArgumentList"; break;
        case Type::Block: os << "Block"; break;
        case Type::LocalVariableDeclaration: os << "LocalVariableDeclaration"; break;
        case Type::VariableDeclarators: os << "VariableDeclarators"; break;
        case Type::IfThenStatement: os << "IfThenStatement"; break;
        case Type::WhileStatement: os << "WhileStatement"; break;
        case Type::ForStatement: os << "ForStatement"; break;
        case Type::ClassDeclaration: os << "ClassDeclaration"; break;
        case Type::Extends: os << "Extends"; break;
        case Type::MethodHeader: os << "MethodHeader"; break;
        default: os << (int) type; break;
    }
}

std::ostream& Node::print(std::ostream& os) const {
    os << "(";
    print_type(os);
    for(size_t i = 0; i < num_args; ++i) {
        os << " ";
        if(args[i] == nullptr) {
            os << "null";
        } else {
            args[i]->print(os);
        }
    }
    os << ")";
    return os;
}

std::ostream& Operator::print(std::ostream& os) const {
    // Grab the operator string
    os << to_string();
    return os;
}

std::string Operator::to_string() const {
    using T = Operator::Type;
    switch(type) {
        case T::Assign:
            return "=";
        case T::GreaterThan:
            return ">";
        case T::LessThan:
            return "<";
        case T::Not:
            return "!";
        case T::Equal:
            return "==";
        case T::LessThanOrEqual:
            return "<=";
        case T::GreaterThanOrEqual:
            return ">=";
        case T::NotEqual:
            return "!=";
        case T::And:
            return "&&";
        case T::Or:
            return "||";
        case T::BitwiseAnd:
            return "&";
        case T::BitwiseOr:
            return "|";
        case T::BitwiseXor:
            return "^";
        case T::BitwiseNot:
            return "~";
        case T::Add:
            return "+";
        case T::Subtract:
            return "-";
        case T::Multiply:
            return "*";
        case T::Divide:
            return "/";
        case T::Modulo:
            return "%";
        case T::Plus:
            return "+";
        case T::Minus:
            return "-";
        case T::InstanceOf:
            return "instanceof";
    }
    return "";
}

std::ostream& Identifier::print(std::ostream& os) const {
    os << "(Id " << name << ")";
    return os;
}

std::ostream& Literal::print(std::ostream& os) const {
    os << "(Literal " << (int) type << " " << value << ")";
    return os;
}

std::ostream& parsetree::operator<< (std::ostream& os, const Node& node) {
    node.print(os);
    return os;
}
