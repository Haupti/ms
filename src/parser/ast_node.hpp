#pragma once

#include "../interned_string.hpp"
#include "../location.hpp"
#include "../symbol.hpp"
#include <vector>

enum class ASTNodeTag : uint8_t {
  NIL,
  FN_CALL,
  FN_DEF,
  VAR_DEF,
  VAR_SET,
  LITERAL_INT,
  LITERAL_STRING,
  LITERAL_FLOAT,
  LITERAL_SYMBOL,
  //...
};

struct node_idx {
  uint64_t idx;
};

struct ASTNode {
  union {
    int64_t INT;
    double FLOAT;
    Symbol SYMBOL;
    InternedString STRING;
    InternedString IDENTIFIER;
  } as;
  LocationRef start = LocationRef{0};
  node_idx next_sibling = node_idx{0};
  node_idx first_child = node_idx{0};
  node_idx next_child = node_idx{0};
  node_idx parent = node_idx{0};
  ASTNodeTag tag = ASTNodeTag::NIL;
};
inline ASTNode make_node(const LocationRef &start, ASTNodeTag tag) {
  ASTNode node;
  node.start = start;
  node.tag = tag;
  return node;
}
inline ASTNode make_node(const LocationRef &start, ASTNodeTag tag,
                         int64_t value) {
  ASTNode node;
  node.start = start;
  node.tag = tag;
  node.as.INT = value;
  return node;
}
inline ASTNode make_node(const LocationRef &start, ASTNodeTag tag,
                         double value) {
  ASTNode node;
  node.start = start;
  node.tag = tag;
  node.as.FLOAT = value;
  return node;
}
inline ASTNode make_node_identifier(const LocationRef &start, ASTNodeTag tag,
                                    InternedString value) {
  ASTNode node;
  node.start = start;
  node.tag = tag;
  node.as.IDENTIFIER = value;
  return node;
}
inline ASTNode make_node(const LocationRef &start, ASTNodeTag tag,
                         InternedString value) {
  ASTNode node;
  node.start = start;
  node.tag = tag;
  node.as.STRING = value;
  return node;
}
inline ASTNode make_node(const LocationRef &start, ASTNodeTag tag,
                         Symbol value) {
  ASTNode node;
  node.start = start;
  node.tag = tag;
  node.as.SYMBOL = value;
  return node;
}

struct nodes {
  std::vector<ASTNode> _nodes;
  ASTNode get(node_idx idx) { return _nodes.at(idx.idx); }
  node_idx add(ASTNode node) {
    _nodes.push_back(node);
    return node_idx{_nodes.size() - 1};
  }
};
