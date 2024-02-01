#include "ParseTreeTypes.h"
#include <iostream>
#include <string>
#include <algorithm>

using namespace parsetree;

void Node::print_type(std::ostream& os) const {
    os << Type_to_string(type, "Unknown");
}

void Node::print_type_and_value(std::ostream& os) const {
    if (num_children() == 0) {
        print(os);
    } else {
        print_type(os);
    }
}

std::ostream& Node::print(std::ostream& os) const {
    os << "(";
    print_type(os);
    for(size_t i = 0; i < num_args; ++i) {
        os << " ";
        if(args[i] == nullptr) {
            os << "ε";
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
    std::string formattedValue = value;
    std::replace(formattedValue.begin(), formattedValue.end(), '\"', ' ');
    os << "(Literal " << Type_to_string(type, "??") << " " << formattedValue << ")";
    return os;
}

std::ostream& parsetree::operator<< (std::ostream& os, const Node& node) {
    node.print(os);
    return os;
}

std::ostream& Modifier::print(std::ostream& os) const {
    os << "(Modifier " << Type_to_string(type, "??") << ")";
    return os;
}

std::ostream& BasicType::print(std::ostream& os) const {
    os << "(BasicType " << Type_to_string(type, "??") << ")";
    return os;
}
