#include "ParseTreeTypes.h"
#include <vector>
#include <string>

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

bool Literal::isValid() const {
    if(type != Type::Integer) return true;
    errno = 0;
    char* endptr{};
    const long long int x = std::strtol(value.c_str(), &endptr, 10);
    const long long int INT_MAX = 2147483647ULL;
    if (errno == ERANGE || *endptr != '\0')
        return false;
    if(x > INT_MAX && !isNegative)
        return false;
    if(x > INT_MAX + 1 && isNegative)
        return false;
    return true;
}

} // namespace parsetree
