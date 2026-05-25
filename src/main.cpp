#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

#include "core/scanner.hpp"
#include "util/format.hpp"

namespace {

constexpr const char* kVersion = "0.1.0";

void print_usage(std::ostream& os) {
    os << "fstat " << kVersion << " - interactive disk-usage analyzer\n\n"
       << "USAGE:\n"
       << "    fstat [OPTIONS] [PATH]\n\n"
       << "ARGS:\n"
       << "    PATH             Directory to analyze (default: current directory)\n\n"
       << "OPTIONS:\n"
       << "    -p, --print      Print the largest entries as text and exit\n"
       << "    -n, --top <N>    With --print, show the top N entries (default: 20)\n"
       << "    -h, --help       Print this help and exit\n"
       << "    -V, --version    Print version and exit\n";
}

struct Options {
    std::string path = ".";
    std::size_t top = 20;
    // v0.1 only has the text report; the interactive TUI lands in the next
    // milestone and will become the default when no --print is given.
    bool print_mode = true;
};

// Render the top-N largest direct children of `root` to stdout.
void print_report(const fstat::Node& root, const fstat::ScanProgress& progress,
                  std::size_t top) {
    std::cout << '\n' << root.name << '\n';
    std::cout << "  total " << fstat::human_size(root.size) << "  ("
              << fstat::with_commas(progress.files_seen) << " files, "
              << fstat::with_commas(progress.dirs_seen) << " dirs";
    if (progress.errors > 0) {
        std::cout << ", " << fstat::with_commas(progress.errors) << " unreadable";
    }
    std::cout << ")\n\n";

    std::size_t shown = 0;
    for (const std::unique_ptr<fstat::Node>& child : root.children) {
        if (shown >= top) {
            break;
        }
        ++shown;
        std::cout << "  " << std::setw(12) << std::right
                  << fstat::human_size(child->size) << "  " << child->name
                  << (child->is_dir ? "/" : "")
                  << (child->had_error ? "  (partial: unreadable)" : "") << '\n';
    }

    if (root.children.size() > shown) {
        std::cout << "  ... and "
                  << fstat::with_commas(root.children.size() - shown)
                  << " more (use --top to show more)\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
    Options opt;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_usage(std::cout);
            return 0;
        } else if (arg == "-V" || arg == "--version") {
            std::cout << "fstat " << kVersion << '\n';
            return 0;
        } else if (arg == "-p" || arg == "--print") {
            opt.print_mode = true;
        } else if (arg == "-n" || arg == "--top") {
            if (i + 1 >= argc) {
                std::cerr << "fstat: " << arg << " requires a value\n";
                return 2;
            }
            opt.top = static_cast<std::size_t>(std::stoul(argv[++i]));
        } else if (!arg.empty() && arg.front() == '-') {
            std::cerr << "fstat: unknown option '" << arg << "'\n\n";
            print_usage(std::cerr);
            return 2;
        } else {
            opt.path = arg;
        }
    }

    fstat::Scanner scanner;
    std::cerr << "Scanning " << opt.path << " ..." << std::endl;

    std::unique_ptr<fstat::Node> root = scanner.scan(opt.path);
    if (!root) {
        std::cerr << "fstat: cannot access '" << opt.path << "'\n";
        return 1;
    }

    root->sort_by_size();
    print_report(*root, scanner.progress(), opt.top);
    return 0;
}
