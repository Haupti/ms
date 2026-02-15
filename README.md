# ideas

here a more 'traditional' syntax idea with macros
```
-include "../file.msl"

function add(a,b) {
    return a+b
}
let x = 1
set x = 1


if ( x == 1) {
 -assert(typeof(x) == #int)
} elif ( x == 2 ){
 -assert(typeof(x) == #float)
} else {
 -assert(typeof(x) == #string)
}


let my_list = list(1,2,3)
put(my_list,0+1,1+2)
my_list.put(1,3);
my_list.at(1);

let my_tab = table("hello", "world", 0, 1)
put(my_tab,"hello","World")

1.add(2) // left apply operator -> add(1,2)
1 | add(2) // right apply operator -> add(2,1)

-setflag HEAD
-ifflag HEAD 
...

-endif

// errors as values:
function might_error(x) {
    if(x > 5) {
        return error("invalid argument")
    }
    return x
}

function main() {
    let my_result = might_error(56)
    if (is_error(my_result)) { return my_result }
    ...
}
// or the short hand to return the error immediately
function main() {
    let my_result = try might_error(56)
    ...
}
// or the short hand to panic if it is an error
function main() {
    let my_result = expect might_error(56)
}


```


## how to handle macros

i would like to keep my parsing etc as simple as possible without module or file name resolution or such.
thus include macros must be handled before that. so there is a preprocessor step that runs before everything else (except tokenization).
logic:
1. discover all preprocessor macros
2. produce respective output code for macros
    * for -assert(x) this just produces if(x) { panic("assertion failed") }
    * for -ifndef/-ifdef/-endif/-define this is completely handled and erased before the the project runs
    * for -include it tracks an import history (for circular imports) and tries to gather the token vectors and concatenates them together into one single large vector (skipping already included files). the result is then placed where the include directive was
    * important is probably that the include must run last to not mess with -ifndef -endif pair finding





