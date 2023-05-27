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
// CHECK-NEXT:  %4 = StoreFrameInst 1: number, [x]: number|bigint
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %6 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "b": string
// CHECK-NEXT:  %8 = CondBranchInst %7: any, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadFrameInst (:number|bigint) [x]: number|bigint
// CHECK-NEXT:  %10 = AsNumericInst (:number|bigint) %9: number|bigint
// CHECK-NEXT:  %11 = UnaryIncInst (:number|bigint) %10: number|bigint
// CHECK-NEXT:  %12 = StoreFrameInst %11: number|bigint, [x]: number|bigint
// CHECK-NEXT:  %13 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, %10: number|bigint
// CHECK-NEXT:  %14 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = TryLoadGlobalPropertyInst (:any) globalObject: object, "c": string
// CHECK-NEXT:  %16 = CondBranchInst %15: any, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %17 = LoadFrameInst (:number|bigint) [x]: number|bigint
// CHECK-NEXT:  %18 = AsNumericInst (:number|bigint) %17: number|bigint
// CHECK-NEXT:  %19 = UnaryIncInst (:number|bigint) %18: number|bigint
// CHECK-NEXT:  %20 = StoreFrameInst %19: number|bigint, [x]: number|bigint
// CHECK-NEXT:  %21 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, %18: number|bigint
// CHECK-NEXT:  %22 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %""(): number|bigint
// CHECK-NEXT:  %24 = ReturnInst %23: object
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
