#pragma once
#include "../interned_string.hpp"
#include "../symbol.hpp"

namespace Constants {
// values
static const Symbol SYM_TRUE = create_symbol("#true");
static const Symbol SYM_FALSE = create_symbol("#false");
static const InternedString EXPECT_ERROR_MSG =
    create_interned_string("expected non-error value");

// types
static const Symbol SYM_T_ERROR = create_symbol("#error");
static const Symbol SYM_T_INT = create_symbol("#int");
static const Symbol SYM_T_FLOAT = create_symbol("#float");
static const Symbol SYM_T_STRING = create_symbol("#string");
static const Symbol SYM_T_SYMBOL = create_symbol("#symbol");
static const Symbol SYM_T_LIST = create_symbol("#list");
static const Symbol SYM_T_NONE = create_symbol("#none");
static const Symbol SYM_T_ITERATOR = create_symbol("#iterator");

// fn names
static const InternedString BUILDIN_FN_PRINT = create_interned_string("print");
static const InternedString BUILDIN_FN_LIST = create_interned_string("list");
static const InternedString BUILDIN_FN_PUT = create_interned_string("put");
static const InternedString BUILDIN_FN_AT = create_interned_string("at");
static const InternedString BUILDIN_FN_APPEND = create_interned_string("append");
static const InternedString BUILDIN_FN_PREPEND = create_interned_string("prepend");
static const InternedString BUILDIN_FN_LINK = create_interned_string("link");
static const InternedString BUILDIN_FN_COPY = create_interned_string("copy");
static const InternedString BUILDIN_FN_TYPEOF = create_interned_string("typeof");
static const InternedString BUILDIN_FN_PANIC = create_interned_string("panic");
static const InternedString BUILDIN_FN_ERROR = create_interned_string("error");
static const InternedString BUILDIN_FN_STR_CONCAT = create_interned_string("<>");
static const InternedString BUILDIN_FN_LEN = create_interned_string("len");
static const InternedString BUILDIN_FN_INT = create_interned_string("int");
static const InternedString BUILDIN_FN_FLOAT = create_interned_string("float");
static const InternedString BUILDIN_FN_STR = create_interned_string("str");
static const InternedString BUILDIN_FN_ASSERT = create_interned_string("assert");
static const InternedString BUILDIN_FN_ASSERTTYPE = create_interned_string("assert_type");
static const InternedString BUILDIN_FN_FILE_READ = create_interned_string("file_read");
static const InternedString BUILDIN_FN_FILE_WRITE = create_interned_string("file_write");
static const InternedString BUILDIN_FN_FILE_APPEND = create_interned_string("file_append");
static const InternedString BUILDIN_FN_SYS_ENV = create_interned_string("sys_env");
static const InternedString BUILDIN_FN_PROCESS_ARGS = create_interned_string("process_args");
static const InternedString BUILDIN_FN_SYS_EXIT = create_interned_string("sys_exit");
static const InternedString BUILDIN_FN_SYS_EXEC = create_interned_string("sys_exec");
static const InternedString BUILDIN_FN_RANDOM = create_interned_string("random");
static const InternedString BUILDIN_FN_TIME_EPOCH_MS = create_interned_string("time_epoch_ms");
static const InternedString BUILDIN_FN_TIME_EPOCH_SEC = create_interned_string("time_epoch_sec");
static const InternedString BUILDIN_FN_TIME_ISO8601 = create_interned_string("time_iso8601");
static const InternedString BUILDIN_FN_STR_SPLIT = create_interned_string("str_split");
static const InternedString BUILDIN_FN_STR_REPLACE = create_interned_string("str_replace");
static const InternedString BUILDIN_FN_STR_CONTAINS = create_interned_string("str_contains");
static const InternedString BUILDIN_FN_STR_HAS_PREFIX = create_interned_string("str_has_prefix");
static const InternedString BUILDIN_FN_STR_HAS_SUFFIX = create_interned_string("str_has_suffix");
static const InternedString BUILDIN_FN_STR_LOWER = create_interned_string("str_lower");
static const InternedString BUILDIN_FN_STR_UPPER = create_interned_string("str_upper");
static const InternedString BUILDIN_FN_STR_TRIM = create_interned_string("str_trim");

static const InternedString BUILDIN_FN_MATH_ABS = create_interned_string("math_abs");
static const InternedString BUILDIN_FN_MATH_FLOOR = create_interned_string("math_floor");
static const InternedString BUILDIN_FN_MATH_CEIL = create_interned_string("math_ceil");
static const InternedString BUILDIN_FN_MATH_ROUND = create_interned_string("math_round");
static const InternedString BUILDIN_FN_MATH_SQRT = create_interned_string("math_sqrt");
static const InternedString BUILDIN_FN_MATH_POW = create_interned_string("math_pow");
static const InternedString BUILDIN_FN_MATH_SIN = create_interned_string("math_sin");
static const InternedString BUILDIN_FN_MATH_COS = create_interned_string("math_cos");
static const InternedString BUILDIN_FN_MATH_TAN = create_interned_string("math_tan");
static const InternedString BUILDIN_FN_MATH_LOG = create_interned_string("math_log");
static const InternedString BUILDIN_FN_MATH_EXP = create_interned_string("math_exp");

static const InternedString BUILDIN_FN_STR_SLICE = create_interned_string("str_slice");
static const InternedString BUILDIN_FN_STR_FIND = create_interned_string("str_find");
static const InternedString BUILDIN_FN_STR_INDEX = create_interned_string("str_index");
static const InternedString BUILDIN_FN_STR_FMT = create_interned_string("str_fmt");

static const InternedString BUILDIN_FN_FS_EXISTS = create_interned_string("fs_exists");
static const InternedString BUILDIN_FN_FS_MKDIR = create_interned_string("fs_mkdir");
static const InternedString BUILDIN_FN_FS_RM = create_interned_string("fs_rm");
static const InternedString BUILDIN_FN_FS_LS = create_interned_string("fs_ls");
static const InternedString BUILDIN_FN_SYS_NOW = create_interned_string("sys_now");
} // namespace Constants
