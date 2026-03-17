#include "location.hpp"
#include "../lib/asap/util.hpp"
#include "sourcecode.hpp"
#include <vector>

namespace {
std::vector<Location> *_location_cache() {
  static std::vector<Location> _cache;
  return &_cache;
}
Location resolve_location(const LocationRef &ref) {
  auto cache = _location_cache();
  return cache->at(ref.index);
}

void rowcol(Filename filename, uint64_t start, uint64_t *row_out,
            uint64_t *col_out) {
  std::string code = find_sourcecode(filename);
  uint64_t row = 1;
  uint64_t col = 0;
  for (uint64_t i = 0; i < start + 1; i++) {
    if (code.at(i) == '\n') {
      row++;
      col = 0;
    } else {
      col++;
    }
  }
  *row_out = row;
  *col_out = col;
}
}; // namespace

LocationRef create_location(Filename filename, uint64_t offset) {
  auto cache = _location_cache();
  for (uint32_t i = 0; i < cache->size(); i++) {
    auto l = cache->at(i);
    if (l.filename.idx == filename.idx && l.offset == offset) {
      return LocationRef{i};
    }
  }
  cache->push_back(Location{.filename = filename, .offset = offset});
  if (cache->size() - 1 > UINT32_MAX) {
    panic("location at index " + std::to_string(cache->size() - 1) +
          "out of limit");
  }
  return LocationRef{.index = static_cast<uint32_t>(cache->size() - 1)};
}
std::string location_to_string(const LocationRef &ref) {
  Location loc = resolve_location(ref);
  uint64_t row;
  uint64_t col;
  rowcol(loc.filename, loc.offset, &row, &col);
  return resolve_filename(loc.filename) + "/" + std::to_string(row) + "/" +
         std::to_string(col);
}
