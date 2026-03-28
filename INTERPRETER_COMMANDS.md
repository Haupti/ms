# The MSL Interpreter CLI

There are not many options at the moment.
* **--credits** prints the credits/licenses used for the project
* **--dump-vm** makes the interpreter run the compilation but not evaluate the final VM instructions but instead dump them to stdout.

Otherwise the interpreter expects just the path to the entrypoint script and all arguments after that are passed to the script.


```
exe myscript.msl --hello
```
