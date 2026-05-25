# Architecture

This document describes how fstat is structured and why. It is meant to be read
top-to-bottom by a new contributor.

## High-level data flow

```
   argv ──▶ main.cpp ──▶ Scanner.scan(path) ──▶ Node tree ──▶ output
                              │                                  │
                              │                                  ├─ TextReport (--print)
                              ▼                                  └─ TUI (ui::App)
                        std::filesystem
```

1. **`main.cpp`** parses arguments into an `Options` struct and chooses an
   output mode.
2. **`Scanner`** walks the filesystem recursively and builds a tree of
   **`Node`** objects, accumulating sizes and counts as it unwinds.
3. The resulting tree is handed to a renderer — either the text reporter or the
   interactive TUI. Neither renderer mutates the tree's topology (the TUI may
   re-sort children and prune deleted nodes).

The key design rule: **the core (`src/core`, `src/util`) has no UI
dependencies.** It can be compiled, tested, and reused without pulling in the
terminal layer. This keeps the scanning logic fast to test and the dependency
graph clean.

## Modules

### `core/Node` — the size tree

A `Node` is one filesystem entry (file or directory). Directories own their
children via `std::vector<std::unique_ptr<Node>>` and hold a non-owning
`parent` back-pointer. Each node caches:

- `size` — aggregate bytes of the subtree (own size for a file),
- `file_count` / `dir_count` — totals for the subtree,
- flags: `is_dir`, `is_link`, `had_error`.

Aggregates are computed once during the scan, so rendering and sorting never
re-walk the filesystem.

### `core/Scanner` — the walk

`Scanner::scan()` performs a depth-first traversal using
`std::filesystem::directory_iterator`. Design decisions:

- **Error tolerance.** Per-entry errors (access denied, vanished files) are
  captured into `std::error_code` and recorded on the node
  (`had_error = true`) instead of throwing. One bad directory never aborts the
  scan.
- **Symlinks / reparse points** are recorded but *not* followed by default,
  which avoids cycles and double-counting. This matters on Windows where
  junctions and `AppData` reparse points are common.
- **Cancellation.** An `std::atomic<bool>` flag lets a future UI thread stop a
  long scan and keep the partial tree.

### `util/format` — presentation helpers

Pure functions with no state: `human_size()` (IEC binary units) and
`with_commas()`. Kept separate so both the text and TUI renderers share them.

### `ui/App` — the TUI (in progress)

Built on [FTXUI](https://github.com/ArthurSonzogni/FTXUI), pulled in via CMake
`FetchContent`. The UI holds a pointer to the current directory node and renders
its children as a sortable, navigable list with proportional size bars.

## Planned work / known limitations

- **Parallel scanning.** The current scanner is single-threaded. The plan is a
  work-stealing pool that fans out on subdirectories; `Node` aggregation already
  unwinds bottom-up, which makes this a localized change in `Scanner`.
- **Hard links** are currently counted once per path, so hard-linked files can
  inflate totals. ncdu de-duplicates by `(device, inode)`; the Windows analogue
  uses `GetFileInformationByHandle`'s file index.
- **Allocated vs. apparent size.** We report apparent file size today; cluster
  (on-disk) size is a planned option.
