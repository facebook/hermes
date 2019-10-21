/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-lir %s | %FileCheck --match-full-lines %s

// Test that StoreNewOwnPropertyInst is lowered to StoreOwnPropertyInst when
// the property name is a valid array index.

function foo() {
    return {a: 1, "10": 2, 11: 3, "999999999999999999999999": 4};
}

//CHECK-LABEL:function foo()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = HBCCreateEnvironmentInst
//CHECK-NEXT:  %1 = HBCLoadConstInst 1 : number
//CHECK-NEXT:  %2 = HBCLoadConstInst 2 : number
//CHECK-NEXT:  %3 = HBCLoadConstInst 3 : number
//CHECK-NEXT:  %4 = HBCLoadConstInst 4 : number
//CHECK-NEXT:  %5 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  %6 = AllocObjectInst 4 : number, empty
//CHECK-NEXT:  %7 = StoreNewOwnPropertyInst %1 : number, %6 : object, "a" : string, true : boolean
//CHECK-NEXT:  %8 = StoreOwnPropertyInst %2 : number, %6 : object, 10 : number, true : boolean
//CHECK-NEXT:  %9 = StoreOwnPropertyInst %3 : number, %6 : object, 11 : number, true : boolean
//CHECK-NEXT:  %10 = StoreNewOwnPropertyInst %4 : number, %6 : object, "999999999999999999999999" : string, true : boolean
//CHECK-NEXT:  %11 = ReturnInst %6 : object
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %12 = ReturnInst %5 : undefined
//CHECK-NEXT:function_end
