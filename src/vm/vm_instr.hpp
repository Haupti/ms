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

struct InstrAddr {
  uint64_t addr;
};

struct VMInstr {
  union {
    int64_t INT;
    double FLOAT;
    Symbol SYMBOL;
    InternedString STRING;
    InstrAddr INSTRADDR;
    StkAddr STKADR;
    // the boolean indicates if wether its explicitly set as a value or
    // represents a undefined value
    // true -> explicit NONE value
    // false -> undefined
    bool NONE;
  } as;              // 64 bit
  LocationRef where; // 32 bit
  union {            //
    uint16_t args;   //
    uint16_t locals; //
  } extra;           // 16 bit
  uint8_t padding2;  // 8 bit
  VMTag tag;         // 8 bit
                     // total: 128 bit including padding
};
