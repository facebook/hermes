/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function load_only_capture(leak, foreach, n) {
    for(var i = 0; i < n; i++){
      leak.k = () => i;
    }
    return i;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "load_only_capture": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %load_only_capture(): number
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "load_only_capture": string
// CHECK-NEXT:  %3 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function load_only_capture(leak: any, foreach: any, n: any): number
// CHECK-NEXT:frame = [i: number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %leak: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %n: any
// CHECK-NEXT:  %2 = StoreFrameInst 0: number, [i]: number
// CHECK-NEXT:  %3 = BinaryLessThanInst (:boolean) 0: number, %1: any
// CHECK-NEXT:  %4 = CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = PhiInst (:number) 0: number, %BB0, %8: number, %BB1
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %""(): number
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: closure, %0: any, "k": string
// CHECK-NEXT:  %8 = UnaryIncInst (:number) %5: number
// CHECK-NEXT:  %9 = StoreFrameInst %8: number, [i]: number
// CHECK-NEXT:  %10 = BinaryLessThanInst (:boolean) %8: number, %1: any
// CHECK-NEXT:  %11 = CondBranchInst %10: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = PhiInst (:number) 0: number, %BB0, %8: number, %BB1
// CHECK-NEXT:  %13 = ReturnInst (:number) %12: number
// CHECK-NEXT:function_end

// CHECK:arrow ""(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:number) [i@load_only_capture]: number
// CHECK-NEXT:  %1 = ReturnInst (:number) %0: number
// CHECK-NEXT:function_end
