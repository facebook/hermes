// RUN: %hermes -dump-ir %s | %FileCheck --match-full-lines %s

({10: 1, "11": 2, "10": 3})

//CHECK:    %2 = AllocObjectInst 2 : number
//CHECK-NEXT:    %3 = StoreOwnPropertyInst 2 : number, %2 : object, "11" : string
//CHECK-NEXT:    %4 = StoreOwnPropertyInst 3 : number, %2 : object, "10" : string
//CHECK-NEXT:    %5 = StoreStackInst %2 : object, %0
