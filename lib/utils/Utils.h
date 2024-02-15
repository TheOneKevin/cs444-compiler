#pragma once

#include <vector>

namespace utils {

template <typename T>
void move_vector(std::pmr::vector<T>& from, std::pmr::vector<T>& to) {
   to.reserve(from.size());
   to.insert(to.end(),
             std::make_move_iterator(from.begin()),
             std::make_move_iterator(from.end()));
}

} // namespace utils
