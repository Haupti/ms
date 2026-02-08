# list of possible optimizations
* [small] symbols are stored with their prefix char '#'. dont do this -> comparison saves one char to compare AND 1 char less memory per unique symbol
* [small] interned strings are stored as `uint64_t` that might not be necessary, maybe `uint16_t` is sufficient? -> less memory per string, faster comparison etc
