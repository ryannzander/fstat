# Development log

A running, chronological narrative of how fstat is being built — decisions,
dead ends, and reasoning that doesn't belong in commit messages or the
changelog. Newest entries at the top.

---

## 2026-05-25 — Project kickoff (v0.1)

**Goal.** Build `fstat`, a disk-usage analyzer for the terminal, in modern C++,
targeting native Windows (MinGW-w64 GCC 13.2, CMake + Ninja).

**Decisions made today:**

- **Layered architecture.** A UI-free core (`core/`, `util/`) compiled as a
  static library `fstat_core`, with the executable on top. This keeps the
  scanner testable and lets us swap renderers (text vs. TUI). See
  [ARCHITECTURE.md](ARCHITECTURE.md).
- **`std::filesystem` for the walk.** Portable and good enough for v1.
  Error-tolerant by design: per-entry failures are recorded on the node, never
  thrown, so an unreadable folder (common in `C:\Windows`, `AppData`) doesn't
  kill the scan.
- **Symlinks/junctions are not followed by default.** Windows is full of
  reparse points; following them invites cycles and double-counting.
- **TUI via FTXUI**, added in a later milestone, pulled through CMake
  `FetchContent` so there's no manual dependency setup.

**Milestone plan:**

1. ✅ Scaffolding: build system, docs, license, tooling.
2. ⏳ Core scanner + `Node` model + formatting utils.
3. ⏳ `--print` text mode to validate the scanner end-to-end.
4. ⏳ Interactive TUI (navigation, size bars, sorting).
5. ⏳ Delete action + live scan progress.

**Open questions / deferred:**

- Parallel scanning (work-stealing pool) — deferred until the single-threaded
  version is correct and we have something to benchmark against.
- Hard-link de-duplication on Windows via file index — deferred.
