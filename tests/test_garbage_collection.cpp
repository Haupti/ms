#include "../lib/asap/t.hpp"
#include "../src/vm/garbage_collection.hpp"
#include "../src/vm/value_and_heap.hpp"
#include <algorithm>

using namespace std;

namespace {
bool is_in_free_list(VMHeap *heap, VMHIDX idx) {
  return find(heap->free_list.begin(), heap->free_list.end(), idx) !=
         heap->free_list.end();
}

bool is_in_free_strings(VMHeap *heap, StringIdx idx) {
  return find(heap->free_strings.begin(), heap->free_strings.end(), idx) !=
         heap->free_strings.end();
}

void test_gc_basic_reclaim(T *t) {
  VMHeap heap(10, 10);
  Stack stack(10);

  // Reachable int
  heap.add(Value::Int(1));
  stack.values[0] = Value::Int(1); 

  VMHIDX list_reachable = heap.new_list();
  stack.values[0] = Value::List(list_reachable);
  stack.stkptr = 1;

  VMHIDX list_unreachable = heap.new_list();

  run_gc(&heap, &stack);

  if (is_in_free_list(&heap, list_reachable))
    t->fail("Reachable list was freed");
  if (!is_in_free_list(&heap, list_unreachable))
    t->fail("Unreachable list was not freed");
}

void test_gc_recursive_list(T *t) {
  VMHeap heap(10, 10);
  Stack stack(10);

  VMHIDX parent = heap.new_list();
  VMHIDX child = heap.add_child(parent, Value::Int(42));

  stack.values[0] = Value::List(parent);
  stack.stkptr = 1;

  run_gc(&heap, &stack);

  if (is_in_free_list(&heap, parent))
    t->fail("Parent list was freed");
  if (is_in_free_list(&heap, child))
    t->fail("Child node was freed");
}

void test_gc_static_strings(T *t) {
  VMHeap heap(10, 10);
  Stack stack(10);
  stack.stkptr = 0;

  // Add a "static" string (simulating hardcoded string in source)
  StringIdx s_idx = 0;
  heap.strings.push_back("static");
  s_idx = heap.strings.size() - 1;
  heap.static_strings[999] = s_idx; // 999 is some arbitrary intern index

  // Add a dynamic string
  VMHIDX dynamic_val_idx = heap.ref_add_string("dynamic");
  StringIdx dynamic_s_idx = heap.node_at(dynamic_val_idx)->value.as.STRING;

  run_gc(&heap, &stack);

  if (!is_in_free_strings(&heap, s_idx)) {
    t->fail("Static string was not reclaimed");
  }
  if (!is_in_free_strings(&heap, dynamic_s_idx)) {
    t->fail("Unreachable dynamic string was not reclaimed");
  }
}

void test_gc_iterators(T *t) {
  VMHeap heap(10, 10);
  Stack stack(10);

  VMHIDX list = heap.new_list();
  VMHIDX e1 = heap.add_child(list, Value::Int(1));
  VMHIDX e2 = heap.add_child(list, Value::Int(2));
  VMHIDX e3 = heap.add_child(list, Value::Int(3));

  // Stack only holds an iterator pointing to the second element
  stack.values[0] = Value::Iterator(e2);
  stack.stkptr = 1;

  run_gc(&heap, &stack);

  if (!is_in_free_list(&heap, list)) {
    t->fail("Original list head should be freed if not referenced");
  }
  if (!is_in_free_list(&heap, e1)) {
    t->fail("First element (before iterator) should be freed");
  }
  if (is_in_free_list(&heap, e2)) {
    t->fail("Current iterator element should be preserved");
  }
  if (is_in_free_list(&heap, e3)) {
    t->fail("Subsequent elements in iterator chain should be preserved");
  }
}

void test_gc_circular(T *t) {
  VMHeap heap(10, 10);
  Stack stack(10);

  VMHIDX l1 = heap.new_list();
  VMHIDX l2 = heap.new_list();

  heap.add_child(l1, Value::List(l2));
  heap.add_child(l2, Value::List(l1));

  stack.values[0] = Value::List(l1);
  stack.stkptr = 1;

  // Should not crash (infinite recursion) and should preserve both
  run_gc(&heap, &stack);

  if (is_in_free_list(&heap, l1)) {
    t->fail("Circular list l1 was freed");
  }
  if (is_in_free_list(&heap, l2)) {
    t->fail("Circular list l2 was freed");
  }

  // Now clear stack and they should both be freed
  stack.stkptr = 0;
  run_gc(&heap, &stack);
  if (!is_in_free_list(&heap, l1)) {
    t->fail("Unreachable circular list l1 was not freed");
  }
  if (!is_in_free_list(&heap, l2)) {
    t->fail("Unreachable circular list l2 was not freed");
  }
}
} // namespace

int main() {
  T t("Garbage Collection");
  t.test("basic reclaim", test_gc_basic_reclaim);
  t.test("recursive list marking", test_gc_recursive_list);
  t.test("static strings persistence", test_gc_static_strings);
  t.test("iterator chain preservation", test_gc_iterators);
  t.test("circular references", test_gc_circular);
  return 0;
}
