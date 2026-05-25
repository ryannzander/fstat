#include "util/format.hpp"

#include <array>
#include <cstdio>

namespace fstat {

std::string human_size(std::uint64_t bytes) {
    static constexpr std::array<const char*, 7> kUnits = {
        "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB"};

    if (bytes < 1024) {
        return std::to_string(bytes) + " B";
    }

    double value = static_cast<double>(bytes);
    std::size_t unit = 0;
    while (value >= 1024.0 && unit + 1 < kUnits.size()) {
        value /= 1024.0;
        ++unit;
    }

    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.1f %s", value, kUnits[unit]);
    return std::string(buf);
}

std::string with_commas(std::uint64_t value) {
    std::string digits = std::to_string(value);
    std::string out;
    out.reserve(digits.size() + digits.size() / 3);

    const std::size_t len = digits.size();
    for (std::size_t i = 0; i < len; ++i) {
        if (i > 0 && (len - i) % 3 == 0) {
            out.push_back(',');
        }
        out.push_back(digits[i]);
    }
    return out;
}

}  // namespace fstat
