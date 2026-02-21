#pragma once

#include "../interned_string.hpp"
#include "../location.hpp"
#include "../symbol.hpp"
#include <cassert>
#include <vector>

struct node_idx {
  uint64_t idx;
  bool is_null() { return idx == 0; }
};

enum class NodeTag : uint8_t {
  NIL,
  FN_CALL,
  FN_DEF,
  VAR_DEF,
  VAR_SET,
  VAR_REF,
  LITERAL_INT,
  LITERAL_STRING,
  LITERAL_FLOAT,
  LITERAL_SYMBOL,
  PREFIX_NOT,
  PREFIX_INVERS,
  INFIX_ADD,
  INFIX_SUB,
  INFIX_MUL,
  INFIX_DIV,
  INFIX_MOD,
  INFIX_AND,
  INFIX_OR,
  INFIX_LT,
  INFIX_LTE,
  INFIX_GT,
  INFIX_GTE,
  INFIX_EQ,
  INFIX_NEQ,
  INFIX_STR_CONCAT,
  IF,
  FUNCTION,
  RETURN,
  // INTERNAL_ prefixed tags are for internal structuring of nodes
  INTERNAL_PARTIAL_CONDITION,
  INTERNAL_PARTIAL_DEFAULT_CONDITION,
  INTERNAL_LIST,
  //...
};

struct Node {
  union {
    int64_t INT;
    double FLOAT;
    Symbol SYMBOL;
    InternedString STRING;
    InternedString IDENTIFIER;
  } as;
  LocationRef start = LocationRef{0};
  NodeTag tag;

  node_idx next_sibling;
  node_idx first_child;
  node_idx next_child;

  Node()
      : tag(NodeTag::NIL), next_sibling(node_idx{0}), first_child(node_idx{0}),
        next_child(node_idx{0}) {
    as.INT = 0;
    start = LocationRef{0};
  }
  Node(LocationRef start, NodeTag tag, uint64_t next_sibling,
       uint64_t first_child, uint64_t next_child)
      : start(start), tag(tag), next_sibling(node_idx{next_sibling}),
        first_child(node_idx{first_child}), next_child(node_idx{next_child}) {
    as.INT = 0;
  }
};

inline Node make_node(const LocationRef &start, NodeTag tag) {
  Node node;
  node.start = start;
  node.tag = tag;
  return node;
}
inline Node make_node(const LocationRef &start, NodeTag tag, int64_t value) {
  Node node;
  node.start = start;
  node.tag = tag;
  node.as.INT = value;
  return node;
}
inline Node make_node(const LocationRef &start, NodeTag tag, double value) {
  Node node;
  node.start = start;
  node.tag = tag;
  node.as.FLOAT = value;
  return node;
}
inline Node make_node_identifier(const LocationRef &start, NodeTag tag,
                                 InternedString value) {
  Node node;
  node.start = start;
  node.tag = tag;
  node.as.IDENTIFIER = value;
  return node;
}
inline Node make_node(const LocationRef &start, NodeTag tag,
                      InternedString value) {
  Node node;
  node.start = start;
  node.tag = tag;
  node.as.STRING = value;
  return node;
}
inline Node make_node(const LocationRef &start, NodeTag tag, Symbol value) {
  Node node;
  node.start = start;
  node.tag = tag;
  node.as.SYMBOL = value;
  return node;
}

struct nodes {
  std::vector<Node> elements;
  uint64_t last_sibling = 0;
  uint64_t first_elem = 0;

  nodes() {
    elements.reserve(10000);
    elements.push_back(Node());
  }
  Node at(node_idx idx) {
    if (elements.size() <= idx.idx) {
      return elements.at(0);
    }
    return elements.at(idx.idx);
  }
  node_idx next_child(node_idx idx) {
    if (elements.size() <= idx.idx) {
      return node_idx{0};
    }
    return elements.at(idx.idx).next_child;
  }
  node_idx nth_child(Node node, uint64_t n) {
    node_idx curr = node.first_child;
    uint64_t i = 0;
    while (i != n) {
      curr = elements.at(curr.idx).next_child;
      i++;
    };
    return curr;
  }
  Node nth_child_node(Node node, uint64_t n) {
    node_idx curr = node.first_child;
    uint64_t i = 0;
    while (i != n) {
      curr = elements.at(curr.idx).next_child;
      i++;
    };
    return elements.at(curr.idx);
  }
  node_idx next_sibling(node_idx idx) {
    if (elements.size() <= idx.idx) {
      return node_idx{0};
    }
    return elements.at(idx.idx).next_sibling;
  }
  node_idx first_child(node_idx idx) {
    if (elements.size() <= idx.idx) {
      return node_idx{0};
    }
    return elements.at(idx.idx).first_child;
  }
  node_idx add_sibling(Node elem) {
    if (last_sibling == 0) {
      elements.push_back(elem);
      last_sibling = 1;
      first_elem = 1;
      return node_idx{1};
    } else {
      elements.push_back(elem);
      uint64_t sib_pos = elements.size() - 1;
      elements.at(last_sibling).next_sibling = node_idx{sib_pos};
      last_sibling = sib_pos;
      return node_idx{sib_pos};
    }
  }
  node_idx add_sibling(node_idx elem) {
    assert(elem.idx != 0);
    if (last_sibling == 0) {
      last_sibling = elem.idx;
      first_elem = elem.idx;
    } else {
      elements.at(last_sibling).next_sibling = elem;
      last_sibling = elem.idx;
    }
    return elem;
  }
  node_idx add_child(node_idx parent, node_idx child) {
    assert(parent.idx != 0);
    assert(child.idx != 0);

    node_idx first_child = elements.at(parent.idx).first_child;
    if (first_child.idx == 0) {
      elements.at(parent.idx).first_child = child;
      return child;
    }

    node_idx curr_idx = first_child;
    Node curr = elements.at(first_child.idx);
    while (curr.next_child.idx != 0) {
      curr_idx = curr.next_child;
      curr = elements.at(curr.next_child.idx);
    }
    elements.at(curr_idx.idx).next_child = child;
    return child;
  }
  void add_next_child(node_idx first_child, node_idx child) {
    assert(first_child.idx != 0);
    assert(child.idx != 0);
    node_idx curr_idx = first_child;
    Node curr = elements.at(first_child.idx);
    while (curr.next_child.idx != 0) {
      curr_idx = curr.next_child;
      curr = elements.at(curr.next_child.idx);
    }
    elements.at(curr_idx.idx).next_child = child;
  }
  node_idx add_dangling(Node node) {
    elements.push_back(node);
    return node_idx{elements.size() - 1};
  }
};
