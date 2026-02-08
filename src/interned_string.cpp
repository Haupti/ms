#include "interned_string.hpp"
#include <vector>

namespace {
std::vector<std::string> *_interned_string_cache() {
  static std::vector<std::string> _cache;
  return &_cache;
}
}; // namespace
InternedString create_interned_string(const std::string &str) {
  auto cache = _interned_string_cache();
  for (uint64_t i = 0; i < cache->size(); i++) {
    auto l = cache->at(i);
    if (l == str) {
      return InternedString{i};
    }
  }
  cache->push_back(str);
  return InternedString{.index = cache->size() - 1};
}
std::string resolve_interned_string(const InternedString &interned_str) {
  auto cache = _interned_string_cache();
  return cache->at(interned_str.index);
}
