// RUN: %hermes -emit-binary -out=%t -O -target=HBC %s &&  %hermes -b %t | %FileCheck --match-full-lines %s

eps = HermesInternal.getEpilogues();
print(eps.length);
// CHECK:1
print(eps[0]);
// CHECK-NEXT:undefined
