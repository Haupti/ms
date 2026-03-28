# MSL Language

This is a small scripting language i made for myself as an alternative for e.g. bash or python scripts for writing of glue code and other small quick scripts.\
It is a dynamically typed language with a large set of buildin (core) functions so installation is as simple as possible.\
Also it is basically a project for me to try out various techniques such as buildin a VM, a garbage collector (mark and sweep) and a 'LAoTs' heap which is also used to represent lists and tables in.
If it is unfamiliar for you look at this video, which i got the idea from and explains it very well i think: https://www.youtube.com/watch?v=ShSGHb65f3M&t=1137s \
So in summary: this is a working but not 'complete' project in a sense that it is a programming language that you should use.

### The Reference Manual
[here](./REFERENCE_MANUAL.md) you can find the reference manual to the language itself

### The Interpreter
[here](./INTERPRETER_COMMANDS.md) you can find a manual on how to use the interpreter once you have it build.


## Under the hood.
Under the hood the language has a preprocessor (just because i liked it) which is used for concatenating all the script files to a single large input and various small other things like constants and flags.\
Then there is a 'compiler' which compiles the language to a sort of high-level virtual machine code which then is interpeted by the internal stack-based VM with a LAoTs Heap and mark and sweep GC.\

### Credits
To get the credits, run the interpreter with the `--credits` flag. This will print all the license notes from other libraries/projects i used.

### Compilation

I am using my own build tool 'asap' for this which you can find [here](https://github.com/Haupti/asapcpp). With that building the project is just a simple `asap build`.\
If you build it with something else (which should not be very difficult), you can do so. I used only the standard library and PicoJSON (included in src/vendor) so you can just build it.\
The project uses C++17 and the `#pragma once` directive. Otherwise its all standard i think.


