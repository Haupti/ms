
#include "symbol.hpp"
#include <limits>
#include <stdexcept>
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
      if (i > std::numeric_limits<uint16_t>::max()) {
        throw std::runtime_error("symbol cache size exceeded\n");
      }
      return Symbol{static_cast<uint16_t>(i)};
    }
  }
  cache->push_back(value);
  size_t pos = cache->size() - 1;
  if (pos > std::numeric_limits<uint16_t>::max()) {
    throw std::runtime_error("symbol cache size exceeded\n");
  }
  return Symbol{.index = static_cast<uint16_t>(pos)};
}

std::string resolve_symbol(const Symbol &symbol) {
  auto cache = _symbol_cache();
  return cache->at(symbol.index);
}
