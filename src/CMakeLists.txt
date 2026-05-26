# ---------------------------------------------------------------------------
# fstat_core: filesystem scanning + size tree + formatting. No UI dependencies.
# ---------------------------------------------------------------------------
add_library(fstat_core STATIC
    core/node.cpp
    core/scanner.cpp
    util/format.cpp
)

target_include_directories(fstat_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fstat_core PRIVATE fstat_warnings)

# ---------------------------------------------------------------------------
# fstat: the command-line executable (core + FTXUI-based TUI).
# ---------------------------------------------------------------------------
add_executable(fstat
    main.cpp
    ui/app.cpp
)

target_link_libraries(fstat PRIVATE fstat_core fstat_warnings ftxui::component)
