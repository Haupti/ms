#include "node.hpp"
#include <cassert>
#include <vector>
using namespace std;
namespace {
void mask_children(std::vector<bool> *mask, nodes *n, node_idx first_child) {
  if (first_child.is_null()) {
    return;
  }
  node_idx curr_idx = first_child;
  Node curr = n->at(curr_idx);
  do {
    mask->at(curr_idx.idx) = true;
    mask_children(mask, n, curr.first_child);
    curr_idx = curr.next_child;
    curr = n->at(curr.next_child);
  } while (!curr_idx.is_null());
}
} // namespace

std::vector<bool> create_mask(nodes *n) {
  std::vector<bool> mask = std::vector<bool>(n->elements.size(), false);
  mask.at(0) = true;
  if (n->first_elem == 0) {
    return mask;
  }
  node_idx curr_idx = node_idx{n->first_elem};
  Node curr = n->at(node_idx{n->first_elem});
  do {
    mask_children(&mask, n, curr.first_child);
    mask.at(curr_idx.idx) = true;
    curr_idx = curr.next_sibling;
    curr = n->at(curr_idx);
  } while (!curr_idx.is_null());
  return mask;
}
