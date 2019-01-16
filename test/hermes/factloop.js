// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
"use strict";

function factloop (lc, fc) {
    print("running ", lc, "iterations of fact(", fc, ")");
// CHECK: running 2 iterations of fact( 4 )
    var n, fact;
    var res = 0;
    while (--lc >= 0) {
        n = fc;
        fact = n;
        while (--n > 1)
            fact *= n;
        res += fact;
    }
    return res;
}

print("result", factloop(2, 4))
// CHECK-NEXT: result 48
