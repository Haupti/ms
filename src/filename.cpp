#include "filename.hpp"
#include "../lib/asap/util.hpp"
#include <string>
#include <vector>
namespace {
std::vector<std::string> *_filename_cache() {
  static std::vector<std::string> _cache;
  return &_cache;
}
} // namespace
std::string resolve_filename(Filename filename) {
  return _filename_cache()->at(filename.idx);
}
Filename get_filename(const std::string *const filename) {
  auto cache = _filename_cache();
  for (uint32_t i = 0; i < cache->size(); i++) {
    auto l = cache->at(i);
    if (l == *filename) {
      return Filename{i};
    }
  }
  cache->push_back(*filename);
  if (cache->size() - 1 > UINT32_MAX) {
    panic("filename at index " + std::to_string(cache->size() - 1) +
          "out of limit");
  }
  return Filename{.idx = static_cast<uint32_t>(cache->size() - 1)};
}
