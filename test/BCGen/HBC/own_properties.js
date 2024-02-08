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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateFunctionEnvironmentInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = HBCCreateFunctionInst (:object) %foo(): functionCode, %0: environment
// CHECK-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:       StorePropertyLooseInst %2: object, %3: object, "foo": string
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %6 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       StoreStackInst %6: undefined, %5: any
// CHECK-NEXT:  %8 = LoadStackInst (:any) %5: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function foo(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateFunctionEnvironmentInst (:environment) %foo(): any, %parentScope: environment
// CHECK-NEXT:  %1 = AllocObjectInst (:object) 5: number, empty: any
// CHECK-NEXT:  %2 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:       StoreNewOwnPropertyInst %2: number, %1: object, "a": string, true: boolean
// CHECK-NEXT:  %4 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:       StoreNewOwnPropertyInst %4: number, %1: object, 10: number, true: boolean
// CHECK-NEXT:  %6 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:       StoreNewOwnPropertyInst %6: number, %1: object, 11: number, true: boolean
// CHECK-NEXT:  %8 = HBCLoadConstInst (:number) 4: number
// CHECK-NEXT:       StoreNewOwnPropertyInst %8: number, %1: object, "999999999999999999999999": string, true: boolean
// CHECK-NEXT:  %10 = HBCLoadConstInst (:number) 5: number
// CHECK-NEXT:        StoreOwnPropertyInst %10: number, %1: object, 42: number, true: boolean
// CHECK-NEXT:        ReturnInst %1: object
// CHECK-NEXT:function_end
