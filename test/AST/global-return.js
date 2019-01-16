// RUN: (! %hermes %s 2>&1 ) | %FileCheck --match-full-lines %s

return;
//CHECK: {{.*}}global-return.js:3:1: error: 'return' not in a function
//CHECK-NEXT: return;
//CHECK-NEXT: ^~~~~~~
