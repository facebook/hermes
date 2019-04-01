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

tryParse("for(var i1 = 10 in {}); i1");
//CHECK-NEXT: OK 10

tryParse("'use strict'; for(var i2 = 10 in {}); i2");
//CHECK-NEXT: caught: 1:28:for-in/for-of variable declaration may not be initialized

tryParse("for(let i3 = 10 in {}); i3");
//CHECK-NEXT: caught: 1:14:for-in/for-of variable declaration may not be initialized

tryParse("'use strict'; for(let i4 = 10 in {}); i4");
//CHECK-NEXT: caught: 1:28:for-in/for-of variable declaration may not be initialized

tryParse("for(let i5 = 10 of []); i5");
//CHECK-NEXT: caught: 1:14:for-in/for-of variable declaration may not be initialized
