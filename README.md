# fstat

**fstat** is a fast, interactive disk-usage analyzer for the terminal — think
[`ncdu`](https://dev.yorhel.nl/ncdu), but built from scratch in modern C++ and
designed to work well on Windows.

It recursively scans a directory, builds an in-memory size tree, and lets you
explore it interactively: see at a glance what is eating your disk, drill into
the biggest folders, and delete junk without leaving the terminal.

```
 fstat  C:\Users\rybot   total 84.2 GiB   312,944 files / 41,002 dirs
────────────────────────────────────────────────────────────────────────────
   52.1 GiB  ############################      /AppData
   18.4 GiB  #########                         /Desktop
    7.9 GiB  ####                              /Downloads
    3.1 GiB  #                                 /Documents
  ...
────────────────────────────────────────────────────────────────────────────
 ↑/↓ move   →/Enter open   ←/Backspace up   s sort   d delete   q quit
```

> **Status:** early development (v0.1). The scanning core, the interactive
> TUI, and a `--print` text mode all work today. Deleting from within the UI
> and live scan progress are next. See [docs/DEVLOG.md](docs/DEVLOG.md) for the
> running history.

## Features

- Recursive, error-tolerant directory scanning (a single unreadable folder
  never aborts the whole scan).
- Human-readable sizes (KiB / MiB / GiB …) and item counts.
- `--print` mode for scripting: dump the largest entries as plain text.
- Interactive TUI: drill into the tree, proportional size bars and
  percentages, sort by size or name, arrow-key or vim-style movement.

## Building

fstat uses CMake (≥ 3.20) and a C++20 compiler. It is developed with the
MinGW-w64 GCC toolchain on Windows, but the core is portable.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The binary is produced at `build/fstat` (`build/fstat.exe` on Windows).

See [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md) for a full setup guide.

## Usage

```sh
fstat                 # analyze the current directory (interactive TUI)
fstat C:\Users\rybot  # analyze a specific directory
fstat --print -n 30   # print the 30 largest entries and exit (no TUI)
fstat --help          # full option list
```

### Key bindings (TUI)

| Key                     | Action                          |
|-------------------------|---------------------------------|
| `↑` / `↓` (or `k` / `j`)| Move the selection              |
| `Enter` / `→` (or `l`)  | Open the selected directory     |
| `Backspace` / `←` (`h`) | Go up to the parent directory   |
| `g` / `G`               | Jump to top / bottom            |
| `s`                     | Toggle sort: size ↔ name        |
| `q` / `Esc`             | Quit                            |

## Project layout

```
fstat/
├── CMakeLists.txt        top-level build configuration
├── src/
│   ├── core/             filesystem scanner + size tree (no UI deps)
│   ├── util/             formatting helpers
│   ├── ui/               interactive TUI (FTXUI)
│   └── main.cpp          argument parsing + entry point
├── docs/                 architecture, development guide, dev log
└── tests/                unit tests
```

## License

MIT — see [LICENSE](LICENSE).
