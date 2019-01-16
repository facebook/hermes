// RUN: (%hermes -hermes-parser %s 2>&1 || true) | %FileCheck %s --match-full-lines

var x;
x + 1 = 10;
//CHECK: {{.*}}lvalue-errors.js:4:1: error: invalid assignment left-hand side
//CHECK-NEXT: x + 1 = 10;
//CHECK-NEXT: ^~~~~

for (x + 1 in x);
//CHECK: {{.*}}lvalue-errors.js:9:6: error: invalid left-hand side in for-in-loop
//CHECK-NEXT: for (x + 1 in x);
//CHECK-NEXT:      ^~~~~

++0;
//CHECK: {{.*}}lvalue-errors.js:14:3: error: invalid operand in update operation
//CHECK-NEXT: ++0;
//CHECK-NEXT:   ^
