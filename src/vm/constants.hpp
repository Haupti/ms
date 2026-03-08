#pragma once
#include "../interned_string.hpp"
#include "../symbol.hpp"

namespace Constants {
static const Symbol SYM_TRUE = create_symbol("#true");
static const Symbol SYM_T = create_symbol("#t");
static const Symbol SYM_FALSE = create_symbol("#false");
static const Symbol SYM_TYPE_ERROR = create_symbol("#error");
static const InternedString BUILDIN_FN_PRINT = create_interned_string("print");
static const InternedString BUILDIN_FN_LIST = create_interned_string("list");
static const InternedString BUILDIN_FN_PUT = create_interned_string("put");
static const InternedString BUILDIN_FN_AT = create_interned_string("at");
static const InternedString BUILDIN_FN_APPEND =
    create_interned_string("append");
static const InternedString BUILDIN_FN_PREPEND =
    create_interned_string("prepend");
static const InternedString BUILDIN_FN_CONCAT =
    create_interned_string("concat");
static const InternedString BUILDIN_FN_PANIC = create_interned_string("panic");
static const InternedString BUILDIN_FN_STR_CONCAT =
    create_interned_string("<>");
static const InternedString EXPECT_ERROR_MSG =
    create_interned_string("expected non-error value");
} // namespace Constants
