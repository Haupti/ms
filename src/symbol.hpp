#pragma once

#include <cstdint>
#include <string>
struct Symbol {
  uint64_t index;
};

Symbol create_symbol(const std::string &value);
