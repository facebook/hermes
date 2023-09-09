/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -target=HBC -dump-lir %s | %FileCheckOrRegen --match-full-lines %s

// Test that StoreNewOwnPropertyInst is lowered to StoreOwnPropertyInst when
// the property name is a valid array index.

function foo() {
    return {a: 1, "10": 2, 11: 3, "999999999999999999999999": 4, ['42']: 5};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:globals = [foo]
// CHECK-NEXT:S{global#0()#1} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHECK-NEXT:  %1 = HBCCreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = HBCGetGlobalObjectInst
// CHECK-NEXT:  %3 = StorePropertyInst %1 : closure, %2 : object, "foo" : string
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %5 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  %6 = StoreStackInst %5 : undefined, %4
// CHECK-NEXT:  %7 = LoadStackInst %4
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function foo#0#1()#2
// CHECK-NEXT:S{foo#0#1()#2} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = AllocObjectInst 5 : number, empty
// CHECK-NEXT:  %2 = HBCLoadConstInst 1 : number
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst %2 : number, %1 : object, "a" : string, true : boolean
// CHECK-NEXT:  %4 = HBCLoadConstInst 2 : number
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst %4 : number, %1 : object, 10 : number, true : boolean
// CHECK-NEXT:  %6 = HBCLoadConstInst 3 : number
// CHECK-NEXT:  %7 = StoreNewOwnPropertyInst %6 : number, %1 : object, 11 : number, true : boolean
// CHECK-NEXT:  %8 = HBCLoadConstInst 4 : number
// CHECK-NEXT:  %9 = StoreNewOwnPropertyInst %8 : number, %1 : object, "999999999999999999999999" : string, true : boolean
// CHECK-NEXT:  %10 = HBCLoadConstInst 5 : number
// CHECK-NEXT:  %11 = StoreOwnPropertyInst %10 : number, %1 : object, 42 : number, true : boolean
// CHECK-NEXT:  %12 = ReturnInst %1 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  %14 = ReturnInst %13 : undefined
// CHECK-NEXT:function_end
