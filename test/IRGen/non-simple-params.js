/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function foo(a, b = a) {
    return a + b;
}

function bar(a = 10, b = glob) {
    return a + b;
}

function baz({a, b}) {
    return a + b;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "bar": string
// CHECK-NEXT:       DeclareGlobalVarInst "baz": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "foo": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %bar(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "bar": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %baz(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "baz": string
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %9: any
// CHECK-NEXT:  %11 = LoadStackInst (:any) %9: any
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any): any
// CHECK-NEXT:frame = [a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [b]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %2: any, [a]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %5 = BinaryStrictlyNotEqualInst (:any) %4: any, undefined: undefined
// CHECK-NEXT:       CondBranchInst %5: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = PhiInst (:any) %4: any, %BB0, %7: any, %BB2
// CHECK-NEXT:        StoreFrameInst %9: any, [b]: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:  %13 = BinaryAddInst (:any) %11: any, %12: any
// CHECK-NEXT:        ReturnInst %13: any
// CHECK-NEXT:function_end

// CHECK:function bar(a: any, b: any): any
// CHECK-NEXT:frame = [a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [b]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %3 = BinaryStrictlyNotEqualInst (:any) %2: any, undefined: undefined
// CHECK-NEXT:       CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = PhiInst (:any) %2: any, %BB0, 10: number, %BB2
// CHECK-NEXT:       StoreFrameInst %6: any, [a]: any
// CHECK-NEXT:  %8 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %9 = BinaryStrictlyNotEqualInst (:any) %8: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %9: any, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst (:any) globalObject: object, "glob": string
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = PhiInst (:any) %8: any, %BB1, %11: any, %BB4
// CHECK-NEXT:        StoreFrameInst %13: any, [b]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %16 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:  %17 = BinaryAddInst (:any) %15: any, %16: any
// CHECK-NEXT:        ReturnInst %17: any
// CHECK-NEXT:function_end

// CHECK:function baz(?anon_0_param: any): any
// CHECK-NEXT:frame = [a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [b]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %?anon_0_param: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: any, "a": string
// CHECK-NEXT:       StoreFrameInst %3: any, [a]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %2: any, "b": string
// CHECK-NEXT:       StoreFrameInst %5: any, [b]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %7: any, %8: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end
