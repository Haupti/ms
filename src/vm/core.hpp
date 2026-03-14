#pragma once

#include "../location.hpp"
#include "constants.hpp"
#include "stack.hpp"
#include "value_and_heap.hpp"
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace core {
std::string value_to_string(Stack *stack, VMHeap *heap, Value value);
bool values_equal(VMHeap *heap, Value left, Value right);

Value print(LocationRef where, Stack *stack, VMHeap *heap, uint16_t args);
Value make_error(LocationRef where, Stack *stack, VMHeap *heap, uint16_t args);
[[noreturn]] Value vmpanic(LocationRef where, Stack *stack, VMHeap *heap,
                           uint16_t args);

static std::unordered_map<
    uint64_t,
    std::function<Value(LocationRef where, Stack *, VMHeap *, uint16_t)>>
    fns = {{Constants::BUILDIN_FN_PRINT.index, print},
           {Constants::BUILDIN_FN_PANIC.index, vmpanic},
           {Constants::BUILDIN_FN_ERROR.index, make_error}

};
} // namespace core
