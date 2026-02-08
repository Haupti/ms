#pragma once

#include <cstdint>
#include <string>
struct LocationRef {
  uint64_t index;
};
struct Location {
  std::string filename;
  uint64_t row;
  uint64_t col;
};

LocationRef create_location(const std::string &filename, uint64_t row,
                            uint64_t col);
Location resolve_location(const LocationRef &ref);
