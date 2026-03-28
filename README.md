# MSL: A Mini Script Language

MSL is a lightweight, dynamically-typed scripting language designed as an alternative to tools like Bash or Python for glue code, automation scripts, and other quick tasks. It aims for simplicity and ease of use with a powerful set of built-in core functions, making installation straightforward.

## Project Overview

This project is a personal endeavor to explore language implementation techniques. It features a stack-based Virtual Machine (VM), a custom heap memory model (LAoTs Heap), and a garbage collector (mark and sweep). The language pipeline includes a preprocessor, parser, compiler, and VM.

## Key Features

-   **Dynamically Typed**: Variables do not require explicit type declarations.
-   **Core Functions**: A rich set of built-in functions for various tasks.
-   **VM & GC**: Implementation of a stack-based Virtual Machine with a custom heap and a mark-and-sweep garbage collector.
-   **Preprocessor**: Handles directives like includes and constants.

## Technologies
-   **Language**: C++17
-   **Build Tool**: `asap` (custom C++ build tool)
-   **JSON Parsing**: PicoJSON (included in `src/vendor`)

## Building and Running

The project uses the `asap` build tool which you can find [here](https://github.com/Haupti/asapcpp) (I made it).\
Since this project includes all dependencies and otherwise only depends on the STL (C++17) it should be straight forward to build it without asap as well.
However, at the moment, i have nothing prepared for you.

## Core Components

The MSL interpreter follows a typical compilation pipeline:
1.  **Preprocessor (`src/preprocessor/`)**: Tokenizes source code and handles macros/includes.
2.  **Parser (`src/parser/`)**: Converts tokens into an Abstract Syntax Tree (AST).
3.  **IR Compiler (`src/ir/`)**: Lowers AST nodes into Intermediate Representation (IR).
4.  **VM Compiler (`src/instr/`)**: Translates IR into VM bytecode.
5.  **VM (`src/vm/`)**: Executes the bytecode using a stack-based architecture and a custom heap.

## Development Conventions

-   **Source Code**: All core implementation files reside in `src/`.
-   **Utilities**: Asap-specific utilities are in `lib/asap/` (includes when creating a project).
-   **MSL Resources**: Example scripts and MSL-level tests are in `res/`.
-   **C++ Tests**: Unit tests for components are in `tests/`.
-   **Coding Style**: Prioritizes performance and cache-friendliness. Uses index-based referencing (`VMHIDX`, `StringIdx`) for heap objects.

## Further Documentation

-   [Language Reference Manual](./REFERENCE_MANUAL.md): Details language syntax, features, and functions.
-   [Interpreter Commands](./INTERPRETER_COMMANDS.md): Explains available command-line options and usage for the interpreter.

## Credits

To view project credits and licenses for used libraries, run the interpreter with the `--credits` flag:
  ./msl --credits
