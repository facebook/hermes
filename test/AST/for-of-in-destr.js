// RUN: (! %hermes %s 2>&1) | %FileCheck --match-full-lines %s

for([a, 0] of x);
//CHECK: {{.*}}for-of-in-destr.js:3:9: error: invalid assignment left-hand side
//CHECK-NEXT: for([a, 0] of x);
//CHECK-NEXT:         ^

for({a : 0, b} of x);
//CHECK: {{.*}}for-of-in-destr.js:8:10: error: invalid assignment left-hand side
//CHECK-NEXT: for({a : 0, b} of x);
//CHECK-NEXT:          ^

for([a, 0] in x);
//CHECK: {{.*}}for-of-in-destr.js:13:9: error: invalid assignment left-hand side
//CHECK-NEXT: for([a, 0] in x);
//CHECK-NEXT:         ^

for({a : 0, b} in x);
//CHECK: {{.*}}for-of-in-destr.js:18:10: error: invalid assignment left-hand side
//CHECK-NEXT: for({a : 0, b} in x);
//CHECK-NEXT:          ^
