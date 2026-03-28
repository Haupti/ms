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
* [DONE] `str_split(s, sep)`
* [DONE] `str_trim(s)`
* [DONE] `str_trim_left(s)`, `str_trim_right(s)`
* [DONE] `str_replace(s, old, new)`
* [DONE] `str_contains(s, sub)`
* [DONE] `str_starts_with(s, prefix)`, `str_ends_with(s, suffix)`
* [] `str_join(list, sep)`

### Regexp 
* [DONE] `regex_match(re, str)`
* [DONE] `regex_replace(re, new, str)`
* [DONE] `regex_has_match(re, str)`
* [] `=~` operator which expects a string left and regexp right (also a string)

### list operations
* [DONE] `list_slice(l, start, end)`
* [DONE] `list_remove(l, i)` - remove element at index
* [DONE] `list_contains(l, val)`
* [DONE] `range(i,j)` -> create a list of numbers including i but excluding j
* [DONE] `list_append(l, val)` - add to end
* [] `list_pop(l)` - remove and return last
* [] `list_sort(l)`

### tables
* [DONE] implement type `ValueTag::TABLE` (hash map)
* [DONE] `table()` - create empty dictionary
* [DONE] `table_get(t, key)` -> rewrite `list_at` to work with any containers + rename to `container_at`
* [DONE] `table_set(t, key, val)` -> rewrite `list_put` to work with any containers + rename to `container_put`
* [DONE] `table_keys(t)` - return list of keys
* [DONE] `table_values(t)` - return list of values
* [DONE] `table_remove(t, key)` -> rewrite `list_remove` to work with any container
* [] `table_merge(t1, t2)`
* [DONE] `table_len(t)` - lists number of entries

### file system 
* [DONE] `fs_exists(path)`
* [DONE] `fs_mkdir(path)`
* [DONE] `fs_rm(path)`
* [DONE] `fs_ls(path)` - list directory contents
* [DONE] `fs_read(path)` - read entire file to string
* [DONE] `fs_write(path, content)` - write string to file
* [DONE] `fs_append(path, content)` - append string to file
* [DONE] `fs_stat(path)` - return table with size, mtime, is_dir, etc.
* [DONE] `fs_copy(src, dst)`
* [DONE] `fs_move(src, dst)`

### system
* [DONE] `sys_now()` - high-res timestamp
* [DONE] `sys_exec(cmd, args)` - run command, return {exit_code, stdout, stderr}
* [DONE] `sys_env_get(key)`, `sys_env_set(key, val)`
* [DONE] `process_args()` - return list of command line arguments passed to script
* [DONE] `sys_exit(code)`
* [DONE] `sys_sleep(ms)`
* [] `sys_spawn(cmd, args)` - run command in background, return pid/handle

### encoding
* [DONE] `table_to_json(t)`, `table_from_json(json_string)`
* [DONE] `str_url_encode(s)`, `str_url_decode(s)`
* [DONE] `base64_encode(s)`, `base64_decode(s)`
* [] `table_from_csv(csv_string, separator)`

### binary
* [DONE] `bit_shift_left(i, n)`, `bit_shift_right(i,n)`
* [DONE] `bit_or(i, j)`, `bit_and(i, j)`, `bit_xor(i, j)`

### number formats
* [DONE] `hex_encode(s)`, `hex_decode(s)`
* [DONE] `binary_encode(s)`, `binary_decode(s)`

### network
* [] `http_request(method, url, body, headers)` -> returns table {status, body, headers}
* [] `http_get(url)`, `http_post(url, body)` wrappers
* [] `tcp_connect(host, port)` -> returns socket handle
* [] `tcp_listen(port)` -> returns server socket handle
* [] `socket_read(s, n)`, `socket_write(s, data)`, `socket_close(s)`
* [] `dns_resolve(host)`
* [] database connection? (e.g. `sqlite_open(path)`)

### terminal support stuff
* [DONE] ansi colors
* [DONE] ansi reset
* [DONE] ansi clears (line, screen, ...)
* [DONE] ansi cursor controls
* [DONE] terminal informations e.g. which terminal? which colors? etc
* [DONE] `term_width()`, `term_height()`
* [DONE] `term_is_tty()`

### time
* [] `time_parse(s, fmt)`, `time_format(t, fmt)`
* [DONE] `time_epoch_sec()`, `time_epoch_ms()`
* [DONE] `time_iso8601()`

### other
* [] coroutines/async? that is wyld but would be fun
* [] `path_join(...)`, `path_base(p)`, `path_dir(p)`, `path_ext(p)`

## language & vm features
* [DONE] verify variables and functions that are referenced actually exist
* [DONE] `for` loop 
* [DONE] `break` loop keyword
* [DONE] `continue` loop keyword
* [DONE] string escape codes for newline, tab, quotes
* [] `while` loop
* [] structs or records for grouping data
* [DONE] garbage collection
* [] optional/default arguments for functions
* [] string interpolation e.g. `"hello {name}"`
* [] improved stack traces on error
* [] debugger support (step by step execution)

## flags/marcros set by the compiler
* [DONE] filename macro replaced at compile time with the LocationRef's filename
* [DONE] main macro: #true if that file was passed to the interpreter as entrypoint, otherwise #false
* [DONE] main flag: set if that file was passed to the interpreter as entrypoint, otherwise not set
* [] os flags: win, posix
* [] line macro: is replaced with the line number of the file its in
* [] column macro: is replaced with the column number 
* [] make flags defineable as interpreter arguments via e.g. "--flag FLAG1" and such. (repeatable)
* [] make macro values defineable as interpreter arguments via e.g. "$define SOMENAME <somevalue>" (repeatable)


# Tables conceptual

a table is a pointer to a 'bucket' list. The bucket list is of size n (the initial hash modifier).
when a new value is inserted
* the hash is calculated via `std::hash(...) & n`
* a new heap slot is allocated for the value to insert
* the bucket slot at `hash & n` gets the idx to the value
    * on collision the current value gets a `next_child` set to the next value

when reading a value from the table its somewhat inverse
* the hash is calculated via `std::hash(...) & n`
* the value pointed to by the bucket slot `hash & n` is fetched and the key is compared with the current key
    * if match -> return
    * if not, get `next_child` and retry


so it 'looks' like that

```
Table(ref) -> [hidx1,0,hidx2]  (bucket)
                |        |
                v        v
              [val]    [val1, val2]
                |         |     |
                v         v     v
               key       key   key

```
