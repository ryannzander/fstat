# Development guide

## Prerequisites

| Tool   | Minimum | Notes                                   |
|--------|---------|-----------------------------------------|
| CMake  | 3.20    | Build configuration                     |
| Ninja  | 1.10    | Recommended generator (fast, parallel)  |
| C++    | C++20   | GCC 11+, Clang 13+, or MSVC 19.3+       |
| Git    | any     | `FetchContent` clones the TUI dependency|

The reference toolchain on Windows is **MinGW-w64 GCC** (the project is
developed with GCC 13.2, UCRT runtime). Make sure `C:\MinGW\bin` (or your
install path) is on `PATH` so CMake can find `g++`, `ninja`, and `cmake`.

## Configure & build

```sh
# Configure once (Release):
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build (re-run after any change):
cmake --build build

# Debug build in a separate directory:
cmake -S . -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

The executable lands at `build/fstat.exe`.

## Running

```sh
./build/fstat --help
./build/fstat --print -n 20 .
```

## Code style

- C++20, 4-space indent, 100-column lines. Run `clang-format` (config in
  `.clang-format`) before committing:
  ```sh
  clang-format -i src/**/*.cpp src/**/*.hpp
  ```
- Headers use `#pragma once`.
- Includes are grouped: the matching header first, then standard library, then
  third-party, then project headers.
- The `core/` and `util/` layers must not include anything from `ui/` or any
  TUI dependency.

## Tests

Unit tests live in `tests/` and are enabled with `-DFSTAT_BUILD_TESTS=ON`.
They cover the formatting helpers and scanner behavior against a temporary
directory tree.

```sh
cmake -S . -B build -G Ninja -DFSTAT_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Commit conventions

Short, imperative subject lines (e.g. "Add proportional size bars to the TUI").
Keep each commit focused; update `CHANGELOG.md` and, for noteworthy sessions,
add an entry to `docs/DEVLOG.md`.
