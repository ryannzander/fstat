cmake_minimum_required(VERSION 3.20)

project(fstat
    VERSION 0.1.0
    DESCRIPTION "A fast, interactive disk-usage analyzer for the terminal"
    HOMEPAGE_URL "https://github.com/ryannzander/fstat"
    LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(FSTAT_BUILD_TESTS "Build unit tests" OFF)

# ---------------------------------------------------------------------------
# Global C++ settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to a Release build when the user didn't pick one (single-config gens).
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    message(STATUS "No build type set; defaulting to Release")
endif()

# Put all binaries in build/ root so `build/fstat` is predictable.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ---------------------------------------------------------------------------
# Shared warning flags (consumed via target_link_libraries)
# ---------------------------------------------------------------------------
add_library(fstat_warnings INTERFACE)
if(MSVC)
    target_compile_options(fstat_warnings INTERFACE /W4 /permissive-)
else()
    target_compile_options(fstat_warnings INTERFACE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
endif()

# ---------------------------------------------------------------------------
# Third-party dependencies
# ---------------------------------------------------------------------------
include(FetchContent)

# FTXUI: a modern C++ terminal UI library, used for the interactive browser.
# Pulled at configure time; the include dirs are marked SYSTEM so its headers
# don't trip our strict warning flags.
FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG        v5.0.0
    GIT_SHALLOW    TRUE
    SYSTEM
)
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(ftxui)

# ---------------------------------------------------------------------------
# Subdirectories
# ---------------------------------------------------------------------------
add_subdirectory(src)

if(FSTAT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
