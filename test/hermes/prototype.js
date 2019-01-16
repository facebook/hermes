// RUN: %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s

print(eval.prototype);
//CHECK: undefined

print(print.prototype);
//CHECK: undefined

