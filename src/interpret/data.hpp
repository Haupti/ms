#pragma once

#include "../../lib/asap/util.hpp"
#include "../interned_string.hpp"
#include "../symbol.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

enum class ValueTag : uint8_t {
  INT,
  STRING,
  FLOAT,
  SYMBOL,
  LIST,
  NONE,
};

struct HeapNodeIdx {
  uint64_t idx;
  bool is_null() { return idx == 0; }
  HeapNodeIdx(uint64_t idx) : idx(idx) {}
  HeapNodeIdx() {}
};

enum class Mark : uint8_t {
  STATIC,
  FREE,
  MARKED,
};

// lists:
// Value of type lists holds heap node of type list
// i.e. value list holds a 'pointer' to a heap list
// which itself can contain heap list 'pointers'
struct HeapNode {
  uint64_t next_sibling;
  uint64_t first_child;
  uint64_t next_child;
  union {
    int64_t INT;
    double FLOAT;
    Symbol SYMBOL;
    HeapNodeIdx LIST;
    std::string *STRING;
  } as;
  ValueTag tag;
};

struct Value {
  union {
    int64_t INT;
    HeapNodeIdx STRING;
    double FLOAT;
    Symbol SYMBOL;
    HeapNodeIdx LIST;
  } as;
  ValueTag tag;
  Mark mark;
  Value() : as({0}), tag(ValueTag::NONE) {}
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
  static Value None() {
    Value val;
    val.tag = ValueTag::NONE;
    return val;
  }
};

struct ListIterator {
  Value current;
  HeapNodeIdx next;
  bool has_next() { return !next.is_null(); }
};

struct Heap {
  std::vector<HeapNode> elements;
  HeapNodeIdx free_list = HeapNodeIdx(0);
  std::unordered_map<uint64_t, HeapNodeIdx> static_strings;

