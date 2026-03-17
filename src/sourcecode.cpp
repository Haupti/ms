#include "sourcecode.hpp"
#include <unordered_map>
namespace {
static std::unordered_map<uint32_t, std::string> cache;
};

std::string find_sourcecode(Filename filename) {
  return cache.at(filename.idx);
}
void cache_sourcecode(Filename filename, const std::string &sourcecode) {
  cache[filename.idx] = sourcecode;
}
