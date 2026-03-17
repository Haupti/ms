#pragma once

#include <cstdint>
#include <string>

struct Filename {
  uint32_t idx;
};

std::string resolve_filename(Filename filename);
Filename get_filename(const std::string *const filename);
