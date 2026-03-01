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
  PUSH_NONE,
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
  RETURN,
  DUP,
  TYPEOF,
  CALL,
  VMCALL,
  INIT_FRAME,
  POP,
  ISTRUE,
  ISTRUE_PEEK_JMPIF,
  ISTRUE_PEEK_JMPIFN,
  JMP,
  JMPIFN,
  JMPIF,
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
    Label LABEL;

    // the boolean indicates if wether its explicitly set as a value or
    // represents a undefined value
    // true -> explicit NONE value
    // false -> undefined
    bool NONE;

  } as;
  IRTag tag;
};
