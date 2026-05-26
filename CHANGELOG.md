# Changelog

All notable changes to fstat are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Project scaffolding: CMake build, MIT license, documentation, and developer
  tooling (`.clang-format`, `.editorconfig`).
- Core scanning library (`fstat_core`): `Node` size-tree model and a
  recursive, error-tolerant `Scanner` built on `std::filesystem`.
- Formatting utilities: human-readable byte sizes and thousands separators.
- CLI `--print` text mode showing the largest entries under a path.
- Interactive TUI (`ui::run`, built on FTXUI): a navigable directory tree with
  proportional size bars and percentages, size/name sorting, and arrow-key or
  vim-style movement. The TUI is now the default mode; `--print` selects the
  text report.
- FTXUI dependency pulled in via CMake `FetchContent` (pinned to v5.0.0).

[Unreleased]: https://github.com/ryannzander/fstat/commits/main
