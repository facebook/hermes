// RUN: %hermesc -Werror -dump-ir %s > /dev/null

// Ensure that we are declaring the correct variable name, "c" not "b".
function foo() {
    "use strict";
    var {a, b:c} = {}
    return a + c;
}

foo();
