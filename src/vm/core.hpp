#pragma once

#include "../location.hpp"
#include "../msl_runtime_error.hpp"
#include "constants.hpp"
#include "stack.hpp"
#include "value_and_heap.hpp"
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace core {
inline bool as_bool(LocationRef ref, Value value) {
  switch (value.tag) {
  case ValueTag::SYMBOL:
    return value.as.SYMBOL.index == Constants::SYM_TRUE.index;
  default:
    throw msl_runtime_error(ref, "expected a symbol");
    break;
  }
}
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
Value vmassert(LocationRef where, Stack *stack, VMHeap *heap);
Value vmassert_type(LocationRef where, Stack *stack, VMHeap *heap);

Value file_read(LocationRef where, Stack *stack, VMHeap *heap);
Value file_write(LocationRef where, Stack *stack, VMHeap *heap);
Value file_append(LocationRef where, Stack *stack, VMHeap *heap);

Value process_args(LocationRef where, Stack *stack, VMHeap *heap);

Value sys_env(LocationRef where, Stack *stack, VMHeap *heap);
Value sys_exit(LocationRef where, Stack *stack, VMHeap *heap);
Value sys_exec(LocationRef where, Stack *stack, VMHeap *heap);

Value random_int(LocationRef where, Stack *stack, VMHeap *heap);

Value time_epoch_ms(LocationRef where, Stack *stack, VMHeap *heap);
Value time_epoch_sec(LocationRef where, Stack *stack, VMHeap *heap);
Value time_iso8601(LocationRef where, Stack *stack, VMHeap *heap);

Value str_split(LocationRef where, Stack *stack, VMHeap *heap);
Value str_replace(LocationRef where, Stack *stack, VMHeap *heap);
Value str_contains(LocationRef where, Stack *stack, VMHeap *heap);
Value str_has_prefix(LocationRef where, Stack *stack, VMHeap *heap);
Value str_has_suffix(LocationRef where, Stack *stack, VMHeap *heap);
Value str_lower(LocationRef where, Stack *stack, VMHeap *heap);
Value str_upper(LocationRef where, Stack *stack, VMHeap *heap);
Value str_trim(LocationRef where, Stack *stack, VMHeap *heap);
Value str_slice(LocationRef where, Stack *stack, VMHeap *heap);
Value str_find(LocationRef where, Stack *stack, VMHeap *heap);
Value str_index(LocationRef where, Stack *stack, VMHeap *heap);
Value str_fmt(LocationRef where, Stack *stack, VMHeap *heap);

Value math_abs(LocationRef where, Stack *stack, VMHeap *heap);
Value math_floor(LocationRef where, Stack *stack, VMHeap *heap);
Value math_ceil(LocationRef where, Stack *stack, VMHeap *heap);
Value math_round(LocationRef where, Stack *stack, VMHeap *heap);
Value math_sqrt(LocationRef where, Stack *stack, VMHeap *heap);
Value math_pow(LocationRef where, Stack *stack, VMHeap *heap);
Value math_sin(LocationRef where, Stack *stack, VMHeap *heap);
Value math_cos(LocationRef where, Stack *stack, VMHeap *heap);
Value math_tan(LocationRef where, Stack *stack, VMHeap *heap);
Value math_log(LocationRef where, Stack *stack, VMHeap *heap);
Value math_exp(LocationRef where, Stack *stack, VMHeap *heap);


void set_args(const std::vector<std::string> &args);

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
    {Constants::BUILDIN_FN_ASSERT.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_ASSERTTYPE.index, ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_FILE_READ.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_FILE_WRITE.index, ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_FILE_APPEND.index,
     ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_SYS_ENV.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_PROCESS_ARGS.index,
     ArgsCount(0, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_SYS_EXIT.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_SYS_EXEC.index,
     ArgsCount(1, ArgsCountType::VARARGS)},
    {Constants::BUILDIN_FN_RANDOM.index, ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_TIME_EPOCH_MS.index,
     ArgsCount(0, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_TIME_EPOCH_SEC.index,
     ArgsCount(0, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_TIME_ISO8601.index,
     ArgsCount(0, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_STR_SPLIT.index, ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_STR_REPLACE.index,
     ArgsCount(3, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_STR_CONTAINS.index,
     ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_STR_HAS_PREFIX.index,
     ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_STR_HAS_SUFFIX.index,
     ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_STR_LOWER.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_STR_UPPER.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_STR_TRIM.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_MATH_ABS.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_MATH_FLOOR.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_MATH_CEIL.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_MATH_ROUND.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_MATH_SQRT.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_MATH_POW.index, ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_MATH_SIN.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_MATH_COS.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_MATH_TAN.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_MATH_LOG.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_MATH_EXP.index, ArgsCount(1, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_STR_SLICE.index, ArgsCount(3, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_STR_FIND.index, ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_STR_INDEX.index, ArgsCount(2, ArgsCountType::ARGS)},
    {Constants::BUILDIN_FN_STR_FMT.index, ArgsCount(1, ArgsCountType::VARARGS)}};
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
           {Constants::BUILDIN_FN_ASSERT.index, vmassert},
           {Constants::BUILDIN_FN_ASSERTTYPE.index, vmassert_type},
           {Constants::BUILDIN_FN_FILE_READ.index, file_read},
           {Constants::BUILDIN_FN_FILE_WRITE.index, file_write},
           {Constants::BUILDIN_FN_FILE_APPEND.index, file_append},
           {Constants::BUILDIN_FN_SYS_ENV.index, sys_env},
           {Constants::BUILDIN_FN_PROCESS_ARGS.index, process_args},
           {Constants::BUILDIN_FN_SYS_EXIT.index, sys_exit},
           {Constants::BUILDIN_FN_SYS_EXEC.index, sys_exec},
           {Constants::BUILDIN_FN_RANDOM.index, random_int},
           {Constants::BUILDIN_FN_TIME_EPOCH_MS.index, time_epoch_ms},
           {Constants::BUILDIN_FN_TIME_EPOCH_SEC.index, time_epoch_sec},
           {Constants::BUILDIN_FN_TIME_ISO8601.index, time_iso8601},
           {Constants::BUILDIN_FN_STR_SPLIT.index, str_split},
           {Constants::BUILDIN_FN_STR_REPLACE.index, str_replace},
           {Constants::BUILDIN_FN_STR_CONTAINS.index, str_contains},
           {Constants::BUILDIN_FN_STR_HAS_PREFIX.index, str_has_prefix},
           {Constants::BUILDIN_FN_STR_HAS_SUFFIX.index, str_has_suffix},
           {Constants::BUILDIN_FN_STR_LOWER.index, str_lower},
           {Constants::BUILDIN_FN_STR_UPPER.index, str_upper},
           {Constants::BUILDIN_FN_STR_TRIM.index, str_trim},
           {Constants::BUILDIN_FN_MATH_ABS.index, math_abs},
           {Constants::BUILDIN_FN_MATH_FLOOR.index, math_floor},
           {Constants::BUILDIN_FN_MATH_CEIL.index, math_ceil},
           {Constants::BUILDIN_FN_MATH_ROUND.index, math_round},
           {Constants::BUILDIN_FN_MATH_SQRT.index, math_sqrt},
           {Constants::BUILDIN_FN_MATH_POW.index, math_pow},
           {Constants::BUILDIN_FN_MATH_SIN.index, math_sin},
           {Constants::BUILDIN_FN_MATH_COS.index, math_cos},
           {Constants::BUILDIN_FN_MATH_TAN.index, math_tan},
           {Constants::BUILDIN_FN_MATH_LOG.index, math_log},
           {Constants::BUILDIN_FN_MATH_EXP.index, math_exp},
           {Constants::BUILDIN_FN_STR_SLICE.index, str_slice},
           {Constants::BUILDIN_FN_STR_FIND.index, str_find},
           {Constants::BUILDIN_FN_STR_INDEX.index, str_index},
           {Constants::BUILDIN_FN_STR_FMT.index, str_fmt}

};

} // namespace core
