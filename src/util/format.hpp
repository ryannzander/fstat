#pragma once

#include <cstdint>
#include <string>

namespace fstat {

// Format a byte count using IEC binary units, e.g. 1536 -> "1.5 KiB".
// Values below 1 KiB are shown as whole bytes; larger values use one decimal.
std::string human_size(std::uint64_t bytes);

// Format an integer with thousands separators, e.g. 1234567 -> "1,234,567".
std::string with_commas(std::uint64_t value);

}  // namespace fstat
