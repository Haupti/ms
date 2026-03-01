#pragma once

#include "../interned_string.hpp"
#include "../location.hpp"
#include "../symbol.hpp"
#include <cstdint>

enum class VMTag : uint8_t {
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
  DUP,
  TYPEOF,
  CALL,
  VMCALL,
  INIT_FRAME,
  DESTROY_FRAME,
  RETURN, // same as destroy frame but also jump to return address
  POP,
  ISTRUE,
  // check if value is true without poping it, then conditional jump
  ISTRUE_PEEK_JMPIF,
  ISTRUE_PEEK_JMPIFN,
  JMP,
  JMPIFN,
  JMPIF,
};

struct StkAddr {
  uint64_t addr;
};

struct VMInstr {
  LocationRef where;
  union {
    int64_t INT;
    double FLOAT;
    Symbol SYMBOL;
    InternedString STRING;
    StkAddr VAR;
    // the boolean indicates if wether its explicitly set as a value or
    // represents a undefined value
    // true -> explicit NONE value
    // false -> undefined
    bool NONE;

  } as;
  VMTag tag;
};
