#pragma once

#include <cstdint>
#include <filesystem>
struct LocationRef {
  uint64_t index;
};
struct Location {
  std::filesystem::path filename;
  uint64_t row;
  uint64_t col;
};

LocationRef create_location(const std::filesystem::path &filename, uint64_t row,
                            uint64_t col);
Location resolve_location(const LocationRef &ref);
std::string location_to_string(const LocationRef &ref);
