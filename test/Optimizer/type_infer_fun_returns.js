/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

function g14(z) {
    var w = function () { return k * 1; }
    if (z > w()) {
        print (w() + 1);
        return {m : function () { w = function() { return false; }; }};
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "g14": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %g14(): undefined|object
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "g14": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function g14(z: any): undefined|object
// CHECK-NEXT:frame = [w: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %w(): number
// CHECK-NEXT:       StoreFrameInst %1: object, [w]: object
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "k": string
// CHECK-NEXT:  %4 = BinaryMultiplyInst (:number) %3: any, 1: number
// CHECK-NEXT:  %5 = BinaryGreaterThanInst (:boolean) %0: any, %4: number
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %8 = LoadFrameInst (:object) [w]: object
// CHECK-NEXT:  %9 = CallInst (:any) %8: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %10 = BinaryAddInst (:string|number) %9: any, 1: number
// CHECK-NEXT:  %11 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %10: string|number
// CHECK-NEXT:  %12 = AllocObjectInst (:object) 1: number, empty: any
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %m(): undefined
// CHECK-NEXT:        StoreNewOwnPropertyInst %13: object, %12: object, "m": string, true: boolean
// CHECK-NEXT:        ReturnInst %12: object
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function w(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "k": string
// CHECK-NEXT:  %1 = BinaryMultiplyInst (:number) %0: any, 1: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:function m(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %"w 1#"(): boolean
// CHECK-NEXT:       StoreFrameInst %0: object, [w@g14]: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "w 1#"(): boolean
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst false: boolean
// CHECK-NEXT:function_end
