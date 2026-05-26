# Development log

A running, chronological narrative of how fstat is being built — decisions,
dead ends, and reasoning that doesn't belong in commit messages or the
changelog. Newest entries at the top.

---

## 2026-05-25 — Interactive TUI (milestone 2)

The `--print` core proved the scanner end-to-end, so this milestone added the
interactive browser in `src/ui/app.cpp`.

**Decisions made:**

- **FTXUI for the TUI**, pulled via CMake `FetchContent` (pinned to `v5.0.0`,
  `GIT_SHALLOW`, `SYSTEM` so its headers don't trip our `-Wconversion`/
  `-Wpedantic` flags). It builds cleanly under MinGW GCC 13.2 — the executable
  links the static `ftxui-component`/`-dom`/`-screen` libs, so there are no new
  runtime DLLs.
- **The core stays UI-free.** `ftxui` links only into the executable, not into
  `fstat_core`; the architecture rule (core has no UI deps) still holds.
- **Rendering approach.** Each child renders as `size | percent | gauge | name`.
  The selected row is `inverted | focus` inside a `frame`, which lets FTXUI
  auto-scroll the selection into view without us tracking a scroll window.
  Bars are colored by fraction-of-parent (green/yellow/red) as a heat hint.
- **Navigation state** is just a `current` node pointer plus an `index_stack`
  of per-level selections, so going up restores where you were.
- **Name clash gotcha:** FTXUI defines `ftxui::Node`, which collides with our
  `fstat::Node`. Resolved by keeping `using namespace ftxui;` *inside* `run()`
  only and referring to our type via a `FsNode` alias / `fstat::Node` — never a
  bare `Node` in that scope.

**Testing note.** The TUI needs a real terminal, so it can't be exercised in
the headless build harness; verified the binary builds, links, and loads, and
that `--print`/`--help`/`--version` still behave. Manual TUI testing is on the
user's terminal.

**Milestone status:** 1–4 done (scaffolding, core, text mode, TUI). Next:
in-UI delete with confirmation, and live progress during the scan.

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
