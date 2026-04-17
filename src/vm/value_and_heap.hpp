#pragma once

#include "../../lib/asap/util.hpp"
#include "../interned_string.hpp"
#include "../symbol.hpp"
#include "vm_instr.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#define INVALID 0

typedef uint64_t VMHIDX;
typedef uint64_t StringIdx;
enum class ValueTag : uint8_t {
  INT,
  FLOAT,
  SYMBOL,
  STRING,
  LIST,
  ITERATOR,
  TABLE,
  BOX,
  ERROR,
  NONE,
  FN_REF,
};

struct Value {
  union {
    int64_t INT;
    double FLOAT;
    StringIdx STRING;
    Symbol SYMBOL;
    VMHIDX LIST;
    VMHIDX TABLE;
    VMHIDX ITERATOR;
    VMHIDX ERROR;
    VMHIDX BOX;
    VMHIDX FN_REF;
    int64_t NONE;
  } as;
  ValueTag tag;
  bool undefined;
  Value() : tag(ValueTag::NONE), undefined(true) {}
  static Value None() {
    Value val;
    val.as.NONE = 0;
    val.tag = ValueTag::NONE;
    val.undefined = false;
    return val;
  }
  static Value FunctionReference(VMHIDX heap_reference) {
    Value val;
    val.as.FN_REF = heap_reference;
    val.tag = ValueTag::FN_REF;
    val.undefined = false;
    return val;
  }
  static Value String(StringIdx value) {
    Value val;
    val.as.STRING = value;
    val.tag = ValueTag::STRING;
    val.undefined = false;
    return val;
  }
  static Value Int(int64_t value) {
    Value val;
    val.as.INT = value;
    val.tag = ValueTag::INT;
    val.undefined = false;
    return val;
  }
  static Value Float(double value) {
    Value val;
    val.as.FLOAT = value;
    val.tag = ValueTag::FLOAT;
    val.undefined = false;
    return val;
  }
  static Value FromSymbol(::Symbol value) {
    Value val;
    val.as.SYMBOL = value;
    val.tag = ValueTag::SYMBOL;
    val.undefined = false;
    return val;
  }
  static Value List(VMHIDX ref) {
    Value val;
    val.as.LIST = ref;
    val.tag = ValueTag::LIST;
    val.undefined = false;
    return val;
  }
  static Value Table(VMHIDX ref) {
    Value val;
    val.as.TABLE = ref;
    val.tag = ValueTag::TABLE;
    val.undefined = false;
    return val;
  }
  static Value Box(VMHIDX ref) {
    Value val;
    val.as.BOX = ref;
    val.tag = ValueTag::BOX;
    val.undefined = false;
    return val;
  }
  static Value Error(VMHIDX boxed_value) {
    Value val;
    val.as.ERROR = boxed_value;
    val.tag = ValueTag::ERROR;
    val.undefined = false;
    return val;
  }
  static Value Iterator(VMHIDX ref) {
    Value val;
    val.as.ITERATOR = ref;
    val.tag = ValueTag::ITERATOR;
    val.undefined = false;
    return val;
  }
};

enum class GCFlag : uint8_t {
  FREE,
  MARKED,
};

struct VMHNode {
  Value value;
  VMHIDX first_child;
  VMHIDX next_child;
  VMHIDX last_child;
  struct {
    InstrAddr addr;
    uint16_t args;
  } extra;
  VMHNode()
      : value(Value()), first_child(0), next_child(0), last_child(0),
        extra({.addr = InstrAddr({0}), .args = 0}), gc_flag(GCFlag::FREE) {}
  VMHNode(Value value)
      : value(value), first_child(0), next_child(0), last_child(0),
        extra({.addr = InstrAddr({0}), .args = 0}), gc_flag(GCFlag::FREE) {}
  VMHNode(Value value, InstrAddr addr, uint16_t args)
      : value(value), first_child(0), next_child(0), last_child(0),
        extra({.addr = addr, .args = args}), gc_flag(GCFlag::FREE) {}

  GCFlag gc_flag;
};

struct VMHeap {
  // private
  std::vector<VMHNode> elements;
  std::vector<VMHIDX> free_list;

