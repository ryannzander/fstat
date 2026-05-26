#include "ui/app.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

#include "util/format.hpp"

namespace fstat::ui {
namespace {

using FsNode = fstat::Node;

enum class SortMode { Size, Name };

// Right-justify `s` in a field of `width` columns (no truncation).
std::string pad_left(const std::string& s, std::size_t width) {
    if (s.size() >= width) {
        return s;
    }
    return std::string(width - s.size(), ' ') + s;
}

// Format a fraction in [0,1] as a percentage string, e.g. 0.421 -> "42.1%".
std::string percent_str(double fraction) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%.1f%%", fraction * 100.0);
    return std::string(buf);
}

// Case-insensitive ascending comparison of node names.
bool name_less(const std::unique_ptr<FsNode>& a, const std::unique_ptr<FsNode>& b) {
    const std::string& x = a->name;
    const std::string& y = b->name;
    const std::size_t n = std::min(x.size(), y.size());
    for (std::size_t i = 0; i < n; ++i) {
        const int cx = std::tolower(static_cast<unsigned char>(x[i]));
        const int cy = std::tolower(static_cast<unsigned char>(y[i]));
        if (cx != cy) {
            return cx < cy;
        }
    }
    return x.size() < y.size();
}

void sort_children(FsNode* dir, SortMode mode) {
    auto& c = dir->children;
    if (mode == SortMode::Size) {
        std::sort(c.begin(), c.end(),
                  [](const auto& a, const auto& b) { return a->size > b->size; });
    } else {
        std::sort(c.begin(), c.end(), name_less);
    }
}

}  // namespace

int run(std::unique_ptr<fstat::Node> root) {
    using namespace ftxui;

    FsNode* current = root.get();
    int selected = 0;
    SortMode mode = SortMode::Size;
    std::vector<int> index_stack;  // remembers the selection at each parent level

    auto screen = ScreenInteractive::Fullscreen();

    // Build one list row for a child node: size | percent | bar | name.
    auto render_row = [](const FsNode& n) -> Element {
        const double frac = n.fraction_of_parent();
        const Color bar_color = frac >= 0.50   ? Color::Red
                                : frac >= 0.15 ? Color::Yellow
                                               : Color::Green;

        std::string label = n.name;
        if (n.is_dir) {
            label += "/";
        }
        if (n.had_error) {
            label += "  !";
        }

        Element name_el = text(label);
        if (n.had_error) {
            name_el = name_el | color(Color::RedLight);
        } else if (n.is_dir) {
            name_el = name_el | color(Color::CyanLight) | bold;
        }

        return hbox({
            text(pad_left(human_size(n.size), 11)),
            text("  "),
            text(pad_left(percent_str(frac), 6)) | dim,
            text(" "),
            gauge(static_cast<float>(frac)) | size(WIDTH, EQUAL, 16) | color(bar_color),
            text("  "),
            name_el | flex,
        });
    };

    auto renderer = Renderer([&] {
        const auto& items = current->children;

        Elements rows;
        if (items.empty()) {
            rows.push_back(text("  (empty)") | dim);
        } else {
            for (int i = 0; i < static_cast<int>(items.size()); ++i) {
                Element row = render_row(*items[i]);
                if (i == selected) {
                    row = row | inverted | focus;
                }
                rows.push_back(std::move(row));
            }
        }
        Element list = vbox(std::move(rows)) | vscroll_indicator | frame | flex;

        Element header = hbox({
            text(" fstat ") | bold | bgcolor(Color::Blue) | color(Color::White),
            text(" " + current->path().string() + " ") | bold,
            filler(),
            text(std::to_string(items.size()) + " items  ") | dim,
            text(human_size(current->size) + " ") | bold,
        });

        const std::string sort_name = (mode == SortMode::Size) ? "size" : "name";
        Element footer =
            hbox({
                text(" [Up/Down] move  [Enter/Right] open  [Left/Bksp] up  "),
                text("[s] sort:" + sort_name + "  "),
                text("[q] quit "),
            }) |
            dim;

        return vbox({header, separator(), list, separator(), footer});
    });

    auto app = CatchEvent(renderer, [&](const Event& e) {
        const auto& items = current->children;
        const int count = static_cast<int>(items.size());

        if (e == Event::Character('q') || e == Event::Escape) {
            screen.Exit();
            return true;
        }
        if (e == Event::ArrowDown || e == Event::Character('j')) {
            if (count > 0) {
                selected = std::min(selected + 1, count - 1);
            }
            return true;
        }
        if (e == Event::ArrowUp || e == Event::Character('k')) {
            selected = std::max(selected - 1, 0);
            return true;
        }
        if (e == Event::Home || e == Event::Character('g')) {
            selected = 0;
            return true;
        }
        if (e == Event::End || e == Event::Character('G')) {
            selected = std::max(count - 1, 0);
            return true;
        }
        if (e == Event::ArrowRight || e == Event::Return || e == Event::Character('l')) {
            if (count > 0 && items[static_cast<std::size_t>(selected)]->is_dir) {
                index_stack.push_back(selected);
                current = items[static_cast<std::size_t>(selected)].get();
                sort_children(current, mode);
                selected = 0;
            }
            return true;
        }
        if (e == Event::ArrowLeft || e == Event::Backspace || e == Event::Character('h')) {
            if (current->parent != nullptr) {
                current = current->parent;
                selected = index_stack.empty() ? 0 : index_stack.back();
                if (!index_stack.empty()) {
                    index_stack.pop_back();
                }
                const int m = static_cast<int>(current->children.size());
                selected = std::min(selected, std::max(m - 1, 0));
            }
            return true;
        }
        if (e == Event::Character('s')) {
            mode = (mode == SortMode::Size) ? SortMode::Name : SortMode::Size;
            sort_children(current, mode);
            selected = 0;
            return true;
        }
        return false;
    });

    screen.Loop(app);
    return 0;
}

}  // namespace fstat::ui
