// RUN: %hermes -target=HBC -dump-ra -O %s | %FileCheck --match-full-lines %s

//CHECK-LABEL:function global() : number
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT: {{.*}} %0 = HBCLoadConstInst 42 : number
//CHECK-NEXT: {{.*}} %1 = ReturnInst %0 : number

42;
