# TODO 

## core functions to add
### math
* [DONE] `math_abs(x)` - absolute value
* [DONE] `math_floor(x)`, `math_ceil(x)`, `math_round(x)`
* [DONE] `math_sqrt(x)`, `math_pow(x, y)`
* [DONE] `math_sin(x)`, `math_cos(x)`, `math_tan(x)`
* [DONE] `math_log(x)`, `math_exp(x)`

### String Manipulation
* [DONE] `str_slice(s, start, end)`
* [DONE] `str_find(s, sub)` - Return index of substring
* [DONE] `str_index(s, i)` - Get character at index (as string)
* [DONE] `str_fmt(fmt, ...)` - Basic string formatting

### list operations
* [] `list_slice(l, start, end)`
* [] `list_remove(l, i)` - remove element at index
* [] `list_contains(l, val)`
* [] `range(i,j)`

### tables
* [] implement type `ValueTag::TABLE` (hash map)
* [] `table()` - create empty dictionary
* [] `table_get(t, key)`
* [] `table_set(t, key, val)`
* [] `table_keys(t)` - return list of keys
* [] `table_remove(t, key)`

### file system & system
* [DONE] `fs_exists(path)`
* [DONE] `fs_mkdir(path)`
* [DONE] `fs_rm(path)`
* [DONE] `fs_ls(path)` - list directory contents
* [DONE] `sys_now()` - high-res timestamp

### encoding
* [] `table_to_json(t)`, `table_from_json(json_string)`
* [] `str_url_encode(s)`, `str_url_decode(s)`

### binary
* [] `binary_shift_left(i, n)`, `binary_shift_right(i,n)`
* [] `binary_or(i, i)`, `binary_and(i, i)`, `binary_xor(i, i)`

### network
* [] `http_request(r)` where r is a table
* [] tcp / upd
* [] database connection?

### other
* [] coroutines? that is wyld but would be fun

## language & vm features
* [DONE] verify variables and functions that are referenced actually exist
* [DONE] `for` loop 
* [] `break` loop keyword
* [] `continue` loop keyword
* [] `while` loop
* [] structs or records for grouping data
* [] binary operators for bitwise ops (`&`, `|`, `^`, `<<`, `>>`)
* [] garbage collection


