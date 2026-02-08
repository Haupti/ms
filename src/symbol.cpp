
#include "symbol.hpp"
#include <string>
#include <vector>

namespace {
std::vector<std::string> *_symbol_cache() {
  static std::vector<std::string> _cache;
  return &_cache;
}
}; // namespace

Symbol create_symbol(const std::string &value) {
  auto cache = _symbol_cache();
  for (uint64_t i = 0; i < cache->size(); i++) {
    auto l = cache->at(i);
    if (value == l) {
      return Symbol{i};
    }
  }
  cache->push_back(value);
  return Symbol{.index = cache->size() - 1};
}
