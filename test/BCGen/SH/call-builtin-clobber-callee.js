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
// CHECK-NEXT:  {loc2}    %4 = CreateScopeInst (:environment) %test_call_after_builtin(): any, {loc2} %3: environment
// CHECK-NEXT:  {loc2}    %5 = CreateFunctionInst (:object) {loc2} %4: environment, %valueOf(): functionCode
// CHECK-NEXT:                 StoreNewOwnPropertyInst {loc2} %5: object, {loc1} %2: object, "valueOf": string, true: boolean
// CHECK-NEXT:  {stack[0]}  %7 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  {stack[4]}  %8 = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  {stack[3]}  %9 = ImplicitMovInst (:empty) empty: empty
// CHECK-NEXT:  {stack[2]} %10 = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  {stack[1]} %11 = MovInst (:object) {loc1} %2: object
// CHECK-NEXT:  {stack[1]} %12 = CallBuiltinInst (:number) [HermesBuiltin.exponentiationOperator]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, {stack[1]} %11: object, {stack[0]} %7: number
// CHECK-NEXT:  {np0}    %13 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {stack[4]} %14 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {stack[3]} %15 = MovInst (:any) {loc0} %1: any
// CHECK-NEXT:  {stack[2]} %16 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {loc0}   %17 = CallInst (:any) {stack[3]} %15: any, empty: any, empty: any, {np0} %13: undefined, {stack[2]} %16: undefined, {stack[1]} %12: number
// CHECK-NEXT:                 ReturnInst {np0} %13: undefined
// CHECK-NEXT:function_end

// CHECK:arrow valueOf(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {np0}     %0 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:                 ReturnInst {np0} %0: number
// CHECK-NEXT:function_end
