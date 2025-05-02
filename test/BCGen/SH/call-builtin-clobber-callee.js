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
// CHECK-NEXT:%BB0:
// CHECK-NEXT:                 DeclareGlobalVarInst "test_call_after_builtin": string
// CHECK-NEXT:  {loc1}    %1 = LIRGetGlobalObjectInst (:object)
// CHECK-NEXT:  {loc0}    %2 = CreateFunctionInst (:object) empty: any, empty: any, %test_call_after_builtin(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {loc0} %2: object, {loc1} %1: object, "test_call_after_builtin": string
// CHECK-NEXT:  {np0}     %4 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %4: undefined
// CHECK-NEXT:function_end

// CHECK:function test_call_after_builtin(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {loc0}    %0 = LIRGetGlobalObjectInst (:object)
// CHECK-NEXT:  {loc0}    %1 = TryLoadGlobalPropertyInst (:any) {loc0} %0: object, "print": string
// CHECK-NEXT:  {loc1}    %2 = LIRAllocObjectFromBufferInst (:object) empty: any, "valueOf": string, null: null
// CHECK-NEXT:  {loc2}    %3 = CreateFunctionInst (:object) empty: any, empty: any, %valueOf(): functionCode
// CHECK-NEXT:                 PrStoreInst {loc2} %3: object, {loc1} %2: object, 0: number, "valueOf": string, false: boolean
// CHECK-NEXT:  {stack[0]}  %5 = LIRLoadConstInst (:number) 3: number
// CHECK-NEXT:  {stack[4]}  %6 = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  {stack[3]}  %7 = ImplicitMovInst (:empty) empty: empty
// CHECK-NEXT:  {stack[2]}  %8 = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  {stack[1]}  %9 = MovInst (:object) {loc1} %2: object
// CHECK-NEXT:  {stack[1]} %10 = CallBuiltinInst (:number) [HermesBuiltin.exponentiationOperator]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, {stack[1]} %9: object, {stack[0]} %5: number
// CHECK-NEXT:  {np0}    %11 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {stack[4]} %12 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {stack[3]} %13 = MovInst (:any) {loc0} %1: any
// CHECK-NEXT:  {stack[2]} %14 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {loc0}   %15 = CallInst (:any) {stack[3]} %13: any, empty: any, false: boolean, empty: any, {np0} %11: undefined, {stack[2]} %14: undefined, {stack[1]} %10: number
// CHECK-NEXT:                 ReturnInst {np0} %11: undefined
// CHECK-NEXT:function_end

// CHECK:arrow valueOf(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {np0}     %0 = LIRLoadConstInst (:number) 2: number
// CHECK-NEXT:                 ReturnInst {np0} %0: number
// CHECK-NEXT:function_end
