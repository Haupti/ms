#include "location.hpp"
#include <filesystem>
#include <vector>

namespace {
std::vector<Location> *_location_cache() {
  static std::vector<Location> _cache;
  return &_cache;
}
}; // namespace

LocationRef create_location(const std::filesystem::path &filename, uint64_t row,
                            uint64_t col) {
  auto cache = _location_cache();
  for (uint64_t i = 0; i < cache->size(); i++) {
    auto l = cache->at(i);
    if (l.filename == filename && l.row == row && l.col == col) {
      return LocationRef{i};
    }
  }
  cache->push_back(Location{.filename = filename, .row = row, .col = col});
  return LocationRef{.index = cache->size() - 1};
}
Location resolve_location(const LocationRef &ref) {
  auto cache = _location_cache();
  return cache->at(ref.index);
}
