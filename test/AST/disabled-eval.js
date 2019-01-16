// RUN: (%hermes -hermes-parser -enable-eval=false -non-strict %s 2>&1 || true) | %FileCheck %s --match-full-lines

var eval;
//CHECK: {{.*}}disabled-eval.js:3:5: error: 'eval' is disabled
//CHECK-NEXT: var eval;
//CHECK-NEXT:     ^~~~

eval("print(1)");
//CHECK: {{.*}}disabled-eval.js:8:1: error: 'eval' is disabled
//CHECK-NEXT: eval("print(1)");
//CHECK-NEXT: ^~~~
