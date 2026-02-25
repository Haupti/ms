#pragma once
#include "../interned_string.hpp"
#include "value_and_heap.hpp"
#include <unordered_map>

struct Scope {
  std::unordered_map<uint64_t, Value> variables;
  Value return_value = Value();
  Scope *parent = nullptr;
  bool is_returning = false;
  bool has_var(InternedString varname) {
    if (variables.count(varname.index) > 0) {
      return true;
    }
    if (parent) {
      return parent->has_var(varname);
    }
    return false;
  }
  Value get_var(InternedString varname) {
    if (variables.count(varname.index) > 0) {
      return variables[varname.index];
    } else {
      return parent->get_var(varname);
    }
  }
  void set_var(InternedString varname, Value value) {
    if (variables.count(varname.index) > 0) {
      variables[varname.index] = value;
    } else {
      parent->set_var(varname, value);
    }
  }
};
