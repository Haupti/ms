#pragma once

#include "../../lib/asap/util.hpp"
#include "../interned_string.hpp"
#include "../symbol.hpp"
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
  ERROR,
  NONE,
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
  static Value Symbol(Symbol value) {
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

  size_t hash() {}
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
  VMHNode()
      : first_child(0), next_child(0), last_child(0), table_size(0),
        gc_flag(GCFlag::FREE) {
    value = Value();
  }
  VMHNode(Value value)
      : value(value), first_child(0), next_child(0), last_child(0),
        table_size(0), gc_flag(GCFlag::FREE) {}
  uint32_t table_size;
  GCFlag gc_flag;
};

struct VMHeap {
  // private
  std::vector<VMHNode> elements;
  std::vector<VMHIDX> free_list;

  std::unordered_map<uint64_t, StringIdx> static_strings;
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

  // ============== GENERAL AND STRINGS ===============
  VMHNode *node_at(VMHIDX idx) { return &elements.at(idx); }
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

  VMHIDX replace(VMHIDX idx, Value value) {
    if (elements.size() <= idx) {
      warn("attempt to overwrite out of bounds index " + std::to_string(idx));
      return INVALID;
    }
    elements[idx].value = value;
    return idx;
  }

  // ============== STRINGS ===============
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

  std::string get_string(StringIdx idx) { return strings.at(idx); }

  // =============== TABLES ===============
  // TODO GC for table
  VMHIDX new_table() {
    ++current_length;
    VMHIDX key_bucket;
    if (free_list.size() == 0) {
      key_bucket = elements.size();
      elements.emplace_back(Value::Table(key_bucket));
    } else {
      key_bucket = free_list.back();
      elements[key_bucket] = VMHNode(Value::Table(key_bucket));
      free_list.pop_back();
    }

    elements[key_bucket].table_size = 16;
    for (uint64_t i = 0; i < 16; ++i) {
      add_child(key_bucket, Value::Int(0));
    }

    return key_bucket;
  }

private:
  size_t hash(Value key, uint32_t mod) {
    size_t hash;
    switch (key.tag) {
    case ValueTag::INT:
      hash = std::hash<int64_t>{}(key.as.INT);
      break;
    case ValueTag::FLOAT:
      hash = std::hash<double>{}(key.as.FLOAT);
      break;
    case ValueTag::SYMBOL:
      hash = std::hash<double>{}(key.as.SYMBOL.index);
      break;
    case ValueTag::STRING:
      hash = std::hash<std::string>{}(strings.at(key.as.STRING));
      break;
    case ValueTag::LIST:
      hash = std::hash<int64_t>{}(key.as.LIST);
      break;
    case ValueTag::ITERATOR:
      hash = std::hash<int64_t>{}(key.as.ITERATOR);
      break;
    case ValueTag::TABLE:
      hash = std::hash<int64_t>{}(key.as.TABLE);
      break;
    case ValueTag::ERROR:
      // TODO maybe hash the value inside ?
      hash = std::hash<int64_t>{}(key.as.ERROR);
      break;
    case ValueTag::NONE:
      hash = std::hash<int64_t>{}(0);
      break;
    }
    return hash & (mod - 1);
  }

public:
  void table_set(VMHIDX table_idx, Value key, Value val) {
    VMHNode *table = node_at(table_idx);

    size_t hash_value = hash(key, table->table_size);
    Value bucket_val = nth_child(table_idx, hash_value);
    VMHIDX val_idx = add(val);

    if (bucket_val.as.INT == 0) {
      // initialized bucket slot and add collision list
      bucket_val.as.INT = val_idx;
      set_nth_child(table_idx, hash_value, bucket_val);
    } else {
      // set next_child of collision lists tail
      VMHNode *slot = node_at(bucket_val.as.INT);
      while (slot->next_child != 0) {
        slot = node_at(slot->next_child);
      }
      slot->next_child = val_idx;
    }
    // TODO check if min 75% of bucket full
    // if so -> increase bucket size, rehash etc
  }

  VMHIDX table_get(VMHIDX table_idx, Value key){
    VMHNode *table = node_at(table_idx);
    size_t hash_value = hash(key, table->table_size);
    Value bucket_val = nth_child(table_idx, hash_value);

    VMHNode * slot = node_at(bucket_val.as.INT);
    // TODO unfinished
  }

  // =============== LISTS ===============
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

  VMHIDX add_child(VMHIDX list_head, Value value) {
    if (elements.size() <= list_head) {
      warn("attempt to access out of bounds element " +
           std::to_string(list_head));
      return INVALID;
    }
    if (elements.at(list_head).value.tag != ValueTag::LIST) {
      warn("attempt to add child to non-list value");
      return INVALID;
    }
    VMHIDX value_idx = add(value);
    // Re-fetch pointer as add() might have reallocated elements
    VMHNode *list_node = &elements.at(list_head);
    if (list_node->last_child == 0) {
      list_node->last_child = value_idx;
      list_node->first_child = value_idx;
    } else {
      elements.at(list_node->last_child).next_child = value_idx;
      list_node->last_child = value_idx;
    }
    return value_idx;
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
  VMHIDX add_child_front(VMHIDX list_head, Value value) {
    if (elements.size() <= list_head) {
      warn("attempt to access out of bounds element " +
           std::to_string(list_head));
      return INVALID;
    }
    if (elements.at(list_head).value.tag != ValueTag::LIST) {
      warn("attempt to add child to non-list value");
      return INVALID;
    }
    VMHIDX value_idx = add(value);
    // Re-fetch pointer as add() might have reallocated elements
    VMHNode *list_node = &elements.at(list_head);
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

  VMHIDX nth_child_idx(VMHIDX list_head, uint64_t n) {
    VMHNode *head = node_at(list_head);
    if (head->value.tag != ValueTag::LIST) {
      warn("attempt to get " + std::to_string(n) + "'th child of non-list");
      return INVALID;
    }
    VMHIDX curr = head->first_child;
    uint64_t i = 0;
    while (i != n) {
      curr = node_at(curr)->next_child;
      i++;
    }
    return curr;
  }
  Value nth_child(VMHIDX list_head, uint64_t n) {
    VMHIDX idx = nth_child_idx(list_head, n);
    return elements.at(idx).value;
  }
};
