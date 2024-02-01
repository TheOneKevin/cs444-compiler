#include "ParseTreeTypes.h"

using namespace parsetree;

ast::CompilationUnit* parsetree::VisitCompilationUnit(Node* node) {
    // Check the node we're visiting
    if(node->get_type() != Node::Type::CompilationUnit) {
        throw std::runtime_error("VisitCompilationUnit called on a node that is not a CompilationUnit");
    }
    // Type check the children nodes
    if (node->num_children() != 3) {
        throw std::runtime_error("CompilationUnit node must have 3 children");
    }
    return new ast::CompilationUnit {
        VisitPackageDeclaration(node->children[0]),
        VisitImportDeclarations(node->children[1]),
        VisitTypeDeclarations(node->children[2])
    };
}
