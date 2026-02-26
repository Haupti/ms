#pragma once

#include "../interned_string.hpp"
#include "../symbol.hpp"
#include "../location.hpp"
#include "label.hpp"
#include <cstdint>

enum class IRTag : uint8_t {
  PUSH_INT,
  PUSH_FLOAT,
  PUSH_ALLOC_STRING,
  PUSH_SYMBOL,
  LABEL,
};

struct IRInstr {
  LocationRef where;
  union {
    int64_t INT;
    double FLOAT;
    Symbol SYMBOL;
    InternedString STRING;
    bool NOTHING;
    Label LABEL;
  } as;
  IRTag tag;
};
