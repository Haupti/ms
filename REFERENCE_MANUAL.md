# MSL Reference Manual

 This document provides a comprehensive reference for the MSL language, covering its syntax, core features, and built-in functions.

 ## 1. Language Syntax

 MSL employs a C-style syntax with some scripting language influences.

 ### 1.1. Comments
Single-line comments start with `;` and continue to the end of the line or the next `;`.
```
;  This is a comment
1 + ; this is a comment ; 2
```

### 1.2. Literals
* **Integers**: Standard decimal integers.
```
123
-45
```

* **Floats**: Standard floating-point numbers.
```
123.45
-0.5
```
* **Strings**: Enclosed in double quotes. Escape sequences like `\n`, `\t`, `\r`, `\"`, and `\\` are supported.
```
"Hello, world!"
"A string with a newline:\nAnd a tab:\t"
```

* **Booleans**: `#true` and `#false`.
* **Null**: `none`

### 1.3. Variables and Assignments
Variables are declared using `let`.
```
let my_variable = 10
let message = "Hello"
```
Variables can be set via the `set` keyword:
```
let x = 5
set x = 10
```
Its not required that a variable keeps its original type.
```
let x = 5
set x = #some_symbol 
```
That is also valid.

### 1.4. Operators
* **Arithmetic**: `+`, `-`, `*`, `/`
* **Comparison**: `==`, `!=`, `<`, `>`, `<=`, `>=`
* **Logical**: `&&` (AND), `||` (OR), `!` (NOT)
* **String Concatenation**: `<>`
* **Bitwise**: `<<` (left shift), `>>` (right shift), `|` (OR), `&` (AND), `^` (XOR)

 ### 1.5. Control Flow
**Conditional Statements**:
 ```
      if (condition) {
          // code block
      } elif (condition2) {
          // another code block
      } else {
          // default code block
      }
```
**Loops**:
```
      for variable in list {
          // code block
      }
```


### 1.6. Functions
Functions are simply defined like that:
```
function add(a,b){
    return a + b
}
```
Note that a function must not return on all branches because it by default returns `none` value, if nothing else is returned.
Functions can be called directly by their name.
 ```
  print("Hello")
```

### 1.7. Scopes
Its very easy to spot what creates a lexical scope: `{}` brackets do.
So conditionals, loops and functions do create their own lexical scopes.

### 1.8. Preprocessor Includes 
Other files can be included using `$include`.
```
  $include "other_file.msl"
```
This is kind of like the C-Preprocessor macro. That means anything can be included and by this is simply copy-pasted in.
BUT the preprocessor checks that the file is not included multiple times.

### 1.9. Preprocessor Flags 
Conditional compilation directives are supported via flags.
```
  $iffset __MAIN__
      // Code specific to main execution
  $endif
```
Where `__MAIN__` is a flag that is automatically set by the preprocessor/interpreter.
But you can set your own flags:
```
$fset SOMETHING
$funset SOMETHING
```
This sets and unsets the flag.

Flags alre only valid in the file they are defined in!

### 1.10. Preprocessor Constants
You can define a constant (literal) value via `define` directive:
```
$define PI = 3
print(PI)
```
This is also a literal replacement by the preprocessor. Definitions are also only valid in the file they are defined in.

## 2. Core Language Features

### 2.1. Data Types
-   **INT**: Signed 64-bit integers.
-   **FLOAT**: Double-precision floating-point numbers.
-   **STRING**: UTF-8 encoded strings.
-   **SYMBOL**: Internal identifiers, often used for keywords, types, and enum-like constants (e.g., `#true`, `#false`, `#file`).
-   **LIST**: Ordered, mutable collection of values. Created with `list(...)`.
-   **TABLE**: Unordered key-value store. Keys can be symbols or strings. Created with `table(...)`. Keys are internally mapped.
-   **NONE**: Represents the absence of a value, similar to `null`.
-   **ERROR**: Represents an error value.
-   **ITERATOR**: Represents an iterator (used internally, e.g., by `for` loops).

### 2.2. Control Flow
-   **`if`/`elif`/`else`**: For conditional execution.
-   **`for...in`**: For iterating over lists.

### 2.3. Symbols
Symbols are unique identifiers used for internal constants, keywords, and specific values like `#true`, `#false`, `#error`, and type identifiers (e.g., `#int`, `#string`). They are distinct from strings and are often used for performance and type checking.

### 2.4. Left-Apply Operator (`.`) and the Right-Apply Operator (`|`)
The left-apply operator (or dot) is used to pass the left argument as the first argument of the function call to the right:
```
; these are equivalent:
1.add(2)
add(1,2)
```
The right-apply operator (or pipe) is used to pass the left argument as the last argument of the function call to the right:
```
; these are equivalent:
1 | add(2)
add(2,1)
```

