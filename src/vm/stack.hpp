#pragma once
#include "value_and_heap.hpp"
#include "vm_instr.hpp"
#include <vector>
struct Stack {
  std::vector<Value> values;
  uint64_t stkptr;

  Stack(uint64_t init) : stkptr(0) {
    values.reserve(init);

    for (uint64_t i = 0; i < init; ++i) {
      values.push_back(Value());
    }
  }
  void push(const Value &value) {
    values.at(stkptr) = value;
    ++stkptr;
  }
  Value pop() {
    Value val = values.at(--stkptr);
    if (val.undefined) {
      panic("UNEXPECTED UNDEFINED VALUE (this is a bug in the VM)");
    }
    return val;
  }
  void pop_void() { --stkptr; }
  Value peek() {
    Value val = values.at(stkptr - 1);
    if (val.undefined) {
      panic("UNEXPECTED UNDEFINED VALUE (this is a bug in the VM)");
    }
    return val;
  }
  void reserve(uint16_t n) {
    uint64_t end = stkptr + n;
    for (uint64_t i = stkptr; i < end; ++i) {
      values.at(i).undefined = false;
      values.at(i).tag = ValueTag::NONE;
    }
    stkptr += n;
  }
  void write_local(uint64_t fptr, StkAddr addr, Value val) {
    values.at(fptr + addr.addr) = val;
  }
  void write_global(StkAddr addr, Value val) { values.at(addr.addr) = val; }
  Value load_global(StkAddr addr) { return values.at(addr.addr); }
  Value load_local(uint64_t fptr, StkAddr addr) {
    return values.at(fptr + addr.addr);
  }
  void dup() {
    Value val = values.at(stkptr - 1);
    push(val);
  }
  void allocate(uint16_t locals) {
    uint64_t end = stkptr + locals;
    for (uint64_t i = stkptr; i < end; ++i) {
      values.at(i).undefined = false;
      values.at(i).tag = ValueTag::NONE;
    }
    stkptr += locals;
  }
  void retreat(uint64_t frame_start) {
    while (stkptr > frame_start) {
      --stkptr;
      values.at(stkptr).undefined = true;
    }
  }
};
