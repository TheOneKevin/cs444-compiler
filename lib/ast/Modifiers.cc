#include "AstNode.h"

namespace ast {

void Modifiers::set(parsetree::Modifier modifier) {
    switch(modifier.get_type()) {
        case parsetree::Modifier::Type::Public:
            setPublic();
            break;
        case parsetree::Modifier::Type::Protected:
            setProtected();
            break;
        case parsetree::Modifier::Type::Static:
            setStatic();
            break;
        case parsetree::Modifier::Type::Final:
            setFinal();
            break;
        case parsetree::Modifier::Type::Abstract:
            setAbstract();
            break;
        case parsetree::Modifier::Type::Native:
            setNative();
            break;
        default:
            break;
    }
}

void Modifiers::set(ast::Modifiers modifier) {
    if (modifier.isPublic_) setPublic();
    if (modifier.isProtected_) setProtected();
    if (modifier.isStatic_) setStatic();
    if (modifier.isFinal_) setFinal();
    if (modifier.isAbstract_) setAbstract();
    if (modifier.isNative_) setNative();
}

void Modifiers::setPublic() {
    if (isPublic_)
        throw std::runtime_error("Cannot have the same modifier twice");
    isPublic_ = true;
}

void Modifiers::setProtected() {
    if (isProtected_)
        throw std::runtime_error("Cannot have the same modifier twice");
    isProtected_ = true;
}

void Modifiers::setStatic() {
    if (isStatic_)
        throw std::runtime_error("Cannot have the same modifier twice");
    isStatic_ = true;
}

void Modifiers::setFinal() {
    if (isFinal_)
        throw std::runtime_error("Cannot have the same modifier twice");
    isFinal_ = true;
}

void Modifiers::setAbstract() {
    if (isAbstract_)
        throw std::runtime_error("Cannot have the same modifier twice");
    isAbstract_ = true;
}

void Modifiers::setNative() {
    if (isNative_)
        throw std::runtime_error("Cannot have the same modifier twice");
    isNative_ = true;
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

} // namespace ast
