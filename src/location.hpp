#pragma once

#include "filename.hpp"
#include <cstdint>
#include <string>

struct LocationRef {
  uint32_t index;
};
struct Location {
  Filename filename;
  uint64_t offset;
};

LocationRef create_location(Filename filename, uint64_t offset);
std::string location_to_string(const LocationRef &ref);