  Value at(HeapNodeIdx idx) { return to_stack(elements.at(idx.idx), idx); }
  HeapNodeIdx next_child(HeapNodeIdx idx) {
    return elements.at(idx.idx).next_child;
  }
  HeapNodeIdx first_child(HeapNodeIdx idx) {
    return elements.at(idx.idx).first_child;
  }
  HeapNodeIdx nth_child(HeapNodeIdx node, uint64_t n) {
    HeapNodeIdx curr = elements.at(node.idx).first_child;
    uint64_t i = 0;
    while (i != n) {
      curr = elements.at(curr.idx).next_child;
      i++;
    };
    return curr;
  }
  void replace(HeapNodeIdx idx, Value value) {
    HeapNode curr = elements.at(idx.idx);
    if (curr.tag == ValueTag::STRING) {
      delete curr.as.STRING;
    }
    HeapNode new_val = to_heap(value);
    new_val.next_sibling = curr.next_sibling;
    new_val.next_child = curr.next_child;
    new_val.first_child = curr.first_child;
    elements.at(idx.idx) = new_val;
  }
  HeapNodeIdx alloc(Value value) {
    if (value.tag == ValueTag::STRING) {
      return value.as.STRING;
    }
    if (free_list.is_null()) {
      elements.push_back(to_heap(value));
      return HeapNodeIdx(elements.size() - 1);
    } else {
      HeapNodeIdx current_free = free_list;
      free_list = elements.at(free_list.idx).next_sibling;
      elements[current_free.idx] = to_heap(value);
      return current_free;
    }
  }
  Value alloc_new_empty_list() {
    HeapNode node = {};
    node.as.LIST = HeapNodeIdx{0};
    node.tag = ValueTag::LIST;
    HeapNodeIdx list_idx = alloc_next_free(node);
    Value value = Value();
    value.as.LIST = list_idx;
    value.tag = ValueTag::LIST;
    return value;
  }
  void list_append(HeapNodeIdx list_idx, Value value) {
    HeapNode list_node = elements.at(list_idx.idx);
    HeapNode list_element_node = to_heap(value);
    HeapNodeIdx list_element_idx = alloc_next_free(list_element_node);
    if (list_node.first_child == 0) {
      elements.at(list_idx.idx).first_child = list_element_idx.idx;
      return;
    }
    uint64_t last_child = list_node.first_child;
    while (elements.at(last_child).next_child != 0) {
      last_child = elements.at(last_child).next_child;
    }
    elements.at(last_child).next_child = list_element_idx.idx;
  }
  ListIterator get_list_iterator(HeapNodeIdx first_elem) {
    ListIterator it;
    it.current = at(first_elem);
    HeapNode list_head = elements.at(first_elem.idx);
    it.next = list_head.first_child;
    return it;
  }
  void list_iterator_advance(ListIterator *it) {
    it->current = at(it->next);
    HeapNodeIdx next_elem = next_child(it->next);
    it->next = next_elem;
  }
  Value alloc_new_string(const std::string &str) {
    HeapNode node = {};
    node.as.STRING = new std::string(str);
    node.tag = ValueTag::STRING;
    HeapNodeIdx new_string_idx = alloc_next_free(node);
    Value value;
    value.as.STRING = new_string_idx;
    value.tag = ValueTag::STRING;
    return value;
  }
  Value alloc_new_string(InternedString str) {
    if (static_strings.count(str.index) > 0) {
      Value value;
      value.as.STRING = static_strings.at(str.index);
      value.tag = ValueTag::STRING;
      return value;
    }
    HeapNode node = {};
    node.as.STRING = new std::string(resolve_interned_string(str));
    node.tag = ValueTag::STRING;
    HeapNodeIdx new_string_idx = alloc_next_free(node);

    Value value;
    value.as.STRING = new_string_idx;
    value.tag = ValueTag::STRING;
    static_strings[str.index] = new_string_idx;
    return value;
  }
  std::string heap_string(HeapNodeIdx idx) {
    std::string aaa = *elements.at(idx.idx).as.STRING;
    return aaa;
  }
  Heap() { elements.reserve(10000); }

private:
  HeapNodeIdx alloc_next_free(HeapNode node) {
    if (free_list.is_null()) {
      elements.push_back(node);
      return HeapNodeIdx(elements.size() - 1);
    }
    HeapNodeIdx current_free = free_list;
    free_list = elements.at(free_list.idx).next_sibling;
    elements[current_free.idx] = node;
    return current_free;
  }
  HeapNode to_heap(const Value &value) {
    HeapNode node = {};
    node.tag = value.tag;
    switch (value.tag) {
    case ValueTag::INT:
      node.as.INT = value.as.INT;
      break;
    case ValueTag::STRING: {
      HeapNode original = elements.at(value.as.STRING.idx);
      HeapNode new_string = {};
      new_string.as.STRING = new std::string(*original.as.STRING);
      new_string.tag = ValueTag::STRING;
      new_string.next_sibling = original.next_sibling;
      new_string.first_child = original.first_child;
      new_string.next_child = original.next_child;
      return new_string;
    } break;
    case ValueTag::FLOAT:
      node.as.FLOAT = value.as.FLOAT;
      break;
    case ValueTag::SYMBOL:
      node.as.SYMBOL = value.as.SYMBOL;
      break;
    case ValueTag::LIST:
      node.as.LIST = value.as.LIST;
      break;
    case ValueTag::NONE:
      break;
    }
    return node;
  }
  Value to_stack(const HeapNode &node, HeapNodeIdx node_idx) {
    Value val;
    val.tag = node.tag;
    switch (val.tag) {
    case ValueTag::INT:
      val.as.INT = node.as.INT;
      break;
    case ValueTag::STRING:
      val.as.STRING = node_idx;
      break;
    case ValueTag::FLOAT:
      val.as.FLOAT = node.as.FLOAT;
      break;
    case ValueTag::SYMBOL:
      val.as.SYMBOL = node.as.SYMBOL;
      break;
    case ValueTag::LIST:
      val.as.LIST = node.as.LIST;
      break;
    case ValueTag::NONE:
      break;
    }
    return val;
  }
};
