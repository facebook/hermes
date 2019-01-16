// RUN: %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s

print(1);
//CHECK: 1
debugger;
print(2);
//CHECK-NEXT: 2
debugger;
print(3);
//CHECK-NEXT: 3
