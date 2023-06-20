/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo(x = () => this) {
    return x();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(x: any): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %this: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = StoreFrameInst %1: object, [?anon_0_this]: any
// CHECK-NEXT:  %3 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %4 = StoreFrameInst %3: undefined|object, [?anon_1_new.target]: undefined|object
// CHECK-NEXT:  %5 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %7 = BinaryStrictlyNotEqualInst (:any) %6: any, undefined: undefined
// CHECK-NEXT:  %8 = CondBranchInst %7: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %x(): any
// CHECK-NEXT:  %10 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = PhiInst (:any) %6: any, %BB0, %9: object, %BB2
// CHECK-NEXT:  %12 = StoreFrameInst %11: any, [x]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %14 = CallInst (:any) %13: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %15 = ReturnInst %14: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow x(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [?anon_0_this@foo]: any
// CHECK-NEXT:  %1 = ReturnInst %0: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
