#pragma once

#include "../../lib/asap/util.hpp"
#include "../interned_string.hpp"
#include "../symbol.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#define INVALID 0

typedef uint64_t ILHIDX;
typedef uint64_t StringIdx;
enum class ValueTag : uint8_t {
  INT,
  FLOAT,
  SYMBOL,
  STRING,
  LIST,
  ERROR,
  NONE,
};

struct Value {
  union {
    int64_t INT;
    double FLOAT;
    StringIdx STRING;
    Symbol SYMBOL;
    ILHIDX LIST;
    ILHIDX ERROR;
    int64_t NONE;
  } as;
  ValueTag tag;
  Value() : tag(ValueTag::NONE) {}
  static Value String(StringIdx value) {
    Value val;
    val.as.STRING = value;
    val.tag = ValueTag::STRING;
    return val;
  }
  static Value Int(int64_t value) {
    Value val;
    val.as.INT = value;
    val.tag = ValueTag::INT;
    return val;
  }
  static Value Float(double value) {
    Value val;
    val.as.FLOAT = value;
    val.tag = ValueTag::FLOAT;
    return val;
  }
  static Value Symbol(Symbol value) {
    Value val;
    val.as.SYMBOL = value;
    val.tag = ValueTag::SYMBOL;
    return val;
  }
  static Value EmptyList() {
    Value val;
    val.as.LIST = 0;
    val.tag = ValueTag::LIST;
    return val;
  }
  static Value Error(ILHIDX boxed_value) {
    Value val;
    val.as.ERROR = boxed_value;
    val.tag = ValueTag::ERROR;
    return val;
  }
};

struct ILHNode {
  Value value;
  ILHIDX first_child;
  ILHIDX next_child;
  ILHIDX last_child;
  ILHNode() {
    first_child = 0;
    last_child = 0;
    next_child = 0;
  }
  ILHNode(Value value) {
    ILHNode node;
    node.value = value;
    first_child = 0;
    last_child = 0;
    next_child = 0;
  }
};

struct ILHeap {
private:
  std::vector<ILHNode> elements;
  std::vector<ILHIDX> free_list;

  std::unordered_map<uint64_t, StringIdx> static_strings;
  std::vector<std::string> strings;
  std::vector<StringIdx> free_strings;

  ILHNode *node_at(ILHIDX idx) { return &elements.at(idx); }

public:
  ILHeap(uint64_t capacity, uint64_t string_capacity) {
    elements.reserve(capacity);
    strings.reserve(string_capacity);
  }
  ILHIDX add_string(InternedString str) {
    if (static_strings.count(str.index) > 0) {
      return add(Value::String(static_strings[str.index]));
    }
    strings.push_back(resolve_interned_string(str));
    static_strings[str.index] = strings.size() - 1;
    return add(Value::String(strings.size() - 1));
  }

  ILHIDX add_string(std::string str) {
    strings.push_back(str);
    return add(Value::String(strings.size() - 1));
  }

  ILHIDX add(Value value) {
    if (free_list.size() == 0) {
      elements.emplace_back();
      elements.back().value = value;
      return elements.size() - 1;
    }
    uint64_t idx = free_list.back();
    elements[idx] = ILHNode(value);
    free_list.pop_back();
    return idx;
  }

  ILHIDX add_child(ILHIDX list_head, Value value) {
    if (elements.size() <= list_head) {
      warn("attempt to access out of bounds element " +
           std::to_string(list_head));
      return INVALID;
    }
    ILHNode *list_node = &elements.at(list_head);
    if (list_node->value.tag != ValueTag::LIST) {
      warn("attempt to add child to non-list value");
      return INVALID;
    }
    ILHIDX value_idx = add(value);
    if (list_node->last_child == 0) {
      list_node->last_child = value_idx;
      list_node->first_child = value_idx;
    } else {
      elements.at(list_node->last_child).next_child = value_idx;
      list_node->last_child = value_idx;
    }
    return value_idx;
  }
  ILHIDX add_child_front(ILHIDX list_head, Value value) {
    if (elements.size() <= list_head) {
      warn("attempt to access out of bounds element " +
           std::to_string(list_head));
      return INVALID;
    }
    ILHNode *list_node = &elements.at(list_head);
    if (list_node->value.tag != ValueTag::LIST) {
      warn("attempt to add child to non-list value");
      return INVALID;
    }
    ILHIDX value_idx = add(value);
    if (list_node->first_child == 0) {
      list_node->first_child = value_idx;
      list_node->last_child = value_idx;
    } else {
      ILHIDX curr_first = list_node->first_child;
      list_node->first_child = value_idx;
      elements.at(list_node->first_child).next_child = curr_first;
    }
    return value_idx;
  }

  Value at(ILHIDX idx) {
    if (elements.size() <= idx) {
      return Value();
    }
    return elements.at(idx).value;
  }

  ILHIDX nth_child_idx(ILHIDX list_head, uint64_t n) {
    ILHNode *head = node_at(list_head);
    if (head->value.tag != ValueTag::LIST) {
      warn("attempt to get " + std::to_string(n) + "'th child of non-list");
      return INVALID;
    }
    ILHIDX curr = head->first_child;
    uint64_t i = 0;
    while (i != n) {
      curr = node_at(curr)->next_child;
      i++;
    }
    return curr;
  }
  Value nth_child(ILHIDX list_head, int64_t n) {
    ILHIDX idx = nth_child_idx(list_head, n);
    return elements.at(idx).value;
  }

  ILHIDX replace(ILHIDX idx, Value value) {
    if (elements.size() <= idx) {
      warn("attempt to overwrite out of bounds index " + std::to_string(idx));
      return INVALID;
    }
    elements[idx].value = value;
    return idx;
  }

  std::string get_string(StringIdx idx) { return strings.at(idx); }
};
