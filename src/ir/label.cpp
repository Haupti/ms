#include "label.hpp"
#include <vector>

namespace {
static uint64_t _label_counter = 0;
std::vector<std::string> *_label_cache() {
  static std::vector<std::string> _cache;
  return &_cache;
}
}; // namespace
Label create_next_label(const std::string &prefix) {
  auto cache = _label_cache();
  // tilde prefix so it cannot accidently be the same as a function name or something else
  cache->push_back("~" + prefix + "_" + std::to_string(_label_counter++));
  return Label{.idx = cache->size() - 1};
}
std::string resolve_label(const Label &label) {
  auto cache = _label_cache();
  return cache->at(label.idx);
}
