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

function load_dedup(foo){
    var x = foo();
    function bar(){
        foo(x);
        return x;
    }
    return bar;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "load_only_capture": string
// CHECK-NEXT:       DeclareGlobalVarInst "load_dedup": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %load_only_capture(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "load_only_capture": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %load_dedup(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "load_dedup": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function load_only_capture(leak: any, foreach: any, n: any): number
// CHECK-NEXT:frame = [i: number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %load_only_capture(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %leak: any
// CHECK-NEXT:  %3 = LoadParamInst (:any) %n: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [i]: number
// CHECK-NEXT:  %5 = BinaryLessThanInst (:boolean) 0: number, %3: any
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = PhiInst (:number) 0: number, %BB0, %10: number, %BB1
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %""(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, %2: any, "k": string
// CHECK-NEXT:  %10 = FAddInst (:number) %7: number, 1: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: number, [i]: number
// CHECK-NEXT:  %12 = BinaryLessThanInst (:boolean) %10: number, %3: any
// CHECK-NEXT:        CondBranchInst %12: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = PhiInst (:number) 0: number, %BB0, %10: number, %BB1
// CHECK-NEXT:        ReturnInst %14: number
// CHECK-NEXT:function_end

// CHECK:function load_dedup(foo: any): object
// CHECK-NEXT:frame = [foo: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %load_dedup(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %foo: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [foo]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %bar(): functionCode
// CHECK-NEXT:  %6 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [x]: any
// CHECK-NEXT:       ReturnInst %5: object
// CHECK-NEXT:function_end

// CHECK:arrow ""(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %load_only_capture(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [i@load_only_capture]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:function bar(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %load_dedup(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:any) %0: environment, [foo@load_dedup]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) %0: environment, [x@load_dedup]: any
// CHECK-NEXT:  %3 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: any
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end
