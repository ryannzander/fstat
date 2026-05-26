#pragma once

#include <memory>

#include "core/node.hpp"

namespace fstat::ui {

// Launch the interactive disk-usage browser on an owned size tree and block
// until the user quits. Takes ownership of the tree (the UI holds raw pointers
// into it for the duration of the session). Returns a process exit code.
int run(std::unique_ptr<Node> root);

}  // namespace fstat::ui
