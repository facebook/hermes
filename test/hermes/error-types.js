// RUN: %hermes -O -target=HBC -Werror %s
"use strict";

var x;
x = Error;
x = EvalError;
x = RangeError;
x = ReferenceError;
x = SyntaxError;
x = TypeError;
x = URIError;
