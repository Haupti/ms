# list of possible optimizations
* [small] symbols are stored with their prefix char '#'. dont do this -> comparison saves one char to compare AND 1 char less memory per unique symbol
* [small] interned strings are stored as `uint64_t` that might not be necessary, maybe `uint16_t` is sufficient? -> less memory per string, faster comparison etc
* [large] as soon as all the bugs are found maybe dont access stack via .at(..) but use non-bounds checking [] operator
* [small] look into std::stack: is that efficient or should use a custom structure to handle those?
* [medium] args length checking during runtime for function calls is expensive. maybe do that ahead of time?
