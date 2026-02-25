#pragma once
#include "../interned_string.hpp"
#include "../symbol.hpp"

static Symbol _SYM_TRUE = create_symbol("#true");
static Symbol _SYM_T = create_symbol("#t");
static Symbol _SYM_FALSE = create_symbol("#false");
static const InternedString _BUILDIN_FN_PRINT = create_interned_string("print");
static const InternedString _BUILDIN_FN_LIST = create_interned_string("list");
static const InternedString _BUILDIN_FN_PUT = create_interned_string("put");
static const InternedString _BUILDIN_FN_AT = create_interned_string("at");
static const InternedString _BUILDIN_FN_APPEND =
    create_interned_string("append");
static const InternedString _BUILDIN_FN_PREPEND =
    create_interned_string("prepend");
static const InternedString _BUILDIN_FN_CONCAT =
    create_interned_string("concat");
