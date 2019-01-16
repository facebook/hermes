// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s

var x = []
x[0] = 10
Object.preventExtensions(x);
print(Object.isSealed(x));
//CHECK: false
