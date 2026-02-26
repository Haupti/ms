#pragma once
#include "../interned_string.hpp"
#include "../symbol.hpp"

static const Symbol _SYM_TRUE = create_symbol("#true");
static const Symbol _SYM_T = create_symbol("#t");
static const Symbol _SYM_FALSE = create_symbol("#false");
static const InternedString _BUILDIN_FN_PRINT = create_next_label("print");
static const InternedString _BUILDIN_FN_LIST = create_next_label("list");
static const InternedString _BUILDIN_FN_PUT = create_next_label("put");
static const InternedString _BUILDIN_FN_AT = create_next_label("at");
static const InternedString _BUILDIN_FN_APPEND = create_next_label("append");
static const InternedString _BUILDIN_FN_PREPEND = create_next_label("prepend");
static const InternedString _BUILDIN_FN_CONCAT = create_next_label("concat");
