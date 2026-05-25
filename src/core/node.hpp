#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace fstat {

// A single entry in the scanned filesystem tree.
//
// Directories own their children; every node keeps a non-owning back-pointer to
// its parent. The Scanner builds the tree bottom-up and populates `size` with
// the aggregate size of the subtree rooted at this node (own size for files).
struct Node {
    std::string name;   // base name, e.g. "Documents"; the root holds a full path
    bool is_dir = false;    // directory vs. regular file
    bool is_link = false;   // symlink / reparse point (recorded, not followed)
    bool had_error = false; // could not be fully read (e.g. access denied)

    std::uint64_t size = 0;        // bytes: own size for files, aggregate for dirs
    std::uint64_t file_count = 0;  // files in this subtree
    std::uint64_t dir_count = 0;   // subdirectories in this subtree

    Node* parent = nullptr;                       // non-owning back-pointer
    std::vector<std::unique_ptr<Node>> children;  // owned children

    // Reconstruct the full path by walking parents. O(depth).
    std::filesystem::path path() const;

    // Sort children by size, largest first. Recurses into subtrees when `deep`.
    void sort_by_size(bool deep = false);

    // Fraction of the parent's size occupied by this node, in [0, 1].
    // Returns 0 when there is no parent or the parent is empty.
    double fraction_of_parent() const;
};

}  // namespace fstat
