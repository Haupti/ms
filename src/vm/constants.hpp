#pragma once
#include "../interned_string.hpp"
#include "../symbol.hpp"

namespace Constants {
// macros
static const InternedString MACRO_FILE = create_interned_string("__FILE__");
static const InternedString MACRO_MAIN = create_interned_string("__MAIN__");
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
static const Symbol SYM_T_TABLE = create_symbol("#table");

// fn names
static const InternedString CORE_FN_PRINT = create_interned_string("print");
static const InternedString CORE_FN_COPY = create_interned_string("copy");
static const InternedString CORE_FN_TYPEOF = create_interned_string("typeof");
static const InternedString CORE_FN_PANIC = create_interned_string("panic");
static const InternedString CORE_FN_ERROR = create_interned_string("error");
static const InternedString CORE_FN_STR_CONCAT = create_interned_string("<>");

static const InternedString CORE_FN_TABLE = create_interned_string("table");
static const InternedString CORE_FN_TABLE_KEYS = create_interned_string("table_keys");
static const InternedString CORE_FN_TABLE_VALUES = create_interned_string("table_values");

static const InternedString CORE_FN_LIST = create_interned_string("list");
static const InternedString CORE_FN_PUT = create_interned_string("put");
static const InternedString CORE_FN_AT = create_interned_string("at");
static const InternedString CORE_FN_APPEND = create_interned_string("append");
static const InternedString CORE_FN_PREPEND = create_interned_string("prepend");
static const InternedString CORE_FN_LINK = create_interned_string("link");
static const InternedString CORE_FN_LEN = create_interned_string("len");
static const InternedString CORE_FN_INT = create_interned_string("int");
static const InternedString CORE_FN_FLOAT = create_interned_string("float");
static const InternedString CORE_FN_STR = create_interned_string("str");
static const InternedString CORE_FN_ASSERT = create_interned_string("assert");
static const InternedString CORE_FN_ASSERTTYPE = create_interned_string("assert_type");

static const InternedString CORE_FN_PROCESS_ARGS = create_interned_string("process_args");

static const InternedString CORE_FN_SYS_ENV_GET = create_interned_string("sys_env_get");
static const InternedString CORE_FN_SYS_EXIT = create_interned_string("sys_exit");
static const InternedString CORE_FN_SYS_EXEC = create_interned_string("sys_exec");
static const InternedString CORE_FN_SYS_NOW = create_interned_string("sys_now");
static const InternedString CORE_FN_SYS_IS_TTY = create_interned_string("sys_is_tty");
static const InternedString CORE_FN_SYS_HAS_COLOR = create_interned_string("sys_has_color");
static const InternedString CORE_FN_SYS_SLEEP = create_interned_string("sys_sleep");
static const InternedString CORE_FN_SYS_TERM_WIDTH = create_interned_string("sys_term_width");
static const InternedString CORE_FN_SYS_TERM_HEIGHT = create_interned_string("sys_term_height");

static const InternedString CORE_FN_RANDOM = create_interned_string("random");

static const InternedString CORE_FN_TIME_EPOCH_MS = create_interned_string("time_epoch_ms");
static const InternedString CORE_FN_TIME_EPOCH_SEC = create_interned_string("time_epoch_sec");
static const InternedString CORE_FN_TIME_ISO8601 = create_interned_string("time_iso8601");

static const InternedString CORE_FN_STR_SPLIT = create_interned_string("str_split");
static const InternedString CORE_FN_STR_REPLACE = create_interned_string("str_replace");
static const InternedString CORE_FN_STR_CONTAINS = create_interned_string("str_contains");
static const InternedString CORE_FN_STR_HAS_PREFIX = create_interned_string("str_has_prefix");
static const InternedString CORE_FN_STR_HAS_SUFFIX = create_interned_string("str_has_suffix");
static const InternedString CORE_FN_STR_LOWER = create_interned_string("str_lower");
static const InternedString CORE_FN_STR_UPPER = create_interned_string("str_upper");
static const InternedString CORE_FN_STR_TRIM = create_interned_string("str_trim");
static const InternedString CORE_FN_STR_TRIM_LEFT = create_interned_string("str_trim_left");
static const InternedString CORE_FN_STR_TRIM_RIGHT = create_interned_string("str_trim_right");
static const InternedString CORE_FN_STR_SLICE = create_interned_string("str_slice");
static const InternedString CORE_FN_STR_FIND = create_interned_string("str_find");
static const InternedString CORE_FN_STR_INDEX = create_interned_string("str_index");
static const InternedString CORE_FN_STR_FMT = create_interned_string("str_fmt");

static const InternedString CORE_FN_MATH_ABS = create_interned_string("math_abs");
static const InternedString CORE_FN_MATH_FLOOR = create_interned_string("math_floor");
static const InternedString CORE_FN_MATH_CEIL = create_interned_string("math_ceil");
static const InternedString CORE_FN_MATH_ROUND = create_interned_string("math_round");
static const InternedString CORE_FN_MATH_SQRT = create_interned_string("math_sqrt");
static const InternedString CORE_FN_MATH_POW = create_interned_string("math_pow");
static const InternedString CORE_FN_MATH_SIN = create_interned_string("math_sin");
static const InternedString CORE_FN_MATH_COS = create_interned_string("math_cos");
static const InternedString CORE_FN_MATH_TAN = create_interned_string("math_tan");
static const InternedString CORE_FN_MATH_LOG = create_interned_string("math_log");
static const InternedString CORE_FN_MATH_EXP = create_interned_string("math_exp");

static const InternedString CORE_FN_FS_EXISTS = create_interned_string("fs_exists");
static const InternedString CORE_FN_FS_MKDIR = create_interned_string("fs_mkdir");
static const InternedString CORE_FN_FS_RM = create_interned_string("fs_rm");
static const InternedString CORE_FN_FS_LS = create_interned_string("fs_ls");
static const InternedString CORE_FN_FS_READ = create_interned_string("fs_read");
static const InternedString CORE_FN_FS_WRITE = create_interned_string("fs_write");
static const InternedString CORE_FN_FS_APPEND = create_interned_string("fs_append");
static const InternedString CORE_FN_FS_COPY = create_interned_string("fs_copy");
static const InternedString CORE_FN_FS_MOVE = create_interned_string("fs_move");
static const InternedString CORE_FN_FS_STAT = create_interned_string("fs_stat");

static const InternedString CORE_FN_BIT_SHIFT_LEFT = create_interned_string("bit_shift_left");
static const InternedString CORE_FN_BIT_SHIFT_RIGHT = create_interned_string("bit_shift_right");
static const InternedString CORE_FN_BIT_OR = create_interned_string("bit_or");
static const InternedString CORE_FN_BIT_AND = create_interned_string("bit_and");
static const InternedString CORE_FN_BIT_XOR = create_interned_string("bit_xor");

static const InternedString CORE_FN_LIST_SLICE = create_interned_string("list_slice");
static const InternedString CORE_FN_LIST_REMOVE = create_interned_string("list_remove");
static const InternedString CORE_FN_LIST_CONTAINS = create_interned_string("list_contains");
static const InternedString CORE_FN_RANGE = create_interned_string("range");

static const InternedString CORE_FN_ANSI_COLOR = create_interned_string("ansi_color");
static const InternedString CORE_FN_ANSI_RESET = create_interned_string("ansi_reset");
static const InternedString CORE_FN_ANSI_SET_CURSOR = create_interned_string("ansi_set_cursor");
static const InternedString CORE_FN_ANSI_MOVE_CURSOR = create_interned_string("ansi_move_cursor");
static const InternedString CORE_FN_ANSI_CLEAR_LINE = create_interned_string("ansi_clear_line");
static const InternedString CORE_FN_ANSI_CLEAR_SCREEN = create_interned_string("ansi_clear_screen");
static const InternedString CORE_FN_ANSI_CLEAR = create_interned_string("ansi_clear");

static const InternedString CORE_FN_REGEX_MATCH = create_interned_string("regex_match");
static const InternedString CORE_FN_REGEX_HAS_MATCH = create_interned_string("regex_has_match");
static const InternedString CORE_FN_REGEX_REPLACE = create_interned_string("regex_replace");

static const InternedString CORE_FN_BASE64_ENCODE = create_interned_string("base64_encode");
static const InternedString CORE_FN_BASE64_DECODE = create_interned_string("base64_decode");

static const InternedString CORE_FN_HEX_ENCODE = create_interned_string("hex_encode");
static const InternedString CORE_FN_HEX_DECODE = create_interned_string("hex_decode");
static const InternedString CORE_FN_BINARY_ENCODE = create_interned_string("binary_encode");
static const InternedString CORE_FN_BINARY_DECODE = create_interned_string("binary_decode");

// ansi symbols
static const Symbol SYM_ANSI_FG = create_symbol("#fg");
static const Symbol SYM_ANSI_BG = create_symbol("#bg");
static const Symbol SYM_ANSI_LINE = create_symbol("#line");
static const Symbol SYM_ANSI_LINE_FROM_START = create_symbol("#line_from_start");
static const Symbol SYM_ANSI_LINE_TO_END = create_symbol("#line_to_end");
static const Symbol SYM_ANSI_SCREEN = create_symbol("#screen");
static const Symbol SYM_ANSI_SCREEN_FROM_START = create_symbol("#screen_from_start");
static const Symbol SYM_ANSI_SCREEN_TO_END = create_symbol("#screen_to_end");
static const Symbol SYM_ANSI_BLACK = create_symbol("#black");
static const Symbol SYM_ANSI_RED = create_symbol("#red");
static const Symbol SYM_ANSI_GREEN = create_symbol("#green");
static const Symbol SYM_ANSI_YELLOW = create_symbol("#yellow");
static const Symbol SYM_ANSI_BLUE = create_symbol("#blue");
static const Symbol SYM_ANSI_MAGENTA = create_symbol("#magenta");
static const Symbol SYM_ANSI_CYAN = create_symbol("#cyan");
static const Symbol SYM_ANSI_WHITE = create_symbol("#white");
static const Symbol SYM_ANSI_BRIGHT_BLACK = create_symbol("#bright_black");
static const Symbol SYM_ANSI_BRIGHT_RED = create_symbol("#bright_red");
static const Symbol SYM_ANSI_BRIGHT_GREEN = create_symbol("#bright_green");
static const Symbol SYM_ANSI_BRIGHT_YELLOW = create_symbol("#bright_yellow");
static const Symbol SYM_ANSI_BRIGHT_BLUE = create_symbol("#bright_blue");
static const Symbol SYM_ANSI_BRIGHT_MAGENTA = create_symbol("#bright_magenta");
static const Symbol SYM_ANSI_BRIGHT_CYAN = create_symbol("#bright_cyan");
static const Symbol SYM_ANSI_BRIGHT_WHITE = create_symbol("#bright_white");

// fs symbols
static const Symbol SYM_FS_FILE = create_symbol("#file");
static const Symbol SYM_FS_DIRECTORY = create_symbol("#directory");
static const Symbol SYM_FS_SYMLINK = create_symbol("#symlink");
static const Symbol SYM_FS_OTHER = create_symbol("#other");
} // namespace Constants
