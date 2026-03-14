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

Value print(LocationRef where, Stack *stack, VMHeap *heap);
Value make_error(LocationRef where, Stack *stack, VMHeap *heap);
[[noreturn]] Value vmpanic(LocationRef where, Stack *stack, VMHeap *heap);
Value list(LocationRef where, Stack *stack, VMHeap *heap);
Value list_put(LocationRef where, Stack *stack, VMHeap *heap);
Value list_at(LocationRef where, Stack *stack, VMHeap *heap);
Value list_append(LocationRef where, Stack *stack, VMHeap *heap);
Value list_prepend(LocationRef where, Stack *stack, VMHeap *heap);
Value list_link(LocationRef where, Stack *stack, VMHeap *heap);
Value value_copy(LocationRef where, Stack *stack, VMHeap *heap);
Value vmtypeof(LocationRef where, Stack *stack, VMHeap *heap);
Value string_concat(LocationRef where, Stack *stack, VMHeap *heap);
Value len(LocationRef where, Stack *stack, VMHeap *heap);
Value value_to_int(LocationRef where, Stack *stack, VMHeap *heap);
Value value_to_float(LocationRef where, Stack *stack, VMHeap *heap);
Value value_to_string_fn(LocationRef where, Stack *stack, VMHeap *heap);
Value file_read(LocationRef where, Stack *stack, VMHeap *heap);
Value file_write(LocationRef where, Stack *stack, VMHeap *heap);
Value file_append(LocationRef where, Stack *stack, VMHeap *heap);

enum class ArgsCountType : uint8_t {
  VARARGS,
  ARGS,
};
struct ArgsCount {
  uint16_t count;
  ArgsCountType type;
  ArgsCount(uint16_t count, ArgsCountType type) : count(count), type(type) {}
};
static std::unordered_map<uint64_t, ArgsCount> fns_args = {
    {Constants::BUILDIN_FN_PRINT.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_PANIC.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_LIST.index, ArgsCount(1, ArgsCountType::VARARGS)},
    {Constants::BUILDIN_FN_PUT.index, ArgsCount(3, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_AT.index, ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_APPEND.index, ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_PREPEND.index, ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_LINK.index, ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_COPY.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_TYPEOF.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_ERROR.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_STR_CONCAT.index, ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_LEN.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_INT.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_FLOAT.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_STR.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_FILE_READ.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_FILE_WRITE.index, ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_FILE_APPEND.index,
     ArgsCount(2, ArgsCountType::ARGS)}};
static std::unordered_map<
    uint64_t, std::function<Value(LocationRef where, Stack *, VMHeap *)>>
    fns = {{Constants::BUILDIN_FN_PRINT.index, print},
           {Constants::BUILDIN_FN_PANIC.index, vmpanic},
           {Constants::BUILDIN_FN_LIST.index, list},
           {Constants::BUILDIN_FN_PUT.index, list_put},
           {Constants::BUILDIN_FN_AT.index, list_at},
           {Constants::BUILDIN_FN_APPEND.index, list_append},
           {Constants::BUILDIN_FN_PREPEND.index, list_prepend},
           {Constants::BUILDIN_FN_LINK.index, list_link},
           {Constants::BUILDIN_FN_COPY.index, value_copy},
           {Constants::BUILDIN_FN_TYPEOF.index, vmtypeof},
           {Constants::BUILDIN_FN_ERROR.index, make_error},
           {Constants::BUILDIN_FN_STR_CONCAT.index, string_concat},
           {Constants::BUILDIN_FN_LEN.index, len},
           {Constants::BUILDIN_FN_INT.index, value_to_int},
           {Constants::BUILDIN_FN_FLOAT.index, value_to_float},
           {Constants::BUILDIN_FN_STR.index, value_to_string_fn},
           {Constants::BUILDIN_FN_FILE_READ.index, file_read},
           {Constants::BUILDIN_FN_FILE_WRITE.index, file_write},
           {Constants::BUILDIN_FN_FILE_APPEND.index, file_append}

};

} // namespace core