### 2.5. String Concatenation (`<>`)
The `<>` operator concatenates two strings.
```
 let greeting = "Hello" <> " " <> "World" ; "Hello World"
```
   
### 2.6. `try`/`expect`
For compy error handling there are the **try** and **expect** operators.
The **try** operator evaluates the right hand side expression and if that returns an error value, the enclosing function returns with the error value immediately.
```
function will_fail(){
    return error("fail")
}
function might_fail(){
    try will_fail()
    print("worked")
}
```
in this example the `might_fail` function will not print anything, because the call above faild.
It is therefore equivalent to something like that:
```
function might_fail(){
    let res = will_fail()
    if(typeof(res) == #error){
        return res
    }
    print("worked")
}
```
The **expect** operator on the other hand is so you can crash the program conventiently when you did not expect something to fail.
E.g.
```
function might_fail(){
    expect will_fail()
    print("worked")
}
```
will also never print anything but also the whole program is terminated with a runtime panic.
It is therefor equivalent to something like that:
```
function might_fail(){
    let res = will_fail()
    if(typeof(res) == #error){
        panic("did not expect an error: " <> str(res))
    }
    print("worked")
}
```


## 3. Core Functions

Core functions are built-in functions available in all MSL scripts. They are typically called directly by name.

### 3.1. Print & Error Handling
-   **`print(value)`**: Prints the string representation of a value to standard output.
-   **`make_error(value)`**: Creates an error value from another value.
-   **`panic(value)`**: Throws a runtime error with the given value and terminates the VM.
-   **`assert(condition)`**: If `condition` is false, throws a runtime error.
-   **`assert_type(value, expected_type_symbol)`**: Checks if a value is of the expected type (symbol).

### 3.2. Table & List Operations
-   **`table(key1, value1, key2, value2, ...)`**: Creates a new table with the given key-value pairs.
-   **`table_keys(table)`**: Returns a list of all keys in the table.
-   **`table_values(table)`**: Returns a list of all values in the table.
-   **`table_to_json(table)`**: Serializes a table (and its contents) into a JSON string.
-   **`table_from_json(json_string)`**: Parses a JSON string into an MSL table (or other MSL value).
-   **`list(item1, item2, ...)`**: Creates a new list with the given items.
-   **`put(container, key, value)`**: Adds or updates a key-value pair in a table or sets a list value at index 'key'.
-   **`at(container, key_or_index)`**: Retrieves a value from a list (using integer index) or a table (using key). Returns `none` if the key/index is not found.
-   **`append(list, item)`**: Adds an item to the end of a list.
-   **`prepend(list, item)`**: Adds an item to the beginning of a list.
-   **`link(list, item)`**: Adds an item to the end of a list (similar to append).
-   **`list_slice(list, start_index, end_index)`**: Returns a new list containing a sub-sequence of the original list.
-   **`list_remove(list, item)`**: Removes the first occurrence of `item` from the list.
-   **`list_contains(list, item)`**: Returns `#true` if the list contains `item`, `#false` otherwise.
-   **`range(start_int, end_int)`**: Creates a list of integers from `start` up to (but not including) `end`.

### 3.3. Utilities
-   **`copy(value)`**: Creates a deep copy of a value.
-   **`typeof(value)`**: Returns a symbol representing the type of the value (e.g., `#int`, `#string`).
-   **`error(value)`**: Creates an error value.
-   **`{value1} <> {value2}`**: Concatenates two strings.
-   **`len(collection_or_string)`**: Returns the length of a string, list, or table.
-   **`int(value)`**: Converts a value to an integer.
-   **`float(value)`**: Converts a value to a float.
-   **`str(value)`**: Converts a value to its string representation.

### 3.4. Process
-   **`process_args()`**: Returns a list of command-line arguments passed to the script.

### 3.5. System
-   **`sys_env_get(variable_name)`**: Retrieves an environment variable.
-   **`sys_exit(exit_code)`**: Exits the script with a given status code.
-   **`sys_exec(command_string, ...args)`**: Executes an external command.
-   **`sys_now()`**: Returns the current system time in nanoseconds since epoch.
-   **`sys_is_tty()`**: Returns `#true` if the script is running in a TTY (terminal).
-   **`sys_has_color()`**: Returns `#true` if the terminal supports color.
-   **`sys_sleep(seconds_float)`**: Pauses execution for a specified duration.
-   **`sys_term_width()`**: Returns the width of the terminal.
-   **`sys_term_height()`**: Returns the height of the terminal.

### 3.6. Random
-   **`random(min_int, max_int)`**: Returns a random integer between `min` (inclusive) and `max` (inclusive).

### 3.7. Time
-   **`time_epoch_ms()`**: Returns current time in milliseconds since epoch.
-   **`time_epoch_sec()`**: Returns current time in seconds since epoch.
-   **`time_iso8601()`**: Returns current time as an ISO 8601 formatted string.