  std::unordered_map<uint64_t, StringIdx> static_strings;
  std::unordered_map<StringIdx, uint64_t> reverse_static_strings;
  std::vector<std::string> strings;
  std::vector<StringIdx> free_strings;

  uint64_t current_length;
  uint64_t reserved_length;

  // public
  void reserve_factor(float factor) {
    elements.reserve(elements.capacity() * factor);
    strings.reserve(strings.capacity() * factor);
    this->reserved_length = elements.capacity() + strings.capacity() - 1;
  }
  VMHeap(uint64_t capacity, uint64_t string_capacity) {
    elements.reserve(capacity);
    add(Value()); // 0 slot must hold the reserved INVALID node
    strings.reserve(string_capacity);
    // -1 for the reserved INVALID node
    this->reserved_length = capacity + string_capacity - 1;
    this->current_length = 0;
  }

  VMHNode *node_at(VMHIDX idx) { return &elements.at(idx); }
  StringIdx _get_next_string_idx() {
    ++current_length;
    if (free_strings.size() > 0) {
      StringIdx idx = free_strings.back();
      free_strings.pop_back();
      return idx;
    }
    strings.push_back("");
    return strings.size() - 1;
  }

  Value add_string(InternedString str) {
    if (static_strings.count(str.index) > 0) {
      auto new_str = Value::String(static_strings[str.index]);
      add(new_str);
      return new_str;
    }
    StringIdx idx = _get_next_string_idx();
    strings[idx] = resolve_interned_string(str);
    static_strings[str.index] = idx;
    reverse_static_strings[idx] = str.index;
    auto new_str = Value::String(idx);
    add(new_str);
    return new_str;
  }

  Value add_string(std::string str) {
    StringIdx idx = _get_next_string_idx();
    strings[idx] = str;
    auto new_str = Value::String(idx);
    add(new_str);
    return new_str;
  }

  VMHIDX ref_add_string(std::string str) {
    StringIdx idx = _get_next_string_idx();
    strings[idx] = str;
    auto new_str = Value::String(idx);
    return add(new_str);
  }

  VMHIDX add(Value value) {
    ++current_length;
    if (free_list.size() == 0) {
      elements.emplace_back(value);
      return elements.size() - 1;
    }
    uint64_t idx = free_list.back();
    elements[idx] = VMHNode(value);
    free_list.pop_back();
    return idx;
  }

  Value at(VMHIDX idx) {
    if (elements.size() <= idx) {
      return Value();
    }
    return elements.at(idx).value;
  }

  VMHIDX nth_child_idx(VMHIDX container_head, uint64_t n) {
    VMHNode *head = node_at(container_head);
    if (head->value.tag != ValueTag::LIST &&
        head->value.tag != ValueTag::TABLE) {
      warn("attempt to get " + std::to_string(n) +
           "'th child of non-container");
      return INVALID;
    }
    VMHIDX curr = head->first_child;
    uint64_t i = 0;
    while (curr != INVALID && i != n) {
      curr = node_at(curr)->next_child;
      i++;
    }
    return curr;
  }
  Value nth_child(VMHIDX container_head, uint64_t n) {
    VMHIDX idx = nth_child_idx(container_head, n);
    if (idx == INVALID) {
      return Value();
    }
    return elements.at(idx).value;
  }

  inline VMHIDX next_child(VMHIDX list_element) {
    return node_at(list_element)->next_child;
  }

  VMHIDX replace(VMHIDX idx, Value value) {
    if (elements.size() <= idx) {
      warn("attempt to overwrite out of bounds index " + std::to_string(idx));
      return INVALID;
    }
    elements[idx].value = value;
    return idx;
  }

  std::string get_string(StringIdx idx) { return strings.at(idx); }

  // ========= TABLE
  VMHIDX new_table() {
    ++current_length;
    if (free_list.size() == 0) {
      VMHIDX idx = elements.size();
      elements.emplace_back(Value::Table(idx));
      return idx;
    }
    uint64_t idx = free_list.back();
    elements[idx] = VMHNode(Value::Table(idx));
    free_list.pop_back();
    return idx;
  }

  // ========= BOX
  Value new_box(Value value) {
    VMHIDX value_idx = add(value);
    return Value::Box(value_idx);
  }

