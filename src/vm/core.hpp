#pragma once

#include "../location.hpp"
#include "constants.hpp"
#include "stack.hpp"
#include "value_and_heap.hpp"
#include <cstdint>
#include <functional>
#include <unordered_map>

class VM;
namespace core {
std::string value_to_string(VM *vm, Value value);
bool values_equal(VMHeap *heap, Value left, Value right);

Value print(LocationRef where, VM *vm);
Value make_error(LocationRef where, VM *vm);
[[noreturn]] Value vmpanic(LocationRef where, VM *vm);

// ===== TABLE
Value table(LocationRef where, VM *vm);
Value table_keys(LocationRef where, VM *vm);
Value table_values(LocationRef where, VM *vm);
Value table_remove(LocationRef where, VM *vm);
Value table_to_json(LocationRef where, VM *vm);
Value table_from_json(LocationRef where, VM *vm);

// ===== LIST
Value list(LocationRef where, VM *vm);
Value container_put(LocationRef where, VM *vm);
Value container_at(LocationRef where, VM *vm);
Value list_append(LocationRef where, VM *vm);
Value list_prepend(LocationRef where, VM *vm);
Value list_link(LocationRef where, VM *vm);
Value list_slice(LocationRef where, VM *vm);
Value list_remove(LocationRef where, VM *vm);
Value list_contains(LocationRef where, VM *vm);
Value range(LocationRef where, VM *vm);

// ===== UTILITIES
Value value_copy(LocationRef where, VM *vm);
Value vmtypeof(LocationRef where, VM *vm);
Value string_concat(LocationRef where, VM *vm);
Value len(LocationRef where, VM *vm);
Value value_to_int(LocationRef where, VM *vm);
Value value_to_float(LocationRef where, VM *vm);
Value value_to_string_fn(LocationRef where, VM *vm);
Value vmassert(LocationRef where, VM *vm);
Value vmassert_type(LocationRef where, VM *vm);

// ===== PROCESS
Value process_args(LocationRef where, VM *vm);

// ===== SYS
Value sys_env_get(LocationRef where, VM *vm);
Value sys_exit(LocationRef where, VM *vm);
Value sys_now(LocationRef where, VM *vm);
Value sys_exec(LocationRef where, VM *vm);
Value sys_sleep(LocationRef where, VM *vm);
Value sys_term_height(LocationRef ref, VM *vm);
Value sys_term_width(LocationRef ref, VM *vm);
Value sys_is_tty(LocationRef where, VM *vm);
Value sys_has_color(LocationRef where, VM *vm);

// ===== RANDOM
Value random_int(LocationRef where, VM *vm);

// ===== TIME
Value time_epoch_ms(LocationRef where, VM *vm);
Value time_epoch_sec(LocationRef where, VM *vm);
Value time_iso8601(LocationRef where, VM *vm);

// ===== STRING
Value str_split(LocationRef where, VM *vm);
Value str_replace(LocationRef where, VM *vm);
Value str_contains(LocationRef where, VM *vm);
Value str_has_prefix(LocationRef where, VM *vm);
Value str_has_suffix(LocationRef where, VM *vm);
Value str_lower(LocationRef where, VM *vm);
Value str_upper(LocationRef where, VM *vm);
Value str_trim(LocationRef where, VM *vm);
Value str_trim_left(LocationRef where, VM *vm);
Value str_trim_right(LocationRef where, VM *vm);
Value str_slice(LocationRef where, VM *vm);
Value str_find(LocationRef where, VM *vm);
Value str_index(LocationRef where, VM *vm);
Value str_fmt(LocationRef where, VM *vm);
Value str_url_encode(LocationRef where, VM *vm);
Value str_url_decode(LocationRef where, VM *vm);

// ===== FILE SYSTEM
Value fs_exists(LocationRef where, VM *vm);
Value fs_mkdir(LocationRef where, VM *vm);
Value fs_rm(LocationRef where, VM *vm);
Value fs_ls(LocationRef where, VM *vm);
Value fs_read(LocationRef where, VM *vm);
Value fs_write(LocationRef where, VM *vm);
Value fs_append(LocationRef where, VM *vm);
Value fs_copy(LocationRef where, VM *vm);
Value fs_move(LocationRef where, VM *vm);
Value fs_stat(LocationRef where, VM *vm);

// ===== BIT
Value bit_shift_left(LocationRef where, VM *vm);
Value bit_shift_right(LocationRef where, VM *vm);
Value bit_or(LocationRef where, VM *vm);
Value bit_and(LocationRef where, VM *vm);
Value bit_xor(LocationRef where, VM *vm);

// ===== MATH
Value math_abs(LocationRef where, VM *vm);
Value math_floor(LocationRef where, VM *vm);
Value math_ceil(LocationRef where, VM *vm);
Value math_round(LocationRef where, VM *vm);
Value math_sqrt(LocationRef where, VM *vm);
Value math_pow(LocationRef where, VM *vm);
Value math_sin(LocationRef where, VM *vm);
Value math_cos(LocationRef where, VM *vm);
Value math_tan(LocationRef where, VM *vm);
Value math_log(LocationRef where, VM *vm);
Value math_exp(LocationRef where, VM *vm);

// ===== ANSI
Value ansi_color(LocationRef where, VM *vm);
Value ansi_reset(LocationRef where, VM *vm);
Value ansi_set_cursor(LocationRef where, VM *vm);
Value ansi_move_cursor(LocationRef where, VM *vm);
Value ansi_clear_line(LocationRef where, VM *vm);
Value ansi_clear_screen(LocationRef where, VM *vm);
Value ansi_clear(LocationRef where, VM *vm);

// ===== REGEX
Value regex_has_match(LocationRef where, VM *vm);
Value regex_match(LocationRef where, VM *vm);
Value regex_replace(LocationRef where, VM *vm);

// ===== BASE64
Value base64_encode(LocationRef where, VM *vm);
Value base64_decode(LocationRef where, VM *vm);

// ===== NUMBER FORMATS
Value hex_encode(LocationRef where, VM *vm);
Value hex_decode(LocationRef where, VM *vm);
Value binary_encode(LocationRef where, VM *vm);
Value binary_decode(LocationRef where, VM *vm);

// ===== HTTP
Value http_get(LocationRef where, VM *vm);
Value http_post(LocationRef where, VM *vm);
Value http_put(LocationRef where, VM *vm);
Value http_patch(LocationRef where, VM *vm);
Value http_delete(LocationRef where, VM *vm);
Value http_on(LocationRef where, VM *vm);
Value http_listen(LocationRef where, VM *vm);

// ===== BOX
Value box_create(LocationRef where, VM *vm);
Value box_unpack(LocationRef where, VM *vm);
Value box_pack(LocationRef where, VM *vm);

void set_args(const std::vector<std::string> &args);

enum class ArgsT : uint8_t {
  VARARGS,
  ARGS,
};
struct ArgsCount {
  uint16_t count;
  ArgsT type;
  ArgsCount(uint16_t count, ArgsT type) : count(count), type(type) {}
};
static std::unordered_map<uint64_t, ArgsCount> fns_args = {
    {Constants::CORE_FN_PRINT.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_PANIC.index, ArgsCount(1, ArgsT::ARGS)},
    // ===== CONTAINER (generic)
    {Constants::CORE_FN_PUT.index, ArgsCount(3, ArgsT::ARGS)},
    {Constants::CORE_FN_AT.index, ArgsCount(2, ArgsT::ARGS)},
    // ===== TABLE
    {Constants::CORE_FN_TABLE.index, ArgsCount(1, ArgsT::VARARGS)},
    {Constants::CORE_FN_TABLE_KEYS.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_TABLE_VALUES.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_TABLE_REMOVE.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_TABLE_TO_JSON.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_TABLE_FROM_JSON.index, ArgsCount(1, ArgsT::ARGS)},
    // ===== LIST
    {Constants::CORE_FN_LIST.index, ArgsCount(1, ArgsT::VARARGS)},
    {Constants::CORE_FN_APPEND.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_PREPEND.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_LIST_SLICE.index, ArgsCount(3, ArgsT::ARGS)},
    {Constants::CORE_FN_LIST_REMOVE.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_LIST_CONTAINS.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_RANGE.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_LINK.index, ArgsCount(2, ArgsT::ARGS)},
    // ===== UTILITY
    {Constants::CORE_FN_COPY.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_TYPEOF.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_ERROR.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_STR_CONCAT.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_LEN.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_INT.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_FLOAT.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_STR.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_ASSERT.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_ASSERTTYPE.index, ArgsCount(2, ArgsT::ARGS)},
    // ===== PROCESS
    {Constants::CORE_FN_PROCESS_ARGS.index, ArgsCount(0, ArgsT::ARGS)},
    // ===== SYSTEM
    {Constants::CORE_FN_SYS_ENV_GET.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_SYS_EXIT.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_SYS_EXEC.index, ArgsCount(1, ArgsT::VARARGS)},
    {Constants::CORE_FN_SYS_NOW.index, ArgsCount(0, ArgsT::ARGS)},
    {Constants::CORE_FN_SYS_IS_TTY.index, ArgsCount(0, ArgsT::ARGS)},
    {Constants::CORE_FN_SYS_HAS_COLOR.index, ArgsCount(0, ArgsT::ARGS)},
    {Constants::CORE_FN_SYS_SLEEP.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_SYS_TERM_WIDTH.index, ArgsCount(0, ArgsT::ARGS)},
    {Constants::CORE_FN_SYS_TERM_HEIGHT.index, ArgsCount(0, ArgsT::ARGS)},
    // ===== RANDOM
    {Constants::CORE_FN_RANDOM.index, ArgsCount(2, ArgsT::ARGS)},
    // ===== TIME
    {Constants::CORE_FN_TIME_EPOCH_MS.index, ArgsCount(0, ArgsT::ARGS)},
    {Constants::CORE_FN_TIME_EPOCH_SEC.index, ArgsCount(0, ArgsT::ARGS)},
    {Constants::CORE_FN_TIME_ISO8601.index, ArgsCount(0, ArgsT::ARGS)},
    // ===== STRING
    {Constants::CORE_FN_STR_SPLIT.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_STR_REPLACE.index, ArgsCount(3, ArgsT::ARGS)},
    {Constants::CORE_FN_STR_CONTAINS.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_STR_HAS_PREFIX.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_STR_HAS_SUFFIX.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_STR_LOWER.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_STR_UPPER.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_STR_TRIM.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_STR_TRIM_LEFT.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_STR_TRIM_RIGHT.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_STR_SLICE.index, ArgsCount(3, ArgsT::ARGS)},
    {Constants::CORE_FN_STR_FIND.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_STR_INDEX.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_STR_FMT.index, ArgsCount(1, ArgsT::VARARGS)},
    {Constants::CORE_FN_STR_URL_ENCODE.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_STR_URL_DECODE.index, ArgsCount(1, ArgsT::ARGS)},
    // ===== MATH
    {Constants::CORE_FN_MATH_ABS.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_MATH_FLOOR.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_MATH_CEIL.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_MATH_ROUND.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_MATH_SQRT.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_MATH_POW.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_MATH_SIN.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_MATH_COS.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_MATH_TAN.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_MATH_LOG.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_MATH_EXP.index, ArgsCount(1, ArgsT::ARGS)},
    // ===== FILE SYSTEM
    {Constants::CORE_FN_FS_READ.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_FS_WRITE.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_FS_APPEND.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_FS_EXISTS.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_FS_MKDIR.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_FS_RM.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_FS_LS.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_FS_COPY.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_FS_MOVE.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_FS_STAT.index, ArgsCount(1, ArgsT::ARGS)},
    // ===== BIT
    {Constants::CORE_FN_BIT_SHIFT_LEFT.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_BIT_SHIFT_RIGHT.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_BIT_OR.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_BIT_AND.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_BIT_XOR.index, ArgsCount(2, ArgsT::ARGS)},
    // ===== ANSI
    {Constants::CORE_FN_ANSI_COLOR.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_ANSI_RESET.index, ArgsCount(0, ArgsT::ARGS)},
    {Constants::CORE_FN_ANSI_SET_CURSOR.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_ANSI_MOVE_CURSOR.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_ANSI_CLEAR_LINE.index, ArgsCount(0, ArgsT::ARGS)},
    {Constants::CORE_FN_ANSI_CLEAR_SCREEN.index, ArgsCount(0, ArgsT::ARGS)},
    {Constants::CORE_FN_ANSI_CLEAR.index, ArgsCount(1, ArgsT::ARGS)},
    // ===== REGEX
    {Constants::CORE_FN_REGEX_HAS_MATCH.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_REGEX_MATCH.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_REGEX_REPLACE.index, ArgsCount(3, ArgsT::ARGS)},
    {Constants::CORE_FN_BASE64_ENCODE.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_BASE64_DECODE.index, ArgsCount(1, ArgsT::ARGS)},
    // ===== NUMBER FORMATS
    {Constants::CORE_FN_HEX_ENCODE.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_HEX_DECODE.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_BINARY_ENCODE.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_BINARY_DECODE.index, ArgsCount(1, ArgsT::ARGS)},
    // ===== HTTP
    {Constants::CORE_FN_HTTP_GET.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_HTTP_POST.index, ArgsCount(3, ArgsT::ARGS)},
    {Constants::CORE_FN_HTTP_PUT.index, ArgsCount(3, ArgsT::ARGS)},
    {Constants::CORE_FN_HTTP_PATCH.index, ArgsCount(3, ArgsT::ARGS)},
    {Constants::CORE_FN_HTTP_DELETE.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_HTTP_ON.index, ArgsCount(3, ArgsT::ARGS)},
    {Constants::CORE_FN_HTTP_LISTEN.index, ArgsCount(1, ArgsT::ARGS)},
    // ===== BOX
    {Constants::CORE_FN_BOX.index, ArgsCount(1, ArgsT::ARGS)},
    {Constants::CORE_FN_BOX_PACK.index, ArgsCount(2, ArgsT::ARGS)},
    {Constants::CORE_FN_BOX_UNPACK.index, ArgsCount(1, ArgsT::ARGS)},
};
static std::unordered_map<
    uint64_t, std::function<Value(LocationRef where, VM *)>>
    fns = {
        {Constants::CORE_FN_PRINT.index, print},
        {Constants::CORE_FN_PANIC.index, vmpanic},
        // ===== CONTAINER (generic)
        {Constants::CORE_FN_PUT.index, container_put},
        {Constants::CORE_FN_AT.index, container_at},
        // ===== TABLE
        {Constants::CORE_FN_TABLE.index, table},
        {Constants::CORE_FN_TABLE_KEYS.index, table_keys},
        {Constants::CORE_FN_TABLE_VALUES.index, table_values},
        {Constants::CORE_FN_TABLE_REMOVE.index, table_remove},
        {Constants::CORE_FN_TABLE_TO_JSON.index, table_to_json},
        {Constants::CORE_FN_TABLE_FROM_JSON.index, table_from_json},
        // ===== LIST
        {Constants::CORE_FN_LIST.index, list},
        {Constants::CORE_FN_APPEND.index, list_append},
        {Constants::CORE_FN_PREPEND.index, list_prepend},
        {Constants::CORE_FN_LINK.index, list_link},
        {Constants::CORE_FN_LIST_SLICE.index, list_slice},
        {Constants::CORE_FN_LIST_REMOVE.index, list_remove},
        {Constants::CORE_FN_LIST_CONTAINS.index, list_contains},
        {Constants::CORE_FN_RANGE.index, range},
        // ===== UTILITY
        {Constants::CORE_FN_COPY.index, value_copy},
        {Constants::CORE_FN_TYPEOF.index, vmtypeof},
        {Constants::CORE_FN_ERROR.index, make_error},
        {Constants::CORE_FN_STR_CONCAT.index, string_concat},
        {Constants::CORE_FN_LEN.index, len},
        {Constants::CORE_FN_INT.index, value_to_int},
        {Constants::CORE_FN_FLOAT.index, value_to_float},
        {Constants::CORE_FN_STR.index, value_to_string_fn},
        {Constants::CORE_FN_ASSERT.index, vmassert},
        {Constants::CORE_FN_ASSERTTYPE.index, vmassert_type},
        // ===== PROCESS
        {Constants::CORE_FN_PROCESS_ARGS.index, process_args},
        // ===== SYSTEM
        {Constants::CORE_FN_SYS_ENV_GET.index, sys_env_get},
        {Constants::CORE_FN_SYS_EXIT.index, sys_exit},
        {Constants::CORE_FN_SYS_EXEC.index, sys_exec},
        {Constants::CORE_FN_SYS_NOW.index, sys_now},
        {Constants::CORE_FN_SYS_IS_TTY.index, sys_is_tty},
        {Constants::CORE_FN_SYS_HAS_COLOR.index, sys_has_color},
        {Constants::CORE_FN_SYS_SLEEP.index, sys_sleep},
        {Constants::CORE_FN_SYS_TERM_WIDTH.index, sys_term_width},
        {Constants::CORE_FN_SYS_TERM_HEIGHT.index, sys_term_height},
        // ===== RANDOM
        {Constants::CORE_FN_RANDOM.index, random_int},
        // ===== TIME
        {Constants::CORE_FN_TIME_EPOCH_MS.index, time_epoch_ms},
        {Constants::CORE_FN_TIME_EPOCH_SEC.index, time_epoch_sec},
        {Constants::CORE_FN_TIME_ISO8601.index, time_iso8601},
        // ===== STRING
        {Constants::CORE_FN_STR_SPLIT.index, str_split},
        {Constants::CORE_FN_STR_REPLACE.index, str_replace},
        {Constants::CORE_FN_STR_CONTAINS.index, str_contains},
        {Constants::CORE_FN_STR_HAS_PREFIX.index, str_has_prefix},
        {Constants::CORE_FN_STR_HAS_SUFFIX.index, str_has_suffix},
        {Constants::CORE_FN_STR_LOWER.index, str_lower},
        {Constants::CORE_FN_STR_UPPER.index, str_upper},
        {Constants::CORE_FN_STR_TRIM.index, str_trim},
        {Constants::CORE_FN_STR_TRIM_LEFT.index, str_trim_left},
        {Constants::CORE_FN_STR_TRIM_RIGHT.index, str_trim_right},
        {Constants::CORE_FN_STR_SLICE.index, str_slice},
        {Constants::CORE_FN_STR_FIND.index, str_find},
        {Constants::CORE_FN_STR_INDEX.index, str_index},
        {Constants::CORE_FN_STR_FMT.index, str_fmt},
        {Constants::CORE_FN_STR_URL_ENCODE.index, str_url_encode},
        {Constants::CORE_FN_STR_URL_DECODE.index, str_url_decode},
        // ===== MATH
        {Constants::CORE_FN_MATH_ABS.index, math_abs},
        {Constants::CORE_FN_MATH_FLOOR.index, math_floor},
        {Constants::CORE_FN_MATH_CEIL.index, math_ceil},
        {Constants::CORE_FN_MATH_ROUND.index, math_round},
        {Constants::CORE_FN_MATH_SQRT.index, math_sqrt},
        {Constants::CORE_FN_MATH_POW.index, math_pow},
        {Constants::CORE_FN_MATH_SIN.index, math_sin},
        {Constants::CORE_FN_MATH_COS.index, math_cos},
        {Constants::CORE_FN_MATH_TAN.index, math_tan},
        {Constants::CORE_FN_MATH_LOG.index, math_log},
        {Constants::CORE_FN_MATH_EXP.index, math_exp},
        // ===== FILE SYSTEM
        {Constants::CORE_FN_FS_READ.index, fs_read},
        {Constants::CORE_FN_FS_WRITE.index, fs_write},
        {Constants::CORE_FN_FS_APPEND.index, fs_append},
        {Constants::CORE_FN_FS_EXISTS.index, fs_exists},
        {Constants::CORE_FN_FS_MKDIR.index, fs_mkdir},
        {Constants::CORE_FN_FS_RM.index, fs_rm},
        {Constants::CORE_FN_FS_LS.index, fs_ls},
        {Constants::CORE_FN_FS_COPY.index, fs_copy},
        {Constants::CORE_FN_FS_MOVE.index, fs_move},
        {Constants::CORE_FN_FS_STAT.index, fs_stat},
        // ===== BIT
        {Constants::CORE_FN_BIT_SHIFT_LEFT.index, bit_shift_left},
        {Constants::CORE_FN_BIT_SHIFT_RIGHT.index, bit_shift_right},
        {Constants::CORE_FN_BIT_OR.index, bit_or},
        {Constants::CORE_FN_BIT_AND.index, bit_and},
        {Constants::CORE_FN_BIT_XOR.index, bit_xor},
        // ===== ANSI
        {Constants::CORE_FN_ANSI_COLOR.index, ansi_color},
        {Constants::CORE_FN_ANSI_RESET.index, ansi_reset},
        {Constants::CORE_FN_ANSI_SET_CURSOR.index, ansi_set_cursor},
        {Constants::CORE_FN_ANSI_MOVE_CURSOR.index, ansi_move_cursor},
        {Constants::CORE_FN_ANSI_CLEAR_LINE.index, ansi_clear_line},
        {Constants::CORE_FN_ANSI_CLEAR_SCREEN.index, ansi_clear_screen},
        {Constants::CORE_FN_ANSI_CLEAR.index, ansi_clear},
        // ===== REGEX
        {Constants::CORE_FN_REGEX_HAS_MATCH.index, regex_has_match},
        {Constants::CORE_FN_REGEX_MATCH.index, regex_match},
        {Constants::CORE_FN_REGEX_REPLACE.index, regex_replace},
        {Constants::CORE_FN_BASE64_ENCODE.index, base64_encode},
        {Constants::CORE_FN_BASE64_DECODE.index, base64_decode},
        // ===== NUMBER FORMATS
        {Constants::CORE_FN_HEX_ENCODE.index, hex_encode},
        {Constants::CORE_FN_HEX_DECODE.index, hex_decode},
        {Constants::CORE_FN_BINARY_ENCODE.index, binary_encode},
        {Constants::CORE_FN_BINARY_DECODE.index, binary_decode},
        // ===== HTTP
        {Constants::CORE_FN_HTTP_GET.index, http_get},
        {Constants::CORE_FN_HTTP_POST.index, http_post},
        {Constants::CORE_FN_HTTP_PUT.index, http_put},
        {Constants::CORE_FN_HTTP_PATCH.index, http_patch},
        {Constants::CORE_FN_HTTP_DELETE.index, http_delete},
        {Constants::CORE_FN_HTTP_ON.index, http_on},
        {Constants::CORE_FN_HTTP_LISTEN.index, http_listen},
        // ===== BOX
        {Constants::CORE_FN_BOX.index, box_create},
        {Constants::CORE_FN_BOX_UNPACK.index, box_unpack},
        {Constants::CORE_FN_BOX_PACK.index, box_pack},
};

} // namespace core
