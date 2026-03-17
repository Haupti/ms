#include "garbage_collection.hpp"
namespace {
void mark_heap_node(VMHeap *heap, std::vector<GCFlag> *string_marks,
                    VMHNode *val);

void mark_iterable_elements(VMHeap *heap, std::vector<GCFlag> *string_marks,
                            VMHIDX first_child) {
  VMHIDX current_idx = first_child;
  VMHNode *curr = nullptr;
  do {
    curr = heap->node_at(current_idx);
    mark_heap_node(heap, string_marks, curr);
    current_idx = curr->next_child;
  } while (current_idx != INVALID);
}

void mark(VMHeap *heap, std::vector<GCFlag> *string_marks, const Value &val) {
  switch (val.tag) {
  case ValueTag::STRING: {
    string_marks->at(val.as.STRING) = GCFlag::MARKED;
  } break;
  case ValueTag::LIST: {
    heap->node_at(val.as.LIST)->gc_flag = GCFlag::MARKED;
    VMHIDX first_child_idx = heap->node_at(val.as.LIST)->first_child;
    if (first_child_idx == INVALID) {
      return;
    }
    mark_iterable_elements(heap, string_marks, first_child_idx);
  } break;
  case ValueTag::ITERATOR: {
    if (val.as.ITERATOR != INVALID) {
      mark_iterable_elements(heap, string_marks, val.as.ITERATOR);
    }
  } break;
  case ValueTag::ERROR:
    heap->node_at(val.as.ERROR)->gc_flag = GCFlag::MARKED;
    break;
  default:
    return;
  }
}
void mark_heap_node(VMHeap *heap, std::vector<GCFlag> *string_marks,
                    VMHNode *val) {
  switch (val->value.tag) {
  case ValueTag::STRING: {
    val->gc_flag = GCFlag::MARKED;
    string_marks->at(val->value.as.STRING) = GCFlag::MARKED;
  } break;
  case ValueTag::LIST: {
    val->gc_flag = GCFlag::MARKED;
    heap->node_at(val->value.as.LIST)->gc_flag = GCFlag::MARKED;
    VMHIDX first_child_idx = heap->node_at(val->value.as.LIST)->first_child;
    if (first_child_idx == INVALID) {
      return;
    }
    mark_iterable_elements(heap, string_marks, first_child_idx);
  } break;
  case ValueTag::ITERATOR: {
    val->gc_flag = GCFlag::MARKED;
    if (val->value.as.ITERATOR != INVALID) {
      mark_iterable_elements(heap, string_marks, val->value.as.ITERATOR);
    }
  } break;
  case ValueTag::ERROR:
    val->gc_flag = GCFlag::MARKED;
    break;
  case ValueTag::INT:
    val->gc_flag = GCFlag::MARKED;
    break;
  case ValueTag::FLOAT:
    val->gc_flag = GCFlag::MARKED;
    break;
  case ValueTag::SYMBOL:
    val->gc_flag = GCFlag::MARKED;
    break;
  case ValueTag::NONE:
    val->gc_flag = GCFlag::MARKED;
    break;
  }
}

void free_values(VMHeap *heap) {
  for (uint64_t i = 0; i < heap->elements.size(); ++i) {
    if (heap->node_at(i)->gc_flag == GCFlag::FREE) {
      heap->free_list.push_back(i);
    }
  }
}
void free_strings(VMHeap *heap, std::vector<GCFlag> *string_marks) {
  for (uint64_t i = 0; i < heap->strings.size(); ++i) {
    if (string_marks->at(i) == GCFlag::FREE) {
      heap->free_strings.push_back(i);
    }
  }
}
}; // namespace

void run_gc(VMHeap *heap, Stack *stack) {

  for (auto element : heap->elements) {
    element.gc_flag = GCFlag::FREE;
  }

  std::vector<GCFlag> string_marks;
  string_marks.resize(heap->strings.size(), GCFlag::FREE);

  Value val;
  for (uint64_t i = 0; i < stack->stkptr; ++i) {
    val = stack->values[i];
    if (val.undefined) {
      continue;
    }
    mark(heap, &string_marks, val);
  }

  free_values(heap);
  free_strings(heap, &string_marks);
}
