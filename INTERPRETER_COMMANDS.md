# MSL Interpreter CLI Reference

This document outlines the available command-line options and arguments for the MSL interpreter.

## Command-Line Options

The MSL interpreter currently supports the following command-line options:

*   **`--credits`**: Prints project credits and license information.
*   **`--dump-vm`**: Compiles the script but does not evaluate the final VM instructions. Instead, it dumps the generated VM instructions to standard output. This is useful for debugging the compilation process.

## Running MSL Scripts

To run an MSL script, provide the path to the entrypoint script followed by any arguments that should be passed to the script itself.

**Example:**
```bash
./msl myscript.msl --arg1 "some value" --flag
```

In this example:
-   `./msl` is the interpreter executable.
-   `myscript.msl` is the entrypoint script.
-   `--arg1 "some value"` and `--flag` are arguments passed to the `myscript.msl` script, which can be accessed via `process_args()`.
