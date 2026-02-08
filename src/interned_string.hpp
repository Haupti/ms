#pragma once
#include <cstdint>
#include <string>
struct InternedString {
  uint64_t index;
};
InternedString create_interned_string(const std::string &str);
std::string resolve_interned_string(const InternedString &interned_str);