### 3.8. String Manipulation
-   **`str_split(str, delimiter)`**: Splits a string by a delimiter.
-   **`str_replace(str, search, replace)`**: Replaces all occurrences of `search` with `replace` in `str`.
-   **`str_contains(str, substring)`**: Returns `#true` if `str` contains `substring`.
-   **`str_has_prefix(str, prefix)`**: Returns `#true` if `str` starts with `prefix`.
-   **`str_has_suffix(str, suffix)`**: Returns `#true` if `str` ends with `suffix`.
-   **`str_lower(str)`**: Converts `str` to lowercase.
-   **`str_upper(str)`**: Converts `str` to uppercase.
-   **`str_trim(str)`**: Removes leading/trailing whitespace from `str`.
-   **`str_trim_left(str)`**: Removes leading whitespace.
-   **`str_trim_right(str)`**: Removes trailing whitespace.
-   **`str_slice(str, start_index, end_index)`**: Returns a sub-string.
-   **`str_find(str, substring)`**: Returns the index of the first occurrence of `substring` in `str`, or `none` if not found.
-   **`str_index(str, substring)`**: Returns the index of the first occurrence of `substring` in `str`.
-   **`str_fmt(format_string, arg1, arg2, ...)`**: Formats a string similar to Python's f-strings or C#'s `string.Format`. Uses `{}` placeholders.
-   **`str_url_encode(str)`**: Percent-encodes a string for URL safety.
-   **`str_url_decode(str)`**: Decodes a percent-encoded string, also converting `+` to spaces.

### 3.9. Math
-   **`math_abs(number)`**: Returns the absolute value.
-   **`math_floor(number)`**: Returns the largest integer less than or equal to the number.
-   **`math_ceil(number)`**: Returns the smallest integer greater than or equal to the number.
-   **`math_round(number)`**: Rounds to the nearest integer.
-   **`math_sqrt(number)`**: Returns the square root.
-   **`math_pow(base, exponent)`**: Returns `base` raised to the power of `exponent`.
-   **`math_sin(radians)`**: Sine of an angle in radians.
-   **`math_cos(radians)`**: Cosine of an angle in radians.
-   **`math_tan(radians)`**: Tangent of an angle in radians.
-   **`math_log(number)`**: Natural logarithm.
-   **`math_exp(number)`**: Returns e raised to the power of the number.

### 3.10. File System
-   **`fs_exists(path)`**: Returns `#true` if the path exists, `#false` otherwise.
-   **`fs_mkdir(path)`**: Creates a directory.
-   **`fs_rm(path)`**: Removes a file or directory (recursively for directories).
-   **`fs_ls(path)`**: Returns a list of files and directories within a given path.
-   **`fs_read(path)`**: Reads the entire content of a file into a string.
-   **`fs_write(path, content)`**: Writes content to a file, overwriting it if it exists.
-   **`fs_append(path, content)`**: Appends content to a file.
-   **`fs_copy(source_path, destination_path)`**: Copies a file or directory.
-   **`fs_move(source_path, destination_path)`**: Moves/renames a file or directory.
-   **`fs_stat(path)`**: Returns a table with file metadata (`#exists`, `#type`, `#size`, `#mtime`).

### 3.11. Bitwise Operations
-   **`bit_shift_left(number, bits)`**: Left shifts a number.
-   **`bit_shift_right(number, bits)`**: Right shifts a number.
-   **`bit_or(a, b)`**: Bitwise OR.
-   **`bit_and(a, b)`**: Bitwise AND.
-   **`bit_xor(a, b)`**: Bitwise XOR.

### 3.12. ANSI Terminal
-   **`ansi_color(color_symbol, text_string)`**: Applies ANSI color to text. (Color symbols are like `#red`, `#bright_blue`, etc.)
-   **`ansi_reset()`**: Resets terminal colors.
-   **`ansi_set_cursor(row, col)`**: Moves cursor to specified row/column.
-   **`ansi_move_cursor(delta_row, delta_col)`**: Moves cursor relative to current position.
-   **`ansi_clear_line()`**: Clears the current line.
-   **`ansi_clear_screen()`**: Clears the terminal screen.
-   **`ansi_clear(clear_symbol)`**: Clears parts of the screen (e.g., `#line`, `#screen`).

### 3.13. Regex
-   **`regex_has_match(string, pattern)`**: Returns `#true` if the string matches the regex pattern.
-   **`regex_match(string, pattern)`**: Returns the matched string (or first match) if the pattern matches.
-   **`regex_replace(string, pattern, replacement)`**: Replaces occurrences of the pattern in the string with the replacement.

### 3.14. Base64
-   **`base64_encode(str)`**: Encodes a string to Base64.
-   **`base64_decode(str)`**: Decodes a Base64 string.

### 3.15. Hexadecimal & Binary
-   **`hex_encode(integer)`**: Converts a positive integer to its hex string representation.
                                                                                                     
