/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

// Test if we can eliminate loads from a captured variable across basic blocks
// when the loads occur before the capture.
function foo(sink){
    var x = 0;
    if (a) sink(x++);
    if (b) sink(x++);
    if (c) sink(x++);
    return () => x++;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): object
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(sink: any): object
// CHECK-NEXT:frame = [x: number|bigint]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:  %1 = StoreFrameInst 0: number, [x]: number|bigint
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "a": string
// CHECK-NEXT:  %3 = CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = LoadFrameInst (:number|bigint) [x]: number|bigint
// CHECK-NEXT:  %5 = AsNumericInst (:number|bigint) %4: number|bigint
// CHECK-NEXT:  %6 = UnaryIncInst (:number|bigint) %5: number|bigint
// CHECK-NEXT:  %7 = StoreFrameInst %6: number|bigint, [x]: number|bigint
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, %5: number|bigint
// CHECK-NEXT:  %9 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = TryLoadGlobalPropertyInst (:any) globalObject: object, "b": string
// CHECK-NEXT:  %11 = CondBranchInst %10: any, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadFrameInst (:number|bigint) [x]: number|bigint
// CHECK-NEXT:  %13 = AsNumericInst (:number|bigint) %12: number|bigint
// CHECK-NEXT:  %14 = UnaryIncInst (:number|bigint) %13: number|bigint
// CHECK-NEXT:  %15 = StoreFrameInst %14: number|bigint, [x]: number|bigint
// CHECK-NEXT:  %16 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, %13: number|bigint
// CHECK-NEXT:  %17 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %18 = TryLoadGlobalPropertyInst (:any) globalObject: object, "c": string
// CHECK-NEXT:  %19 = CondBranchInst %18: any, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = LoadFrameInst (:number|bigint) [x]: number|bigint
// CHECK-NEXT:  %21 = AsNumericInst (:number|bigint) %20: number|bigint
// CHECK-NEXT:  %22 = UnaryIncInst (:number|bigint) %21: number|bigint
// CHECK-NEXT:  %23 = StoreFrameInst %22: number|bigint, [x]: number|bigint
// CHECK-NEXT:  %24 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, %21: number|bigint
// CHECK-NEXT:  %25 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = CreateFunctionInst (:object) %""(): number|bigint
// CHECK-NEXT:  %27 = ReturnInst %26: object
// CHECK-NEXT:function_end

// CHECK:arrow ""(): number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:number|bigint) [x@foo]: number|bigint
// CHECK-NEXT:  %1 = AsNumericInst (:number|bigint) %0: number|bigint
// CHECK-NEXT:  %2 = UnaryIncInst (:number|bigint) %1: number|bigint
// CHECK-NEXT:  %3 = StoreFrameInst %2: number|bigint, [x@foo]: number|bigint
// CHECK-NEXT:  %4 = ReturnInst %1: number|bigint
// CHECK-NEXT:function_end
