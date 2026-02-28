#pragma once

#include "../interned_string.hpp"
#include "../location.hpp"
#include "../symbol.hpp"
#include "label.hpp"
#include <cstdint>

enum class IRTag : uint8_t {
  PUSH_INT,
  PUSH_FLOAT,
  PUSH_ALLOC_STRING,
  PUSH_SYMBOL,
  STORE,
  LOAD,
  NOT,
  INVERT,
  ADD,
  SUB,
  MUL,
  DIV,
  MOD,
  LT,
  LTE,
  GT,
  GTE,
  EQ,
  NEQ,
  STR_CONCAT,
  AND,
  OR,
  JMP,
  JMPIFN,
  LABEL,
};

struct IRInstr {
  LocationRef where;
  union {
    int64_t INT;
    double FLOAT;
    Symbol SYMBOL;
    InternedString STRING;
    InternedString VAR;
    bool NOTHING;
    Label LABEL;
  } as;
  IRTag tag;
};
