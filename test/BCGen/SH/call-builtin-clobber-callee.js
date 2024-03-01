/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O -dump-lra %s | %FileCheckOrRegen --match-full-lines %s

/// Ensure that the callee stack entry is clobbered by the builtin call.
function test_call_after_builtin() {
    print({valueOf: () => 2} ** 3);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:                 DeclareGlobalVarInst "test_call_after_builtin": string
// CHECK-NEXT:  {loc0}    %1 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  {loc1}    %2 = CreateFunctionInst (:object) {loc0} %1: environment, %test_call_after_builtin(): functionCode
// CHECK-NEXT:  {loc0}    %3 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:                 StorePropertyLooseInst {loc1} %2: object, {loc0} %3: object, "test_call_after_builtin": string
// CHECK-NEXT:  {np0}     %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %5: undefined
// CHECK-NEXT:function_end

// CHECK:function test_call_after_builtin(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {loc0}    %0 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  {loc0}    %1 = TryLoadGlobalPropertyInst (:any) {loc0} %0: object, "print": string
// CHECK-NEXT:  {loc1}    %2 = AllocObjectInst (:object) 1: number, empty: any
// CHECK-NEXT:  {loc2}    %3 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  {loc2}    %4 = CreateFunctionInst (:object) {loc2} %3: environment, %valueOf(): functionCode
// CHECK-NEXT:                 StoreNewOwnPropertyInst {loc2} %4: object, {loc1} %2: object, "valueOf": string, true: boolean
// CHECK-NEXT:  {stack[0]}  %6 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  {stack[4]}  %7 = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  {stack[3]}  %8 = ImplicitMovInst (:empty) empty: empty
// CHECK-NEXT:  {stack[2]}  %9 = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  {stack[1]} %10 = MovInst (:object) {loc1} %2: object
// CHECK-NEXT:  {stack[1]} %11 = CallBuiltinInst (:number) [HermesBuiltin.exponentiationOperator]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, {stack[1]} %10: object, {stack[0]} %6: number
// CHECK-NEXT:  {np0}    %12 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {stack[4]} %13 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {stack[3]} %14 = MovInst (:any) {loc0} %1: any
// CHECK-NEXT:  {stack[2]} %15 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {loc0}   %16 = CallInst (:any) {stack[3]} %14: any, empty: any, empty: any, {np0} %12: undefined, {stack[2]} %15: undefined, {stack[1]} %11: number
// CHECK-NEXT:                 ReturnInst {np0} %12: undefined
// CHECK-NEXT:function_end

// CHECK:arrow valueOf(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {np0}     %0 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:                 ReturnInst {np0} %0: number
// CHECK-NEXT:function_end
