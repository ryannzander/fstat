#pragma once

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>

#include "core/node.hpp"

namespace fstat {

// Running totals produced during a scan; useful for live progress display.
struct ScanProgress {
    std::uint64_t files_seen = 0;
    std::uint64_t dirs_seen = 0;
    std::uint64_t bytes_seen = 0;
    std::uint64_t errors = 0;
};

// Options controlling a scan.
struct ScanOptions {
    // Descend into symlinks / reparse points. Off by default to avoid cycles
    // and double-counting (Windows junctions, AppData reparse points, ...).
    bool follow_symlinks = false;

    // Called periodically (every N directories) with running totals. Optional.
    std::function<void(const ScanProgress&)> on_progress;
};

// Recursively scans a directory tree and builds an in-memory Node tree.
//
// The scanner is single-threaded for now (see docs/ARCHITECTURE.md for the
// planned parallel design). It never throws on per-entry errors; instead it
// marks the offending Node with `had_error = true` and continues, so a single
// unreadable directory does not abort the whole scan.
class Scanner {
public:
    explicit Scanner(ScanOptions options = {});

    // Scan `root`. The returned root Node's `name` is the full root path.
    // Returns nullptr only if `root` itself cannot be accessed.
    std::unique_ptr<Node> scan(const std::filesystem::path& root);

    // Request cancellation (thread-safe). An in-progress scan stops and returns
    // the partial tree built so far.
    void cancel() { cancelled_.store(true, std::memory_order_relaxed); }

    const ScanProgress& progress() const { return progress_; }

private:
    // Recurse into `dir` (located at `dir_path`); returns the subtree's bytes.
    std::uint64_t scan_dir(Node& dir, const std::filesystem::path& dir_path);

    ScanOptions options_;
    ScanProgress progress_;
    std::atomic<bool> cancelled_{false};
};

}  // namespace fstat
