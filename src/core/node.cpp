#include "core/node.hpp"

#include <algorithm>

namespace fstat {

std::filesystem::path Node::path() const {
    if (parent == nullptr) {
        return std::filesystem::path(name);
    }
    return parent->path() / name;
}

void Node::sort_by_size(bool deep) {
    std::sort(children.begin(), children.end(),
              [](const std::unique_ptr<Node>& a, const std::unique_ptr<Node>& b) {
                  return a->size > b->size;
              });
    if (deep) {
        for (auto& child : children) {
            child->sort_by_size(true);
        }
    }
}

double Node::fraction_of_parent() const {
    if (parent == nullptr || parent->size == 0) {
        return 0.0;
    }
    return static_cast<double>(size) / static_cast<double>(parent->size);
}

}  // namespace fstat
