# TODO 

## core functions to add
### math
* [DONE] `math_abs(x)` - absolute value
* [DONE] `math_floor(x)`, `math_ceil(x)`, `math_round(x)`
* [DONE] `math_sqrt(x)`, `math_pow(x, y)`
* [DONE] `math_sin(x)`, `math_cos(x)`, `math_tan(x)`
* [DONE] `math_log(x)`, `math_exp(x)`

### string manipulation
* [] `str_slice(s, start, end)`
* [] `str_find(s, sub)` - return index of substring
* [] `str_index(s, i)` - get character at index (as string)
* [] `str_fmt(fmt, ...)` - basic string formatting

### list operations
* [] `list_slice(l, start, end)`
* [] `list_remove(l, i)` - remove element at index
* [] `list_contains(l, val)`
* [] `list_map(l, fn_name)` - apply function to all elements
* [] `list_filter(l, fn_name)` - filter elements based on predicate
* [] `list_reduce(l, fn_name, initial)`

### dictionary (new type needed)
* [] implement `valuetag::dict` (hash map)
* [] `dict()` - create empty dictionary
* [] `dict_get(d, key)`
* [] `dict_set(d, key, val)`
* [] `dict_keys(d)` - return list of keys
* [] `dict_remove(d, key)`

### file system & system
* [] `fs_exists(path)`
* [] `fs_mkdir(path)`
* [] `fs_rm(path)`
* [] `fs_ls(path)` - list directory contents
* [] `sys_now()` - high-res timestamp

## language & vm features
* [DONE] verify variables and functions that are referenced actually exist
* [] `loop` / `while` constructs in parser/compiler
* [] `for v in list` / `for i in range(start, end)`
* [] structs or records for grouping data
* [] binary operators for bitwise ops (`&`, `|`, `^`, `<<`, `>>`)


