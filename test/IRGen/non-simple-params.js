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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "bar": string
// CHECK-NEXT:       DeclareGlobalVarInst "baz": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "foo": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %bar(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "bar": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %baz(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "baz": string
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any): any
// CHECK-NEXT:frame = [a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %foo(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [b]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [a]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %7 = BinaryStrictlyNotEqualInst (:any) %6: any, undefined: undefined
// CHECK-NEXT:       CondBranchInst %7: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = PhiInst (:any) %6: any, %BB0, %9: any, %BB1
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: any, [b]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:  %15 = BinaryAddInst (:any) %13: any, %14: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:function bar(a: any, b: any): any
// CHECK-NEXT:frame = [a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %bar(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [b]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %5 = BinaryStrictlyNotEqualInst (:any) %4: any, undefined: undefined
// CHECK-NEXT:       CondBranchInst %5: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = PhiInst (:any) %4: any, %BB0, 10: number, %BB1
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: any, [a]: any
// CHECK-NEXT:  %10 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %11 = BinaryStrictlyNotEqualInst (:any) %10: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %11: any, %BB4, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = TryLoadGlobalPropertyInst (:any) globalObject: object, "glob": string
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = PhiInst (:any) %10: any, %BB2, %13: any, %BB3
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: any, [b]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:  %18 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:  %19 = BinaryAddInst (:any) %17: any, %18: any
// CHECK-NEXT:        ReturnInst %19: any
// CHECK-NEXT:function_end

// CHECK:function baz(?anon_0_param: any): any
// CHECK-NEXT:frame = [a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %baz(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [b]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %?anon_0_param: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "a": string
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: any, [a]: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %4: any, "b": string
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: any, [b]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:  %11 = BinaryAddInst (:any) %9: any, %10: any
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end
