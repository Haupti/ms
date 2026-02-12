#include "node.hpp"
#include <cassert>
#include <vector>
using namespace std;
namespace {
void mask_children(std::vector<bool> *mask, nodes *n, node_idx first_child) {
  mask->at(first_child.idx) = true;
  Node curr = n->at(first_child);
  if (!curr.first_child.is_null()) {
    mask_children(mask, n, curr.first_child);
  }
  while (!curr.next_child.is_null()) {
    if (!curr.first_child.is_null()) {
      mask_children(mask, n, curr.first_child);
    }
    mask->at(curr.next_child.idx) = true;
    curr = n->at(curr.next_child);
  }
}
} // namespace

std::vector<bool> maked_mask(nodes *n) {
  std::vector<bool> mask = std::vector<bool>(n->len(), false);
  if (n->first_elem == 0) {
    return mask;
  }
  mask.at(n->first_elem) = true;
  Node curr = n->at(n->first_elem);
  while (curr.next_sibling.idx != 0) {
    if (curr.first_child.idx != 0) {
      mask_children(&mask, n, curr.first_child);
    }
    mask.at(curr.next_sibling.idx) = true;
    curr = n->at(curr.next_sibling);
  }
  return mask;
}
