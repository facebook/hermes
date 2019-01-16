// RUN: %hermes -hermes-parser -dump-ir %s | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

({prop1: 10});
({"prop1": 10});

//CHECK: function global()
//CHECK: frame = []
//CHECK:   %BB0:
//CHECK:     %0 = AllocStackInst $?anon_0_ret
//CHECK:     %1 = StoreStackInst undefined : undefined, %0
//CHECK:     %2 = AllocObjectInst 1 : number
//CHECK:     %3 = StoreOwnPropertyInst 10 : number, %2 : object, "prop1" : string
//CHECK:     %4 = StoreStackInst %2 : object, %0
//CHECK:     %5 = AllocObjectInst 1 : number
//CHECK:     %6 = StoreOwnPropertyInst 10 : number, %5 : object, "prop1" : string
//CHECK:     %7 = StoreStackInst %5 : object, %0
//CHECK:     %8 = LoadStackInst %0
//CHECK:     %9 = ReturnInst %8
//CHECK: function_end

