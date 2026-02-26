#pragma once

#include <cstdint>
#include <string>

struct Label {
  uint64_t idx;
};

Label create_next_label(const std::string &prefix);
std::string resolve_label(const Label &label);
