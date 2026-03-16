#pragma once
#include "value_and_heap.hpp"
#include "vm_instr.hpp"
#include <vector>

struct Stack {
  std::vector<Value> values;
  uint64_t stkptr;

  Stack(uint64_t init) : stkptr(0) { values.resize(init, Value()); }
  void push(const Value &value) { values[stkptr++] = value; }
  Value pop() {
    Value val = values[--stkptr];
    if (val.undefined) {
      panic("UNEXPECTED UNDEFINED VALUE (this is a bug in the VM)");
    }
    return val;
  }
  void pop_void() { --stkptr; }
  Value peek() {
    Value val = values[stkptr - 1];
    if (val.undefined) {
      panic("UNEXPECTED UNDEFINED VALUE (this is a bug in the VM)");
    }
    return val;
  }
  void reserve(uint16_t n) {
    for (uint64_t i = 0; i < n; ++i) {
      values[stkptr + i].undefined = false;
      values[stkptr + i].tag = ValueTag::NONE;
    }
    stkptr += n;
  }
  void write_local(uint64_t fptr, StkAddr addr, Value val) {
    values[fptr + addr.addr] = val;
  }
  void write_global(StkAddr addr, Value val) { values[addr.addr] = val; }
  Value load_global(StkAddr addr) { return values[addr.addr]; }
  Value load_local(uint64_t fptr, StkAddr addr) {
    return values[fptr + addr.addr];
  }
  void dup() {
    Value val = values[stkptr - 1];
    push(val);
  }
  void allocate(uint16_t locals) {
    for (uint64_t i = 0; i < locals; ++i) {
      values[stkptr + i].undefined = false;
      values[stkptr + i].tag = ValueTag::NONE;
    }
    stkptr += locals;
  }
  void retreat(uint64_t frame_start) {
    while (stkptr > frame_start) {
      values[--stkptr].undefined = true;
    }
  }
};
