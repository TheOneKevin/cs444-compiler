#include "AstNode.h"

namespace ast {

void Modifiers::set(parsetree::Modifier modifier) {
    switch(modifier.get_type()) {
        case parsetree::Modifier::Type::Public:
            isPublic_ = true;
            break;
        case parsetree::Modifier::Type::Protected:
            isProtected_ = true;
            break;
        case parsetree::Modifier::Type::Static:
            isStatic_ = true;
            break;
        case parsetree::Modifier::Type::Final:
            isFinal_ = true;
            break;
        case parsetree::Modifier::Type::Abstract:
            isAbstract_ = true;
            break;
        case parsetree::Modifier::Type::Native:
            isNative_ = true;
            break;
        default:
            break;
    }
}

void Modifiers::set(ast::Modifiers modifier) {
    isPublic_ |= modifier.isPublic_;
    isProtected_ |= modifier.isProtected_;
    isStatic_ |= modifier.isStatic_;
    isFinal_ |= modifier.isFinal_;
    isAbstract_ |= modifier.isAbstract_;
    isNative_ |= modifier.isNative_;
}

std::string Modifiers::toString() const {
    std::string result;
    if(isPublic_) result += "public ";
    if(isProtected_) result += "protected ";
    if(isStatic_) result += "static ";
    if(isFinal_) result += "final ";
    if(isAbstract_) result += "abstract ";
    if(isNative_) result += "native ";
    return result;
}

bool Modifiers::isValidClass() const {
    return !isAbstract_ || !isFinal_;
}

} // namespace ast
