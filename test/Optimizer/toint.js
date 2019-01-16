// RUN: %hermes -O -dump-ir %s | %FileCheck --match-full-lines %s

function foo (x) {
    return 1e20 | 0;
}

//CHECK-LABEL: function foo(x) : number
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = ReturnInst 1661992960 : number
//CHECK-NEXT: function_end