  Value box_pack(VMHIDX boxed_value, Value new_value) {
    Value old_value = at(boxed_value);
    replace(boxed_value, new_value);
    return old_value;
  }

  Value box_unpack(VMHIDX boxed_value) { return at(boxed_value); }

  // ========= FUNCTION REFERENCES
  Value new_fn_ref(InstrAddr addr, uint16_t args) {
    ++current_length;
    if (free_list.size() == 0) {
      VMHIDX idx = elements.size();
      elements.emplace_back(Value::FunctionReference(idx), addr, args);
      return elements[idx].value;
    }
    uint64_t idx = free_list.back();
    elements[idx] = VMHNode(Value::FunctionReference(idx), addr, args);
    free_list.pop_back();
    return elements[idx].value;
  }
  void get_fn_ref(VMHIDX idx, InstrAddr *addr_out, uint16_t *args_out) {
    VMHNode *node = node_at(idx);
    *addr_out = node->extra.addr;
    *args_out = node->extra.args;
  }

  // ========= LISTS
  VMHIDX new_list() {
    ++current_length;
    if (free_list.size() == 0) {
      VMHIDX idx = elements.size();
      elements.emplace_back(Value::List(idx));
      return idx;
    }
    uint64_t idx = free_list.back();
    elements[idx] = VMHNode(Value::List(idx));
    free_list.pop_back();
    return idx;
  }

  void link_lists(VMHIDX left_list, VMHIDX right_list) {
    VMHNode *left = node_at(left_list);
    VMHNode right = elements.at(right_list);
    if (left->first_child == INVALID) {
      left->first_child = right.first_child;
      left->last_child = right.last_child;
      return;
    }
    VMHNode *left_last = node_at(left->last_child);
    left_last->next_child = right.first_child;
    left->last_child = right.last_child;
    return;
  }

  void set_nth_child(VMHIDX list_head, uint64_t n, Value value) {
    if (elements.size() <= list_head) {
      warn("attempt to access out of bounds element " +
           std::to_string(list_head));
      return;
    }
    if (elements.at(list_head).value.tag != ValueTag::LIST) {
      warn("attempt to add child to non-list value");
      return;
    }
    VMHIDX nth_child_ref = nth_child_idx(list_head, n);
    if (nth_child_ref == INVALID) {
      return;
    }
    elements.at(nth_child_ref).value = value;
  }

  VMHIDX add_child(VMHIDX container_head, Value value) {
    VMHIDX value_idx = add(value);
    return link_existing_child(container_head, value_idx);
  }

  VMHIDX link_existing_child(VMHIDX container_head, VMHIDX value_idx) {
    if (elements.size() <= container_head) {
      warn("attempt to access out of bounds element " +
           std::to_string(container_head));
      return INVALID;
    }
    if (elements.at(container_head).value.tag != ValueTag::LIST &&
        elements.at(container_head).value.tag != ValueTag::TABLE) {
      warn("attempt to add child to non-container value");
      return INVALID;
    }
    VMHNode *container_node = node_at(container_head);
    if (container_node->last_child == 0) {
      container_node->first_child = value_idx;
      container_node->last_child = value_idx;
    } else {
      elements.at(container_node->last_child).next_child = value_idx;
      container_node->last_child = value_idx;
    }
    return value_idx;
  }

  VMHIDX add_child_front(VMHIDX list_head, Value value) {
    VMHIDX value_idx = add(value);
    return link_existing_child_front(list_head, value_idx);
  }
  VMHIDX link_existing_child_front(VMHIDX container_head, VMHIDX value_idx) {
    if (elements.size() <= container_head) {
      warn("attempt to access out of bounds element " +
           std::to_string(container_head));
      return INVALID;
    }
    if (elements.at(container_head).value.tag != ValueTag::LIST &&
        elements.at(container_head).value.tag != ValueTag::TABLE) {
      warn("attempt to add child to non-container value");
      return INVALID;
    }
    VMHNode *list_node = &elements.at(container_head);
    if (list_node->first_child == 0) {
      list_node->first_child = value_idx;
      list_node->last_child = value_idx;
    } else {
      VMHIDX curr_first = list_node->first_child;
      list_node->first_child = value_idx;
      elements.at(list_node->first_child).next_child = curr_first;
    }
    return value_idx;
  }
};
