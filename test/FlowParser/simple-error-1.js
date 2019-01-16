// RUN: (! %hermesc -Xflow-parser -dump-ast %s 2>&1 ) | %FileCheck --match-full-lines %s
// REQUIRES: flowparser

print("hello"+;)
//CHECK: {{.*}}simple-error-1.js:4:14: error: Unexpected token ;
//CHECK-NEXT: print("hello"+;)
//CHECK-NEXT:              ^
