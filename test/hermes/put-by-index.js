// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s

// Check that object literal with numeric properties works
var x = {0:10, 1:20}
print(x[0], x[1]);
//CHECK: 10 20
