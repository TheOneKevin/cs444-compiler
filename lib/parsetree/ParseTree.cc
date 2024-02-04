#include "parsetree/ParseTree.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace parsetree {

bool Literal::isValid() const {
   if(type != Type::Integer) return true;
   errno = 0;
   char* endptr{};
   const long long int x = std::strtol(value.c_str(), &endptr, 10);
   const long long int INT_MAX = 2147483647ULL;
   if(errno == ERANGE || *endptr != '\0') return false;
   if(x > INT_MAX && !isNegative) return false;
   if(x > INT_MAX + 1 && isNegative) return false;
   return true;
}

// Leaf node printers //////////////////////////////////////////////////////////

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
   os << "(Literal " << Type_to_string(type, "??") << " " << formattedValue
      << ")";
   return os;
}

std::ostream& Modifier::print(std::ostream& os) const {
   os << "(Modifier " << Type_to_string(modty, "??") << ")";
   return os;
}

std::ostream& BasicType::print(std::ostream& os) const {
   os << "(BasicType " << Type_to_string(type, "??") << ")";
   return os;
}

// Node printers ///////////////////////////////////////////////////////////////

void Node::printType(std::ostream& os) const {
   os << Type_to_string(type, "Unknown");
}

void Node::printTypeAndValue(std::ostream& os) const {
   if(num_children() == 0) {
      print(os);
   } else {
      printType(os);
   }
}

std::ostream& Node::print(std::ostream& os) const {
   os << "(";
   printType(os);
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

int Node::printDotRecursive(std::ostream& os,
                            const Node& node,
                            int& id_counter) const {
   const int id = id_counter++;
   os << "  " << id << " [label=\"";
   node.printTypeAndValue(os);
   os << "\" shape=rect";
   if(node.get_node_type() == Node::Type::Poison) {
      os << " style=filled fillcolor=red fontcolor=white";
   } else if(node.num_children() == 0) {
      os << " style=filled fillcolor=lightblue";
   }
   os << "];\n";
   for(size_t i = 0; i < node.num_children(); i++) {
      const Node* child = node.child(i);
      int child_id = 0;
      if(child != nullptr) {
         child_id = printDotRecursive(os, *child, id_counter);
      } else {
         child_id = id_counter++;
         os << child_id << " ["
            << "label=\"ε\" "
            << "shape=rect style=filled fillcolor=lightgrey "
            << "width=0.25 height=0.25 "
            << "fontsize=20 "
            << "];\n";
      }
      os << "  " << id << " -> " << child_id << ";\n";
   }
   return id;
}

std::ostream& Node::printDot(std::ostream& os) const {
   os << "digraph G {\n";
   int id_counter = 0;
   printDotRecursive(os, *this, id_counter);
   os << "}\n";
   return os;
}

std::ostream& operator<<(std::ostream& os, Node const& n) {
   return n.print(os);
}

} // namespace parsetree
