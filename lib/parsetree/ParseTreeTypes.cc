#include "ParseTreeTypes.h"
#include <vector>

namespace parsetree {

static std::vector<Node*> poison_pool_;

Node* make_poison() {
    poison_pool_.push_back(new Node{Node::Type::Poison});
    return poison_pool_.back();
}

void clear_poison_pool() {
    for (auto node : poison_pool_) {
        delete node;
    }
    poison_pool_.clear();
}

} // namespace parsetree
