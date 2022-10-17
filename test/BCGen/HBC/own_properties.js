/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -target=HBC -dump-lir %s | %FileCheckOrRegen --match-full-lines %s

// Test that StoreNewOwnPropertyInst is lowered to StoreOwnPropertyInst when
// the property name is a valid array index.
// We use a computed key to avoid emitting AllocObjectLiteral.

function foo() {
    return {a: 1, "10": 2, 11: 3, "999999999999999999999999": 4, ['42']: 5};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHECK-NEXT:  %1 = HBCGetGlobalObjectInst
// CHECK-NEXT:  %2 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  %3 = HBCCreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, %1 : object, "foo" : string
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %6 = StoreStackInst %2 : undefined, %5
// CHECK-NEXT:  %7 = LoadStackInst %5
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function foo#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = HBCLoadConstInst 1 : number
// CHECK-NEXT:  %2 = HBCLoadConstInst 2 : number
// CHECK-NEXT:  %3 = HBCLoadConstInst 3 : number
// CHECK-NEXT:  %4 = HBCLoadConstInst 4 : number
// CHECK-NEXT:  %5 = HBCLoadConstInst 5 : number
// CHECK-NEXT:  %6 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  %7 = AllocObjectInst 5 : number, empty
// CHECK-NEXT:  %8 = StoreNewOwnPropertyInst %1 : number, %7 : object, "a" : string, true : boolean
// CHECK-NEXT:  %9 = StoreNewOwnPropertyInst %2 : number, %7 : object, 10 : number, true : boolean
// CHECK-NEXT:  %10 = StoreNewOwnPropertyInst %3 : number, %7 : object, 11 : number, true : boolean
// CHECK-NEXT:  %11 = StoreNewOwnPropertyInst %4 : number, %7 : object, "999999999999999999999999" : string, true : boolean
// CHECK-NEXT:  %12 = StoreOwnPropertyInst %5 : number, %7 : object, 42 : number, true : boolean
// CHECK-NEXT:  %13 = ReturnInst %7 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = ReturnInst %6 : undefined
// CHECK-NEXT:function_end
