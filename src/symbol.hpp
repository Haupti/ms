#pragma once

#include <cstdint>
#include <string>
struct Symbol {
  uint16_t index;
};

Symbol create_symbol(const std::string &value);
std::string resolve_symbol(const Symbol &symbol);
