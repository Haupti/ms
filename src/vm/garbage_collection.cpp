#include "garbage_collection.hpp"
#include <vector>

namespace {
void mark_heap_node(VMHeap *heap, std::vector<GCFlag> *string_marks, VMHIDX idx);

void mark_value(VMHeap *heap, std::vector<GCFlag> *string_marks, const Value &val) {
  switch (val.tag) {
  case ValueTag::STRING:
    string_marks->at(val.as.STRING) = GCFlag::MARKED;
    break;
  case ValueTag::LIST:
    mark_heap_node(heap, string_marks, val.as.LIST);
    break;
  case ValueTag::ITERATOR:
    mark_heap_node(heap, string_marks, val.as.ITERATOR);
    break;
  case ValueTag::ERROR:
    mark_heap_node(heap, string_marks, val.as.ERROR);
    break;
  default:
    break;
  }
}

void mark_iterable_elements(VMHeap *heap, std::vector<GCFlag> *string_marks,
                            VMHIDX first_child) {
  VMHIDX current_idx = first_child;
  while (current_idx != INVALID) {
    mark_heap_node(heap, string_marks, current_idx);
    current_idx = heap->node_at(current_idx)->next_child;
  }
}

void mark_heap_node(VMHeap *heap, std::vector<GCFlag> *string_marks, VMHIDX idx) {
  if (idx == INVALID) return;
  VMHNode *node = heap->node_at(idx);
  if (node->gc_flag == GCFlag::MARKED) return;
  
  node->gc_flag = GCFlag::MARKED;

  // Mark the value contained in this node
  mark_value(heap, string_marks, node->value);

  // If this is a list head or an element in a list, we might have structural links.
  // Actually, list elements are connected via next_child.
  // List heads point to the first child.
  if (node->value.tag == ValueTag::LIST) {
      mark_iterable_elements(heap, string_marks, node->first_child);
  }
  // next_child is handled by mark_iterable_elements of the parent list.
}

void free_values(VMHeap *heap) {
  heap->free_list.clear();
  // Skip index 0 (INVALID)
  for (uint64_t i = 1; i < heap->elements.size(); ++i) {
    if (heap->node_at(i)->gc_flag == GCFlag::FREE) {
      heap->free_list.push_back(i);
      *heap->node_at(i) = VMHNode();
    }
  }
}

void free_strings(VMHeap *heap, std::vector<GCFlag> *string_marks) {
  heap->free_strings.clear();
  for (uint64_t i = 0; i < heap->strings.size(); ++i) {
    if (string_marks->at(i) == GCFlag::FREE) {
      heap->free_strings.push_back(i);
      heap->strings[i].clear();
      heap->strings[i].shrink_to_fit();
    }
  }
}
} // namespace

void run_gc(VMHeap *heap, Stack *stack) {
  for (auto &element : heap->elements) {
    element.gc_flag = GCFlag::FREE;
  }
  if (heap->elements.size() > 0) {
    heap->elements[0].gc_flag = GCFlag::MARKED;
  }

  std::vector<GCFlag> string_marks(heap->strings.size(), GCFlag::FREE);

  for (uint64_t i = 0; i < stack->stkptr; ++i) {
    if (!stack->values[i].undefined) {
      mark_value(heap, &string_marks, stack->values[i]);
    }
  }

  free_values(heap);
  free_strings(heap, &string_marks);

  for (auto it = heap->static_strings.begin(); it != heap->static_strings.end();) {
    if (string_marks[it->second] == GCFlag::FREE) {
      it = heap->static_strings.erase(it);
    } else {
      ++it;
    }
  }
}
