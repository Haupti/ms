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
  STORE_NEW,
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
  RETURN,
  DUP,
  TYPEOF,
  CALL,
  VMCALL,
  FUNCTION_START,
  FUNCTION_END,
  SCOPE_START,
  SCOPE_END,
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
  } as;              // 64 bit
  LocationRef where; // 32 bit
  union {            // extra information for e.g. vmfncall or frame_init
    uint16_t args;   //
    uint16_t locals; //
  } extra;           // 16 bit
  IRTag tag;         // 8 bit
                     // 8 bit padding
};
