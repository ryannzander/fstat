#include "core/scanner.hpp"

#include <system_error>
#include <utility>

namespace fs = std::filesystem;

namespace fstat {

namespace {
// Report progress at most once every this many directories, to keep the
// callback cheap on huge trees.
constexpr std::uint64_t kProgressInterval = 256;
}  // namespace

Scanner::Scanner(ScanOptions options) : options_(std::move(options)) {}

std::unique_ptr<Node> Scanner::scan(const fs::path& root) {
    std::error_code ec;
    const fs::file_status st = fs::symlink_status(root, ec);
    if (ec || !fs::exists(st)) {
        return nullptr;
    }

    auto node = std::make_unique<Node>();
    node->name = root.string();

    if (fs::is_symlink(st)) {
        node->is_link = true;
        if (!options_.follow_symlinks) {
            return node;
        }
    }

    if (fs::is_directory(st)) {
        node->is_dir = true;
        node->size = scan_dir(*node, root);
    } else {
        node->size = fs::file_size(root, ec);
        if (ec) {
            node->had_error = true;
            node->size = 0;
        }
        node->file_count = 1;
        ++progress_.files_seen;
        progress_.bytes_seen += node->size;
    }
    return node;
}

std::uint64_t Scanner::scan_dir(Node& dir, const fs::path& dir_path) {
    std::error_code ec;
    fs::directory_iterator it(dir_path, fs::directory_options::skip_permission_denied, ec);
    if (ec) {
        dir.had_error = true;
        ++progress_.errors;
        return 0;
    }

    std::uint64_t total = 0;
    const fs::directory_iterator end;
    for (; it != end; it.increment(ec)) {
        if (ec) {
            dir.had_error = true;
            ++progress_.errors;
            break;
        }
        if (cancelled_.load(std::memory_order_relaxed)) {
            break;
        }

        const fs::directory_entry& entry = *it;

        std::error_code st_ec;
        const fs::file_status sym_st = entry.symlink_status(st_ec);
        if (st_ec) {
            ++progress_.errors;
            continue;
        }

        auto child = std::make_unique<Node>();
        child->name = entry.path().filename().string();
        child->parent = &dir;
        child->is_link = fs::is_symlink(sym_st);

        if (child->is_link && !options_.follow_symlinks) {
            // Record the link but do not descend or count its target's size.
            dir.children.push_back(std::move(child));
            continue;
        }

        std::error_code kind_ec;
        if (entry.is_directory(kind_ec) && !kind_ec) {
            child->is_dir = true;
            const std::uint64_t child_bytes = scan_dir(*child, entry.path());
            dir.dir_count += 1 + child->dir_count;
            dir.file_count += child->file_count;
            total += child_bytes;
        } else {
            std::error_code sz_ec;
            const std::uint64_t fsize = entry.file_size(sz_ec);
            if (sz_ec) {
                child->had_error = true;
            } else {
                child->size = fsize;
                total += fsize;
                progress_.bytes_seen += fsize;
            }
            child->file_count = 1;
            dir.file_count += 1;
            ++progress_.files_seen;
        }

        dir.children.push_back(std::move(child));
    }

    ++progress_.dirs_seen;
    if (options_.on_progress && progress_.dirs_seen % kProgressInterval == 0) {
        options_.on_progress(progress_);
    }

    dir.size = total;
    return total;
}

}  // namespace fstat
