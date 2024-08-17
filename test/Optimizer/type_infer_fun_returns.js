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

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "g14": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %g14(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "g14": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [w: object]

// CHECK:function g14(z: any): undefined|object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %w(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %3: object, [%VS1.w]: object
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "k": string
// CHECK-NEXT:  %6 = BinaryMultiplyInst (:number) %5: any, 1: number
// CHECK-NEXT:  %7 = BinaryGreaterThanInst (:boolean) %2: any, %6: number
// CHECK-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %10 = LoadFrameInst (:object) %1: environment, [%VS1.w]: object
// CHECK-NEXT:  %11 = CallInst (:any) %10: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %12 = BinaryAddInst (:string|number) %11: any, 1: number
// CHECK-NEXT:  %13 = CallInst (:any) %9: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %12: string|number
// CHECK-NEXT:  %14 = AllocObjectLiteralInst (:object) empty: any, "m": string, null: null
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %1: environment, %m(): functionCode
// CHECK-NEXT:        PrStoreInst %15: object, %14: object, 0: number, "m": string, false: boolean
// CHECK-NEXT:        ReturnInst %14: object
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function w(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "k": string
// CHECK-NEXT:  %1 = BinaryMultiplyInst (:number) %0: any, 1: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:function m(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %"w 1#"(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: object, [%VS1.w]: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "w 1#"(): boolean
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst false: boolean
// CHECK-NEXT:function_end
