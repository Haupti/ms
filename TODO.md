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
* [DONE] `list_slice(l, start, end)`
* [DONE] `list_remove(l, i)` - remove element at index
* [DONE] `list_contains(l, val)`
* [DONE] `range(i,j)` -> create a list of numbers including i but excluding j

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
* [DONE] `bit_shift_left(i, n)`, `bit_shift_right(i,n)`
* [DONE] `bit_or(i, j)`, `bit_and(i, j)`, `bit_xor(i, j)`

### network
* [] `http_request(r)` where r is a table
* [] tcp / upd
* [] database connection?

### terminal support stuff
* [DONE] ansi colors
* [DONE] ansi reset
* [DONE] ansi clears (line, screen, ...)
* [DONE] ansi cursor controls
* [DONE] terminal informations e.g. which terminal? which colors? etc

### other
* [] coroutines? that is wyld but would be fun

## language & vm features
* [DONE] verify variables and functions that are referenced actually exist
* [DONE] `for` loop 
* [] `break` loop keyword
* [] `continue` loop keyword
* [] `while` loop
* [] structs or records for grouping data
* [DONE] garbage collection

## flags/marcros set by the compiler
* [DONE] filename macro replaced at compile time with the LocationRef's filename
* [DONE] main macro: #true if that file was passed to the interpreter as entrypoint, otherwise #false
* [DONE] main flag: set if that file was passed to the interpreter as entrypoint, otherwise not set
* [] os flags: win, posix
* [] line macro: is replaced with the line number of the file its in
* [] column macro: is replaced with the column number 
* [] make flags defineable as interpreter arguments via e.g. "--flag FLAG1" and such. (repeatable)
* [] make macro values defineable as interpreter arguments via e.g. "$define SOMENAME <somevalue>" (repeatable)

