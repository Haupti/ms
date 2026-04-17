#include "garbage_collection.hpp"
#include <vector>

namespace {
void mark_value(VMHeap *heap, std::vector<GCFlag> *string_marks,
                const Value &val);
void mark_iterable_elements(VMHeap *heap, std::vector<GCFlag> *string_marks,
                            VMHIDX first_child);

void mark_heap_node(VMHeap *heap, std::vector<GCFlag> *string_marks,
                    VMHIDX idx) {
  if (idx == INVALID) {
    return;
  }
  VMHNode *node = heap->node_at(idx);
  if (node->gc_flag == GCFlag::MARKED) {
    return;
  }

  node->gc_flag = GCFlag::MARKED;
  mark_value(heap, string_marks, node->value);
}

void mark_iterable_elements(VMHeap *heap, std::vector<GCFlag> *string_marks,
                            VMHIDX first_child) {
  VMHIDX current_idx = first_child;
  while (current_idx != INVALID) {
    mark_heap_node(heap, string_marks, current_idx);
    current_idx = heap->node_at(current_idx)->next_child;
  }
}

void mark_value(VMHeap *heap, std::vector<GCFlag> *string_marks,
                const Value &val) {
  switch (val.tag) {
  case ValueTag::STRING:
    string_marks->at(val.as.STRING) = GCFlag::MARKED;
    break;
  case ValueTag::LIST:
    mark_heap_node(heap, string_marks, val.as.LIST);
    mark_iterable_elements(heap, string_marks,
                           heap->node_at(val.as.LIST)->first_child);
    break;
  case ValueTag::ITERATOR:
    mark_iterable_elements(heap, string_marks, val.as.ITERATOR);
    break;
  case ValueTag::ERROR:
    mark_heap_node(heap, string_marks, val.as.ERROR);
    break;
  case ValueTag::TABLE:
    mark_heap_node(heap, string_marks, val.as.TABLE);
    mark_iterable_elements(heap, string_marks,
                           heap->node_at(val.as.TABLE)->first_child);
  case ValueTag::INT:
    break;
  case ValueTag::FLOAT:
    break;
  case ValueTag::SYMBOL:
    break;
  case ValueTag::NONE:
    break;
  case ValueTag::FN_REF:
    mark_heap_node(heap, string_marks, val.as.FN_REF);
    break;
  }
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

  heap->current_length = (heap->elements.size() - heap->free_list.size()) +
                         (heap->strings.size() - heap->free_strings.size());
}
