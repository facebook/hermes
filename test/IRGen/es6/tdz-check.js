/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xenable-tdz -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function check1() {
    return x + y;
    let x = 10;
    const y = 1;
}

function check2(p) {
    var b = a;
    let a;
    return a + b;
}

function check3() {
    let x = check3_inner();
    function check3_inner() {
        return x + 1;
    }
    return x;
}

function check4() {
    x = 10;
    let x;
    return x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "check1": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "check2": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "check3": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "check4": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %check1(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "check1": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %check2(): any
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: closure, globalObject: object, "check2": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:closure) %check3(): any
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: closure, globalObject: object, "check3": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:closure) %check4(): any
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: closure, globalObject: object, "check4": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %13 = StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:  %15 = ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function check1(): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst empty: empty, [x]: any
// CHECK-NEXT:  %1 = StoreFrameInst empty: empty, [y]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %3 = ThrowIfEmptyInst (:nonempty) %2: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %5 = ThrowIfEmptyInst (:nonempty) %4: any
// CHECK-NEXT:  %6 = BinaryAddInst (:any) %3: nonempty, %5: nonempty
// CHECK-NEXT:  %7 = ReturnInst %6: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = StoreFrameInst 10: number, [x]: any
// CHECK-NEXT:  %9 = StoreFrameInst 1: number, [y]: any
// CHECK-NEXT:  %10 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function check2(p: any): any
// CHECK-NEXT:frame = [p: any, b: any, a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %p: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [p]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [b]: any
// CHECK-NEXT:  %3 = StoreFrameInst empty: empty, [a]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %5 = ThrowIfEmptyInst (:nonempty) %4: any
// CHECK-NEXT:  %6 = StoreFrameInst %5: nonempty, [b]: any
// CHECK-NEXT:  %7 = StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %9 = ThrowIfEmptyInst (:nonempty) %8: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:  %11 = BinaryAddInst (:any) %9: nonempty, %10: any
// CHECK-NEXT:  %12 = ReturnInst %11: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function check3(): any
// CHECK-NEXT:frame = [x: any, check3_inner: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst empty: empty, [x]: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %check3_inner(): any
// CHECK-NEXT:  %2 = StoreFrameInst %1: closure, [check3_inner]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [check3_inner]: any
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [x]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %7 = ThrowIfEmptyInst (:nonempty) %6: any
// CHECK-NEXT:  %8 = ReturnInst %7: nonempty
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function check4(): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst empty: empty, [x]: any
// CHECK-NEXT:  %1 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %2 = ThrowIfEmptyInst (:nonempty) %1: any
// CHECK-NEXT:  %3 = StoreFrameInst 10: number, [x]: any
// CHECK-NEXT:  %4 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %6 = ThrowIfEmptyInst (:nonempty) %5: any
// CHECK-NEXT:  %7 = ReturnInst %6: nonempty
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function check3_inner(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [x@check3]: any
// CHECK-NEXT:  %1 = ThrowIfEmptyInst (:nonempty) %0: any
// CHECK-NEXT:  %2 = BinaryAddInst (:any) %1: nonempty, 1: number
// CHECK-NEXT:  %3 = ReturnInst %2: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
