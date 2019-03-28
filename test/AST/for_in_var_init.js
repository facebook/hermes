// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

var global = Function("return this;")();

print("START");
//CHECK: START

function tryParse(str) {
    try {
        var result = global.eval(str);
    } catch (e) {
        print("caught:", e.message);
        return;
    }
    print("OK", result);
}

tryParse("for(var i = 10 in {}); i");
//CHECK-NEXT: OK 10

tryParse("'use strict'; for(var i = 10 in {}); i");
//CHECK-NEXT: caught: 1:27:for-in variable declaration may not be initialized
