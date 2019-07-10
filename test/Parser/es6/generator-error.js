// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

function *foo(yield) {}
// CHECK:{{.*}}:3:15: error: Unexpected usage of 'yield' as an identifier
// CHECK-NEXT:function *foo(yield) {}
// CHECK-NEXT:              ^~~~~

function *bar() { var yield = 1; }
// CHECK:{{.*}}:8:23: error: Unexpected usage of 'yield' as an identifier
// CHECK-NEXT:function *bar() { var yield = 1; }
// CHECK-NEXT:                      ^~~~~
